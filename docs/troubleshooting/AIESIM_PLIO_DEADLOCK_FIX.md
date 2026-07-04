# aiesim PLIO Deadlock — Root Cause & Fix

**Status:** Resolved (2026-07-03)
**Affects:** the cycle-accurate `aiesimulator` run for any graph whose build recipe
regenerates `scsim_config.json`, most visibly `GRAPH_IMPL=5k_split`.

> This documents why the cycle-accurate AIE simulation appeared to **hang / never
> stop** and how to make it run to completion. The root cause was a build recipe
> **overwriting** the auto-generated simulator config and stripping out the PLIO
> file drivers, which starved the input stream and deadlocked the cascade.

---

## Symptom

The cycle-accurate simulator "never stops". It loads the ELFs, enables the cores,
prints *"Waiting for core(s)… to finish execution"*, and then all cores stall:

```
WARNING: All the cores are in stalled state at T=3731200 ps ...
|---------------- Core Stall Status ----------------|
 (0,5) -> Stream stall ->  SS0 detected at T=712800 ps
 (1,5) -> Acc stall ->  In Accumulator stall detected at T=729600 ps
 (2,5) -> Acc stall ...
 (3,5) -> Acc stall ...
 (4,5) -> Acc stall ...
```

The hang-detector then has to grind through the remaining simulation time in the slow
`aiesimmsm_dbg` model, so the run appears to hang for a very long time. No
`me2_out_0.txt` is produced. (This is also the likely cause of the earlier multi-hour
VCD run that never finished.)

`(0,5)` is K1 (`kernel_k1_wfgen`). Its only stream is the **input** PLIO (`SS0`). It
stalls waiting for input that never arrives; K2–K4 then accumulator-stall on the empty
cascade. It is **input starvation**, not a physics/reconvergent-FIFO deadlock.

---

## Root cause

The AIE compiler (`aiecompiler --target=hw`) **already auto-generates the correct**
`Work/config/scsim_config.json`. That file contains, per PLIO, a file-driver block:

- input  → `libpl_sender.so`  (`direction: pl_to_me`)
- output → `libpl_receiver.so` (`direction: me_to_pl`)

plus a PS-IP block for the graph's control processor.

The build's `gen_scsim_config` recipe **overwrote** this with a block that contained
**only** the PS-IP entry — no PLIO drivers.

The critical aiesimulator behaviour:

> Once a `scsim_config.json` declares a `pl_ip_block`, aiesimulator uses **only** those
> entries and does **not** auto-instantiate the internal PLIO file readers/writers.

So with the PS-only config, the AIE input stream had no driver at all → K1 stalled on its
first `readincr` → global deadlock.

---

## Diagnostic evidence

A minimal isolation test — one kernel `scale_by_two` (`out = in * 2`), one `input_plio`,
one `output_plio`, **no PS/GMIO** — simulates correctly out-of-the-box:

| Simulator | Command | Output (`dout.txt`) from input `1..8` |
|-----------|---------|----------------------------------------|
| x86 functional | `x86simulator --pkg-dir=Work_x86` | `2 4 6 8 10 12 14 16` |
| cycle-accurate | `aiesimulator --pkg-dir=Work_hw` | `2 4 6 … 16` (+ per-sample timestamps) |

and its auto-generated `Work_hw/config/scsim_config.json` shows exactly the expected
structure:

```json
{ "name": "din",  "ip": "release", "lib_path": "data/pl_fileio/libpl_sender.so",
  "axi_stream": [{ "port_name": "do", "column": 24, "stream_id": 2, "direction": "pl_to_me", "bus_width": 32 }] }
{ "name": "dout", "ip": "release", "lib_path": "data/pl_fileio/libpl_receiver.so",
  "axi_stream": [{ "port_name": "di", "column": 24, "stream_id": 0, "direction": "me_to_pl", "bus_width": 32 }] }
{ "name": "ps_i3_ps_main", "ip": "ps", ... }
```

### PLIO placement is build-specific

The `column` and `stream_id` come from `Work/ps/c_rts/aie_control_config.json →
aie_metadata.PLIOs`. For `5k_split`:

| logical name | shim_column | stream_id | slaveOrMaster | driver |
|--------------|-------------|-----------|---------------|--------|
| `psp_in_0`   | 6 | 4 | 0 (into AIE)  | `libpl_sender.so`, `pl_to_me`, port `do` |
| `me2_out_0`  | 6 | 3 | 1 (out of AIE)| `libpl_receiver.so`, `me_to_pl`, port `di` |

Using the wrong `stream_id` (e.g. 0) makes aiesimulator fail elaboration with
`port not bound: tl.pl.xtlm_ss_0_port_0`. Omitting the PS-IP block makes it fail with
`port not bound: tl.ps_debug_interconnect.initiator_wr_socket_0`. Deleting the config
entirely fails with `[ERROR]: AIE Simulator config Json file missing`. The config
therefore needs **both** the PS-IP block **and** the correctly-parameterised PLIO drivers.

---

## Fix (applied)

1. **[`5k_impl/scripts/gen_scsim_config.py`](../../5k_impl/scripts/gen_scsim_config.py)** —
   parses `aie_control_config.json` (PLIO shim column / stream_id / direction) and the PS
   lib, then emits a config with the PS-IP block **plus** one
   `libpl_sender`/`libpl_receiver` block per PLIO. Reproduces what the aiecompiler emits.
   Usage: `gen_scsim_config.py <WORK_DIR>`.

