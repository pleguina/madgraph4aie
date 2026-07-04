// =============================================================================
// flops_probe.cpp
//
// Measures the floating-point operation count of ONE gg -> t t~ g matrix
// element (|M|^2) in a platform-independent way, for the "Arithmetic Cost per
// Matrix Element" result in the paper (~1.85e5 FLOP/ME).
//
// It evaluates sigmaKin() -- one full amplitude (32 helicity combinations plus
// the colour sum) -- repeatedly on a SINGLE, FIXED phase-space point. RAMBO is
// called only ONCE, before the loop, so that differencing hardware performance
// counters between two run lengths (e.g. N and 10*N iterations) cancels all
// one-time set-up and leaves the pure per-ME arithmetic:
//
//     FLOP/ME = ( count(10N) - count(N) ) / (10N - N)
//
// Build it SCALAR, without FMA and without vectorisation, so that every retired
// floating-point instruction maps to exactly one operation (no FMA-doubling, no
// SIMD packing). See measure_cpu_flops.sh for the exact flags and perf events.
//
// BUILD: NOT self-contained. Compile inside a MadGraph5 standalone export of
// "g g > t t~ g", i.e. SubProcesses/P1_Sigma_sm_gg_ttxg (provides CPPProcess.*,
// rambo.*, libmodel_sm.a). Drop this file there; measure_cpu_flops.sh builds it.
//
// Prints the ME value so you can confirm it reproduces the fp64 golden
// (2.0931907741e-04 GeV^-2 at the reference point) -- a correctness check that
// the scalar/no-FMA build did not perturb the physics.
// =============================================================================
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>

#include "CPPProcess.h"
#include "rambo.h"

using namespace std;

void set_rambo_seed(int ij, int kl);

int main(int argc, char** argv) {
    long n_iter = (argc > 1) ? atol(argv[1]) : 200000;
    double energy = (argc > 2) ? atof(argv[2]) : 1500.0;
    int seed = (argc > 3) ? atoi(argv[3]) : 1802;

    CPPProcess process;
    process.initProc("../../Cards/param_card.dat");
    set_rambo_seed(seed, 9373);

    // ONE phase-space point, sampled once, then held fixed for the whole loop.
    double weight;
    vector<double*> p = get_momenta(process.ninitial, energy,
                                    process.getMasses(), weight);
    process.setMomenta(p);

    // Tight loop over the matrix-element kernel on the fixed point.
    double me = 0.0;
    for (long i = 0; i < n_iter; ++i) {
        process.sigmaKin();
        const double* mes = process.getMatrixElements();
        me = mes[0];              // read result to prevent dead-code elimination
    }

    cout << scientific << setprecision(10);
    cout << "n_iter=" << n_iter
         << " ME=" << me << " GeV^-2" << endl;
    return 0;
}
