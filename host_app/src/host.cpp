/*
 * XRT Host Application for 80-Pipeline Packet-Switched Architecture (BENCHMARK / PAPER MODE)
 * Process: g g > t t~ g
 *
 * Key goals for paper-quality benchmarking:
 *   1) Correct BO placement (use kernel group_id for m_axi args)
 *   2) Correct buffer sizing (use psps_per_trunk, not fixed 10)
 *   3) Stable, repeatable timing numbers:
 *        - separate one-time setup from steady-state
 *        - report steady-state averages excluding warmup iterations
 *        - provide per-iteration and per-PSP metrics
 *        - provide phase breakdown: graph.run cmd, kernel start, mm2s wait, s2mm wait, graph.wait
 *
 * Assumptions:
 *   - pktsplit<10> routes packet IDs 0..9 => max 10 PSPs/trunk per iteration
 *   - Each iteration processes: psps_per_trunk * 8 PSPs total
 *   - Benchmark mode: input PSPs are generated once and reused for all iterations
 *
 * USAGE:
 *   ./host.exe <xclbin> [psps_per_trunk] [num_iterations] [energy] [options]
 *
 * Examples:
 *   ./host.exe hw.xclbin                  # 10 PSP/trunk, 1 iter
 *   ./host.exe hw.xclbin 10 100           # 100 iters (steady-state stats)
 *   ./host.exe hw.xclbin 10 1000          # 1000 iters
 *
 * Options:
 *   --warmup <N>      : exclude first N iterations from steady-state stats (default: 1)
 *   --csv <file>      : save metrics to CSV
 *   --quiet           : minimal output
 *   --seed <int>      : RAMBO seed (only affects one-time PSP generation)
 *
 * Notes:
 *   - In benchmark mode we do NOT regenerate PSPs each iteration.
 *   - For pure steady-state kernel/graph timing, set num_iterations large and warmup >= 1.
 */

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <complex>
#include <cstring>
#include <vector>
#include <chrono>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <signal.h>
#include <csignal>

#include "xrt/xrt_kernel.h"
#include "xrt/xrt_aie.h"
#include "xrt/xrt_bo.h"
#include "ert.h"  // For ERT_CMD_STATE_* constants
#include "psp_generator.h"

// Architecture parameters — 10 column-groups × 8 rows = 80 pipelines
static constexpr int NUM_TRUNKS       = 10;  // column groups (one PLIO each)
static constexpr int PSPS_PER_TRUNK   = 8;   // rows per group (pktsplit<8> limit)
static constexpr int PSP_SIZE_BYTES   = 80;   // 5 particles × 4 components × 4 bytes
static constexpr int PSP_SIZE_FLOATS  = 20;   // 5 particles × 4 components
static constexpr int RESULT_SIZE_BYTES = 4;   // 1 float per PSP
static constexpr int TOTAL_PIPES      = NUM_TRUNKS * PSPS_PER_TRUNK;

// Global flag (kept for compatibility; benchmark mode uses finite iterations)
volatile sig_atomic_t keep_running = 1;
void signal_handler(int) {
  std::cout << "\n\n[Signal received: stopping after current iteration...]\n";
  keep_running = 0;
}

static inline double us_to_ms(long long us) { return (double)us / 1000.0; }
static inline double ns_to_us(long long ns) { return (double)ns / 1000.0; }

// ---------------- Metrics helpers ----------------

struct Stats {
  double mean = 0.0;
  double min  = 0.0;
  double max  = 0.0;
  double std  = 0.0;
  size_t n    = 0;
};

static Stats compute_stats(const std::vector<double>& v, size_t start_idx = 0) {
  Stats s{};
  if (v.empty() || start_idx >= v.size()) return s;

  s.n = v.size() - start_idx;
  auto b = v.begin() + (long)start_idx;
  auto e = v.end();

  s.min = *std::min_element(b, e);
  s.max = *std::max_element(b, e);
  s.mean = std::accumulate(b, e, 0.0) / (double)s.n;

  if (s.n >= 2) {
    double sq = 0.0;
    for (auto it = b; it != e; ++it) {
      double d = (*it - s.mean);
      sq += d * d;
    }
    s.std = std::sqrt(sq / (double)s.n);
  }
  return s;
}

