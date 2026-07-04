#!/usr/bin/env python3
"""
Overwrite the AI-Engine compiler's flat `int_core_load="0.6"` on the five kernel
tiles of a single 5k_split pipeline with the *measured* steady-state duty cycles from
the cycle-accurate aiesimulator run (see
docs/analysis/SUSTAINED_TILE_OCCUPANCY_POWER.md).

The compiler emits a uniform 0.6 for every kernel tile; the cycle-accurate profile shows
the pipeline is K4-gated, so the real per-tile loads differ substantially. Feeding the
corrected .xpe to Xilinx Power Estimator / AMD Power Design Manager yields a refined
AIE core-dynamic power (0.935x the flat assumption for this design).

Measured duty cycles are keyed by the KERNEL instance name in the .xpe
(i2..i6 -> K1..K4b in tile-column order). Adjust MEASURED_LOADS if the mapping changes.

Usage:
    apply_measured_loads.py <in.xpe> <out.xpe>
"""

import re
import sys

# Kernel-tile coordinates (col,row) -> measured steady-state duty cycle.
# From profile_funct_<col>_4.txt: cycles-per-PSP / K4 bottleneck (89459 cyc).
MEASURED_LOADS = {
    "0,5": 0.615,   # K1  kernel_k1_wfgen     (55024 / 89459)
    "1,5": 0.266,   # K2a kernel_k2a_ff_diag  (23757 / 89459)
    "2,5": 0.407,   # K2b kernel_k2b_ff_diag  (36423 / 89459)
    "3,5": 0.517,   # K3  kernel_k3_vvv_amp   (46277 / 89459)
    "4,5": 1.000,   # K4  kernel_k4_vvvv_color(89459 / 89459) - bottleneck
}


def apply_measured_loads(in_xpe: str, out_xpe: str) -> None:
    with open(in_xpe, "r") as f:
        content = f.read()

    changed = 0

    def repl(match: "re.Match[str]") -> str:
        nonlocal changed
        tile = match.group(0)
        coord_m = re.search(r'coordinates="(\d+,\d+)"', tile)
        if not coord_m:
            return tile
        coord = coord_m.group(1)
        if coord not in MEASURED_LOADS:
            return tile
        load = MEASURED_LOADS[coord]
        # Replace the TILE-level int_core_load (first occurrence in the opening tag).
        new_tile = re.sub(
            r'(int_core_load=")[0-9.]+(")',
            rf'\g<1>{load:g}\g<2>',
            tile,
            count=1,
        )
        changed += 1
        return new_tile

    # Match each full opening <TILE ...> tag (single line in the compiler output).
    content = re.sub(r'<TILE[^>]*>', repl, content)

    with open(out_xpe, "w") as f:
        f.write(content)

    print(f"Applied measured loads to {changed} kernel tiles -> {out_xpe}")
    if changed != len(MEASURED_LOADS):
        print(f"WARNING: expected {len(MEASURED_LOADS)} tiles, patched {changed}. "
              f"Check tile coordinates in the .xpe.")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    apply_measured_loads(sys.argv[1], sys.argv[2])
