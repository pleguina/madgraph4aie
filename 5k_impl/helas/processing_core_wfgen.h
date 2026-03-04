// ============================================================================
// processing_core_wfgen.h
//
// External wavefunction generators: vxxxxx, ixxxxx, oxxxxx
// These compute the external particle wavefunctions from 4-momenta
// ============================================================================
#ifndef PROCESSING_CORE_WFGEN_H
#define PROCESSING_CORE_WFGEN_H

#include "processing_core_common.h"

// ============================================================================
// MASSIVE INCOMING FERMION
// ============================================================================
static inline v8c ixxxxx(const v4f& P, float fmass, int nhel, int nsf) {
  v8c fi = wf8_zero();
  const float p0=P[0], p1=P[1], p2=P[2], p3=P[3];

  // momentum packing (MG convention)
  fi[0] = { -p0 * nsf, -p3 * nsf };
  fi[1] = { -p1 * nsf, -p2 * nsf };

  const int nh = nhel * nsf;

  if (fmass != 0.f) {
    const float pt2  = p1*p1 + p2*p2;
    const float pabs = aie_safe_sqrt(pt2 + p3*p3);
    const float pp   = (p0 < pabs) ? p0 : pabs;

    if (pp == 0.f) {
      const float sqm0 = aie_safe_sqrt(fabsf(fmass));
      const float sqm1 = (fmass < 0.f) ? -sqm0 : sqm0;
      const int ip = (1 + nh) >> 1;
      const int im = (1 - nh) >> 1;
      fi[2] = { ip * (ip ? sqm1 : sqm0), 0.f };
      fi[3] = { im * nsf * (ip ? sqm1 : sqm0), 0.f };
      fi[4] = { ip * nsf * (im ? sqm1 : sqm0), 0.f };
      fi[5] = { im * (im ? sqm1 : sqm0), 0.f };
    } else {
      const float sf0 = 0.5f * (1.f + nsf + (1 - nsf)*nh);
      const float sf1 = 0.5f * (1.f + nsf - (1 - nsf)*nh);

      const float omega0 = aie_safe_sqrt(p0 + pp);
      const float omega1 = fmass * aie_safe_inv(omega0);
      const int   ip = (1 + nh) >> 1;
      const int   im = (1 - nh) >> 1;
      const float sfomega0 = sf0 * (ip ? omega1 : omega0);
      const float sfomega1 = sf1 * (im ? omega1 : omega0);

      const float pp3 = max0f(pp + p3);
      // OPTIMIZED: Hoist invpp outside conditional - compute ONCE
      const float invpp = aie_safe_inv(pp);
      if (pp3 == 0.f) {
        const cfloat chi0 = { 0.f, 0.f };
        const cfloat chi1 = { (float)(-nh), 0.f };
        const int ip = (1 + nh) >> 1;
        const int im = (1 - nh) >> 1;
        const cfloat chi_im = (im == 0) ? chi0 : chi1;
        const cfloat chi_ip = (ip == 0) ? chi0 : chi1;
        fi[2] = chi_im * sfomega0;  fi[3] = chi_ip * sfomega0;
        fi[4] = chi_im * sfomega1;  fi[5] = chi_ip * sfomega1;
      } else {
        const float rs    = aie_safe_rsqrt(2.f * pp * pp3);
        const cfloat chi0 = { aie_safe_sqrt(0.5f * pp3 * invpp), 0.f };
        const cfloat chi1 = { nh * p1 * rs, p2 * rs };
        const int ip = (1 + nh) >> 1;
        const int im = (1 - nh) >> 1;
        const cfloat chi_im = (im == 0) ? chi0 : chi1;
        const cfloat chi_ip = (ip == 0) ? chi0 : chi1;
        fi[2] = chi_im * sfomega0;  fi[3] = chi_ip * sfomega0;
        fi[4] = chi_im * sfomega1;  fi[5] = chi_ip * sfomega1;
      }
    }
  } else {
    // massless
    const bool  special = (p1==0.f) && (p2==0.f) && (p3<0.f);
    const float sqp0p3  = special ? 0.f : (aie_safe_sqrt(max0f(p0 + p3)) * nsf);

    const cfloat chi0 = { sqp0p3, 0.f };
    cfloat chi1;
    if (sqp0p3 == 0.f) {
      chi1 = { (float)(-nhel) * aie_safe_sqrt(2.f * p0), 0.f };
    } else {
      const float r = aie_safe_inv(sqp0p3);
      chi1 = { (float)((nhel*nsf) * p1) * r, (float)nsf * p2 * r };
    }

    if (nh == 1) {
      fi[2] = {0.f,0.f}; fi[3] = {0.f,0.f};
      fi[4] = chi0;      fi[5] = chi1;
    } else {
      fi[2] = chi1;      fi[3] = chi0;
      fi[4] = {0.f,0.f}; fi[5] = {0.f,0.f};
    }
  }
  return fi;
}