struct Metrics {
  // one-time setup
  double xclbin_load_ms   = 0.0;
  double bo_alloc_ms      = 0.0;
  double psp_gen_ms       = 0.0;
  double h2d_sync_ms      = 0.0;
  double graph_create_ms  = 0.0;
  double runobj_create_ms = 0.0;
  double total_setup_ms   = 0.0;

  // per-iteration vectors
  std::vector<double> iter_total_ms;

  std::vector<double> t_graph_run_cmd_ms;  // graph.run() call time (command issue)
  std::vector<double> t_kernel_start_ms;   // time to start all 20 kernels (10 s2mm + 10 mm2s)
  std::vector<double> t_mm2s_wait_ms;      // wait for all mm2s to complete
  std::vector<double> t_s2mm_wait_ms;      // wait for all s2mm to complete
  std::vector<double> t_graph_wait_ms;     // graph.wait() time (blocking)
  std::vector<double> t_graph_total_ms;    // from graph.run() start to graph.wait() end

  // derived
  int iterations_completed = 0;
  int total_psps_processed = 0;
};

static void print_usage(const char* prog) {
  std::cout
    << "Usage: " << prog << " <xclbin> [psps_per_trunk] [num_iterations] [energy] [options]\n\n"
    << "Positional:\n"
    << "  <xclbin>         Path to xclbin (required)\n"
    << "  [psps_per_trunk] PSPs per trunk per iteration (default 8, max 8)\n"
    << "  [num_iterations] Number of iterations (default 1)\n"
    << "  [energy]         CoM energy GeV (default 1500)\n\n"
    << "Options:\n"
    << "  --warmup <N>     Exclude first N iterations from steady-state stats (default 1)\n"
    << "  --csv <file>     Save metrics to CSV\n"
    << "  --quiet          Minimal output\n"
    << "  --seed <int>     RAMBO seed for one-time PSP generation\n";
}

