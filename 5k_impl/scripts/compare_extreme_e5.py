#!/usr/bin/env python3
"""E5: per-region float32 vs float64 precision comparison for extreme phase-space points.

Reads:
  --fp32     : AIE fp32 output (one ME per line, may have trailing whitespace)
  --golden   : fp64 golden file (index me2 weight)
  --labels   : region labels file (index region observable)
Reports mean / median / p99 / max relative error (in ppm) overall and per region.
"""
import argparse
import statistics


def load_fp32(path):
    vals = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('T '):  # skip aiesim 'T <ns>' header lines
                continue
            vals.append(float(line.split()[0]))
    return vals


def load_golden(path):
    vals = []
    with open(path) as f:
        for line in f:
            p = line.split()
            if len(p) >= 2:
                vals.append(float(p[1]))
    return vals


def load_labels(path):
    labs = []
    with open(path) as f:
        for line in f:
            p = line.split()
            if len(p) >= 2:
                labs.append(p[1])
    return labs


def pct(sorted_vals, q):
    if not sorted_vals:
        return float('nan')
    k = (len(sorted_vals) - 1) * q
    lo = int(k)
    hi = min(lo + 1, len(sorted_vals) - 1)
    return sorted_vals[lo] + (sorted_vals[hi] - sorted_vals[lo]) * (k - lo)


def report(name, errs_ppm):
    if not errs_ppm:
        print(f"  {name:12s}: (none)")
        return
    s = sorted(errs_ppm)
    print(f"  {name:12s}: n={len(s):4d}  mean={statistics.mean(s):8.3f}  "
          f"median={statistics.median(s):8.3f}  p99={pct(s,0.99):8.3f}  max={s[-1]:8.3f}  (ppm)")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--fp32', required=True)
    ap.add_argument('--golden', required=True)
    ap.add_argument('--labels', required=True)
    args = ap.parse_args()

    fp32 = load_fp32(args.fp32)
    gold = load_golden(args.golden)
    labs = load_labels(args.labels)

    n = min(len(fp32), len(gold), len(labs))
    if not (len(fp32) == len(gold) == len(labs)):
        print(f"WARNING: length mismatch fp32={len(fp32)} gold={len(gold)} labels={len(labs)}; using first {n}")

    by_region = {}
    overall = []
    for i in range(n):
        g = gold[i]
        if g == 0:
            continue
        rel = abs(fp32[i] - g) / abs(g) * 1e6  # ppm
        overall.append(rel)
        by_region.setdefault(labs[i], []).append(rel)

    print(f"E5 extreme-region float32 vs float64 relative error ({n} points)")
    print("-" * 78)
    report("OVERALL", overall)
    for reg in sorted(by_region):
        report(reg, by_region[reg])
    print("-" * 78)
    # bulk reference note
    worst = max(overall)
    print(f"Worst-case relative error across all extreme points: {worst:.3f} ppm "
          f"({worst/1e6*100:.2e} %)")


if __name__ == '__main__':
    main()
