#!/usr/bin/env python3
"""
parse_precision_results.py

Reads the batched-kernel x86sim output (one float32 |M|^2 per phase-space
point, in the same order as the concatenated input built by
convert_reference.py), splits it back into the three reference segments
using manifest.json, and reports per set the mean/max absolute and relative
error plus the soft/collinear tail percentiles.

x86sim's PLIO output format is plain one-value-per-line (no "T <time>"
timestamp headers -- that format is specific to aiesimulator's --profile
cycle-accurate hw runs, see ../../scripts/parse_full_grid_trace.py). This parser accepts either, stripping any leading
"T ..." line pairs defensively in case the same PLIO dump format is reused.

Usage:
    python3 parse_precision_results.py \
        --out-file <x86sim output dir>/out_0.txt \
        --manifest ../data/manifest.json \
        --ref-dir ../../../reference_fp64/data \
        --summary precision_summary.json
"""
import argparse
import json
import math
import os


def load_out_values(path):
    vals = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.startswith("T "):
                continue  # timestamp header line, if the hw-style dump format is reused
            vals.append(float(line))
    return vals


def load_me2_ref(path):
    return [float(x.strip()) for x in open(path) if x.strip()]


def stats(seg_name, measured_f32, ref_f64):
    n = len(measured_f32)
    assert len(ref_f64) == n, (seg_name, len(ref_f64), n)

    abs_err = []
    rel_err = []
    worst_rel = (0.0, -1, 0.0, 0.0)  # (rel_err, index, measured, ref)
    for i in range(n):
        m = measured_f32[i]
        r = ref_f64[i]
        ae = abs(m - r)
        re = ae / abs(r) if r != 0.0 else float("inf")
        abs_err.append(ae)
        rel_err.append(re)
        if re > worst_rel[0]:
            worst_rel = (re, i, m, r)

    mean_abs = sum(abs_err) / n
    max_abs = max(abs_err)
    mean_rel = sum(rel_err) / n
    max_rel = worst_rel[0]

    sr = sorted(rel_err)

    def pctl(p):
        idx = min(n - 1, int(round(p / 100.0 * (n - 1))))
        return sr[idx]

    return {
        "segment": seg_name,
        "n": n,
        "mean_abs_error": mean_abs,
        "max_abs_error": max_abs,
        "mean_rel_error": mean_rel,
        "mean_rel_error_ppm": mean_rel * 1e6,
        "max_rel_error": max_rel,
        "max_rel_error_ppm": max_rel * 1e6,
        "max_rel_error_at_index": worst_rel[1],
        "max_rel_error_measured_ME2": worst_rel[2],
        "max_rel_error_ref_ME2": worst_rel[3],
        "median_rel_error_ppm": pctl(50) * 1e6,
        "p99_rel_error_ppm": pctl(99) * 1e6,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out-file", required=True,
                     help="x86sim PLIO output file (out_<lane>.txt)")
    ap.add_argument("--manifest", required=True)
    ap.add_argument("--ref-dir", required=True,
                     help="reference_fp64/data directory")
    ap.add_argument("--summary", default="precision_summary.json")
    args = ap.parse_args()

    manifest = json.load(open(args.manifest))
    measured_all = load_out_values(args.out_file)

    expected_total = manifest["total_n_padded"]
    if len(measured_all) != expected_total:
        print(f"WARNING: expected {expected_total} output values "
              f"(NUM_ITER={manifest['num_iter']} x 8), got {len(measured_all)}. "
              f"Proceeding with what is present; segment slicing may fail if "
              f"the run did not complete.")

    results = []
    for seg in manifest["segments"]:
        name = seg["name"]
        off = seg["offset"]
        n_real = seg["n_real"]
        n_padded = seg["n_padded"]

        measured_seg_padded = measured_all[off:off + n_padded]
        # Only the first n_real are genuine reference points -- padding
        # points (present only in collinear396) are repeats of a real point
        # and are dropped here, not scored.
        measured_seg = measured_seg_padded[:n_real]

        ref_path = os.path.join(args.ref_dir, seg["me2_ref_file"])
        ref_seg = load_me2_ref(ref_path)
        assert len(ref_seg) == n_real, (name, len(ref_seg), n_real)

        s = stats(name, measured_seg, ref_seg)
        results.append(s)
        print(f"=== {name} (N={s['n']}) ===")
        print(f"  mean abs error   = {s['mean_abs_error']:.4e}")
        print(f"  max  abs error   = {s['max_abs_error']:.4e}")
        print(f"  mean rel error   = {s['mean_rel_error']:.4e} "
              f"({s['mean_rel_error_ppm']:.2f} ppm)")
        print(f"  max  rel error   = {s['max_rel_error']:.4e} "
              f"({s['max_rel_error_ppm']:.2f} ppm) "
              f"at index {s['max_rel_error_at_index']} "
              f"(measured={s['max_rel_error_measured_ME2']:.4e}, "
              f"ref={s['max_rel_error_ref_ME2']:.4e})")
        print(f"  median rel error = {s['median_rel_error_ppm']:.2f} ppm")
        print(f"  p99    rel error = {s['p99_rel_error_ppm']:.2f} ppm")
        print()

    with open(args.summary, "w") as f:
        json.dump({"segments": results}, f, indent=2)
    print(f"wrote {args.summary}")


if __name__ == "__main__":
    main()
