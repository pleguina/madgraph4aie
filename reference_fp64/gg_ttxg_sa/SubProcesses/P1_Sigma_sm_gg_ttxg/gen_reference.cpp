// =============================================================================
// gen_reference.cpp
//
// fp64 reference generator for g g > t t~ g (LO QCD), MT=173.0, WT=1.4915,
// fixed partonic sqrt(s) = 1500 GeV.
//
// Built against a real MadGraph5_aMC@NLO v3.5.6 standalone_cpp export of the
// process (this directory): every |M|^2 comes directly from the generated
// CPPProcess::sigmaKin(), not from any hand-rolled matrix-element code.
//
// Produces:
//   psp_1000.txt / me2_ref_1000.txt
//       1000 flat-RAMBO phase-space points (one point per line, 20 floats:
//       E,px,py,pz for g1,g2,t,tbar,g3) and the aligned |M|^2 per line.
//   psp_soft400.txt / me2_ref_soft400.txt
//       From a 2e5-point RAMBO pool: the 400 lowest final-gluon-energy
//       (E(g3)) points, sorted softest-first.
//   psp_collinear396.txt / me2_ref_collinear396.txt
//       From the same pool: the 400 smallest-opening-angle points (largest
//       cosine between g3 and any of {beam+, beam-, t, tbar}), most-collinear
//       first, MINUS any point already selected into the soft400 set (the
//       two extreme regions overlap slightly at this pool size/seed) -> 396
//       unique points.
//
// Particle order in every momenta line: g1(+z beam) g2(-z beam) t tbar g3.
// =============================================================================
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>

#include "CPPProcess.h"
#include "rambo.h"

using namespace std;

void rn_seed(int ij, int kl); // deterministic RAMBO seeding (see rambo.cc patch)

struct Event {
    double p[5][4]; // [particle][E,px,py,pz]
    double me2;
};

static double p3norm(const double* v) {
    return sqrt(v[1]*v[1] + v[2]*v[2] + v[3]*v[3]);
}

static double cosAngle3(const double* a, const double* b) {
    double na = p3norm(a), nb = p3norm(b);
    if (na == 0 || nb == 0) return -1.0;
    return (a[1]*b[1] + a[2]*b[2] + a[3]*b[3]) / (na*nb);
}

// Final-state gluon energy E(g3): the soft observable.
static double softObservable(const Event& e) { return e.p[4][0]; }

// Max cosine between g3 and {beam+, beam-, t, tbar}: the collinear observable
// (closer to 1 = more collinear).
static double collinearObservable(const Event& e) {
    const double beamP[4] = {1.0, 0.0, 0.0, 1.0};
    const double beamM[4] = {1.0, 0.0, 0.0, -1.0};
    double c = cosAngle3(e.p[4], beamP);
    c = max(c, cosAngle3(e.p[4], beamM));
    c = max(c, cosAngle3(e.p[4], e.p[2])); // t
    c = max(c, cosAngle3(e.p[4], e.p[3])); // tbar
    return c;
}

static vector<Event> generatePool(CPPProcess& process, int n, double energy,
                                   int seed_ij, int seed_kl) {
    rn_seed(seed_ij, seed_kl);
    vector<Event> out;
    out.reserve(n);
    for (int evt = 0; evt < n; evt++) {
        double weight;
        vector<double*> p = get_momenta(process.ninitial, energy,
                                         process.getMasses(), weight);
        process.setMomenta(p);
        process.sigmaKin();
        const double* me = process.getMatrixElements();

        Event e;
        for (int i = 0; i < process.nexternal; i++)
            for (int j = 0; j < 4; j++)
                e.p[i][j] = p[i][j];
        e.me2 = me[0];
        out.push_back(e);

        for (int i = 0; i < process.nexternal; i++) delete[] p[i];
    }
    return out;
}

static void writePair(const string& pspPath, const string& mePath,
                       const vector<Event>& evts) {
    ofstream psp(pspPath);
    ofstream me2(mePath);
    psp << scientific << setprecision(17);
    me2 << scientific << setprecision(17);
    for (const auto& e : evts) {
        for (int i = 0; i < 5; i++)
            for (int j = 0; j < 4; j++)
                psp << e.p[i][j] << (i == 4 && j == 3 ? '\n' : ' ');
        me2 << e.me2 << '\n';
    }
}

int main(int argc, char** argv) {
    const double energy = 1500.0; // fixed partonic sqrt(s), GeV

    // Seeds (Marsaglia-Zaman ij,kl fed to rambo.cc's rn_seed()).
    const int FLAT_SEED_IJ = 1000, FLAT_SEED_KL = 2000;
    const int POOL_SEED_IJ = 20260703, POOL_SEED_KL = 9373;
    const int N_FLAT = 1000;
    const int N_POOL = 200000;
    const int K_SOFT = 400;
    const int K_COLLINEAR = 396;

    CPPProcess process;
    process.initProc("../../Cards/param_card.dat");

    cout << "=== 1) flat-RAMBO 1000-point sample ===" << endl;
    vector<Event> flat = generatePool(process, N_FLAT, energy, FLAT_SEED_IJ, FLAT_SEED_KL);
    writePair("psp_1000.txt", "me2_ref_1000.txt", flat);
    cout << "  wrote " << flat.size() << " points" << endl;

    cout << "=== 2) 2e5-point RAMBO pool ===" << endl;
    vector<Event> pool = generatePool(process, N_POOL, energy, POOL_SEED_IJ, POOL_SEED_KL);
    cout << "  generated " << pool.size() << " points" << endl;

    // Rank by soft observable (ascending: softest first).
    vector<int> bySoft(pool.size());
    for (size_t i = 0; i < pool.size(); i++) bySoft[i] = (int)i;
    sort(bySoft.begin(), bySoft.end(), [&](int a, int b) {
        return softObservable(pool[a]) < softObservable(pool[b]);
    });

    // Rank by collinear observable (descending: most collinear first).
    vector<int> byColl(pool.size());
    for (size_t i = 0; i < pool.size(); i++) byColl[i] = (int)i;
    sort(byColl.begin(), byColl.end(), [&](int a, int b) {
        return collinearObservable(pool[a]) > collinearObservable(pool[b]);
    });

    set<int> softIdx(bySoft.begin(), bySoft.begin() + K_SOFT);

    vector<Event> soft400;
    for (int i = 0; i < K_SOFT; i++) soft400.push_back(pool[bySoft[i]]);

    // Walk the full collinear ranking (most-collinear first), skipping any
    // point already claimed by the soft400 set, until K_COLLINEAR unique
    // points are collected.
    vector<Event> collinear;
    int overlapSkipped = 0, lastRank = -1;
    for (size_t i = 0; i < byColl.size() && (int)collinear.size() < K_COLLINEAR; i++) {
        int idx = byColl[i];
        if (softIdx.count(idx)) { overlapSkipped++; continue; }
        collinear.push_back(pool[idx]);
        lastRank = (int)i;
    }

    cout << "  soft threshold  E(g3) <= " << softObservable(pool[bySoft[K_SOFT-1]]) << " GeV" << endl;
    cout << "  collinear threshold cos >= " << collinearObservable(pool[byColl[lastRank]])
         << " (scanned to rank " << lastRank << " of the sorted pool, "
         << overlapSkipped << " skipped as already in soft400)" << endl;
    cout << "  soft400 = " << soft400.size() << ", collinear396 = " << collinear.size() << endl;

    writePair("psp_soft400.txt", "me2_ref_soft400.txt", soft400);
    writePair("psp_collinear396.txt", "me2_ref_collinear396.txt", collinear);

    cout << "Done." << endl;
    return 0;
}