2. **The `gen_scsim_config` build step** — must **no longer clobber** the compiler output.
   It should:
   - **keep** the aiecompiler-generated `scsim_config.json` if it already contains
     `libpl_sender` (i.e. it drives PLIO), otherwise
   - **regenerate** an equivalent one via the helper above.

With the corrected config the 5k pipeline runs to completion:
`71 us core(s) are done executing`.

---

## How to run the 5k cycle-accurate sim

```bash
source /tools/Xilinx/Vitis/2024.1/settings64.sh
# build the graph, then ensure the scsim_config.json keeps its PLIO drivers
# (see fix above). Run the cycle-accurate simulator:
aiesimulator --pkg-dir=Work_5k_split --simulation-cycle-timeout=150000
# results:
#   Work_5k_split/aiesimulator_output/data/me2_out_0.txt      (ME^2 stream)
#   Work_5k_split/aiesimulator_output/default.aierun_summary  (per-tile cycles/occupancy)
```

Note: the cycle-accurate `aiesimmsm_dbg` model is slow; a single PSP through the 5-kernel
cascade (32 helicities) takes several minutes of wall-clock even though it is only tens of
microseconds of simulated time. The cores reach *"71 us core(s) are done executing"*.

---

## Verified run — batch 1 (single PSP)

The design is compiled at `GGTTG_BATCH=1` (see
[`5k_impl/src/ggttg_config.h`](../../5k_impl/src/ggttg_config.h)): K1 reads exactly one PSP
(5 particles × 4 floats = 20 samples) and K4 emits **one** ME² result. A 20-line
`data/psp_in_0.txt` is therefore the correct single-PSP input — no need to pad the input
with extra events.

Run (`--simulation-cycle-timeout=150000`, single PSP):

```
72 us core(s) are done executing
[INFO] : Simulation Finished, Sim result: 0  Total Simulation time 120 us
```

`aiesimulator_output/data/me2_out_0.txt`:

```
T 72292 ns
2.093191870e-04
```

This **exactly matches** the x86 functional-sim golden value
(`x86simulator_output/data/me2_out_0.txt`, first PSP = `2.093191870e-04`), confirming the
cycle-accurate pipeline is functionally correct.

> The end-of-run throughput table shows `me2_out_0  OUT  0.000000 MBps`. This is **not** an
> error: a single output sample averaged over the full 120 us simulated window rounds to
> ~0 MBps. The output file contains the value. Feeding more PSPs gives a meaningful rate.

### Per-tile compute occupancy (single PSP, from `profile_funct_*.txt`)

| Tile  | Kernel                 | Compute cycles (self) | Occupancy of run window |
|-------|------------------------|-----------------------|-------------------------|
| (0,5) | K1 `kernel_k1_wfgen`   | 55,024                | 61.4% |
| (1,5) | K2a `kernel_k2a_ff_diag` | 23,757              | 26.5% |
| (2,5) | K2b `kernel_k2b_ff_diag` | 36,423              | 40.5% |
| (3,5) | K3 `kernel_k3_vvv_amp`   | 46,277              | 51.4% |
| (4,5) | K4 `kernel_k4_vvvv_color`| 89,433              | **99.2%** |

K4 (colour reduction) is the pipeline bottleneck at ~99% occupancy; the upstream kernels
idle waiting on the cascade. This per-tile balance is the cycle-accurate evidence for the
pipeline-occupancy discussion. (Occupancy is the self-cycle count divided by the ~89.6–90.1k
cycle report window; total elapsed including drain is ~149k cycles.)

### `--simulation-cycle-timeout` must exceed the pipeline latency

`--simulation-cycle-timeout <N>` is an **I/O-inactivity** window (in AIE cycles at 1.25 GHz),
**not** an absolute cap. The sim ends once no PLIO input/output activity occurs for `N`
cycles. This interacts badly with a deep cascade fed by a **single** PSP:

- The one PSP input drains in a few µs; the first ME² output only appears at the tail of the
  ~71 µs cascade latency.
- If `N` is **too small** (e.g. 15000 ≈ 12 µs) the window elapses **before** any output is
  produced → sim stops at ~12 µs with `me2_out_0 OUT 0.000000 MBps` (no output, no file).
- If `N` is **too large** (e.g. 100000000 ≈ 80 ms) the sim free-runs for the whole idle
  window after the cores finish → appears to hang and never flushes in reasonable wall-clock.

Choose `N` **greater than the time-to-first-output** (~72 µs ⇒ ~90000 cycles) with margin,
e.g. `--simulation-cycle-timeout=150000` (~120 µs). The sim then produces output at ~72 µs
and stops one idle window later, flushing `me2_out_0.txt` and the run summary. (Feeding more
PSPs keeps the I/O active and also avoids premature timeout.)

---

## Gotchas for a standalone AIE stream kernel

- `input_stream<T>` / `output_stream<T>` are in the **global** namespace, **not** `adf::`.
- Call `aiecompiler` with its **native** flags
  (`--target=x86sim|hw --platform=… --include=src --workdir=Work src/graph.cpp`),
  not the `v++`-style `-c --mode aie --aie.*` flags.
