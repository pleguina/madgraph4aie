// ============================================================================
// processing_core_common.h
//
// Common types, constants, and helper functions shared by all HELAS modules
// This file MUST be included by all processing_core_*.h files
// ============================================================================
#ifndef PROCESSING_CORE_COMMON_H
#define PROCESSING_CORE_COMMON_H

// Force recomputation of iV3/iV4 instead of using cached V3[6]/V3[7]
// Required because cascade streams don't populate lanes 6-7
#define FORCE_NO_LANE67_CACHE

#include "adf/intrinsics.h"
#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include <aie_api/utils.hpp>
#include <cmath>
#include "ggttg_config.h"
#include "ggttg_params.h"

// ============================================================================
// Types
// ============================================================================
using v4f = aie::vector<float, 4>;
using v4c = aie::vector<cfloat, 4>;
using v8c = aie::vector<cfloat, 8>;

// ============================================================================
// Constants
// ============================================================================
static constexpr cfloat cI  = ggttg::CI;
static constexpr cfloat mcI = ggttg::MCI;
static constexpr float  SQH = 0.70710678118654752440f; // 1/sqrt(2)

// ============================================================================
// Compiler attributes
// ============================================================================
#if defined(__GNUG__) || defined(__clang__)
  #define AIE_ALWAYS_INLINE inline __attribute__((always_inline))
  #define AIE_NOINLINE __attribute__((noinline))
#else
  #define AIE_ALWAYS_INLINE inline
  #define AIE_NOINLINE
#endif

// ============================================================================
// Debug macros (enabled in simulation only)
// ============================================================================
#if defined(__X86SIM__) || defined(__AIESIM__)
  #define DEBUG_HELAS_PRINT(...) printf(__VA_ARGS__)
  #define DEBUG_PRINT_V8C(name, v) \
    do { \
      cfloat _c[6]; \
      for (int _i = 0; _i < 6; _i++) _c[_i] = (v)[_i]; \
      printf("  %s=[(%e,%e),(%e,%e),(%e,%e),(%e,%e),(%e,%e),(%e,%e)]\n", name, \
        (double)_c[0].real, (double)_c[0].imag, (double)_c[1].real, (double)_c[1].imag, \
        (double)_c[2].real, (double)_c[2].imag, (double)_c[3].real, (double)_c[3].imag, \
        (double)_c[4].real, (double)_c[4].imag, (double)_c[5].real, (double)_c[5].imag); \
    } while(0)
  #define DEBUG_PRINT_V4F(name, v) \
    do { \
      float _f[4]; \
      for (int _i = 0; _i < 4; _i++) _f[_i] = (v)[_i]; \
      printf("  %s=[%e,%e,%e,%e]\n", name, \
        (double)_f[0], (double)_f[1], (double)_f[2], (double)_f[3]); \
    } while(0)
  #define DEBUG_PRINT_CFLOAT(name, c) \
    printf("  %s=(%e,%e)\n", name, (double)(c).real, (double)(c).imag)
#else
  #define DEBUG_HELAS_PRINT(...)
  #define DEBUG_PRINT_V8C(name, v)
  #define DEBUG_PRINT_V4F(name, v)
  #define DEBUG_PRINT_CFLOAT(name, c)
#endif

// ============================================================================
// Scalar helper functions
// ============================================================================

// Branchless max(0,x)
AIE_ALWAYS_INLINE float max0f(float x) { return (x > 0.0f) ? x : 0.0f; }
AIE_ALWAYS_INLINE v4f   max0f(v4f x)   { return aie::max(x, aie::zeros<float,4>()); }

// Safe sqrt: clamp negatives to 0
AIE_ALWAYS_INLINE float aie_safe_sqrt(float x) { return aie::sqrt(max0f(x)); }
AIE_ALWAYS_INLINE v4f   aie_safe_sqrt(v4f x)   { return aie::sqrt(max0f(x)); }

// Safe reciprocal (0 if x==0)
AIE_ALWAYS_INLINE float aie_safe_inv(float x) { return (x != 0.0f) ? aie::inv(x) : 0.0f; }
AIE_ALWAYS_INLINE v4f aie_safe_inv(v4f x) {
    v4f invx  = aie::inv(x);
    v4f zeros = aie::zeros<float,4>();
    auto m    = aie::neq(x, zeros);
    return aie::select(zeros, invx, m);
}

// Safe rsqrt (0 if x<=0)
AIE_ALWAYS_INLINE float aie_safe_rsqrt(float x) { return (x > 0.0f) ? aie::inv(aie::sqrt(x)) : 0.0f; }
AIE_ALWAYS_INLINE v4f aie_safe_rsqrt(v4f x) {
    v4f z   = aie::zeros<float,4>();
    v4f cl  = aie::max(x, z);
    v4f r   = aie::inv(aie::sqrt(cl));
    auto m  = aie::gt(cl, z);
    return aie::select(z, r, m);
}

// Zero an 8-lane wf vector
AIE_ALWAYS_INLINE v8c wf8_zero() { return aie::zeros<cfloat, 8>(); }

// ============================================================================
// Momentum extraction/packing
// ============================================================================

