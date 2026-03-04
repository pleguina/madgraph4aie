/*
 * XRT Host Application for 80-Pipeline Packet-Switched Architecture
 * Process: g g > t t~ g (gluon-gluon -> top-antitop + gluon)
 * 
 * Architecture:
 *   - 8 trunks (rows 0-7)
 *   - 10 PSPs per trunk
 *   - Packet-switched routing via pktsplit<10>/pktmerge<10>
 *   - MM2S_pkt_gen: DDR -> PLIO (packet generation)
 *   - S2MM_pkt_parser: PLIO -> DDR (result collection)
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
#include "xrt/xrt_kernel.h"
#include "xrt/xrt_aie.h"
#include "xrt/xrt_bo.h"

// Architecture parameters
#define NUM_TRUNKS 8
#define PSPS_PER_TRUNK 10
#define PSP_SIZE_BYTES 80        // 5 particles × 4 components × 4 bytes
#define PSP_SIZE_FLOATS 20       // 5 particles × 4 components
#define RESULT_SIZE_BYTES 4      // 1 float per PSP (ME² result)
#define TOTAL_PSPS (NUM_TRUNKS * PSPS_PER_TRUNK)

// Global flag for continuous mode
volatile sig_atomic_t keep_running = 1;

void signal_handler(int signum) {
    std::cout << "\n\n[Signal received: Stopping after current iteration...]" << std::endl;
    keep_running = 0;
}

// Performance metrics structure
struct Metrics {
    double setup_time_ms;
    double xclbin_load_time_ms;
    double buffer_alloc_time_ms;
    double data_load_time_ms;
    double h2d_transfer_time_ms;
    std::vector<double> iteration_times_ms;
    std::vector<double> kernel_exec_times_ms;
    std::vector<double> d2h_transfer_times_ms;
    int total_psps_processed;
    int num_iterations;
    
    double get_avg_iteration_ms() const {
        if (iteration_times_ms.empty()) return 0.0;
        return std::accumulate(iteration_times_ms.begin(), iteration_times_ms.end(), 0.0) / iteration_times_ms.size();
    }
    
    double get_min_iteration_ms() const {
        if (iteration_times_ms.empty()) return 0.0;
        return *std::min_element(iteration_times_ms.begin(), iteration_times_ms.end());
    }
    
    double get_max_iteration_ms() const {
        if (iteration_times_ms.empty()) return 0.0;
        return *std::max_element(iteration_times_ms.begin(), iteration_times_ms.end());
    }
    
    double get_stddev_iteration_ms() const {
        if (iteration_times_ms.size() < 2) return 0.0;
        double avg = get_avg_iteration_ms();
        double sq_sum = 0.0;
        for (auto t : iteration_times_ms) {
            sq_sum += (t - avg) * (t - avg);
        }
        return std::sqrt(sq_sum / iteration_times_ms.size());
    }
};

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <xclbin> <data_dir> [num_psps] [num_iterations] [options]" << std::endl;
    std::cout << "  <xclbin>         - Path to compiled hardware bitstream" << std::endl;
    std::cout << "  <data_dir>       - Directory containing psp_in_*.bin files" << std::endl;
    std::cout << "  [num_psps]       - PSPs per trunk to process (default: 10, max: 10)" << std::endl;
    std::cout << "  [num_iterations] - Number of times to run (default: 1, 0=continuous)" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --csv <file>     - Save metrics to CSV file for analysis" << std::endl;
    std::cout << "  --quiet          - Minimal output (good for benchmarking)" << std::endl;
    std::cout << "  --power-mode     - Continuous run for power measurement (Ctrl+C to stop)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << prog << " hw.xclbin ./data                      # Process 10 PSPs once" << std::endl;
    std::cout << "  " << prog << " hw.xclbin ./data 5                    # Process 5 PSPs once" << std::endl;
    std::cout << "  " << prog << " hw.xclbin ./data 10 100               # Process 10 PSPs, 100x" << std::endl;
    std::cout << "  " << prog << " hw.xclbin ./data 10 1000 --csv out.csv # Save metrics to CSV" << std::endl;
    std::cout << "  " << prog << " hw.xclbin ./data 10 0                 # Continuous (Ctrl+C to stop)" << std::endl;
    std::cout << "  " << prog << " hw.xclbin ./data 10 0 --power-mode    # Power measurement mode" << std::endl;
}

// Save metrics to CSV file
void save_metrics_csv(const std::string& filename, const Metrics& m, int psps_per_trunk) {
    std::ofstream csv(filename);
    csv << std::fixed << std::setprecision(6);
    
    // Header
    csv << "metric,value,unit\n";
    
    // Setup metrics
    csv << "setup_time," << m.setup_time_ms << ",ms\n";
    csv << "xclbin_load_time," << m.xclbin_load_time_ms << ",ms\n";
    csv << "buffer_alloc_time," << m.buffer_alloc_time_ms << ",ms\n";
    csv << "data_load_time," << m.data_load_time_ms << ",ms\n";
    csv << "h2d_transfer_time," << m.h2d_transfer_time_ms << ",ms\n";
    
    // Execution metrics
    csv << "num_iterations," << m.num_iterations << ",count\n";
    csv << "total_psps_processed," << m.total_psps_processed << ",count\n";
    csv << "avg_iteration_time," << m.get_avg_iteration_ms() << ",ms\n";
    csv << "min_iteration_time," << m.get_min_iteration_ms() << ",ms\n";
    csv << "max_iteration_time," << m.get_max_iteration_ms() << ",ms\n";
    csv << "stddev_iteration_time," << m.get_stddev_iteration_ms() << ",ms\n";
    
    // Performance metrics
    double total_time = std::accumulate(m.iteration_times_ms.begin(), m.iteration_times_ms.end(), 0.0);
    double throughput = (m.total_psps_processed * 1000.0) / total_time;
    double latency_us = (total_time * 1000.0) / m.total_psps_processed;
    double input_bw = (m.total_psps_processed * PSP_SIZE_BYTES * 1000.0) / (total_time * 1024.0 * 1024.0);
    double output_bw = (m.total_psps_processed * RESULT_SIZE_BYTES * 1000.0) / (total_time * 1024.0 * 1024.0);
    
    csv << "throughput," << throughput << ",PSP/s\n";
    csv << "latency_per_psp," << latency_us << ",us\n";
    csv << "input_bandwidth," << input_bw << ",MB/s\n";
    csv << "output_bandwidth," << output_bw << ",MB/s\n";
    csv << "total_bandwidth," << (input_bw + output_bw) << ",MB/s\n";
    
    // Per-iteration times
    csv << "\niteration,time_ms\n";
    for (size_t i = 0; i < m.iteration_times_ms.size(); i++) {
        csv << i + 1 << "," << m.iteration_times_ms[i] << "\n";
    }
    
    csv.close();
    std::cout << "  Metrics saved to: " << filename << std::endl;
}

// Load PSP data from binary file
bool load_psp_data(const std::string& filename, float* buffer, size_t expected_size) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return false;
    }
    
    file.read(reinterpret_cast<char*>(buffer), expected_size);
    size_t bytes_read = file.gcount();
    
    if (bytes_read != expected_size) {
        std::cerr << "Error: Expected " << expected_size << " bytes, read " << bytes_read << std::endl;
        return false;
    }
    
    file.close();
    return true;
}

// Save results to text file
void save_results(const std::string& filename, float* results, int num_results) {
    std::ofstream file(filename);
    file << std::scientific << std::setprecision(10);
    
    for (int i = 0; i < num_results; i++) {
        file << "PSP " << i << ": ME² = " << results[i] << std::endl;
    }
    
    file.close();
    std::cout << "  Results saved to " << filename << std::endl;
}

int run(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    char* xclbin_filename = argv[1];
    std::string data_dir = argv[2];
    
    // Parse optional arguments
    int psps_to_process = PSPS_PER_TRUNK;  // Default: 10
    int num_iterations = 1;                 // Default: 1 (0 = continuous)
    bool continuous_mode = false;
    bool quiet_mode = false;
    bool power_mode = false;
    std::string csv_filename = "";
    
    if (argc >= 4) {
        psps_to_process = std::atoi(argv[3]);
        if (psps_to_process < 1 || psps_to_process > PSPS_PER_TRUNK) {
            std::cerr << "Error: num_psps must be between 1 and " << PSPS_PER_TRUNK << std::endl;
            return EXIT_FAILURE;
        }
    }
    
    if (argc >= 5) {
        num_iterations = std::atoi(argv[4]);
        if (num_iterations < 0) {
            std::cerr << "Error: num_iterations must be >= 0" << std::endl;
            return EXIT_FAILURE;
        }
        if (num_iterations == 0) {
            continuous_mode = true;
            num_iterations = 999999999;  // Large number
        }
    }
    
    // Parse flags
    for (int i = 5; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--csv" && i + 1 < argc) {
            csv_filename = argv[++i];
        } else if (arg == "--quiet") {
            quiet_mode = true;
        } else if (arg == "--power-mode") {
            power_mode = true;
            continuous_mode = true;
            num_iterations = 999999999;
        }
    }
    
    // Setup signal handler for continuous mode
    if (continuous_mode) {
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    }
    
    Metrics metrics;
    auto total_start = std::chrono::high_resolution_clock::now();

    if (!quiet_mode) {
        std::cout << "============================================================================="<< std::endl;
        std::cout << " 80-Pipeline Packet-Switched Matrix Element Accelerator" << std::endl;
        std::cout << "============================================================================="<< std::endl;
        std::cout << " Process:       g g > t t~ g" << std::endl;
        std::cout << " Trunks:        " << NUM_TRUNKS << std::endl;
        std::cout << " PSPs/trunk:    " << psps_to_process << " (max: " << PSPS_PER_TRUNK << ")" << std::endl;
        std::cout << " Total PSPs:    " << NUM_TRUNKS * psps_to_process << std::endl;
        std::cout << " Iterations:    " << (continuous_mode ? "∞ (continuous)" : std::to_string(num_iterations)) << std::endl;
        if (power_mode) std::cout << " Mode:          POWER MEASUREMENT (Ctrl+C to stop)" << std::endl;
        if (!csv_filename.empty()) std::cout << " CSV output:    " << csv_filename << std::endl;
        std::cout << " XCLBIN:        " << xclbin_filename << std::endl;
        std::cout << " Data dir:      " << data_dir << std::endl;
        std::cout << "============================================================================="<< std::endl;
    }

    // Calculate buffer sizes
    size_t input_size_per_trunk = PSPS_PER_TRUNK * PSP_SIZE_BYTES;
    size_t output_size_per_trunk = PSPS_PER_TRUNK * RESULT_SIZE_BYTES;

    // Open xclbin and get device
    if (!quiet_mode) std::cout << "\n[1/7] Opening device and loading XCLBIN..." << std::endl;
    auto xclbin_start = std::chrono::high_resolution_clock::now();
    auto device = xrt::device(0);
    auto uuid = device.load_xclbin(xclbin_filename);
    auto xclbin_end = std::chrono::high_resolution_clock::now();
    metrics.xclbin_load_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(xclbin_end - xclbin_start).count() / 1000.0;
    if (!quiet_mode) std::cout << "  Device opened, XCLBIN loaded (" << metrics.xclbin_load_time_ms << " ms)" << std::endl;

    // Get kernel handles
    if (!quiet_mode) std::cout << "\n[2/7] Getting kernel handles..." << std::endl;
    xrt::kernel mm2s_kernels[NUM_TRUNKS];
    xrt::kernel s2mm_kernels[NUM_TRUNKS];

    for (int i = 0; i < NUM_TRUNKS; i++) {
        std::string mm2s_name = "mm2s_pkt_gen:{mm2s_pkt_gen_" + std::to_string(i) + "}";
        std::string s2mm_name = "s2mm_pkt_parser:{s2mm_pkt_parser_" + std::to_string(i) + "}";
        
        mm2s_kernels[i] = xrt::kernel(device, uuid, mm2s_name);
        s2mm_kernels[i] = xrt::kernel(device, uuid, s2mm_name);
        
        if (!quiet_mode) std::cout << "  Trunk " << i << ": " << mm2s_name << " & " << s2mm_name << std::endl;
    }

    // Allocate input buffers
    if (!quiet_mode) std::cout << "\n[3/7] Allocating input buffers..." << std::endl;
    auto alloc_start = std::chrono::high_resolution_clock::now();
    xrt::bo input_bo[NUM_TRUNKS];
    float* input_host[NUM_TRUNKS];

    for (int i = 0; i < NUM_TRUNKS; i++) {
        input_bo[i] = xrt::bo(device, input_size_per_trunk, mm2s_kernels[i].group_id(0));
        input_host[i] = input_bo[i].map<float*>();
        if (!quiet_mode) std::cout << "  Trunk " << i << ": " << input_size_per_trunk << " bytes" << std::endl;
    }

    // Allocate output buffers
    if (!quiet_mode) std::cout << "\n[4/7] Allocating output buffers..." << std::endl;
    xrt::bo output_bo[NUM_TRUNKS];
    float* output_host[NUM_TRUNKS];

    for (int i = 0; i < NUM_TRUNKS; i++) {
        output_bo[i] = xrt::bo(device, output_size_per_trunk, s2mm_kernels[i].group_id(0));
        output_host[i] = output_bo[i].map<float*>();
        if (!quiet_mode) std::cout << "  Trunk " << i << ": " << output_size_per_trunk << " bytes" << std::endl;
    }
    auto alloc_end = std::chrono::high_resolution_clock::now();
    metrics.buffer_alloc_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(alloc_end - alloc_start).count() / 1000.0;

    // Load input data
    if (!quiet_mode) std::cout << "\n[5/7] Loading PSP data..." << std::endl;
    auto load_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_TRUNKS; i++) {
        std::string filename = data_dir + "/psp_in_" + std::to_string(i) + ".bin";
        if (!load_psp_data(filename, input_host[i], input_size_per_trunk)) {
            std::cerr << "Failed to load data for trunk " << i << std::endl;
            return EXIT_FAILURE;
        }
        if (!quiet_mode) std::cout << "  Trunk " << i << ": loaded " << PSPS_PER_TRUNK << " PSPs from " << filename << std::endl;
    }
    auto load_end = std::chrono::high_resolution_clock::now();
    metrics.data_load_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(load_end - load_start).count() / 1000.0;

    // Sync input buffers to device
    if (!quiet_mode) std::cout << "\n[6/7] Syncing input data to device..." << std::endl;
    auto h2d_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_TRUNKS; i++) {
        input_bo[i].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    }
    auto h2d_end = std::chrono::high_resolution_clock::now();
    metrics.h2d_transfer_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(h2d_end - h2d_start).count() / 1000.0;
    if (!quiet_mode) std::cout << "  All input buffers synced to DDR (" << metrics.h2d_transfer_time_ms << " ms)" << std::endl;
    
    auto setup_end = std::chrono::high_resolution_clock::now();
    metrics.setup_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(setup_end - total_start).count() / 1000.0;

    // Start kernels and graph
    if (!quiet_mode) std::cout << "\n[7/7] Starting execution..." << std::endl;
    if (power_mode) {
        std::cout << "\n*** POWER MEASUREMENT MODE ***" << std::endl;
        std::cout << "*** Device running continuously - measure power now ***" << std::endl;
        std::cout << "*** Press Ctrl+C when done ***\n" << std::endl;
    }
    
    double total_elapsed = 0.0;
    int completed_iterations = 0;
    auto last_update = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < num_iterations && keep_running; iter++) {
        if (!quiet_mode && !power_mode && num_iterations > 1) {
            std::cout << "\n--- Iteration " << (iter + 1) << "/" << num_iterations << " ---" << std::endl;
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();

        // Launch S2MM kernels first (they wait for data)
        if (iter == 0 && !quiet_mode && !power_mode) std::cout << "  Starting S2MM parsers..." << std::endl;
        xrt::run s2mm_runs[NUM_TRUNKS];
        for (int i = 0; i < NUM_TRUNKS; i++) {
            s2mm_runs[i] = s2mm_kernels[i](
                output_bo[i],       // output_results
                0,                  // base_offset
                psps_to_process,    // num_packets (configurable)
                nullptr             // error_count (optional)
            );
        }

        // Launch MM2S kernels (they send data to AIE)
        if (iter == 0 && !quiet_mode && !power_mode) std::cout << "  Starting MM2S generators..." << std::endl;
        xrt::run mm2s_runs[NUM_TRUNKS];
        for (int i = 0; i < NUM_TRUNKS; i++) {
            mm2s_runs[i] = mm2s_kernels[i](
                input_bo[i],        // input_psps
                0,                  // base_offset
                psps_to_process     // num_packets (configurable)
            );
        }

        // Start AIE graph
        if (iter == 0 && !quiet_mode && !power_mode) std::cout << "  Starting AIE graph (80 pipelines)..." << std::endl;
        auto graph_handle = xrt::graph(device, uuid, "ggttg_graph_80pipe");
        graph_handle.run(psps_to_process);  // Run for N iterations per pipeline (configurable)
        
        if (iter == 0 && !quiet_mode && !power_mode) std::cout << "  Waiting for AIE graph completion..." << std::endl;
        graph_handle.end();

        // Wait for S2MM kernels to complete
        if (iter == 0 && !quiet_mode && !power_mode) std::cout << "  Waiting for S2MM parsers..." << std::endl;
        for (int i = 0; i < NUM_TRUNKS; i++) {
            s2mm_runs[i].wait();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        double elapsed_ms = duration.count() / 1000.0;
        
        metrics.iteration_times_ms.push_back(elapsed_ms);
        total_elapsed += elapsed_ms;
        completed_iterations++;
        
        // Progress update for power mode
        if (power_mode) {
            auto now = std::chrono::high_resolution_clock::now();
            auto since_update = std::chrono::duration_cast<std::chrono::seconds>(now - last_update).count();
            if (since_update >= 5) {  // Update every 5 seconds
                double avg_throughput = (completed_iterations * NUM_TRUNKS * psps_to_process * 1000.0) / total_elapsed;
                std::cout << "[" << completed_iterations << " iterations] "
                          << "Avg: " << std::fixed << std::setprecision(2) << total_elapsed / completed_iterations << " ms/iter, "
                          << std::setprecision(0) << avg_throughput << " PSP/s" << std::endl;
                last_update = now;
            }
        } else if (!quiet_mode && num_iterations > 1 && iter == 0) {
            std::cout << "  First iteration: " << elapsed_ms << " ms" << std::endl;
        }
    }
    
    metrics.num_iterations = completed_iterations;

    metrics.total_psps_processed = NUM_TRUNKS * psps_to_process * completed_iterations;
    
    if (!quiet_mode) {
        std::cout << "\n✓ All iterations complete!" << std::endl;
        std::cout << "  Completed:      " << completed_iterations << " iterations" << std::endl;
        std::cout << "  Total time:     " << std::fixed << std::setprecision(3) << total_elapsed << " ms" << std::endl;
        std::cout << "  Avg per iter:   " << total_elapsed / completed_iterations << " ms" << std::endl;
        std::cout << "  Min/Max:        " << metrics.get_min_iteration_ms() << " / " 
                  << metrics.get_max_iteration_ms() << " ms" << std::endl;
        std::cout << "  Std dev:        " << metrics.get_stddev_iteration_ms() << " ms" << std::endl;
        std::cout << "  Per PSP:        " << std::setprecision(2) << (total_elapsed * 1000.0) / metrics.total_psps_processed << " µs" << std::endl;
        std::cout << "  Throughput:     " << std::setprecision(0) << (metrics.total_psps_processed * 1000.0) / total_elapsed << " PSP/s" << std::endl;
    }

    // Sync output buffers from device
    if (!quiet_mode) std::cout << "\nSyncing results from device..." << std::endl;
    auto d2h_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_TRUNKS; i++) {
        output_bo[i].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    }
    auto d2h_end = std::chrono::high_resolution_clock::now();
    double d2h_time = std::chrono::duration_cast<std::chrono::microseconds>(d2h_end - d2h_start).count() / 1000.0;
    metrics.d2h_transfer_times_ms.push_back(d2h_time);

    // Save and display results (only from last iteration)
    if (!quiet_mode && !power_mode) {
        std::cout << "\nResults (ME² values) - Last Iteration:" << std::endl;
        std::cout << "---------------------------------------------" << std::endl;
        
        for (int trunk = 0; trunk < NUM_TRUNKS; trunk++) {
            std::cout << "Trunk " << trunk << ":" << std::endl;
            for (int psp = 0; psp < psps_to_process; psp++) {
                std::cout << "  PSP " << psp << ": " << std::scientific << std::setprecision(10)
                          << output_host[trunk][psp] << std::endl;
            }
            
            // Save to file
            std::string result_file = data_dir + "/me2_out_" + std::to_string(trunk) + ".txt";
            save_results(result_file, output_host[trunk], psps_to_process);
        }
    }

    std::cout << "\n=============================================================================" << std::endl;
    std::cout << " Execution Summary" << std::endl;
    std::cout << "=============================================================================" << std::endl;
    std::cout << " PSPs/iteration:    " << NUM_TRUNKS * psps_to_process << " (" << NUM_TRUNKS 
              << " trunks × " << psps_to_process << " PSPs)" << std::endl;
    std::cout << " Iterations:        " << num_iterations << std::endl;
    std::cout << " Total PSPs:        " << total_psps_processed << std::endl;
    std::cout << " Total time:        " << total_elapsed << " ms" << std::endl;
    std::cout << " Avg time/iter:     " << total_elapsed / num_iterations << " ms" << std::endl;
    std::cout << " Throughput:        " << (total_psps_processed * 1000.0) / total_elapsed << " PSP/s" << std::endl;
    std::cout << " Latency (avg):     " << (total_elapsed * 1000.0) / total_psps_processed << " µs/PSP" << std::endl;
    std::cout << "=============================================================================" << std::endl;

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    try {
        int result = run(argc, argv);
        std::cout << "\nTEST " << (result ? "FAILED" : "PASSED") << std::endl;
        return result;
    } catch (std::exception const& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        std::cout << "FAILED TEST" << std::endl;
        return EXIT_FAILURE;
    }
}
