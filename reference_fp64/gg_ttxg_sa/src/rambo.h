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


double rn(int idummy);

// Call rn_seed(ij, kl) before any get_momenta()/rambo() invocation to make
// the RAMBO PSP sequence reproducible (0 <= ij <= 31328 ; 0 <= kl <= 30081).
void rn_seed(int ij, int kl);

vector<double*> get_momenta(int ninitial, double energy, 
				    vector<double> masses, double& wgt);

vector<double*> rambo(double et, vector<double>& xm, double& wt);
