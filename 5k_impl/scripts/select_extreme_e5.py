#!/usr/bin/env python3
"""
E5 (referee 3): select extreme-region phase-space points from a large flat
RAMBO sample and carry the fp64 golden ME along, for a float32-precision
stress test of the AIE cascade.

Input  (from generate_test_data):
  <prefix>_momenta.txt : "# Event N" / "# Weight: W" / "idx E px py pz" x5
  <prefix>_results.txt : "evt ME weight"   (fp64 golden ME per event)

Particles (E,px,py,pz):
  0 g1 (initial, +z)   1 g2 (initial, -z)
  2 t                  3 tbar               4 g3 (final gluon)

Extreme regions (where float32 cancellations are worst):
  SOFT      : smallest final-gluon energy E(g3)
  COLLINEAR : g3 closest in angle to any other parton (beam +/-z, t, tbar)
  HIMASS    : largest tt-bar invariant mass  m(t,tbar)

Output:
  <out>_momenta.txt : selected events (MadGraph format, renumbered) for
                      convert_momenta_to_psp.py
  <out>_golden.txt  : "index me2 weight" aligned to the selected order
  <out>_labels.txt  : "index region observable" for reporting
"""
import argparse, math
from pathlib import Path


def parse_momenta(path):
    evts = []
    cur = []
    for line in open(path):
        s = line.strip()
        if not s:
            continue
        if s.startswith('#'):
            if s.startswith('# Event') and cur:
                evts.append(cur); cur = []
            continue
        parts = s.split()
        if len(parts) == 5:
            cur.append([float(x) for x in parts[1:]])
            if len(cur) == 5:
                evts.append(cur); cur = []
    return evts  # list of [ [E,px,py,pz]*5 ]


def p3(v):  # 3-vector norm
    return math.sqrt(v[1] * v[1] + v[2] * v[2] + v[3] * v[3])


def cos_angle(a, b):
    na, nb = p3(a), p3(b)
    if na == 0 or nb == 0:
        return -1.0
    return (a[1] * b[1] + a[2] * b[2] + a[3] * b[3]) / (na * nb)


def inv_mass(a, b):
    E = a[0] + b[0]
    px, py, pz = a[1] + b[1], a[2] + b[2], a[3] + b[3]
    m2 = E * E - (px * px + py * py + pz * pz)
    return math.sqrt(m2) if m2 > 0 else 0.0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--prefix', default='/tmp/e5_big')
    ap.add_argument('--out', default='/tmp/e5_extreme')
    ap.add_argument('-n', '--per_region', type=int, default=400)
    args = ap.parse_args()

    evts = parse_momenta(Path(args.prefix + '_momenta.txt'))
    res = [l.split() for l in open(args.prefix + '_results.txt') if l.strip()]
    me = [float(r[1]) for r in res]
    wt = [float(r[2]) for r in res]
    n = min(len(evts), len(me))
    evts, me, wt = evts[:n], me[:n], wt[:n]
    print(f"Parsed {n} events")

    beam_p = [1.0, 0, 0, 1.0]   # +z direction
    beam_m = [1.0, 0, 0, -1.0]  # -z direction

    soft, coll, mass = [], [], []
    for i, e in enumerate(evts):
        g3 = e[4]
        t, tbar = e[2], e[3]
        soft.append((e[4][0], i))                      # E(g3)
        cmax = max(cos_angle(g3, beam_p), cos_angle(g3, beam_m),
                   cos_angle(g3, t), cos_angle(g3, tbar))
        coll.append((cmax, i))                          # closer to 1 = collinear
        mass.append((inv_mass(t, tbar), i))             # m(tt)

    soft.sort()                       # ascending: softest first
    coll.sort(reverse=True)           # descending: most collinear first
    mass.sort(reverse=True)           # descending: highest mass first

    k = args.per_region
    picked = {}   # global_idx -> (region, observable)
    for _, i in soft[:k]:
        picked.setdefault(i, ('SOFT', evts[i][4][0]))
    for c, i in coll[:k]:
        picked.setdefault(i, ('COLLINEAR', c))
    for m, i in mass[:k]:
        picked.setdefault(i, ('HIMASS', m))

    order = sorted(picked.keys())
    print(f"Selected {len(order)} unique extreme events "
          f"(soft<= {soft[k-1][0]:.3f} GeV, cos>= {coll[k-1][0]:.5f}, "
          f"m(tt)>= {mass[k-1][0]:.1f} GeV)")

    mom = open(args.out + '_momenta.txt', 'w')
    gold = open(args.out + '_golden.txt', 'w')
    lab = open(args.out + '_labels.txt', 'w')
    for new_i, gi in enumerate(order):
        e = evts[gi]
        mom.write(f"# Event {new_i}\n# Weight: {wt[gi]:.17e}\n")
        for pi, v in enumerate(e):
            mom.write(f"{pi} {v[0]:.17e} {v[1]:.17e} {v[2]:.17e} {v[3]:.17e}\n")
        mom.write("\n")
        gold.write(f"{new_i} {me[gi]:.17e} {wt[gi]:.17e}\n")
        region, obs = picked[gi]
        lab.write(f"{new_i} {region} {obs:.6e}\n")
    mom.close(); gold.close(); lab.close()
    print(f"Wrote {args.out}_momenta.txt / _golden.txt / _labels.txt")


if __name__ == '__main__':
    main()
