#ifndef PSP_GENERATOR_H
#define PSP_GENERATOR_H

#include <vector>
#include <string>
#include "rambo.h"

// Process parameters for g g > t t~ g
const int NINITIAL = 2;
const int NEXTERNAL = 5;
const int NFINAL = NEXTERNAL - NINITIAL;

// Particle masses (GeV)
const double MASS_GLUON = 0.0;
const double MASS_TOP = 173.0;  // Top quark mass

// Generate a single PSP using RAMBO
// Returns vector of 5 four-momenta (each is array of 4 doubles: E, px, py, pz)
// weight is the phase-space weight returned by RAMBO
std::vector<double*> generate_psp(double energy, double& weight);

// Convert PSP to flat float array (20 floats: 5 particles × 4 components)
// Format: [p0_E, p0_px, p0_py, p0_pz, p1_E, p1_px, p1_py, p1_pz, ...]
void psp_to_float_array(const std::vector<double*>& p, float* output);

// Free memory allocated for PSP momenta
void free_psp(std::vector<double*>& p);

#endif // PSP_GENERATOR_H