static void save_metrics_csv(const std::string& filename,
                             const Metrics& m,
                             int psps_per_trunk,
                             int warmup) {
  std::ofstream csv(filename);
  csv << std::fixed << std::setprecision(6);

  const int psps_per_iter = psps_per_trunk * NUM_TRUNKS;

  Stats st_iter = compute_stats(m.iter_total_ms, warmup);
  Stats st_graph_total = compute_stats(m.t_graph_total_ms, warmup);
  Stats st_graph_run   = compute_stats(m.t_graph_run_cmd_ms, warmup);
  Stats st_graph_wait  = compute_stats(m.t_graph_wait_ms, warmup);
  Stats st_start       = compute_stats(m.t_kernel_start_ms, warmup);
  Stats st_mm2s        = compute_stats(m.t_mm2s_wait_ms, warmup);
  Stats st_s2mm        = compute_stats(m.t_s2mm_wait_ms, warmup);

  auto safe_div = [](double a, double b) { return (b == 0.0) ? 0.0 : (a / b); };

  // Throughput/latency using steady-state mean (preferred for paper)
  double ss_iter_ms = st_iter.mean;
  double ss_psp_per_s = safe_div((double)psps_per_iter * 1000.0, ss_iter_ms);
  double ss_us_per_psp = safe_div(ss_iter_ms * 1000.0, (double)psps_per_iter);

  // Bandwidth (input+output) in MB/s (MiB/s)
  double ss_in_MBps  = safe_div((double)psps_per_iter * PSP_SIZE_BYTES * 1000.0,
                                ss_iter_ms * 1024.0 * 1024.0);
  double ss_out_MBps = safe_div((double)psps_per_iter * RESULT_SIZE_BYTES * 1000.0,
                                ss_iter_ms * 1024.0 * 1024.0);

  csv << "metric,value,unit\n";
  csv << "psps_per_trunk," << psps_per_trunk << ",count\n";
  csv << "psps_per_iter," << psps_per_iter << ",count\n";
  csv << "warmup_excluded," << warmup << ",count\n";
  csv << "iterations_completed," << m.iterations_completed << ",count\n";
  csv << "total_psps_processed," << m.total_psps_processed << ",count\n";

  // Setup
  csv << "setup_xclbin_load,"   << m.xclbin_load_ms   << ",ms\n";
  csv << "setup_bo_alloc,"      << m.bo_alloc_ms      << ",ms\n";
  csv << "setup_psp_gen,"       << m.psp_gen_ms       << ",ms\n";
  csv << "setup_h2d_sync,"      << m.h2d_sync_ms      << ",ms\n";
  csv << "setup_graph_create,"  << m.graph_create_ms  << ",ms\n";
  csv << "setup_runobj_create," << m.runobj_create_ms << ",ms\n";
  csv << "setup_total,"         << m.total_setup_ms   << ",ms\n";

  // Steady-state totals
  csv << "ss_iter_mean," << st_iter.mean << ",ms\n";
  csv << "ss_iter_min,"  << st_iter.min  << ",ms\n";
  csv << "ss_iter_max,"  << st_iter.max  << ",ms\n";
  csv << "ss_iter_std,"  << st_iter.std  << ",ms\n";

  // Phase steady-state
  csv << "ss_graph_total_mean," << st_graph_total.mean << ",ms\n";
  csv << "ss_graph_run_cmd_mean," << st_graph_run.mean << ",ms\n";
  csv << "ss_graph_wait_mean," << st_graph_wait.mean << ",ms\n";
  csv << "ss_kernel_start_mean," << st_start.mean << ",ms\n";
  csv << "ss_mm2s_wait_mean," << st_mm2s.mean << ",ms\n";
  csv << "ss_s2mm_wait_mean," << st_s2mm.mean << ",ms\n";

  // Derived steady-state throughput/latency/bw
  csv << "ss_throughput," << ss_psp_per_s << ",PSP_per_s\n";
  csv << "ss_latency," << ss_us_per_psp << ",us_per_psp\n";
  csv << "ss_input_bw," << ss_in_MBps << ",MiB_per_s\n";
  csv << "ss_output_bw," << ss_out_MBps << ",MiB_per_s\n";
  csv << "ss_total_bw," << (ss_in_MBps + ss_out_MBps) << ",MiB_per_s\n";

  // Per-iteration times
  csv << "\niteration,iter_ms,graph_total_ms,graph_run_cmd_ms,graph_wait_ms,kernel_start_ms,mm2s_wait_ms,s2mm_wait_ms\n";
  for (size_t i = 0; i < m.iter_total_ms.size(); i++) {
    csv << (i + 1) << ","
        << m.iter_total_ms[i] << ","
        << m.t_graph_total_ms[i] << ","
        << m.t_graph_run_cmd_ms[i] << ","
        << m.t_graph_wait_ms[i] << ","
        << m.t_kernel_start_ms[i] << ","
        << m.t_mm2s_wait_ms[i] << ","
        << m.t_s2mm_wait_ms[i] << "\n";
  }

  csv.close();
  std::cout << "  Metrics saved to: " << filename << "\n";
}

// ---------------- PSP generation (one-time; benchmark mode) ----------------

static void generate_psps(float* buffer, int num_psps, double energy, bool quiet) {
  for (int i = 0; i < num_psps; i++) {
    double weight;
    std::vector<double*> psp = generate_psp(energy, weight);
    psp_to_float_array(psp, buffer + (i * PSP_SIZE_FLOATS));
    free_psp(psp);

    if (!quiet && i == 0) {
      std::cout << "    Sample PSP[0]: first float=" << buffer[0] << " weight=" << weight << "\n";
    }
  }
}

static void save_results_txt(const std::string& filename, float* results, int num_results) {
  std::ofstream file(filename);
  file << std::scientific << std::setprecision(10);
  for (int i = 0; i < num_results; i++) {
    file << "PSP " << i << ": ME2 = " << results[i] << "\n";
  }
  file.close();
}

