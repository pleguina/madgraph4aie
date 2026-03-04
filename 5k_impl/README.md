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

## Verified Output

- **x86sim**: 0.00020931918698 (reference)
- **AIEsim**: 2.093191870e-04 (matches! ✓)

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
