# 5K Implementation - GG→TTG AIE Pipeline

This directory contains the **working 5-kernel cascade pipeline** for the gg→tt̄g matrix element calculation on Versal AIE.

## Architecture

5-kernel token cascade pipeline (K1→K2a→K2b→K3→K4):
- **K1 (WFGEN)**: External WFs + VVV1P0_1 + FFV1P0_3
- **K2a (FF_DIAG_A)**: FFV1_1, FFV1_2, FFV1_0 (diagrams D2-D7)
- **K2b (FF_DIAG_B)**: FFV1_0 (diagrams D8-D14)
- **K3 (VVV_AMP)**: VVV1_0 computation
- **K4 (VVVV_COLOR)**: VVVV contact + color reduction

## Key Files

### Active Implementation
- `src/ggttg_graph_5k_split.h` - Main graph definition
- `src/graph_5k_split.cpp` - Graph instantiation
- `src/kernel_k1_wfgen.cpp` - K1 kernel
- `src/kernel_k2a_ff_diag.cpp` - K2a kernel
- `src/kernel_k2b_ff_diag.cpp` - K2b kernel
- `src/kernel_k3_vvv_amp.cpp` - K3 kernel
- `src/kernel_k4_vvvv_color.cpp` - K4 kernel (with optimizations)

### Configuration
- `src/ggttg_config.h` - PLIO widths, batch size
- `src/ggttg_params.h` - Physics constants (including color optimization constants)
- `src/kernels_4k_token.h` - Token read/write helpers

### HELAS Functions
- `src/helas/` - High Energy Physics amplitude library

## K4 Optimizations

The K4 kernel includes the following performance optimizations:
1. **Pre-normalized color constants**: `INV_COLOR_DENOM`, `COLOR_DIAG_NORM[6]`, `COLOR_OFFDIAG_NORM[15]`
2. **Fully unrolled 6×6 color reduction**: Eliminates loops, explicit 21 terms
3. **Scalar jamp variables**: `j0-j5` instead of `jamp[6]` array
4. **Removed fdiv operations**: Multiply by `INV_COLOR_DENOM` instead of divide by 9.0

## Build Instructions

```bash
cd build/hw
make clean
make aiesim  # AIE simulator
# or
make x86sim  # Fast x86 simulation (functional verification)
```

> **If the cycle-accurate `aiesim` appears to hang / never stops:** this is a known
> PLIO input-starvation deadlock caused by a build recipe overwriting the
> auto-generated `scsim_config.json` and dropping its `libpl_sender`/`libpl_receiver`
> drivers. See [docs/troubleshooting/AIESIM_PLIO_DEADLOCK_FIX.md](../docs/troubleshooting/AIESIM_PLIO_DEADLOCK_FIX.md).
> The fix helper is `scripts/gen_scsim_config.py`.

## Verified Output

- **x86sim**: 0.00020931918698 (reference)
- **AIEsim**: 2.093191870e-04 (matches! ✓)

## Reproducing the precision studies

The AIE float32 pipeline is validated against an fp64 MadGraph golden. Two
studies are reproducible from this directory: the **bulk** 1000-point flat-RAMBO
comparison and the **extreme-region** soft/collinear stress test (796 points).
See `docs/validation/PRECISION_ANALYSIS.md` for the numbers and interpretation.

### 1. Generate the fp64 golden reference

The golden generator source is `scripts/generate_test_data.cpp`. It is **not
self-contained**: it must be built inside a MadGraph5 standalone C++ export of
`g g > t t~ g`, which supplies `CPPProcess.{h,cc}`, `rambo.{h,cc}` and
`libmodel_sm.a`.

```bash
# 1) Export the standalone process with MadGraph5 (once):
#      mg5_aMC
#      > generate g g > t t~ g
#      > output standalone_cpp ggttg_sa
# 2) Drop the generator into the process directory and build it:
cp scripts/generate_test_data.cpp \
   ggttg_sa/SubProcesses/P1_Sigma_sm_gg_ttxg/
cd ggttg_sa/SubProcesses/P1_Sigma_sm_gg_ttxg/
make generate_test_data
# 3) Generate a large flat-RAMBO pool at sqrt(s)=1500 GeV:
./generate_test_data -n 200000 -e 1500 -s 20260703 -o pool
#   -> pool_momenta.txt  (E,px,py,pz for g1,g2,t,tbar,g3)
#   -> pool_results.txt  ("evt  ME2  weight")
#   -> pool_momenta.bin
```

Particle order is `0=g1(+z), 1=g2(-z), 2=t, 3=tbar, 4=g3`. CLI options:
`-n <events> -s <seed> -e <energyGeV> -o <prefix>`.

### 2. Select the extreme (soft/collinear) points

```bash
python scripts/select_extreme_e5.py \
    --prefix pool --per_region 400 --out data/extreme796
#   -> data/extreme796_momenta.txt   (renumbered MadGraph momenta)
#   -> data/extreme796_golden.txt    (index me2 weight, fp64)
#   -> data/extreme796_labels.txt    (index region observable)
```

Pre-generated copies of these files are already committed under `data/`
(`extreme796_momenta.txt`, `expected_me2_extreme796.txt`, `extreme796_labels.txt`).

### 3. Convert momenta to the PLIO input stream

The AIE input is one float per line, 20 floats per phase-space point,
PSP-major `E,px,py,pz` for `g1,g2,t,tbar,g3`:

```bash
awk '!/^#/ && NF==5 {print $2; print $3; print $4; print $5}' \
    data/extreme796_momenta.txt > data/psp_in_extreme796_fp32.txt
```

### 4. Run the x86 simulation over all points

`src/graph_5k_split.cpp` runs `GGTTG_NUM_ITER` iterations (default 1). Point the
graph at the PLIO input and set the iteration count to the number of points:

```bash
# copy the extreme input to the graph's expected input path, then:
make x86run GRAPH_IMPL=5k_split CXXFLAGS="-DGGTTG_NUM_ITER=796"
#   -> x86simulator_output/data/me2_out_0.txt  (one fp32 ME2 per line)
```

### 5. Compare against the golden

```bash
# Extreme-region, per-region breakdown:
python scripts/compare_extreme_e5.py \
    --fp32   data/aie_me2_extreme796_fp32.txt \
    --golden data/expected_me2_extreme796.txt \
    --labels data/extreme796_labels.txt

# Bulk 1000-point study:
python scripts/precision_compare_1000.py
```

## Directory Structure

- `alternatives/` - Alternative graph configurations (multilane, max_throughput)
- `archive/` - Old kernel versions (baseline, K1 v1-v3, legacy K2)
- `hslice_experimental/` - Helicity-parallel experimental design
- `build/` - Build outputs (hw, x86sim)
- `data/` - Input/output data files
- `src/` - Active source code

## Related Directories

- `../daisy_chain_impl/` - Daisy chain architecture (separate design)
- `../k4_test/` - K4 standalone test bench
- `../cascade_loop_test/` - Cascade deadlock investigation