// ============================================================================
// MASSIVE OUTGOING FERMION
// ============================================================================
static inline v8c oxxxxx(const v4f& P, float fmass, int nhel, int nsf) {
  v8c fo = wf8_zero();
  const float p0=P[0], p1=P[1], p2=P[2], p3=P[3];

  fo[0] = {  p0 * nsf,  p3 * nsf };
  fo[1] = {  p1 * nsf,  p2 * nsf };

  const int nh = nhel * nsf;

  if (fmass != 0.f) {
    const float pt2  = p1*p1 + p2*p2;
    const float pabs = aie_safe_sqrt(pt2 + p3*p3);
    const float pp   = (p0 < pabs) ? p0 : pabs;

    if (pp == 0.f) {
      const float sqm0 = aie_safe_sqrt(fabsf(fmass));
      const float sqm1 = (fmass < 0.f) ? -sqm0 : sqm0;
      const int ip = -((1 - nh)/2) * nhel;
      const int im =  ((1 + nh)/2) * nhel;
      // OPTIMIZED: Avoid std::abs() to reduce PMEM
      // ip, im are 0 or ±1, so |ip| = (ip != 0) ? 1 : 0
      const int abs_ip = (ip < 0) ? -ip : ip;
      const int abs_im = (im < 0) ? -im : im;
      const float a_ip = abs_ip ? sqm1 : sqm0;
      const float a_im = abs_im ? sqm1 : sqm0;
      fo[2] = { (float)im * a_ip, 0.f };
      fo[3] = { (float)ip * nsf * a_ip, 0.f };
      fo[4] = { (float)im * nsf * a_im, 0.f };
      fo[5] = { (float)ip * a_im, 0.f };
    } else {
      const float sf0 = 0.5f * (1.f + nsf + (1 - nsf)*nh);
      const float sf1 = 0.5f * (1.f + nsf - (1 - nsf)*nh);

      const float omega0 = aie_safe_sqrt(p0 + pp);
      const float omega1 = fmass * aie_safe_inv(omega0);
      const int ip = (1 + nh) >> 1;
      const int im = (1 - nh) >> 1;
      const float sfomeg0 = sf0 * (ip ? omega1 : omega0);
      const float sfomeg1 = sf1 * (im ? omega1 : omega0);

      const float pp3 = max0f(pp + p3);
      // OPTIMIZED: Hoist invpp outside conditional - compute ONCE
      const float invpp = aie_safe_inv(pp);
      if (pp3 == 0.f) {
        const cfloat chi0 = {0.f,0.f};
        const cfloat chi1 = { (float)(-nh), 0.f };
        const cfloat chi_im = (im == 0) ? chi0 : chi1;
        const cfloat chi_ip = (ip == 0) ? chi0 : chi1;
        fo[2] = chi_im * sfomeg1;  fo[3] = chi_ip * sfomeg1;
        fo[4] = chi_im * sfomeg0;  fo[5] = chi_ip * sfomeg0;
      } else {
        const float rs    = aie_safe_rsqrt(2.f * pp * pp3);
        const cfloat chi0 = { aie_safe_sqrt(0.5f * pp3 * invpp), 0.f };
        const cfloat chi1 = { (float)(nh * p1) * rs, (float)(-p2) * rs };
        const cfloat chi_im = (im == 0) ? chi0 : chi1;
        const cfloat chi_ip = (ip == 0) ? chi0 : chi1;
        fo[2] = chi_im * sfomeg1;  fo[3] = chi_ip * sfomeg1;
        fo[4] = chi_im * sfomeg0;  fo[5] = chi_ip * sfomeg0;
      }
    }
  } else {
    // massless
    const cfloat chi0 = { aie_safe_sqrt(max0f(p0 + p3)), 0.f };
    const float  r    = (chi0.real != 0.f) ? aie_safe_inv(chi0.real) : 0.f;
    const cfloat chi1 = (chi0.real == 0.f) ? cfloat{0.f,0.f}
                                           : cfloat{ nh * p1 * r, -p2 * r };
    if (nh == 1) {
      fo[2] = chi0;      fo[3] = chi1;
      fo[4] = {0.f,0.f}; fo[5] = {0.f,0.f};
    } else {
      fo[2] = {0.f,0.f}; fo[3] = {0.f,0.f};
      fo[4] = chi1;      fo[5] = chi0;
    }
  }
  return fo;
}

