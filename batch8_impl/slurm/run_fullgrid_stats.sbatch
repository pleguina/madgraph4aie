#!/bin/bash
# ============================================================================
# run_fullgrid_stats.sbatch
#
# Full 50-lane / 400-tile grid: hw compile, program-memory check on every
# tile, cycle-accurate aiesimulator run and II / throughput extraction.
#
# NUM_ITER=6 is the minimum parse_full_grid_trace.py needs (1 warm-up,
# 1 drain, >= 2 steady-state invocations per lane). II and throughput come
# from the PLIO output trace timestamps, which aiesimulator writes with or
# without --profile; --profile is not used (it only adds per-tile reports
# and roughly doubles the wall-clock time).
#
# Reference run (Vitis 2024.2, 8 cores): compile 13 min, simulation ~8 h to
# the last output sample.
#
# Usage: sbatch run_fullgrid_stats.sbatch
# ============================================================================
#SBATCH --job-name=aie_fullgrid_stats
#SBATCH --partition=infinite
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=48G
#SBATCH --output=slurm_fullgrid_stats_%j.log

set -eo pipefail

# ---- SITE-SPECIFIC: adjust the lines below for your cluster ----
VITIS_ROOT=/tools/Xilinx/Vitis/2024.2
PLATFORM_XPFM="${VITIS_ROOT}/base_platforms/xilinx_vck190_base_202420_1/xilinx_vck190_base_202420_1.xpfm"
export XILINXD_LICENSE_FILE=<port>@<your-licence-server>

cd "${SLURM_SUBMIT_DIR:?Submit with sbatch}"

set +u
source "${VITIS_ROOT}/settings64.sh"
set -u
export PATH="${SLURM_SUBMIT_DIR}/../bin_shim:${PATH}:/usr/bin:/bin"
which aiecompiler
aiecompiler --version 2>&1 | head -3

NLANES=50
NUM_ITER=6
WORKDIR=Work_hw_N${NLANES}

echo "=== compiling ${NLANES}-lane / $((NLANES*8))-tile grid (NUM_ITER=${NUM_ITER}) ==="
time aiecompiler --target=hw \
  --platform="${PLATFORM_XPFM}" \
  --include=../src \
  --workdir="${WORKDIR}" --stacksize=16384 --heapsize=1024 --xlopt=2 \
  --nodot-graph \
  --Xpreproc=-DGGTTG_NUM_ITER=${NUM_ITER} \
  --Xpreproc=-DGGTTG_NLANES=${NLANES} \
  --Xpreproc=-DGGTTG_NO_HELAS_DEBUG=1 \
  ../src/graph_batch8_grid.cpp

echo "=== PM budget check across all $((NLANES*8)) tiles ==="
over=0
for f in "${WORKDIR}"/aie/*/Release/*.map; do
  total=$(grep -A8 "Section summary for memory 'PM'" "$f" | grep "Total" | grep -oE '[0-9]+' | head -1)
  if [ "${total:-0}" -gt 16384 ]; then
    echo "OVER BUDGET: $f = ${total}B"
    over=$((over+1))
  fi
done
echo "tiles over budget: ${over} / $(ls -d "${WORKDIR}"/aie/*_*/ | wc -l)"

echo "=== input data: the same 8-PSP batch on every lane ==="
mkdir -p data
for i in $(seq 0 $((NLANES-1))); do cp ../data/psp_in_0.txt "data/psp_in_${i}.txt"; done

echo "=== aiesimulator, cycle-accurate, ${NLANES} lanes ==="
rm -rf aiesimulator_output
time aiesimulator --pkg-dir="${WORKDIR}" --simulation-cycle-timeout=20000000

echo "=== lanes with output ==="
for i in $(seq 0 $((NLANES-1))); do
  if [ -s "aiesimulator_output/data/out_${i}.txt" ]; then echo -n "."; else echo -n "X"; fi
done
echo ""

echo "=== II / R_chip / spread_pct from the PLIO output traces ==="
python3 ../scripts/parse_full_grid_trace.py --batch 8 \
  --glob "aiesimulator_output/data/out_*.txt" \
  --n-lanes ${NLANES} \
  --summary "${WORKDIR}/full_grid_summary.json"
cat "${WORKDIR}/full_grid_summary.json"
echo "=== done ==="
