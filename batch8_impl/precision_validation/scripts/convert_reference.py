#!/usr/bin/env python3
"""
convert_reference.py

Converts the reference_fp64/data/psp_*.txt files (one PSP per line, 20
space-separated fp64 values) into the single-float-per-line PLIO input format
the AIE graph's `psp_in_<lane>` port expects (see batch8_impl/data/psp_in_0.txt
-- one value per line, no separators, no header).

The graph consumes phase-space points in batches of NPSP=8 per kernel
invocation (kernel_k1a_wfgen_batch8's `for (int p = 0; p < NPSP; p++)` loop),
and the number of invocations (GGTTG_NUM_ITER) is a compile-time constant.
To validate all three reference sets (1000 + 400 + 396 points) in a single
compile+run, they are concatenated into one input stream, in this fixed
order:

    [0    .. 1000)  psp_1000.txt        (bulk flat-RAMBO sample)
    [1000 .. 1400)  psp_soft400.txt     (soft stress test)
    [1400 .. 1800)  psp_collinear396.txt + 4 padding points

396 is not a multiple of 8, so collinear396 is padded to 400 by repeating its
own last point 4 times. The padding points are real, on-shell, momentum-
conserving phase-space points (not zeros) -- they just duplicate real physics
so the kernel's fixed-iteration-count loop structure (statically matched
token counts on every cascade hop) is not violated. They are
excluded from the precision statistics by manifest.json's `collinear396.n`
field (396, not 400) when parse_precision_results.py splits the output back
apart.

1000 + 400 + 400 = 1800 = 225 * 8, so GGTTG_NUM_ITER=225, NLANES=1 covers all
three sets in one x86sim run.

Usage:
    python3 convert_reference.py \
        --src-dir ../../reference_fp64/data \
        --out-dir ../data
"""
import argparse
import json
import os


def load_psp_lines(path):
    """Each line: 20 space-separated fp64 values, one phase-space point."""
    rows = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            vals = [float(x) for x in line.split()]
            assert len(vals) == 20, f"{path}: expected 20 floats, got {len(vals)}"
            rows.append(vals)
    return rows


def load_me2_lines(path):
    return [float(x.strip()) for x in open(path) if x.strip()]


def pad_to_multiple_of_8(rows, label):
    n = len(rows)
    rem = n % 8
    if rem == 0:
        return rows, n
    pad_n = 8 - rem
    print(f"  {label}: N={n}, padding with {pad_n} repeat(s) of the last point "
          f"-> {n + pad_n}")
    return rows + [rows[-1]] * pad_n, n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src-dir", required=True,
                     help="reference_fp64/data directory")
    ap.add_argument("--out-dir", required=True,
                     help="precision_validation/data directory to write "
                          "psp_in_0.txt into")
    ap.add_argument("--lane", type=int, default=0,
                     help="PLIO lane index (matches psp_in_<lane>.txt / "
                          "out_<lane>.txt naming; NLANES=1 in the compile "
                          "means lane 0)")
    args = ap.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)

    print("Loading reference sets...")
    sets = [
        ("bulk1000", "psp_1000.txt", "me2_ref_1000.txt"),
        ("soft400", "psp_soft400.txt", "me2_ref_soft400.txt"),
        ("collinear396", "psp_collinear396.txt", "me2_ref_collinear396.txt"),
    ]

    manifest = {"segments": [], "npsp_per_batch": 8}
    all_rows = []
    all_me2_real = []  # only the REAL points' reference ME2, aligned to real (unpadded) count
    offset = 0

    for name, psp_fname, me2_fname in sets:
        psp_path = os.path.join(args.src_dir, psp_fname)
        me2_path = os.path.join(args.src_dir, me2_fname)
        rows = load_psp_lines(psp_path)
        me2 = load_me2_lines(me2_path)
        assert len(rows) == len(me2), f"{name}: PSP/ME2 count mismatch"
        n_real = len(rows)
        padded_rows, n_real_check = pad_to_multiple_of_8(rows, name)
        assert n_real_check == n_real
        n_padded = len(padded_rows)

        manifest["segments"].append({
            "name": name,
            "offset": offset,          # index into the concatenated PLIO stream
            "n_real": n_real,          # points to actually score
            "n_padded": n_padded,      # points written to the stream (incl. padding)
            "me2_ref_file": me2_fname,
        })

        all_rows.extend(padded_rows)
        offset += n_padded

    total_n = len(all_rows)
    assert total_n % 8 == 0, total_n
    num_iter = total_n // 8
    manifest["total_n_padded"] = total_n
    manifest["num_iter"] = num_iter
    manifest["lane"] = args.lane

    out_path = os.path.join(args.out_dir, f"psp_in_{args.lane}.txt")
    print(f"Writing {out_path}: {total_n} PSPs x 20 floats = {total_n*20} lines")
    with open(out_path, "w") as f:
        for row in all_rows:
            for v in row:
                # match the existing psp_in_*.txt formatting style (repr-ish,
                # scientific notation, 17 sig figs -- x86sim reads plain
                # floats so exact formatting doesn't matter for correctness,
                # but this keeps diffs against existing files meaningful)
                f.write(f"{v:.17e}\n")

    manifest_path = os.path.join(args.out_dir, "manifest.json")
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2)

    print(f"Wrote {manifest_path}:")
    print(json.dumps(manifest, indent=2))
    print(f"\n==> compile with -DGGTTG_NUM_ITER={num_iter} -DGGTTG_NLANES=1")


if __name__ == "__main__":
    main()
