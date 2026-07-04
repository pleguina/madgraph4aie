#!/usr/bin/env python3
"""Generate a correct aiesimulator scsim_config.json for a compiled AIE graph.

The default Makefile recipe emitted a config containing ONLY a PS-IP block.
aiesimulator, when given a scsim_config.json with a pl_ip_block, uses ONLY the
listed drivers and does NOT auto-instantiate the internal PLIO file readers/writers.
The AIE input stream was therefore never driven, K1 stalled on its first readincr,
the cascade accumulator-stalled, and the sim "never stopped" (hang-detector grind).

This script derives the correct PLIO sender/receiver blocks (shim column + stream_id)
from <work>/ps/c_rts/aie_control_config.json so input is actually fed and output
captured, and keeps the PS-IP block so the ps_debug_interconnect binds.

Usage: gen_scsim_config.py <WORK_DIR>
"""
import json
import os
import sys


def main() -> int:
    if len(sys.argv) != 2:
        sys.stderr.write("usage: gen_scsim_config.py <WORK_DIR>\n")
        return 2
    work = sys.argv[1]
    ctrl_path = os.path.join(work, "ps", "c_rts", "aie_control_config.json")
    with open(ctrl_path) as f:
        ctrl = json.load(f)

    plios = ctrl["aie_metadata"]["GMIOAndPLIOMetadata"]["PLIOs"] \
        if "GMIOAndPLIOMetadata" in ctrl["aie_metadata"] \
        else ctrl["aie_metadata"]["PLIOs"]

    # locate the PS lib
    ps_name = None
    ps_dir = os.path.join(work, "ps")
    for root, _dirs, files in os.walk(ps_dir):
        for fn in files:
            if fn.startswith("ps_") and fn.endswith(".so"):
                ps_name = os.path.splitext(fn)[0]
                break
        if ps_name:
            break

    blocks = []
    if ps_name:
        blocks.append({
            "name": f"{ps_name}_ps_main",
            "ip": "ps",
            "lib_path": f"ps/c_rts/systemC/generated-objects/{ps_name}.so",
            "pl_freq": 300000000.0,
            "axi_mm": [{"port_name": "ps_axi", "direction": "ps_to_gm", "bus_width": 0}],
            "event_bus": [],
        })

    for _key, p in sorted(plios.items(), key=lambda kv: kv[1]["id"]):
        logical = p["logical_name"]
        column = p["shim_column"]
        stream_id = p["stream_id"]
        is_master = p["slaveOrMaster"]  # 0 = into AIE (input), 1 = out of AIE (output)
        if is_master == 0:
            blocks.append({
                "name": logical,
                # trailing fields must be "1","1","0" (matches aiecompiler output):
                # the middle "1"s make the driver run a single pass so the sim
                # self-terminates; "0","0" would make the receiver free-run forever.
                "arguments": [f"data/{logical}.txt", "32", "32", "1", "1", "0"],
                "ip": "release",
                "lib_path": "data/pl_fileio/libpl_sender.so",
                "pl_freq": 500000000.0,
                "axi_stream": [{"port_name": "do", "column": column,
                                "stream_id": stream_id, "direction": "pl_to_me",
                                "bus_width": 32}],
                "event_bus": [],
            })
        else:
            blocks.append({
                "name": logical,
                # trailing fields "1","1","0": run one pass then stop so the sim ends.
                "arguments": [f"data/{logical}.txt", "32", "32", "1", "1", "0"],
                "ip": "release",
                "lib_path": "data/pl_fileio/libpl_receiver.so",
                "pl_freq": 500000000.0,
                "axi_stream": [{"port_name": "di", "column": column,
                                "stream_id": stream_id, "direction": "me_to_pl",
                                "bus_width": 32}],
                "event_bus": [],
            })

    cfg = {
        "SimulationConfig": {
            "device_json": {"directory": "data/devices", "file": "VC1902.json"},
            "phy_device_file": "xcvc1902-vsva2197-2MP-e-S",
            "aiearch": "aie",
            "aie_freq": 1250000000.0,
            "use_real_noc": 1,
            "evaluate_fifo_depth": 0,
            "shim_sol": "arch/aieshim_solution.aiesol",
            "xpe_report": "reports/ggttg_design.xpe",
            "pl_ip_block": blocks,
        }
    }

    out_dir = os.path.join(work, "config")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "scsim_config.json")
    with open(out_path, "w") as f:
        json.dump(cfg, f, indent=4)
        f.write("\n")
    sys.stderr.write(
        f"==> Wrote {out_path} with PS-IP + {sum(1 for b in blocks if b.get('ip') == 'release')} PLIO driver block(s)\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
