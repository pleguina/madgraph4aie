#include "psp_generator.h"
#include "rambo.h"
#include <vector>

using namespace std;

// Generate a single PSP using RAMBO
vector<double*> generate_psp(double energy, double& weight) {
    // Set particle masses: 2 gluons (incoming) + t + t~ + g (outgoing)
    vector<double> masses;
    masses.push_back(MASS_GLUON);  // g
    masses.push_back(MASS_GLUON);  // g
    masses.push_back(MASS_TOP);    // t
    masses.push_back(MASS_TOP);    // t~
    masses.push_back(MASS_GLUON);  // g

    // Generate phase space point using RAMBO
    return get_momenta(NINITIAL, energy, masses, weight);
}

// Convert PSP to flat float array (20 floats)
void psp_to_float_array(const vector<double*>& p, float* output) {
    int idx = 0;
    for (int i = 0; i < NEXTERNAL; i++) {
        for (int j = 0; j < 4; j++) {
            output[idx++] = static_cast<float>(p[i][j]);
        }
    }
}

// Free memory allocated for PSP momenta
void free_psp(vector<double*>& p) {
    for (size_t i = 0; i < p.size(); i++) {
        delete[] p[i];
    }
    p.clear();
}
