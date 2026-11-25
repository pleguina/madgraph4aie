#!/usr/bin/env python3
"""
Extract steady-state initiation interval (II) and full-grid throughput
(R_chip) from an aiesimulator run of the full-grid batched pipeline.

Every lane is an instance of the 8-tile batched pipeline, which emits a
burst of 8 samples per invocation (one per PSP in the batch), then a large
gap to the next invocation's burst. The II is therefore taken between
bursts, not between consecutive samples.

The script also checks consistency across lanes: DMA/NoC bandwidth
contention between the simultaneously streaming lanes would show up as
lane-to-lane II variation.

INPUT FORMAT (aiesimulator PLIO output, one file per lane):
    T 72292 ns
    2.093191870e-04
    T 158499200 ps
    3.989184042e-04
    ...
(unit is not always consistent within a run -- ns/us/ps are all handled
and normalized to fractional nanoseconds, same as parse_aie_trace.py)

USAGE
    # Single lane, sanity check the parsing/grouping before trusting it:
    python3 parse_full_grid_trace.py --batch 8 out_0.txt

    # All 50 lanes at once, full-grid summary + measured R_chip:
    python3 parse_full_grid_trace.py --batch 8 \
        --glob "Work_hw_N48/aiesimulator_output/data/out_*.txt" \
        --n-lanes 50 --summary full_grid_summary.json

    # If your data files sit elsewhere (one out_<l>.txt per lane), just
    # point --glob at them; lane index is parsed from the filename.
"""
import argparse
import glob as globmod
import json
import re
import statistics
import sys
from pathlib import Path

TS_RE = re.compile(r"^\s*T\s+([\d.]+)\s*(ns|us|ps)\s*$")
_UNIT_TO_NS = {"ns": 1.0, "us": 1e3, "ps": 1e-3}
LANE_IDX_RE = re.compile(r"(\d+)(?!.*\d)")  # last integer in the filename


def parse_trace(path: Path):
    """Return list of (timestamp_ns, value) for every PLIO sample, in file order."""
    events = []
    ts_ns = None
    with open(path) as f:
        for line in f:
            line = line.rstrip("\n")
            if not line.strip():
                continue
            m = TS_RE.match(line)
            if m:
                ts_ns = float(m.group(1)) * _UNIT_TO_NS[m.group(2)]
                continue
            try:
                val = float(line.strip())
            except ValueError:
                continue
            if ts_ns is None:
                raise ValueError(
                    f"{path}: value with no preceding 'T <n> <unit>' timestamp -- "
                    "this looks like an x86simulator (functional-only) output, "
                    "not a cycle-accurate aiesimulator trace."
                )
            events.append((ts_ns, val))
            ts_ns = None
    return events


def group_into_invocations(events, batch):
    """Split a flat event list into invocation groups of `batch` samples each.

    Drops a trailing partial group (a run that was still mid-invocation when
    the simulation stopped) rather than silently corrupting the last gap.
    """
    n_complete = len(events) // batch
    if n_complete * batch != len(events):
        dropped = len(events) - n_complete * batch
        print(f"  (dropping {dropped} trailing sample(s), incomplete final invocation)",
              file=sys.stderr)
    groups = [events[i * batch:(i + 1) * batch] for i in range(n_complete)]
    return groups


def analyze_lane(events, batch, warmup, drain):
    groups = group_into_invocations(events, batch)
    if len(groups) < warmup + drain + 2:
        raise ValueError(
            f"only {len(groups)} complete invocation(s) after grouping by batch={batch} "
            f"-- need at least warmup({warmup})+drain({drain})+2 to compute any gap. "
            f"Rerun with a larger GGTTG_NUM_ITER."
        )
    group_start_ts = [g[0][0] for g in groups]  # first timestamp of each invocation
    lo, hi = warmup, len(group_start_ts) - 1 - drain
    steady_gaps = [group_start_ts[i + 1] - group_start_ts[i] for i in range(lo, hi)]

    mean_gap = statistics.mean(steady_gaps)
    return {
        "n_invocations_total": len(groups),
        "steady_state_window": [lo, hi],
        "n_gaps_used": len(steady_gaps),
        "II_batch_ns": mean_gap,
        "II_batch_us": mean_gap / 1e3,
        "II_per_psp_us": mean_gap / 1e3 / batch,
        "stdev_ns": statistics.pstdev(steady_gaps) if len(steady_gaps) > 1 else 0.0,
        "min_gap_ns": min(steady_gaps),
        "max_gap_ns": max(steady_gaps),
        "first_invocation_latency_us": group_start_ts[0] / 1e3,
    }


