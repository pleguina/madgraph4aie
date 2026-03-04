#ifndef RAMBO_H
#define RAMBO_H

#include <vector>

using namespace std;

class Random
{
  public:
    double ranmar();
    void rmarin(int ij, int kl);
    
  private:
    double ranu[98];
    double ranc, rancd, rancm;
    int iranmr, jranmr;
};

// Random number generator
double rn(int idummy);

// Set RAMBO random seed for reproducible testing
void set_rambo_seed(int ij, int kl);

// Get momenta for initial particles + final state
// ninitial: number of initial particles (1 or 2)
// energy: center-of-mass energy
// masses: vector of all particle masses (initial + final)
// wgt: output weight of the phase space point
vector<double*> get_momenta(int ninitial, double energy,
                            vector<double> masses, double& wgt);

// RAMBO phase space generator
// et: total energy
// xm: final state particle masses
// wt: output weight
vector<double*> rambo(double et, vector<double>& xm, double& wt);

#endif // RAMBO_H
