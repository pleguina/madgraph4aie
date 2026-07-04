#!/usr/bin/env python3
"""
1000-point precision comparison: AIE fp32 (x86sim, single-pipeline 5-kernel
cascade) vs MadGraph5 fp64 reference.

Data (all index-aligned 0..999, in data/):
  - psp_in_1000_fp32.txt : PLIO input, 20000 lines (1000 PSP x 20 floats,
                            32-bit PLIO / 1 float per line, PSP-major
                            E,px,py,pz for g1,g2,t,tbar,g3).
  - aie_me2_1000_fp32.txt : AIE fp32 |M|^2 output, 1000 lines.
  - expected_me2_1000.txt : MadGraph fp64 reference, "index me2 weight".

Run:  python3 scripts/precision_compare_1000.py
"""
import numpy as np
from pathlib import Path

DATA = Path(__file__).resolve().parent.parent / "data"
AIE = DATA / "aie_me2_1000_fp32.txt"
REF = DATA / "expected_me2_1000.txt"

aie = np.array([float(t) for t in open(AIE).read().split()], dtype=np.float64)
ref = np.array([float(l.split()[1]) for l in open(REF) if l.strip()], dtype=np.float64)
n = min(aie.size, ref.size)
aie, ref = aie[:n], ref[:n]

rel = np.abs(aie - ref) / np.abs(ref)
ae = np.abs(aie - ref)
ppm = rel * 1e6
signed = (aie - ref) / ref
worst = int(np.argmax(rel))

print(f"Samples: {n}")
print(f"Abs err   mean {ae.mean():.6e}  median {np.median(ae):.6e}  "
      f"max {ae.max():.6e}  stddev {ae.std():.6e}")
print(f"Rel err   mean {rel.mean():.3e} ({ppm.mean():.2f} ppm)  "
      f"median {np.median(rel):.3e} ({np.median(ppm):.2f} ppm)  "
      f"stddev {rel.std():.3e} ({ppm.std():.2f} ppm)")
for p in (50, 90, 95, 99, 99.9):
    print(f"  p{p:<5}: {np.percentile(ppm, p):8.2f} ppm")
print(f"Max rel   {rel.max():.3e} ({ppm.max():.2f} ppm) at idx {worst} "
      f"(ref {ref[worst]:.6e}, aie {aie[worst]:.6e})")
print(f"Frac > 10 ppm  {100*np.mean(ppm>10):.2f}%   "
      f"> 100 ppm {100*np.mean(ppm>100):.2f}%")
print(f"Mean signed bias {signed.mean()*1e6:+.2f} ppm   "
      f"NaN/inf {int(np.sum(~np.isfinite(aie)))}")
