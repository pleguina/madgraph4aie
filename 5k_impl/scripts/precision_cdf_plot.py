#!/usr/bin/env python3
"""
Generate the paper precision figure: cumulative distribution of the AIE float32
relative error against the MadGraph5 float64 reference.

Three curves are overlaid on a log-x axis:
  * bulk        - 1,000 flat-RAMBO phase-space points (generic hard kinematics)
  * soft        - 400 softest final-state-gluon configurations
  * collinear   - 396 most-collinear configurations

Relative error is defined per point as |ME_fp32 - ME_fp64| / |ME_fp64|,
with the fp64 MadGraph5 result as reference. The shaded band marks the
~1% percent-level physical (scale + PDF) uncertainty of the LO prediction.

Reproduces the numbers quoted in the Results section of the paper:
  bulk       median 0.9 ppm   p99 6.3 ppm    max 168 ppm
  soft       median 5.7 ppm   p99 490 ppm    max 3.4%
  collinear  median 10.9 ppm  p99 1584 ppm   max 2.4%

Data (index-aligned, one ME per line), tracked under 5k_impl/data/:
  aie_me2_1000_fp32.txt         fp32 ME, single column
  expected_me2_1000.txt         idx  ME_fp64  (col 1 is the reference ME)
  aie_me2_extreme796_fp32.txt   fp32 ME, single column
  expected_me2_extreme796.txt   idx  ME_fp64
  extreme796_labels.txt         idx  {SOFT|COLLINEAR}  value

Usage:
  python3 precision_cdf_plot.py [--data-dir DIR] [--out-dir DIR]
"""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_DATA_DIR = SCRIPT_DIR.parent / "data"


def rel_err_ppm(fp32, fp64):
    fp32 = np.asarray(fp32, dtype=np.float64)
    fp64 = np.asarray(fp64, dtype=np.float64)
    return np.abs(fp32 - fp64) / np.abs(fp64) * 1.0e6


def summarise(name, a):
    p = np.percentile(a, [50, 90, 99])
    print(f"{name:10s} n={a.size:4d}  median={p[0]:8.2f}  p90={p[1]:8.2f}  "
          f"p99={p[2]:9.2f}  max={a.max():10.2f}  (ppm)")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--data-dir", default=str(DEFAULT_DATA_DIR),
                    help="directory holding the *_me2_*.txt data files")
    ap.add_argument("--out-dir", default=str(SCRIPT_DIR),
                    help="directory for the output PDF/PNG")
    args = ap.parse_args()

    data_dir = Path(args.data_dir).resolve()
    out_dir = Path(args.out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    # ---- load bulk 1,000-point sample -------------------------------------
    bulk_fp32 = np.loadtxt(data_dir / "aie_me2_1000_fp32.txt")
    bulk_fp64 = np.loadtxt(data_dir / "expected_me2_1000.txt", usecols=1)
    bulk = rel_err_ppm(bulk_fp32, bulk_fp64)

    # ---- load 796 extreme points and split by label ----------------------
    ext_fp32 = np.loadtxt(data_dir / "aie_me2_extreme796_fp32.txt")
    ext_fp64 = np.loadtxt(data_dir / "expected_me2_extreme796.txt", usecols=1)
    ext = rel_err_ppm(ext_fp32, ext_fp64)
    labels = np.loadtxt(data_dir / "extreme796_labels.txt", usecols=1, dtype=str)

    soft = ext[labels == "SOFT"]
    coll = ext[labels == "COLLINEAR"]

    print(f"data dir: {data_dir}")
    summarise("bulk", bulk)
    summarise("soft", soft)
    summarise("collinear", coll)

    # ---- style (matches figures/plotter.py in the paper) ------------------
    COLOR_BULK = "#58729A"   # soft blue
    COLOR_SOFT = "#DDB468"   # soft ochre
    COLOR_COLL = "#BC7B7B"   # soft red
    COLOR_BAND = "#BCCF7B"   # soft green (physics-uncertainty band)
    COLOR_TEXT = "#222222"
    COLOR_EDGE = "#58729A"

    plt.rcParams.update({
        "text.usetex": False,
        "font.family": "serif",
        "font.serif": ["Times New Roman", "Times", "DejaVu Serif"],
        "font.size": 8.5,
        "axes.labelsize": 8.5,
        "xtick.labelsize": 7.8,
        "ytick.labelsize": 7.8,
        "legend.fontsize": 7.6,
        "axes.unicode_minus": False,
        "pdf.fonttype": 42,
        "ps.fonttype": 42,
    })

    fig, ax = plt.subplots(figsize=(3.35, 2.55), constrained_layout=True)

    def cdf(a):
        xs = np.sort(a)
        ys = np.arange(1, xs.size + 1) / xs.size
        return xs, ys

    # ~1% percent-level physical-uncertainty band (10,000 ppm and above)
    ax.axvspan(1.0e4, 1.0e7, color=COLOR_BAND, alpha=0.35, zorder=0, linewidth=0)
    ax.text(3.16e4, 0.5, "percent-level LO theory uncertainty",
            fontsize=7.0, color="#5a6b33", ha="center", va="center", rotation=90)

    for a, color, lab in ((bulk, COLOR_BULK, "Bulk (1,000, flat RAMBO)"),
                          (soft, COLOR_SOFT, "Soft tail (400)"),
                          (coll, COLOR_COLL, "Collinear tail (396)")):
        xs, ys = cdf(a)
        ax.step(xs, ys, where="post", color=color, linewidth=1.3, label=lab,
                zorder=3)

    ax.set_xscale("log")
    ax.set_xlim(0.3, 1.0e5)
    ax.set_ylim(0, 1.002)
    ax.set_xticks([1e0, 1e1, 1e2, 1e3, 1e4, 1e5])
    ax.set_xticklabels(["1", "10", "100", "1,000", "10,000", "100,000"])
    ax.set_xlabel("float32 relative error (ppm)")
    ax.set_ylabel("Cumulative fraction of points")

    for spine in ("top", "right"):
        ax.spines[spine].set_visible(False)
    ax.spines["left"].set_color(COLOR_EDGE)
    ax.spines["bottom"].set_color(COLOR_EDGE)
    ax.spines["left"].set_linewidth(0.7)
    ax.spines["bottom"].set_linewidth(0.7)
    ax.tick_params(axis="both", colors=COLOR_TEXT, width=0.7, length=3)
    ax.grid(True, which="major", axis="x", color="#E2E2E2", linewidth=0.5,
            zorder=0)

    ax.legend(loc="lower right", bbox_to_anchor=(0.80, 0.03), frameon=False,
              handlelength=1.4, borderaxespad=0.2)

    out_pdf = out_dir / "Fig4.pdf"
    out_png = out_dir / "Fig4.png"
    fig.savefig(out_pdf, dpi=300, bbox_inches="tight")
    fig.savefig(out_png, dpi=300, bbox_inches="tight")
    print(f"Saved: {out_pdf}")
    print(f"Saved: {out_png}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
