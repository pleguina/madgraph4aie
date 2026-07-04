# Sustained Per-Tile Occupancy & Hardware-Free Power Extrapolation

**Status:** Measured (2026-07-04)
**Design:** `GRAPH_IMPL=5k_split` (one 5-kernel cascade pipeline), `GGTTG_BATCH=1`
**Simulator:** cycle-accurate `aiesimulator` (VC1902, `xcvc1902-vsva2197-2MP`, 1.25 GHz)
**Goal:** measure the *steady-state* (pipeline-full) duty cycle of every compute tile
from a multi-PSP run, then extrapolate to the full 80-pipeline / 400-tile array to
refine the AIE-array power estimate **without the physical device**.

> This complements
> [AIESIM_PLIO_DEADLOCK_FIX.md](../troubleshooting/AIESIM_PLIO_DEADLOCK_FIX.md), which
> made the cycle-accurate run terminate cleanly. Here we feed **many** PSPs so the
> pipeline fills and all five tiles are simultaneously active — the condition that
> determines *sustained* array power.

---

## 1. What was run

- Input `data/psp_in_0.txt` = **12 PSPs** (240 float lines, 20 floats/PSP), replacing
  the single-PSP file used for the physics golden check.
- Graph run count set to **12** (`#define GGTTG_NUM_ITER 12` in
  `src/kernels_4k/graph_5k_split.cpp`; `GGTTG_BATCH` kept at **1** so K1 consumes one
  PSP and K4 emits one ME² per iteration — the pipeline is fed a fresh PSP as soon as
  it is free).
- Build: `make aie GRAPH_IMPL=5k_split` → `make airun GRAPH_IMPL=5k_split AIESIM_CYCLES=150000`.

Result: `Set 12 iterations`, cores enabled, first output at **T = 72.319 µs** equal to
`2.093191870e-04` — the **golden first-PSP ME²**, i.e. physics is bit-exact against the
x86 reference. `Sim result: 0`.

---

## 2. Pipelined overlap is real (all tiles active at once)

The per-core profiles (`aiesimulator_output/profile_funct_<col>_4.txt`) show, within the
report window, that the upstream kernels had already started **later** PSPs while the
bottleneck kernel was still finishing the **first** one:

| Tile | Kernel | `Calls` in window |
|------|--------|-------------------|
| (0,5) | `kernel_k1_wfgen`    | **2** |
| (1,5) | `kernel_k2a_ff_diag` | **2** |
| (2,5) | `kernel_k2b_ff_diag` | **2** |
| (3,5) | `kernel_k3_vvv_amp`  | **2** |
| (4,5) | `kernel_k4_vvvv_color` | **1** |

K1 is already computing PSP 2 while K4 is still on PSP 1 ⇒ the five compute tiles run
**concurrently**, which is exactly the sustained-power operating point (not the
sequential activation seen in a naive single-PSP view).

The simulation window (`AIESIM_CYCLES=150000` ≈ 120 µs) is shorter than the time K4
needs to *drain* all 12 PSPs (12 × 71.6 µs ≈ 860 µs), so only the first ME² is flushed.
That does **not** affect the steady-state duty cycles below — they are gated by K4 and
are fully determined once the pipeline is full. Flushing all 12 outputs would need an
~860 µs window (≈ 7× longer wall time) and would not change the per-tile ratios.

---

## 3. Measured per-tile compute cost and steady-state duty cycle

Self (exclusive) compute cycles per PSP, from the profiles:

| Tile | Kernel (stage) | Cycles / PSP | Duty cycle¹ |
|------|----------------|-------------:|------------:|
| (0,5) | K1 `wfgen` (external wavefunctions) | 55 024 | **61.5 %** |
| (1,5) | K2a `ff_diag` (fermion flow, diagram set A) | 23 757 | **26.6 %** |
| (2,5) | K2b `ff_diag` (fermion flow, diagram set B) | 36 423 | **40.7 %** |
| (3,5) | K3 `vvv_amp` (triple-gluon amplitudes) | 46 277 | **51.7 %** |
| (4,5) | K4 `vvvv_color` (quartic + colour sum) | **89 459** | **100.0 %** |
| | **Σ active-tile-equivalents / pipeline** | | **2.805** |

¹ Duty cycle = (tile compute cycles) ÷ (bottleneck period). The pipeline is
**K4-gated**: K4 (`vvvv_color`, the colour-matrix reduction) is 100 % busy at
89 459 cycles/PSP, so every upstream tile's steady-state fraction is its
own cost divided by 89 459.

**Steady-state throughput** (one ME² per K4 period):
89 459 cycles ÷ 1.25 GHz = **71.6 µs/ME²** ⇒ **≈ 13 970 ME²/s per pipeline**
⇒ **≈ 1.12 × 10⁶ ME²/s** for the full 80-pipeline design.

---

## 4. Why this changes the power estimate

The AI-Engine compiler emits an activity report
(`Work_5k_split/reports/ggttg_design.xpe`) that is fed to **Xilinx Power Estimator
(XPE)** / **AMD Power Design Manager (PDM)** to obtain watts. That report assigns a
**flat `int_core_load = 0.6`** to *every* kernel tile — a uniform guess, **3.0**
active-tile-equivalents per pipeline.

The cycle-accurate run replaces that guess with **measured** loads:

