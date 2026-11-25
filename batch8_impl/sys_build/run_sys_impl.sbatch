#!/bin/bash
# ============================================================================
# run_sys_impl.sbatch
#
# System-level (PL + NoC + AIE) implementation of the 8-kernel batched grid:
# v++ link, Vivado synthesis + implementation, then power by domain
# (including the AI Engine domain) and PL resource utilisation.
#
# TOPOLOGY: 50 dedicated PLIO input/output pairs, NO packet switching -- the
# graph is used exactly as simulated by ../slurm/run_fullgrid_stats.sbatch.
# Each lane keeps its own physically separate AXI4-Stream. Six mm2s / six s2mm
# instances (eight streams each) serve lanes 0..47 and one mm2s2 / one s2mm2
# instance (two streams each) serves lanes 48 and 49; they exist only to share
# the DDR access ports and never multiplex the streams themselves.
#
# Reuses the libadf.a of the NLANES=50 hw compile rather than recompiling the
# AIE graph. The graph iteration count does not affect placement, routing,
# utilisation or power.
#
# Reference run (Vitis/Vivado 2024.2, 8 jobs): HLS compile of the four kernels
# ~45 s each, v++ hw link (synthesis + implementation) 27 min.
#
# Usage: sbatch run_sys_impl.sbatch
# ============================================================================
#SBATCH --job-name=aie_sys_impl
#SBATCH --partition=infinite
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=64G
#SBATCH --output=slurm_sys_impl_%j.log

set -eo pipefail

# ---- SITE-SPECIFIC: adjust the three lines below for your cluster ----
VITIS_ROOT=/tools/Xilinx/Vitis/2024.2
VIVADO_ROOT=/tools/Xilinx/Vivado/2024.2
PLATFORM_XPFM="${VITIS_ROOT}/base_platforms/xilinx_vck190_base_202420_1/xilinx_vck190_base_202420_1.xpfm"
export XILINXD_LICENSE_FILE=<port>@<your-licence-server>

cd "${SLURM_SUBMIT_DIR:?Submit with sbatch}"

set +u
source "${VITIS_ROOT}/settings64.sh"
source "${VIVADO_ROOT}/settings64.sh"
set -u
# bin_shim supplies a `zip` replacement; compute nodes have no system zip.
export PATH="${SLURM_SUBMIT_DIR}/../bin_shim:${PATH}:/usr/bin:/bin"

which v++ vivado zip
v++ --version 2>&1 | head -3 || true

NPROC=${SLURM_CPUS_PER_TASK:-16}
LIBADF=../libadf.a   # produced by ../slurm/run_fullgrid_stats.sbatch (NLANES=50 hw compile)
test -f "${LIBADF}" || { echo "FATAL: ${LIBADF} missing"; exit 1; }
echo "using AIE archive: $(ls -la ${LIBADF})"

# --- 1. HLS kernels -> .xo --------------------------------------------------
for K in mm2s s2mm mm2s2 s2mm2; do
  echo "=== v++ -c ${K} ==="
  time v++ -c -t hw \
    --platform "${PLATFORM_XPFM}" \
    -k ${K} \
    --temp_dir _x \
    --save-temps \
    --clock.default_freqhz 312500000 \
    -o ${K}.xo src/${K}.cpp
done
ls -la mm2s.xo s2mm.xo mm2s2.xo s2mm2.xo

# --- 2. v++ hw link (Vivado synthesis + implementation) ---------------------
echo "=== v++ --link (hw) : 14 PL kernels + 400-tile AIE graph ==="
time v++ --link -t hw \
  --platform "${PLATFORM_XPFM}" \
  --config system.cfg \
  --clock.default_freqhz 312500000 \
  --temp_dir _x \
  --save-temps \
  --report_level 2 \
  --vivado.synth.jobs ${NPROC} \
  --vivado.impl.jobs ${NPROC} \
  -o ggttg_batch8.xsa \
  mm2s.xo s2mm.xo mm2s2.xo s2mm2.xo "${LIBADF}"

echo "=== link artifacts ==="
ls -la ggttg_batch8.xsa || true
# Disposable listing of the routed checkpoint and reports; never fatal.
find _x -name "*routed.dcp" -o -name "*.rpt" 2>/dev/null > /tmp/rpt_list.$$.txt || true
head -40 /tmp/rpt_list.$$.txt || true
rm -f /tmp/rpt_list.$$.txt

# --- 3. Power + utilisation from the routed checkpoint ----------------------
echo "=== report_power / report_utilization ==="
REPORTS=reports
vivado -mode batch -nojournal -nolog \
  -source report_power_util.tcl -tclargs "${REPORTS}"

echo "=== SUMMARY ==="
if [ -f "${REPORTS}/power_summary.txt" ]; then cat "${REPORTS}/power_summary.txt"; fi
echo "--- utilisation head ---"
sed -n '1,60p' "${REPORTS}/utilization.rpt" 2>/dev/null || true
echo "--- power head ---"
sed -n '1,80p' "${REPORTS}/power.rpt" 2>/dev/null || true

echo "=== done ==="