// ============================================================================
// VECTOR BOSON (gluon/photon)
// ============================================================================
static inline v8c vxxxxx(const v4f& P, float vmass, int nhel, int nsv) {
  v8c vc = wf8_zero();
  const float p0=P[0], p1=P[1], p2=P[2], p3=P[3];
  const float hel = (float)nhel;

  vc[0] = {  p0 * nsv,  p3 * nsv };
  vc[1] = {  p1 * nsv,  p2 * nsv };

  if (vmass != 0.f) {
    // OPTIMIZED: Avoid std::abs() to reduce PMEM
    // nhel is ±1, so |nhel| = (nhel < 0) ? -nhel : nhel
    const int abs_nhel = (nhel < 0) ? -nhel : nhel;
    const int   nsvahl = nsv * abs_nhel;
    const float pt2    = p1*p1 + p2*p2;
    const float pabs   = aie_safe_sqrt(pt2 + p3*p3);
    const float pp     = (p0 < pabs) ? p0 : pabs;
    const float pt     = (pp < aie_safe_sqrt(pt2)) ? pp : aie_safe_sqrt(pt2);
    // hel is float ±1.0, |hel| = (hel < 0.f) ? -hel : hel
    const float abs_hel = (hel < 0.f) ? -hel : hel;
    const float hel0    = 1.f - abs_hel;

    // OPTIMIZED: Hoist reciprocals ONCE outside all conditionals
    const float inv_vm = aie_safe_inv(vmass);
    const float rpp    = aie_safe_inv(pp);
    
    if (pp == 0.f) {
      vc[2] = {0.f,0.f};
      vc[3] = { -hel * SQH, 0.f };
      vc[4] = { 0.f, (float)nsvahl * SQH };
      vc[5] = { hel0, 0.f };
    } else {
      const float emp    = p0 * inv_vm * rpp;

      vc[2] = { hel0 * pp * inv_vm, 0.f };
      vc[5] = { hel0 * p3 * emp + hel * ((pt>0.f)? (pt * rpp) : 0.f) * SQH, 0.f };

      if (pt != 0.f) {
        const float rpt   = aie_safe_inv(pt);
        const float rpppt = rpp * rpt;
        const float pzpt  = p3 * rpppt * SQH * hel;
        vc[3] = { hel0 * p1 * emp - p1 * pzpt, (float)(-nsvahl) * p2 * rpt * SQH };
        vc[4] = { hel0 * p2 * emp - p2 * pzpt, (float)(+nsvahl) * p1 * rpt * SQH };
      } else {
        vc[3] = { -hel * SQH, 0.f };
        vc[4] = { 0.f, (float)nsvahl * ((p3 < 0.f) ? -SQH : SQH) };
      }
    }
  } else {
    // massless vector - OPTIMIZED: Hoist reciprocals ONCE
    const float pt = aie_safe_sqrt(p1*p1 + p2*p2);
    const float pp = p0;
    const float rpp = aie_safe_inv(pp);
    const float rpt = aie_safe_inv(pt);

    vc[2] = { 0.f, 0.f };
    vc[5] = { hel * ((pt>0.f)? (pt * rpp) : 0.f) * SQH, 0.f };

    if (pt != 0.f) {
      const float pzpt = p3 * (rpp * rpt) * SQH * hel;
      vc[3] = { -p1 * pzpt, (float)(-nsv) * p2 * rpt * SQH };
      vc[4] = { -p2 * pzpt, (float)(+nsv) * p1 * rpt * SQH };
    } else {
      vc[3] = { -hel * SQH, 0.f };
      vc[4] = { 0.f, (float)nsv * ((p3 < 0.f) ? -SQH : SQH) };
    }
  }
  // OPTIMIZED: Stamp lane6/7 cache for downstream FFV functions
  stamp_vec_i_cache(vc);
  return vc;
}

#endif // PROCESSING_CORE_WFGEN_H
