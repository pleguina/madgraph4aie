# Event-batched cascade pipeline for gg → tt̄g matrix elements on Versal AI Engines

AI Engine implementation of the leading-order QCD matrix element |M|² for `g g > t t~ g`
on the AMD Versal VC1902 (VCK190): 50 independent lanes × 8 cascaded AI Engine tiles,
using all 400 tiles of the array. Each SIMD register holds the same quantity for 8
different phase-space points, so every kernel invocation evaluates 8 events. Each lane has
its own PLIO input/output pair; there is no packet switching.

## Layout

```
batch8_impl/
├── src/                     AIE kernels and graph
│   ├── graph_batch8_grid.h/.cpp   50-lane grid, serpentine tile placement (tile_of())
│   ├── kernel_*_batch8.cpp        the 8 pipeline stages (see table below)
│   ├── helas_batch8.h             8-event batched HELAS routines
│   ├── token_batch8.h             cascade token encoding between stages
│   ├── helas/processing_core_common.h, ggttg_config.h, ggttg_params.h
│   │                              shared types, couplings, masses, helicity table
├── sys_build/               system build for VCK190
│   ├── src/mm2s*.cpp, s2mm*.cpp   HLS data movers DDR <-> AIE (AXI4-Stream)
│   ├── system.cfg                 v++ link connectivity (100 stream_connect)
│   ├── run_sys_impl.sbatch        HLS + v++ link + Vivado implementation
│   └── report_power_util.tcl      utilisation and power from the routed checkpoint
├── slurm/run_fullgrid_stats.sbatch  50-lane hw compile, PM check, cycle-accurate simulation
├── precision_validation/    single-lane x86sim run against the fp64 reference
├── scripts/parse_full_grid_trace.py  II / throughput / lane spread from PLIO traces
├── data/psp_in_0.txt        input batch for the throughput run (8 PSPs × 6 invocations)
└── bin_shim/zip             zip replacement for compute nodes without one
reference_fp64/              MG5_aMC v3.5.6 standalone_cpp fp64 reference generator and data
```

## Pipeline stages

| Stage | Kernel | Work |
|---|---|---|
| 1 | `kernel_k1a_wfgen_batch8` | 5 external wavefunctions w0-w4 |
| 2 | `kernel_k1b_vvvprop_batch8` | 3 VVV1P0_1 propagators (w5_01, w10, w5_04) |
| 3 | `kernel_k2a_propA_batch8` | FFV1_1 propagators (w7, w5_20, w11) |
| 4 | `kernel_k2a_propB_batch8` | FFV1_2 propagators (w8, w9, w5_30) |
| 5 | `kernel_k2a_amps_batch8` | diagrams D2-D4 (FFV1_0) |
| 6 | `kernel_k2b_batch8` | diagrams D5-D11, D13, D14 (FFV1_0) |
| 7 | `kernel_k3_batch8` | w6 (FFV1P0_3), diagrams D1, D12, D15 (VVV1_0) |
| 8 | `kernel_k4color_batch8` | diagram D16 (VVVV), colour sum, helicity sum → \|M\|² |

The split follows the 16 KB program-memory budget of each tile. Each stage loops over the
32 helicity combinations and forwards one cascade token per helicity (wavefunctions still
needed downstream plus the partial colour amplitudes); the token layouts are in
`token_batch8.h`.

## Tile placement

The array is treated as one serpentine cascade chain of 400 tiles: even rows run from low
to high column, odd rows from high to low, and the chain continues vertically at the array
edge. Stage `s` of lane `l` sits at chain position `8*l + s`. The row direction is fixed
by the silicon (a chain declared against it is rejected with `AIE-PRE-MAPPER-46`). Lanes
6, 12, 18, 31, 37 and 43 span two rows through the vertical cascade link; the other 44
lanes stay within one row.

`WFBatch8 { v8c c[8]; }` has eight slots but a HELAS wavefunction has six components;
`c[6]` and `c[7]` are zeroed by every producer and are never sent on the cascade.

## Building

Tools: Vitis/Vivado **2024.2** with `xilinx_vck190_base_202420_1` for the hw compile,
simulation and system build. Vitis 2025.2 mis-compiles the cascade writes of this design.

All three jobs are Slurm scripts. Edit the `SITE-SPECIFIC` block at the top of each
(Vitis root, platform `.xpfm`, licence server) and submit from the script's directory.

```bash
# 1. 50-lane hw compile + PM budget check + cycle-accurate simulation
#    (compile ~13 min, simulation ~8 h on 8 cores)
cd batch8_impl/slurm && sbatch run_fullgrid_stats.sbatch
#    -> Work_hw_N50/, libadf.a, aiesimulator_output/, Work_hw_N50/full_grid_summary.json

# 2. system implementation (copy libadf.a from step 1 to batch8_impl/libadf.a; ~30 min)
cd batch8_impl/sys_build && sbatch run_sys_impl.sbatch
#    -> ggttg_batch8.xsa, reports/{utilization,timing_summary,power}.rpt

# 3. precision against reference_fp64/data (single lane, x86sim, ~50 min)
cd batch8_impl/precision_validation && sbatch run_precision_x86sim.sbatch
#    -> precision_summary.json
```

Compile-time switches: `GGTTG_NLANES` (lanes, max 50), `GGTTG_NUM_ITER` (graph
iterations), `GGTTG_NO_HELAS_DEBUG` (disables debug printing).

## Measured figures (Vitis/Vivado 2024.2 unless noted)

| Quantity | Value |
|---|---|
| AIE tiles used | 400 / 400, every tile within the 16 KB program-memory budget |
| Initiation interval per lane | 163.63 µs per 8 events, lane-to-lane spread 0.014 % |
| Full-array throughput | 2.44 × 10⁶ ME/s (aiesimulator, cycle-accurate, all 50 lanes) |
| First-invocation latency | 194.5 µs |
| PL utilisation | 31 846 LUT (3.5 %), 30 BRAM, 14 NMU512, PL clock 312.5 MHz |
| Timing | met, WNS +0.096 ns |
| Power (vectorless estimate) | AI Engine 62.2 W, total on-chip 89.9 W |
| Precision vs MG5 fp64 (Vitis 2024.1, x86sim) | 1.3 ppm mean, 111 ppm max relative error |

The outputs of all 50 lanes are bit-identical for identical inputs.
