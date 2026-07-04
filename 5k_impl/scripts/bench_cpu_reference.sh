#!/usr/bin/env bash
# =============================================================================
# bench_cpu_reference.sh
#
# Reproduces the CPU rows of the paper's cross-platform comparison table
# (throughput in ME/s + RAPL package energy => power and energy-per-ME) for the
# gg -> t t~ g matrix element, on the SAME class of host used in the paper
# (Intel Core i5-10600, AVX2, no AVX-512).
#
# It benchmarks four configurations of the MadGraph5 CUDACPP standalone:
#     - fp64 scalar, 1 core   (build.none_d_inl0_hrd0)
#     - fp32 scalar, 1 core   (build.none_f_inl0_hrd0)
#     - fp32 AVX2,   1 core   (build.avx2_f_inl0_hrd0)
#     - fp32 AVX2,   6 cores  (6 concurrent pinned single-thread procs)
#
# Energy is measured with Linux perf RAPL counters (power/energy-pkg/), which is
# a package(socket)-wide domain -- the same domain the paper's fp64 baseline row
# uses. E/ME = avg_package_power / EvtsPerSec[MECalcOnly].
#
# NOTE on the RAPL caveat: package energy is socket-wide, so a single active core
# still pays the full ~13-14 W package/uncore static power. Single-core E/ME is
# therefore inflated by idle-core overhead; the fully-loaded 6-core figure
# amortises this and is the representative vectorised-CPU energy point.
#
# -----------------------------------------------------------------------------
# PREREQUISITES
#   1. Linux `perf` with RAPL support:  perf list | grep energy-pkg
#      (may need: sudo sysctl kernel.perf_event_paranoid=0)
#   2. A CUDACPP gg_ttg.sa standalone checkout, built for the three backends.
#      Obtain and build it (public, reproducible; this is [Valassi:2025xfn]):
#         git clone https://github.com/madgraph5/madgraph4gpu.git
#         cd madgraph4gpu/epochX/cudacpp/gg_ttg.sa/SubProcesses/P1_Sigma_sm_gg_ttxg
#         make -f ../cudacpp.mk BACKEND=cppnone FPTYPE=d USEBUILDDIR=1
#         make -f ../cudacpp.mk BACKEND=cppnone FPTYPE=f USEBUILDDIR=1
#         make -f ../cudacpp.mk BACKEND=cppavx2 FPTYPE=f USEBUILDDIR=1
#
# USAGE
#   PROC_DIR=/path/to/P1_Sigma_sm_gg_ttxg ./bench_cpu_reference.sh
#   (or)  ./bench_cpu_reference.sh /path/to/P1_Sigma_sm_gg_ttxg
#
# ENV OVERRIDES
#   PROC_DIR   process dir containing build.<backend>_inl0_hrd0/check_cpp.exe
#   CORES      number of physical cores for the multi-core run (default 6)
#   PIN        first core id for single-core pinning (default 2)
#   GRID       cudacpp grid "blk thr iter" (default "2560 256 1" = 655360 evts)
# =============================================================================
set -euo pipefail

PROC_DIR="${PROC_DIR:-${1:-}}"
CORES="${CORES:-6}"
PIN="${PIN:-2}"
GRID="${GRID:-2560 256 1}"

if [[ -z "$PROC_DIR" || ! -d "$PROC_DIR" ]]; then
  echo "ERROR: set PROC_DIR to the CUDACPP P1_Sigma_sm_gg_ttxg directory." >&2
  echo "       See PREREQUISITES in the header of this script." >&2
  exit 1
fi

if ! perf list 2>/dev/null | grep 'energy-pkg' >/dev/null; then
  echo "ERROR: perf RAPL event power/energy-pkg/ not available." >&2
  echo "       Try: sudo sysctl kernel.perf_event_paranoid=0" >&2
  exit 1
fi

exe_for() { echo "$PROC_DIR/build.${1}_inl0_hrd0/check_cpp.exe"; }

extract_thr()  { grep 'EvtsPerSec\[MECalcOnly' "$1" | grep -oE '[0-9.]+e\+[0-9]+' | head -1; }
extract_pkg()  { grep 'energy-pkg' "$1" | grep -oE '[0-9]+[.,][0-9]+' | head -1 | tr ',' '.'; }
extract_wall() { grep 'seconds time elapsed' "$1" | grep -oE '[0-9]+[.,][0-9]+' | head -1 | tr ',' '.'; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

printf "%-26s %14s %10s %8s %10s\n" "config" "ME/s(MECalc)" "power(W)" "wall(s)" "E/ME(uJ)"
printf "%-26s %14s %10s %8s %10s\n" "--------------------------" "--------------" "----------" "--------" "----------"

run_single() {
  local cfg="$1" label="$2" exe out perf thr pkg wall
  exe="$(exe_for "$cfg")"
  [[ -x "$exe" ]] || { echo "SKIP $label: missing $exe" >&2; return; }
  out="$TMP/out_$cfg.txt"; perf="$TMP/perf_$cfg.txt"
  OMP_NUM_THREADS=1 taskset -c "$PIN" \
     perf stat -e power/energy-pkg/,power/energy-ram/ \
     "$exe" -p $GRID 1>"$out" 2>"$perf"
  thr="$(extract_thr "$out")"; pkg="$(extract_pkg "$perf")"; wall="$(extract_wall "$perf")"
  awk -v l="$label" -v t="$thr" -v p="$pkg" -v w="$wall" \
      'BEGIN{pw=p/w; printf "%-26s %14s %10.2f %8.2f %10.1f\n", l, t, pw, w, 1e6*pw/t}'
}

run_single none_d "i5 fp64 scalar 1c"
run_single none_f "i5 fp32 scalar 1c"
run_single avx2_f "i5 fp32 AVX2   1c"

# ---- multi-core AVX2: N concurrent pinned single-thread procs, one RAPL window
exe="$(exe_for avx2_f)"
if [[ -x "$exe" ]]; then
  perf="$TMP/perf_avx2_${CORES}c.txt"
  perf stat -e power/energy-pkg/,power/energy-ram/ bash -c '
    for c in $(seq 0 '"$((CORES-1))"'); do
      OMP_NUM_THREADS=1 taskset -c "$c" "'"$exe"'" -p '"$GRID"' \
         1>"'"$TMP"'/out_mc_$c.txt" 2>/dev/null &
    done
    wait' 2>"$perf"
  tot=0
  for c in $(seq 0 $((CORES-1))); do
    t="$(extract_thr "$TMP/out_mc_$c.txt")"
    tot="$(awk -v a="$tot" -v b="$t" 'BEGIN{print a+b}')"
  done
  pkg="$(extract_pkg "$perf")"; wall="$(extract_wall "$perf")"
  awk -v l="i5 fp32 AVX2 ${CORES}c" -v t="$tot" -v p="$pkg" -v w="$wall" \
      'BEGIN{pw=p/w; printf "%-26s %14s %10.2f %8.2f %10.1f\n", l, sprintf("%.2e",t), pw, w, 1e6*pw/t}'
fi

echo
echo "Note: E/ME = avg package power / MECalcOnly throughput (matches the paper's"
echo "      fp64 baseline convention). RAPL package energy is socket-wide; the"
echo "      ${CORES}-core row is the representative vectorised-CPU energy point."