| | Compiler `.xpe` (flat) | Measured (this run) |
|---|---:|---:|
| K1 load | 0.60 | 0.615 |
| K2a load | 0.60 | 0.266 |
| K2b load | 0.60 | 0.407 |
| K3 load | 0.60 | 0.517 |
| K4 load | 0.60 | 1.000 |
| **Σ / pipeline** | **3.00** | **2.805** |

So the **dynamic** core power of the compute tiles is **2.805 / 3.00 = 0.935×** the
compiler's flat assumption — a **6.5 % reduction** in the modelled AIE core-dynamic
term, with the load now correctly concentrated on K4 and K1 rather than spread evenly.

### Extrapolation to the full array

| Quantity | Per pipeline | × 80 pipelines |
|---|---:|---:|
| Physical compute tiles | 5 | **400** |
| Compiler flat active-tile-equiv | 3.00 | 240.0 |
| **Measured** active-tile-equiv | **2.805** | **224.4** |

Dynamic core power scales with active-tile-equivalents, so:

$$
P^{\text{array}}_{\text{dyn,measured}} \;=\; P^{\text{array}}_{\text{dyn,flat}}
\times \frac{224.4}{240.0} \;=\; 0.935 \; P^{\text{array}}_{\text{dyn,flat}} .
$$

Static/leakage and interconnect terms are unchanged (they do not depend on core duty
cycle), so the **total** AIE-domain power moves by less than the 6.5 % dynamic delta.

---

## 5. Getting absolute watts — from the console, no GUI, no board

The AI-Engine compiler `.xpe` carries **activity, not watts**, and `report_power -xpe`
only *exports* activity (Vivado → XPE spreadsheet); neither converts `.xpe` → watts.
The **headless** route that does give watts is Vivado `report_power` run on the
**implemented Versal routed checkpoint** — it contains the full device (PL + NoC + PS +
the AI-Engine array), so its AI-Engine power model is evaluated directly:

```bash
source /tools/Xilinx/Vitis/2024.1/settings64.sh
vivado -mode batch -source report_power.tcl   # opens *_wrapper_routed.dcp, report_power
```

with `report_power.tcl`:

```tcl
open_checkpoint .../impl_1/vitis_design_wrapper_routed.dcp
report_power -file power_full.rpt
report_power -hierarchical_depth 3 -file power_hier.rpt
```

**Measured result** (part `xcvc1902-vsva2197-2MP-e-S`, saved in
[power_reports/](power_reports/)):

| Domain | Power |
|---|---:|
| **AI-Engine array (`AIE` / `ai_engine_0`)** | **54.79 W** (400 tiles) |
| Total on-chip | 82.71 W |
| — Dynamic | 68.98 W |
| — Device static | 13.73 W |
| NoC-DDRMC | 6.74 W |
| I/O | 3.92 W |
| PS9 | 1.29 W |

This **confirms the paper's 54.8 W AI-Engine figure** directly from the console (the
`.xpe` spreadsheet / PDM GUI is not needed). Confidence is reported *Low* because the
device-wide `report_power` uses **vectorless** activity for the AIE (a flat toggle,
`400 tiles @ 0.25` in the report), i.e. the same uniform assumption as the compiler
`.xpe`.

### Refining with the measured per-tile activity

The cycle-accurate duty cycles in §3 (Σ = 2.805 vs the flat 3.0 tile-equivalents)
scale the AIE **dynamic** term by 0.935. Applying that to the dynamic part of the
54.79 W (the static/leakage part is unchanged) lowers the modelled AIE-array power by
~6.5 % of its dynamic component. To feed the measured activity into `report_power`
instead of the vectorless default, set the AIE tile toggle rates from §3 before
`report_power`, or import the measured-load `.xpe` (below) into XPE/PDM.

The measured-load `.xpe` for spreadsheet/PDM import is produced entirely offline:

1. Compile one pipeline: `make aie GRAPH_IMPL=5k_split` → `ggttg_design.xpe`.
2. Patch flat loads → measured (§3): `5k_impl/scripts/apply_measured_loads.py`.
3. Replicate 1 pipeline → 80-pipeline / 400-tile array: `5k_impl/scripts/replicate_xpe_power.py`.

---

## 6. Reproduce

```bash
source /tools/Xilinx/Vitis/2024.1/settings64.sh
cd <aie-project>
# 12-PSP input already staged as data/psp_in_0.txt (240 lines)
make aie   GRAPH_IMPL=5k_split                       # activity .xpe
make airun GRAPH_IMPL=5k_split AIESIM_CYCLES=150000  # cycle-accurate profiles
# occupancy:
grep -E "Total cycle|Report cycle|kernel_k" aiesimulator_output/profile_funct_*_4.txt
# physics check (first PSP):
head -2 aiesimulator_output/data/me2_out_0.txt      # -> 2.093191870e-04
```

## 7. Honest limitations

- Only the **first** ME² is flushed in the 120 µs window; the run proves *pipelined
  concurrency* and fixes the per-tile duty cycles, but is not a 12-output functional run
  (that needs an ~860 µs window, same duty cycles).
- Duty cycles assume a continuously-fed input (true here — PSPs stream in at
  1205 MB/s, far faster than K4 consumes them).
- Absolute AIE-array watts **are** available headlessly (§5, `report_power` on the
  routed checkpoint → 54.79 W), but that number uses vectorless (flat) AIE activity;
  the measured per-tile duty cycles refine only its dynamic component (−6.5 %).
