// =============================================================================
// generate_test_data.cpp
//
// fp64 golden reference generator for the gg -> tt~g matrix element.
//
// This is the double-precision baseline used to validate the AIE float32
// pipeline (see docs/validation/PRECISION_ANALYSIS.md). It draws phase-space
// points with RAMBO and evaluates |M|^2 with the MadGraph5 standalone C++
// process code, writing:
//   <prefix>_momenta.txt  - human-readable 5-particle momenta (E,px,py,pz)
//   <prefix>_results.txt  - "evt  ME2  weight" per event
//   <prefix>_momenta.bin  - packed float64 momenta for the AIE host
//
// Particle order: 0=g1(+z), 1=g2(-z), 2=t, 3=tbar, 4=g3.
//
// BUILD: this file is NOT self-contained. It must be compiled inside a
// MadGraph5 standalone export of "g g > t t~ g", specifically the
// SubProcesses/P1_Sigma_sm_gg_ttxg directory, which provides CPPProcess.{h,cc},
// rambo.{h,cc} and libmodel_sm.a. Drop this file there and run:
//     make generate_test_data
// Run e.g.:
//     ./generate_test_data -n 200000 -e 1500 -s 20260703 -o e5_big
// See the "Reproducing the precision studies" section of 5k_impl/README.md.
// =============================================================================
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>

#include "CPPProcess.h"
#include "rambo.h"

using namespace std;

// External function to set RAMBO seed - we need to modify rambo.cc
void set_rambo_seed(int ij, int kl);

void print_usage(const char* prog) {
    cout << "Usage: " << prog << " [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -n <N>        Number of events to generate (default: 10)" << endl;
    cout << "  -s <seed>     Random seed (default: 1802)" << endl;
    cout << "  -e <energy>   Center-of-mass energy in GeV (default: 1500)" << endl;
    cout << "  -o <file>     Output file prefix (default: test_data)" << endl;
    cout << "  -h            Show this help" << endl;
}

int main(int argc, char** argv) {
    // Default parameters
    int n_events = 10;
    int seed = 1802;
    double energy = 1500.0;
    string output_prefix = "test_data";

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-n" && i + 1 < argc) {
            n_events = atoi(argv[++i]);
        } else if (arg == "-s" && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (arg == "-e" && i + 1 < argc) {
            energy = atof(argv[++i]);
        } else if (arg == "-o" && i + 1 < argc) {
            output_prefix = argv[++i];
        } else {
            cerr << "Unknown option: " << arg << endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    cout << "=============================================================================" << endl;
    cout << " Test Data Generator for AIE Matrix Element Verification" << endl;
    cout << "=============================================================================" << endl;
    cout << " Number of events: " << n_events << endl;
    cout << " Random seed:      " << seed << endl;
    cout << " CM energy:        " << energy << " GeV" << endl;
    cout << " Output prefix:    " << output_prefix << endl;
    cout << "=============================================================================" << endl;

    // Initialize the process
    CPPProcess process;
    process.initProc("../../Cards/param_card.dat");

    // Set the random seed for RAMBO
    set_rambo_seed(seed, 9373);

    // Open output files
    ofstream momenta_file(output_prefix + "_momenta.txt");
    ofstream results_file(output_prefix + "_results.txt");
    ofstream binary_file(output_prefix + "_momenta.bin", ios::binary);

    if (!momenta_file || !results_file || !binary_file) {
        cerr << "Error: Could not open output files" << endl;
        return 1;
    }

    // Set precision for text output
    momenta_file << scientific << setprecision(17);
    results_file << scientific << setprecision(17);

    cout << "\nGenerating " << n_events << " events..." << endl;

    for (int evt = 0; evt < n_events; evt++) {
        double weight;

        // Generate phase space point
        vector<double*> p = get_momenta(process.ninitial, energy,
                                       process.getMasses(), weight);

        // Set momenta for this event
        process.setMomenta(p);

        // Calculate matrix element
        process.sigmaKin();

        const double* matrix_elements = process.getMatrixElements();

        // Write to text files
        momenta_file << "# Event " << evt << endl;
        momenta_file << "# Weight: " << weight << endl;
        for (int i = 0; i < process.nexternal; i++) {
            momenta_file << i << " "
                        << p[i][0] << " "
                        << p[i][1] << " "
                        << p[i][2] << " "
                        << p[i][3] << endl;
        }
        momenta_file << endl;

        results_file << evt << " " << matrix_elements[0] << " " << weight << endl;

        // Write binary format for AIE (all momenta as float32)
        for (int i = 0; i < process.nexternal; i++) {
            for (int j = 0; j < 4; j++) {
                float val = static_cast<float>(p[i][j]);
                binary_file.write(reinterpret_cast<const char*>(&val), sizeof(float));
            }
        }

        // Progress indicator
        if ((evt + 1) % 100 == 0 || evt == 0) {
            cout << "  Event " << evt + 1 << "/" << n_events
                 << " - ME = " << matrix_elements[0] << endl;
        }

        // Clean up momenta
        for (int i = 0; i < process.nexternal; i++) {
            delete[] p[i];
        }
    }

    momenta_file.close();
    results_file.close();
    binary_file.close();

    cout << "\n=============================================================================" << endl;
    cout << " Data generation complete!" << endl;
    cout << " Output files:" << endl;
    cout << "   " << output_prefix << "_momenta.txt  - Human-readable momenta" << endl;
    cout << "   " << output_prefix << "_results.txt  - Event results (ME values)" << endl;
    cout << "   " << output_prefix << "_momenta.bin  - Binary momenta for AIE" << endl;
    cout << "=============================================================================" << endl;

    return 0;
}