// Extract 4-momentum from packed current/spinor (MG5 convention)
AIE_ALWAYS_INLINE v4f extractP(const v8c& V) {
  v4f P = aie::zeros<float,4>();
  P[0] = aie::real(V[0]);
  P[1] = aie::real(V[1]);
  P[2] = aie::imag(V[1]);
  P[3] = aie::imag(V[0]);
  return P;
}
AIE_ALWAYS_INLINE v4f extractP_neg(const v8c& V) { return aie::neg(extractP(V)); }

// Extract polarization lanes [2..5]
AIE_ALWAYS_INLINE v4c pol4(const v8c& V) { 
  return aie::shuffle_down(V, 2).template extract<4>(0); 
}

// ============================================================================
// Minkowski metric and denominators
// ============================================================================

// Minkowski p^2 with (+,-,-,-)
AIE_ALWAYS_INLINE float minkowski_p2(const v4f& P) {
  auto sq = aie::mul_square(P).to_vector<float>();
  sq[1] = aie::neg(sq[1]); sq[2] = aie::neg(sq[2]); sq[3] = aie::neg(sq[3]);
  return aie::reduce_add(sq);
}

// ============================================================================
// Optimized complex arithmetic (CRITICAL FOR AIE PERFORMANCE)
// ============================================================================

// Complex inversion: 1/(a+ib) = (a-ib)/(a²+b²)
// Uses ONLY ONE scalar reciprocal instead of aie::inv(cfloat) which uses TWO!
AIE_ALWAYS_INLINE cfloat cinv(cfloat z) {
  const float norm2 = z.real*z.real + z.imag*z.imag;
  const float inv_norm2 = aie::inv(norm2);  // ONE scalar reciprocal
  return cfloat{z.real * inv_norm2, -z.imag * inv_norm2};
}

// Complex multiplication: (a+ib)×(c+id) = (ac-bd) + i(ad+bc)
AIE_ALWAYS_INLINE cfloat cmul(cfloat a, cfloat b) {
  return cfloat{
    a.real*b.real - a.imag*b.imag,
    a.real*b.imag + a.imag*b.real
  };
}

// Breit–Wigner denominator: COUP / (p^2 - M^2 + iMW)
// OPTIMIZED: Uses explicit cinv() → ONE reciprocal instead of TWO!
AIE_ALWAYS_INLINE cfloat denom(const v4f& P, float M, float W, cfloat COUP) {
  const float  p2      = minkowski_p2(P);
  const cfloat mm_iMw  = { M * M, -M * W };
  const cfloat den     = cfloat{ p2, 0.f } - mm_iMw;
  return cmul(COUP, cinv(den));  // Explicit: COUP × (1/den)
}

// ============================================================================
// V3Comb: Precomputed linear combinations for FFV vertices
// ============================================================================
struct V3Comb {
  cfloat p2p5, m2p5, p2m5, m2m5, mci2p5;
  cfloat p3pci4, m3pci4, p3mci4, m3mci4;
  cfloat pci3p4, mci3p4, pci3m4, mci3m4;
};

// Build V3 combos - uses lanes 6-7 cache when available
static AIE_ALWAYS_INLINE V3Comb v3_make_combos(const v8c& V3) {
  V3Comb c;
  c.p2p5   = V3[2] + V3[5];
  c.m2p5   = -V3[2] + V3[5];
  c.p2m5   = -c.m2p5;
  c.m2m5   = -c.p2p5;
  c.mci2p5 = -cI * c.p2p5;

#if defined(FORCE_NO_LANE67_CACHE)
  const cfloat iV4 = cI * V3[4];
  const cfloat iV3 = cI * V3[3];
  c.p3pci4 = V3[3] + iV4;
  c.m3pci4 = -V3[3] + iV4;
  c.pci3p4 = iV3 + V3[4];
  c.mci3p4 = -iV3 + V3[4];
#else
  // Use cached lanes 6-7 (iV4, iV3)
  c.p3pci4 = V3[3] + V3[6];
  c.m3pci4 = -V3[3] + V3[6];
  c.pci3p4 = V3[7] + V3[4];
  c.mci3p4 = -V3[7] + V3[4];
#endif

  c.p3mci4 = -c.m3pci4;
  c.m3mci4 = -c.p3pci4;
  c.pci3m4 = -c.mci3p4;
  c.mci3m4 = -c.pci3p4;
  return c;
}

// ============================================================================
// Metric dot products on polarization lanes [2..5]
// ============================================================================

AIE_ALWAYS_INLINE cfloat tmp_calc_vv(const v8c& v1, const v8c& v2) {
  const v4c a = pol4(v1), b = pol4(v2);
  auto prod = aie::negmul(a, b).to_vector<cfloat>();
  prod[0] = aie::neg(prod[0]);
  return aie::reduce_add(prod);
}

AIE_ALWAYS_INLINE cfloat tmp_calc_vp(const v8c& v, const v4f& p) {
  const v4c a = pol4(v);
  auto prod = aie::negmul(a, p).to_vector<cfloat>();
  prod[0] = aie::neg(prod[0]);
  return aie::reduce_add(prod);
}

// ============================================================================
// Lane 6-7 cache stamping for vector wavefunctions
// ============================================================================
AIE_ALWAYS_INLINE void stamp_vec_i_cache(v8c& V) {
  V[6] = cI * V[4];  // iV4
  V[7] = cI * V[3];  // iV3
}

#endif // PROCESSING_CORE_COMMON_H