// ---------------- Main run ----------------

int run(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  const char* xclbin_filename = argv[1];

  // Defaults
  int psps_per_trunk = PSPS_PER_TRUNK; // default 8
  int num_iterations = 1;
  double energy = 1500.0;
  int warmup = 1;                      // exclude first iteration from steady-state
  int random_seed = -1;
  bool quiet = false;
  std::string csv_filename;

  // Positional parsing (argv[2..])
  int arg_idx = 2;
  if (argc > arg_idx && argv[arg_idx][0] != '-') {
    psps_per_trunk = std::atoi(argv[arg_idx++]);
  }
  if (argc > arg_idx && argv[arg_idx][0] != '-') {
    num_iterations = std::atoi(argv[arg_idx++]);
  }
  if (argc > arg_idx && argv[arg_idx][0] != '-') {
    energy = std::atof(argv[arg_idx++]);
  }

  // Flags
  for (int i = arg_idx; i < argc; i++) {
    std::string a = argv[i];
    if (a == "--quiet") quiet = true;
    else if (a == "--csv" && i + 1 < argc) csv_filename = argv[++i];
    else if (a == "--warmup" && i + 1 < argc) warmup = std::atoi(argv[++i]);
    else if (a == "--seed" && i + 1 < argc) random_seed = std::atoi(argv[++i]);
  }

  // Validate
  if (psps_per_trunk < 1 || psps_per_trunk > 8) {
    std::cerr << "ERROR: psps_per_trunk must be 1..8 (pktsplit<8> routing limit)\n";
    return EXIT_FAILURE;
  }
  if (num_iterations < 1) {
    std::cerr << "ERROR: num_iterations must be >= 1 for benchmark mode\n";
    return EXIT_FAILURE;
  }
  if (energy <= 0.0) {
    std::cerr << "ERROR: energy must be > 0\n";
    return EXIT_FAILURE;
  }
  if (warmup < 0) warmup = 0;

  // For long runs, if warmup >= iterations, clamp.
  if (warmup >= num_iterations) warmup = std::max(0, num_iterations - 1);

  if (random_seed >= 0) {
    set_rambo_seed(random_seed, random_seed + 1000);
  }

  Metrics metrics;
  auto t_setup0 = std::chrono::high_resolution_clock::now();

  const int psps_per_iter = psps_per_trunk * NUM_TRUNKS;
  const size_t input_size_per_trunk  = (size_t)psps_per_trunk * (size_t)PSP_SIZE_BYTES;
  const size_t output_size_per_trunk = (size_t)psps_per_trunk * (size_t)RESULT_SIZE_BYTES;

  if (!quiet) {
    std::cout << "=============================================================================\n";
    std::cout << " 80-Pipeline Packet-Switched Matrix Element Accelerator (BENCHMARK MODE)\n";
    std::cout << "=============================================================================\n";
    std::cout << " Trunks:          " << NUM_TRUNKS << "\n";
    std::cout << " PSPs/trunk/iter: " << psps_per_trunk << " (max 8)\n";
    std::cout << " PSPs/iter total: " << psps_per_iter << "\n";
    std::cout << " Iterations:      " << num_iterations << "\n";
    std::cout << " Warmup excluded: " << warmup << "\n";
    std::cout << " Energy:          " << energy << " GeV\n";
    if (random_seed >= 0) std::cout << " Seed:            " << random_seed << "\n";
    std::cout << " XCLBIN:          " << xclbin_filename << "\n";
    std::cout << "=============================================================================\n";
  }

  // ---------------- Device + xclbin ----------------
  auto t_xcl0 = std::chrono::high_resolution_clock::now();
  xrt::device device(0);
  auto uuid = device.load_xclbin(xclbin_filename);
  auto t_xcl1 = std::chrono::high_resolution_clock::now();
  metrics.xclbin_load_ms = us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_xcl1 - t_xcl0).count());

  // ---------------- Kernel handles ----------------
  xrt::kernel mm2s_k[NUM_TRUNKS];
  xrt::kernel s2mm_k[NUM_TRUNKS];

  for (int i = 0; i < NUM_TRUNKS; i++) {
    std::string mm2s_name = "mm2s_pkt_gen:{mm2s_" + std::to_string(i) + "}";
    std::string s2mm_name = "s2mm_pkt_parser:{s2mm_" + std::to_string(i) + "}";
    mm2s_k[i] = xrt::kernel(device, uuid, mm2s_name);
    s2mm_k[i] = xrt::kernel(device, uuid, s2mm_name);
  }

  // ---------------- BO allocate (FIXED: sizes + group_id) ----------------
  auto t_bo0 = std::chrono::high_resolution_clock::now();

  xrt::bo input_bo[NUM_TRUNKS];
  xrt::bo output_bo[NUM_TRUNKS];
  float* input_host[NUM_TRUNKS];
  float* output_host[NUM_TRUNKS];

  for (int i = 0; i < NUM_TRUNKS; i++) {
    // mm2s: psp_buffer(arg0), axis_out stream(arg1, skipped), num_packets(arg2)
    // s2mm: axis_in stream(arg0, skipped), result_buffer(arg1), num_packets(arg2)
    auto g_in  = mm2s_k[i].group_id(0); // psp_buffer = arg 0
    auto g_out = s2mm_k[i].group_id(1); // result_buffer = arg 1 (stream is arg 0)

    input_bo[i]  = xrt::bo(device, input_size_per_trunk,  xrt::bo::flags::normal, g_in);
    output_bo[i] = xrt::bo(device, output_size_per_trunk, xrt::bo::flags::normal, g_out);

    input_host[i]  = input_bo[i].map<float*>();
    output_host[i] = output_bo[i].map<float*>();
  }

  auto t_bo1 = std::chrono::high_resolution_clock::now();
  metrics.bo_alloc_ms = us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_bo1 - t_bo0).count());

  // ---------------- One-time PSP generation (benchmark mode) ----------------
  auto t_psp0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_TRUNKS; i++) {
    generate_psps(input_host[i], psps_per_trunk, energy, quiet);
  }
  auto t_psp1 = std::chrono::high_resolution_clock::now();
  metrics.psp_gen_ms = us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_psp1 - t_psp0).count());

  // ---------------- One-time H2D sync ----------------
  auto t_h2d0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_TRUNKS; i++) {
    input_bo[i].sync(XCL_BO_SYNC_BO_TO_DEVICE);
  }
  auto t_h2d1 = std::chrono::high_resolution_clock::now();
  metrics.h2d_sync_ms = us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_h2d1 - t_h2d0).count());

  // ---------------- Graph handle (one-time) ----------------
  auto t_g0 = std::chrono::high_resolution_clock::now();
  xrt::graph graph(device, uuid, "ggttg_graph_80pipe");
  auto t_g1 = std::chrono::high_resolution_clock::now();
  metrics.graph_create_ms = us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_g1 - t_g0).count());

  // Recommended: reset once before timed loop (keeps loop steady-state)
  graph.reset();

  // ---------------- Pre-create run objects (one-time) ----------------
  auto t_r0 = std::chrono::high_resolution_clock::now();

  xrt::run mm2s_run[NUM_TRUNKS];
  xrt::run s2mm_run[NUM_TRUNKS];

  for (int i = 0; i < NUM_TRUNKS; i++) {
    // mm2s: psp_buffer(arg0), axis_out stream(arg1 - skip), num_packets(arg2)
    mm2s_run[i] = xrt::run(mm2s_k[i]);
    mm2s_run[i].set_arg(0, input_bo[i]);      // psp_buffer
    mm2s_run[i].set_arg(2, psps_per_trunk);   // num_packets (arg1 is stream, skip it)

    // s2mm: axis_in stream(arg0 - skip), result_buffer(arg1), num_packets(arg2)
    s2mm_run[i] = xrt::run(s2mm_k[i]);
    s2mm_run[i].set_arg(1, output_bo[i]);     // result_buffer
    s2mm_run[i].set_arg(2, psps_per_trunk);   // num_packets (arg0 is stream, skip it)
  }

  auto t_r1 = std::chrono::high_resolution_clock::now();
  metrics.runobj_create_ms = us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_r1 - t_r0).count());

  auto t_setup1 = std::chrono::high_resolution_clock::now();
  metrics.total_setup_ms = us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_setup1 - t_setup0).count());

  if (!quiet) {
    std::cout << "\n[Setup timings]\n";
    std::cout << "  XCLBIN load:    " << metrics.xclbin_load_ms << " ms\n";
    std::cout << "  BO alloc:       " << metrics.bo_alloc_ms << " ms\n";
    std::cout << "  PSP gen (once): " << metrics.psp_gen_ms << " ms\n";
    std::cout << "  H2D sync (once):" << metrics.h2d_sync_ms << " ms\n";
    std::cout << "  Graph create:   " << metrics.graph_create_ms << " ms\n";
    std::cout << "  Runobj create:  " << metrics.runobj_create_ms << " ms\n";
    std::cout << "  TOTAL setup:    " << metrics.total_setup_ms << " ms\n\n";
  }

  // ---------------- Timed execution loop ----------------
  // For paper benchmarking:
  //   - we keep input fixed
  //   - we do NOT D2H sync inside loop
  //   - we measure iteration time as: from graph.run() issue to after graph.wait() completion,
  //     plus the explicit mm2s/s2mm waits and start overhead as separate fields.
  //
  // Important ordering for streaming:
  //   - Start S2MM first (ready to consume AIE output)
  //   - Start MM2S next (push data to AIE)
  //   - graph.run() can be issued before or after; we keep it FIRST to overlap its controller overhead.
  //
  // Per-iteration: reset+run(1)+wait. graph.reset() is required before each
  // graph.run(1) after the first, because graph.wait() leaves it in 'ended' state.

  keep_running = 1; // benchmark mode, but keep for Ctrl+C safety
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  for (int iter = 0; iter < num_iterations && keep_running; iter++) {

    auto t_iter0 = std::chrono::high_resolution_clock::now();

    // ---- Phase A: reset + graph.run(1) ----
    if (iter > 0) graph.reset();
    auto t_gr0 = std::chrono::high_resolution_clock::now();
    graph.run(1);
    auto t_gr1 = std::chrono::high_resolution_clock::now();
    double graph_run_cmd_ms =
      us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_gr1 - t_gr0).count());

    // ---- Phase B: start all kernels (reused run objects) ----
    auto t_st0 = std::chrono::high_resolution_clock::now();
    for (int t = 0; t < NUM_TRUNKS; t++) s2mm_run[t].start();
    for (int t = 0; t < NUM_TRUNKS; t++) mm2s_run[t].start();
    auto t_st1 = std::chrono::high_resolution_clock::now();
    double kernel_start_ms =
      us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_st1 - t_st0).count());

    // ---- Phase C: wait mm2s ----
    auto t_m0 = std::chrono::high_resolution_clock::now();
    for (int t = 0; t < NUM_TRUNKS; t++) {
      auto st = mm2s_run[t].wait(std::chrono::milliseconds(5000));
      if (st != ERT_CMD_STATE_COMPLETED) {
        std::cerr << "ERROR: mm2s trunk " << t << " did not complete (state=" << st << ")\n";
        throw std::runtime_error("MM2S timeout/error");
      }
    }
    auto t_m1 = std::chrono::high_resolution_clock::now();
    double mm2s_wait_ms =
      us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_m1 - t_m0).count());

    // ---- Phase D: wait s2mm ----
    auto t_s0 = std::chrono::high_resolution_clock::now();
    for (int t = 0; t < NUM_TRUNKS; t++) {
      auto st = s2mm_run[t].wait(std::chrono::milliseconds(5000));
      if (st != ERT_CMD_STATE_COMPLETED) {
        std::cerr << "ERROR: s2mm trunk " << t << " did not complete (state=" << st << ")\n";
        throw std::runtime_error("S2MM timeout/error");
      }
    }
    auto t_s1 = std::chrono::high_resolution_clock::now();
    double s2mm_wait_ms =
      us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_s1 - t_s0).count());

    // ---- Phase E: graph.wait ----
    auto t_gw0 = std::chrono::high_resolution_clock::now();
    graph.wait(0); // 0 = wait indefinitely
    auto t_gw1 = std::chrono::high_resolution_clock::now();
    double graph_wait_ms =
      us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_gw1 - t_gw0).count());
    double graph_total_ms =
      us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_gw1 - t_gr0).count());

    auto t_iter1 = std::chrono::high_resolution_clock::now();
    double iter_total_ms =
      us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_iter1 - t_iter0).count());

    // Store
    metrics.iter_total_ms.push_back(iter_total_ms);
    metrics.t_graph_run_cmd_ms.push_back(graph_run_cmd_ms);
    metrics.t_kernel_start_ms.push_back(kernel_start_ms);
    metrics.t_mm2s_wait_ms.push_back(mm2s_wait_ms);
    metrics.t_s2mm_wait_ms.push_back(s2mm_wait_ms);
    metrics.t_graph_wait_ms.push_back(graph_wait_ms);
    metrics.t_graph_total_ms.push_back(graph_total_ms);

    // Progress printing (light, still useful)
    if (!quiet && (iter == 0 || (iter + 1) % 50 == 0 || iter + 1 == num_iterations)) {
      double us_per_psp = (iter_total_ms * 1000.0) / (double)psps_per_iter;
      double psp_per_s  = ((double)psps_per_iter * 1000.0) / iter_total_ms;
      std::cout << "[iter " << (iter + 1) << "/" << num_iterations << "] "
                << "iter=" << std::fixed << std::setprecision(3) << iter_total_ms << " ms, "
                << "throughput=" << std::setprecision(0) << psp_per_s << " PSP/s, "
                << "lat=" << std::setprecision(2) << us_per_psp << " us/PSP\n";
    }
  }

  // Wait for all graph iterations to complete, then stop
  graph.end();

  metrics.iterations_completed = (int)metrics.iter_total_ms.size();
  metrics.total_psps_processed = metrics.iterations_completed * psps_per_iter;

  // One D2H sync at the end (benchmark mode)
  auto t_d2h0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_TRUNKS; i++) {
    output_bo[i].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  }
  auto t_d2h1 = std::chrono::high_resolution_clock::now();
  double d2h_ms =
    us_to_ms(std::chrono::duration_cast<std::chrono::microseconds>(t_d2h1 - t_d2h0).count());

  // Paper-ready stats: steady-state excludes warmup
  Stats st_iter        = compute_stats(metrics.iter_total_ms, warmup);
  Stats st_graph_total = compute_stats(metrics.t_graph_total_ms, warmup);
  Stats st_grcmd       = compute_stats(metrics.t_graph_run_cmd_ms, warmup);
  Stats st_gwait       = compute_stats(metrics.t_graph_wait_ms, warmup);
  Stats st_start       = compute_stats(metrics.t_kernel_start_ms, warmup);
  Stats st_mm2s        = compute_stats(metrics.t_mm2s_wait_ms, warmup);
  Stats st_s2mm        = compute_stats(metrics.t_s2mm_wait_ms, warmup);

  auto safe_div = [](double a, double b) { return (b == 0.0) ? 0.0 : (a / b); };

  double ss_iter_ms   = st_iter.mean;
  double ss_psp_per_s = safe_div((double)psps_per_iter * 1000.0, ss_iter_ms);
  double ss_us_per_psp= safe_div(ss_iter_ms * 1000.0, (double)psps_per_iter);

  double ss_in_MBps  = safe_div((double)psps_per_iter * PSP_SIZE_BYTES * 1000.0,
                                ss_iter_ms * 1024.0 * 1024.0);
  double ss_out_MBps = safe_div((double)psps_per_iter * RESULT_SIZE_BYTES * 1000.0,
                                ss_iter_ms * 1024.0 * 1024.0);


  if (!quiet) {
    std::cout << "\n=============================================================================\n";
    std::cout << " Benchmark Summary (steady-state excludes warmup=" << warmup << ")\n";
    std::cout << "=============================================================================\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << " Iterations completed: " << metrics.iterations_completed << "\n";
    std::cout << " PSPs/iter:            " << psps_per_iter << "\n";
    std::cout << " Total PSPs:           " << metrics.total_psps_processed << "\n\n";

    std::cout << " Steady-state iteration time (ms):\n";
    std::cout << "   mean=" << st_iter.mean << "  std=" << st_iter.std
              << "  min=" << st_iter.min << "  max=" << st_iter.max << "\n\n";

    std::cout << " Steady-state throughput/latency:\n";
    std::cout << std::setprecision(0);
    std::cout << "   throughput = " << ss_psp_per_s << " PSP/s\n";
    std::cout << std::setprecision(2);
    std::cout << "   latency    = " << ss_us_per_psp << " us/PSP\n\n";

    std::cout << " Steady-state bandwidth (MiB/s):\n";
    std::cout << "   input  = " << ss_in_MBps << "\n";
    std::cout << "   output = " << ss_out_MBps << "\n";
    std::cout << "   total  = " << (ss_in_MBps + ss_out_MBps) << "\n\n";

    std::cout << " Phase breakdown (steady-state mean, ms per iteration):\n";
    std::cout << "   graph.run()+reset  = " << st_grcmd.mean << " ms\n";
    std::cout << "   kernels .start()   = " << st_start.mean << " ms\n";
    std::cout << "   mm2s wait          = " << st_mm2s.mean << " ms\n";
    std::cout << "   s2mm wait          = " << st_s2mm.mean << " ms   (AIE compute + DMA)\n";
    std::cout << "   graph.wait()       = " << st_gwait.mean << " ms\n";
    std::cout << "   graph total        = " << st_graph_total.mean << " ms\n";
    std::cout << "   iter total         = " << st_iter.mean << " ms\n\n";

    std::cout << " Phase breakdown (steady-state mean, us per PSP):\n";
    auto ms_to_us_per_psp = [&](double ms) { return safe_div(ms * 1000.0, (double)psps_per_iter); };
    std::cout << "   graph.run()+reset  = " << ms_to_us_per_psp(st_grcmd.mean) << " us/PSP\n";
    std::cout << "   kernels .start()   = " << ms_to_us_per_psp(st_start.mean) << " us/PSP\n";
    std::cout << "   mm2s wait          = " << ms_to_us_per_psp(st_mm2s.mean) << " us/PSP\n";
    std::cout << "   s2mm wait          = " << ms_to_us_per_psp(st_s2mm.mean) << " us/PSP\n";
    std::cout << "   graph.wait()       = " << ms_to_us_per_psp(st_gwait.mean) << " us/PSP\n";
    std::cout << "   iter total         = " << ms_to_us_per_psp(st_iter.mean) << " us/PSP\n\n";

    std::cout << " D2H sync (end, once): " << d2h_ms << " ms\n";
    std::cout << "=============================================================================\n";
  }

  // Save last iteration results (optional; keep off by default for clean benchmarking)
  // If you want them: uncomment
  /*
  for (int t = 0; t < NUM_TRUNKS; t++) {
    std::string fn = "me2_out_" + std::to_string(t) + ".txt";
    save_results_txt(fn, output_host[t], psps_per_trunk);
  }
  */

  if (!csv_filename.empty()) {
    save_metrics_csv(csv_filename, metrics, psps_per_trunk, warmup);
  }

  return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
  try {
    int r = run(argc, argv);
    std::cout << "\nTEST " << (r ? "FAILED" : "PASSED") << "\n";
    return r;
  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\nFAILED TEST\n";
    return EXIT_FAILURE;
  }
}
