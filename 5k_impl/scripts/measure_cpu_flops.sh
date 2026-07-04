#!/usr/bin/env bash
# =============================================================================
# measure_cpu_flops.sh
#
# Measures the floating-point operation count per gg -> t t~ g matrix element
# (the paper's "Arithmetic Cost per Matrix Element", ~1.85e5 FLOP/ME) using
# Linux perf hardware counters (fp_arith_inst_retired.*) on an Intel host.
#
# Method (differential, cancels one-time set-up):
#   - flops_probe.cpp loops sigmaKin() N times on a FIXED phase-space point
#     (RAMBO called once, outside the loop).
#   - Run it for N and 10*N iterations under perf; the difference in retired FP
#     instructions divided by (10N - N) is the pure per-ME operation count.
#   - The probe is built SCALAR, no-FMA, no-vectorise, so each retired FP
#     instruction is exactly one operation (no FMA-doubling / SIMD packing).
#     Only two events are non-zero for this build:
#       fp_arith_inst_retired.scalar_double      (1 op each)
#       fp_arith_inst_retired.128b_packed_double (2 ops each => x2)
#
# -----------------------------------------------------------------------------
# PREREQUISITES
#   1. Linux perf with fp_arith_inst_retired.* (Intel Core, e.g. i5-10600):
#        perf list | grep fp_arith_inst_retired
#      (may need: sudo sysctl kernel.perf_event_paranoid=0)
#   2. A MadGraph5 standalone export of "g g > t t~ g":
#        mg5_aMC
#          > generate g g > t t~ g
#          > output standalone_cpp ggttg_sa
#      then copy flops_probe.cpp (this directory) into
#        ggttg_sa/SubProcesses/P1_Sigma_sm_gg_ttxg/
#
# USAGE
#   PROC_DIR=/path/to/ggttg_sa/SubProcesses/P1_Sigma_sm_gg_ttxg ./measure_cpu_flops.sh
#   (or)  ./measure_cpu_flops.sh /path/to/.../P1_Sigma_sm_gg_ttxg
#
# ENV OVERRIDES
#   PROC_DIR  standalone process dir (contains CPPProcess.*, rambo.*, ../../lib)
#   N         base iteration count (default 200000; also runs 10*N)
#   PIN       core to pin to (default 2)
# =============================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROC_DIR="${PROC_DIR:-${1:-}}"
N="${N:-200000}"
PIN="${PIN:-2}"
N10=$((N * 10))

if [[ -z "$PROC_DIR" || ! -d "$PROC_DIR" ]]; then
  echo "ERROR: set PROC_DIR to the standalone P1_Sigma_sm_gg_ttxg directory." >&2
  echo "       See PREREQUISITES in the header of this script." >&2
  exit 1
fi
if ! perf list 2>/dev/null | grep 'fp_arith_inst_retired' >/dev/null; then
  echo "ERROR: perf fp_arith_inst_retired.* not available on this CPU." >&2
  echo "       Try: sudo sysctl kernel.perf_event_paranoid=0" >&2
  exit 1
fi

cp -f "$HERE/flops_probe.cpp" "$PROC_DIR/flops_probe.cpp"
cd "$PROC_DIR"

# Scalar, no-FMA, no-vectorise build => 1 retired FP instruction == 1 operation.
FLAGS="-O2 -fno-tree-vectorize -fno-tree-slp-vectorize -mno-fma -mno-avx -mno-avx2 -ffp-contract=off -I../../src -I."
echo ">> building scalar/no-FMA probe ..."
g++ $FLAGS -c flops_probe.cpp -o flops_probe.o
g++ flops_probe.o CPPProcess.o -L../../lib -lmodel_sm -o flops_probe 2>/dev/null \
  || g++ $FLAGS flops_probe.cpp CPPProcess.cc rambo.cc read_slha.cc \
         Parameters_sm.cc -L../../lib -lmodel_sm -o flops_probe

EVENTS="fp_arith_inst_retired.scalar_double,fp_arith_inst_retired.128b_packed_double"

run() {  # $1 = iterations -> prints "scalar128 <scalar_double> <128b_packed_double>"
  local it="$1" pf; pf="$(mktemp)"
  taskset -c "$PIN" perf stat -e "$EVENTS" ./flops_probe "$it" 1>"$pf.out" 2>"$pf"
  grep -m1 'ME=' "$pf.out" >&2
  local sd pd
  sd="$(grep 'scalar_double'      "$pf" | grep -oE '[0-9][0-9.,]*' | head -1 | tr -d '.,')"
  pd="$(grep '128b_packed_double' "$pf" | grep -oE '[0-9][0-9.,]*' | head -1 | tr -d '.,')"
  echo "$sd $pd"
  rm -f "$pf" "$pf.out"
}

echo ">> N=$N ..."      ; read s1 p1 < <(run "$N")
echo ">> 10N=$N10 ..."  ; read s2 p2 < <(run "$N10")

awk -v s1="$s1" -v p1="$p1" -v s2="$s2" -v p2="$p2" -v n="$N" -v n10="$N10" 'BEGIN{
  dn = n10 - n;
  sd = (s2 - s1)/dn;          # scalar_double ops per ME
  pd = 2.0*(p2 - p1)/dn;      # 128b packed double = 2 ops each
  tot = sd + pd;
  printf "\n=== FLOP per matrix element ===\n";
  printf "  scalar_double      : %.1f FLOP/ME\n", sd;
  printf "  128b_packed_double : %.1f FLOP/ME (x2)\n", pd;
  printf "  TOTAL              : %.0f FLOP/ME  (%.1f%% scalar)\n", tot, 100.0*sd/tot;
}'

# housekeeping
rm -f "$PROC_DIR/flops_probe.o" "$PROC_DIR/flops_probe" "$PROC_DIR/flops_probe.cpp"
echo
echo "Reference: ~1.85e5 FLOP/ME (98% scalar) on i5-10600; the printed ME above"
echo "should reproduce the fp64 golden 2.0931907741e-04 GeV^-2 at the ref point."
