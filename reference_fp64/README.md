# fp64 MadGraph reference: g g > t t~ g

Double-precision |M|² reference for `g g > t t~ g` (LO QCD), computed with the
`sigmaKin()` of a MadGraph5_aMC@NLO **v3.5.6** `standalone_cpp` export (UFO SM model,
MT = 173.0 GeV, WT = 1.4915 GeV, default `param_card.dat`). The batched AIE kernels are
scored against these files by `batch8_impl/precision_validation/`.

## Contents

- `gg_ttxg_sa/` — MG5 v3.5.6 `standalone_cpp` export of `generate g g > t t~ g`.
  `src/rambo.{h,cc}` add `rn_seed(ij, kl)` so the RAMBO generator can be seeded; this is
  the only change to the generated code.
- `gg_ttxg_sa/SubProcesses/P1_Sigma_sm_gg_ttxg/gen_reference.cpp` — driver that links the
  generated `CPPProcess` and `libmodel_sm.a`, draws phase-space points with RAMBO, calls
  `sigmaKin()` per point and writes the six data files.
- `data/` — the generated reference sets.

## Building and running the generator

```
cd gg_ttxg_sa/src && make
cd ../SubProcesses/P1_Sigma_sm_gg_ttxg
g++ -Ofast -I../../src -I. -c CPPProcess.cc gen_reference.cpp
g++ -o gen_reference CPPProcess.o gen_reference.o -L../../lib -lmodel_sm
./gen_reference        # writes psp_*.txt and me2_ref_*.txt in the current directory
```

Fixed partonic √s = 1500 GeV (two massless initial-state gluons, no PDFs).

## Data sets

| psp file | me2 file | N | content |
|---|---|---|---|
| `psp_1000.txt` | `me2_ref_1000.txt` | 1000 | flat RAMBO sample (seed ij=1000, kl=2000) |
| `psp_soft400.txt` | `me2_ref_soft400.txt` | 400 | lowest final-gluon energy E(g3) |
| `psp_collinear396.txt` | `me2_ref_collinear396.txt` | 396 | most collinear g3, disjoint from soft400 |

soft400 and collinear396 are selected from one pool of 2×10⁵ RAMBO points
(seed ij=20260703, kl=9373):

- **soft400**: the 400 points with the lowest E(g3), ascending (E(g3) ≤ 33.43 GeV).
- **collinear396**: points ranked by descending max cos θ between g3 and
  {beam+, beam−, t, t̄}, skipping points already in soft400, until 396 are collected
  (cos θ ≥ 0.998424, θ_min ≈ 3.24°).

Each line of a `psp_*.txt` file holds one phase-space point as 20 space-separated fp64
values (17 significant digits), particle order `g1(+z) g2(−z) t t̄ g3`:

```
E1 px1 py1 pz1  E2 px2 py2 pz2  Et pxt pyt pzt  Etbar pxtbar pytbar pztbar  E3 px3 py3 pz3
```

The matching `me2_ref_*.txt` holds one |M|² per line, aligned by line number.
