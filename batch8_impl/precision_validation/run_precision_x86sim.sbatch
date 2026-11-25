#!/bin/bash
# ============================================================================
# run_precision_x86sim.sbatch
#
# Precision of the 8-kernel batched graph against the MG5_aMC v3.5.6
# standalone_cpp fp64 reference sets in ../../reference_fp64/data/
# (1000 bulk + 400 soft + 396 collinear points).
#
# NLANES=1, functional x86 simulation (numerical result only, no timing).
# convert_reference.py concatenates the three sets into one 225-invocation
# input stream; parse_precision_results.py splits the output back and scores
# each set (mean/max absolute and relative error, tail percentiles).
#
# Usage: sbatch run_precision_x86sim.sbatch
# ============================================================================
#SBATCH --job-name=aie_precision_x86sim
#SBATCH --partition=infinite
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=32G
#SBATCH --output=slurm_precision_x86sim_%j.log

set -eo pipefail

# ---- SITE-SPECIFIC: adjust the three lines below for your cluster ----
VITIS_ROOT=/tools/Xilinx/Vitis/2024.1
PLATFORM_XPFM="${VITIS_ROOT}/base_platforms/xilinx_vck190_base_202410_1/xilinx_vck190_base_202410_1.xpfm"
export XILINXD_LICENSE_FILE=<port>@<your-licence-server>
export LM_LICENSE_FILE=<port>@<your-licence-server>

cd "${SLURM_SUBMIT_DIR:?Submit with sbatch}"

set +u
source "${VITIS_ROOT}/settings64.sh"
set -u
export PATH="${SLURM_SUBMIT_DIR}/../bin_shim:${PATH}:/usr/bin:/bin"

which aiecompiler x86simulator python3 zip
aiecompiler --version 2>&1 | head -3

echo "=== 1. convert reference sets to PLIO input format ==="
python3 scripts/convert_reference.py \
  --src-dir ../../reference_fp64/data \
  --out-dir data \
  --lane 0

NUM_ITER=$(python3 -c "import json; print(json.load(open('data/manifest.json'))['num_iter'])")
echo "NUM_ITER (from manifest) = ${NUM_ITER}"

echo "=== 2. aiecompiler --target=x86sim (functional, NLANES=1) ==="
WORKDIR=Work_x86sim_precision
time aiecompiler --target=x86sim \
  --platform="${PLATFORM_XPFM}" \
  --include=../src \
  --workdir="${WORKDIR}" \
  --Xpreproc=-DGGTTG_NUM_ITER=${NUM_ITER} \
  --Xpreproc=-DGGTTG_NLANES=1 \
  --Xpreproc=-DGGTTG_NO_HELAS_DEBUG=1 \
  ../src/graph_batch8_grid.cpp

echo "=== 3. x86simulator (functional run) ==="
time x86simulator --pkg-dir="${WORKDIR}"

echo "=== 4. locate output (default path varies by tool version) ==="
OUT_FILE=""
for cand in \
  "x86simulator_output/data/out_0.txt" \
  "aiesimulator_output/data/out_0.txt" \
  "data/out_0.txt" \
  "${WORKDIR}/x86simulator_output/data/out_0.txt"
do
  if [ -s "$cand" ]; then OUT_FILE="$cand"; break; fi
done

if [ -z "$OUT_FILE" ]; then
  echo "FATAL: could not find out_0.txt in any expected location. Listing tree:"
  find . -maxdepth 4 -iname "out_0.txt" -o -iname "out_*.txt" 2>/dev/null
  find . -maxdepth 2 -type d
  exit 1
fi
echo "found output: ${OUT_FILE} ($(wc -l < "$OUT_FILE") lines)"

echo "=== 5. score against fp64 reference ==="
python3 scripts/parse_precision_results.py \
  --out-file "${OUT_FILE}" \
  --manifest data/manifest.json \
  --ref-dir ../../reference_fp64/data \
  --summary precision_summary.json

echo "=== SUMMARY ==="
cat precision_summary.json

echo "=== done ==="