def lane_index_from_path(path: Path):
    m = LANE_IDX_RE.search(path.stem)
    return int(m.group(1)) if m else None


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("trace", type=Path, nargs="?",
                     help="single aiesimulator PLIO output file (out_<l>.txt)")
    ap.add_argument("--glob", type=str, default=None,
                     help='glob pattern matching multiple lane output files, '
                          'e.g. "Work_hw_N48/aiesimulator_output/data/out_*.txt"')
    ap.add_argument("--batch", type=int, default=8,
                     help="PSPs per invocation (8 for this pipeline, default 8)")
    ap.add_argument("--n-lanes", type=int, default=None,
                     help="tiles/lane x this = pipelines used for R_chip; "
                          "if omitted, inferred from the number of files matched")
    ap.add_argument("--warmup", type=int, default=1,
                     help="leading invocations to exclude as pipeline fill (default 1)")
    ap.add_argument("--drain", type=int, default=1,
                     help="trailing invocations to exclude as pipeline drain (default 1)")
    ap.add_argument("--summary", type=Path, default=None, help="write full JSON summary here")
    args = ap.parse_args()

    if not args.trace and not args.glob:
        ap.error("give either a single trace file or --glob for multiple lanes")

    paths = [args.trace] if args.trace else sorted(
        (Path(p) for p in globmod.glob(args.glob)), key=lambda p: lane_index_from_path(p) or 0)
    if not paths:
        ap.error(f"no files matched: {args.glob}")

    per_lane = {}
    for p in paths:
        lane = lane_index_from_path(p)
        events = parse_trace(p)
        try:
            result = analyze_lane(events, args.batch, args.warmup, args.drain)
            per_lane[lane if lane is not None else str(p)] = result
        except ValueError as e:
            print(f"lane {lane} ({p}): SKIPPED -- {e}", file=sys.stderr)

    if not per_lane:
        print("no lane produced a usable result -- rerun with more invocations", file=sys.stderr)
        sys.exit(1)

    all_ii_us = [r["II_batch_us"] for r in per_lane.values()]
    n_lanes = args.n_lanes or len(per_lane)

    summary = {
        "n_lanes_analyzed": len(per_lane),
        "n_lanes_for_r_chip": n_lanes,
        "batch_per_invocation": args.batch,
        "per_lane": per_lane,
        "II_batch_us_across_lanes": {
            "mean": statistics.mean(all_ii_us),
            "stdev": statistics.pstdev(all_ii_us) if len(all_ii_us) > 1 else 0.0,
            "min": min(all_ii_us),
            "max": max(all_ii_us),
            "spread_pct": (max(all_ii_us) - min(all_ii_us)) / statistics.mean(all_ii_us) * 100
                          if statistics.mean(all_ii_us) else 0.0,
        },
    }

    mean_ii_us = summary["II_batch_us_across_lanes"]["mean"]
    r_chip_mpsp_s = n_lanes * args.batch / mean_ii_us  # us -> MPSP/s falls out directly
    summary["measured_R_chip_MPSP_per_s"] = r_chip_mpsp_s

    print(json.dumps(summary, indent=2))
    print(f"\n=== measured full-grid R_chip = {r_chip_mpsp_s:.4f} MPSP/s "
          f"({n_lanes} lanes x {args.batch} PSP / {mean_ii_us:.4f} us) ===", file=sys.stderr)
    spread = summary["II_batch_us_across_lanes"]["spread_pct"]
    if len(per_lane) > 1:
        flag = "  <-- investigate: possible cross-lane contention" if spread > 1.0 else "  (tight, as expected -- no cross-lane contention signal)"
        print(f"=== lane-to-lane II spread: {spread:.3f}% {flag} ===", file=sys.stderr)

    if args.summary:
        with open(args.summary, "w") as f:
            json.dump(summary, f, indent=2)
        print(f"wrote {args.summary}", file=sys.stderr)


if __name__ == "__main__":
    main()
