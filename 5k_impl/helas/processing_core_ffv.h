// ============================================================================
// processing_core_ffv.h
//
// FFV vertex functions:
//   - FFV1_0:   Fermion-fermion-vector amplitude (scalar result)
//   - FFV1_1:   Off-shell incoming fermion from (F2, V3)
//   - FFV1_2:   Off-shell outgoing fermion from (F1, V3)
//   - FFV1P0_3: Off-shell vector from (F1, F2)
// ============================================================================
#ifndef PROCESSING_CORE_FFV_H
#define PROCESSING_CORE_FFV_H

#include "processing_core_common.h"

// ============================================================================
// FFV temporary calculation (MG5 "TMP9")
// ============================================================================
static inline cfloat tmp9_calc(const v8c& F1, const v8c& F2, const v8c& V3) {
  const V3Comb C = v3_make_combos(V3);

  v4c f42, f53, t1, t2, F1v;
  f42[0]=F2[4]; f42[1]=F2[4]; f42[2]=F2[2]; f42[3]=F2[2];
  f53[0]=F2[5]; f53[1]=F2[5]; f53[2]=F2[3]; f53[3]=F2[3];
  F1v[0]=F1[2]; F1v[1]=F1[3]; F1v[2]=F1[4]; F1v[3]=F1[5];

  t1[0]=C.p2p5;   t1[1]=C.p3mci4; t1[2]=C.p2m5;   t1[3]=C.m3pci4;
  t2[0]=C.p3pci4; t2[1]=C.p2m5;   t2[2]=C.m3mci4; t2[3]=C.p2p5;

  auto acc = aie::mul(f42, t1);
  acc      = aie::mac(acc, f53, t2);

  auto mul = aie::mul(F1v, acc.to_vector<cfloat>(0));
  
#if defined(__X86SIM__) || defined(__AIESIM__)
  static int debug_counter = 0;
  if (debug_counter < 1) {
    cfloat result = aie::reduce_add(mul.to_vector<cfloat>(0));
    cfloat f1_2 = F1[2], f1_3 = F1[3];
    cfloat f2_2 = F2[2], f2_4 = F2[4];
    cfloat v3_2 = V3[2], v3_3 = V3[3];
    printf("[TMP9_DEBUG] F1[2]=(%.6e,%.6e) F1[3]=(%.6e,%.6e)\n", 
           (double)f1_2.real, (double)f1_2.imag, (double)f1_3.real, (double)f1_3.imag);
    printf("[TMP9_DEBUG] F2[2]=(%.6e,%.6e) F2[4]=(%.6e,%.6e)\n",
           (double)f2_2.real, (double)f2_2.imag, (double)f2_4.real, (double)f2_4.imag);
    printf("[TMP9_DEBUG] V3[2]=(%.6e,%.6e) V3[3]=(%.6e,%.6e)\n",
           (double)v3_2.real, (double)v3_2.imag, (double)v3_3.real, (double)v3_3.imag);
    printf("[TMP9_DEBUG] C.p2p5=(%.6e,%.6e) C.p3pci4=(%.6e,%.6e)\n",
           (double)C.p2p5.real, (double)C.p2p5.imag, (double)C.p3pci4.real, (double)C.p3pci4.imag);
    printf("[TMP9_DEBUG] TMP9=(%.10e,%.10e)\n", (double)result.real, (double)result.imag);
    debug_counter++;
  }
#endif
  
  return aie::reduce_add(mul.to_vector<cfloat>(0));
}

// ============================================================================
// FFV1_0: Fermion-fermion-vector amplitude
// Returns: scalar amplitude (cfloat)
// ============================================================================
static inline cfloat FFV1_0(v8c F1, v8c F2, v8c V3, cfloat COUP)
{
  DEBUG_HELAS_PRINT("  FFV1_0: Computing FFV amplitude\n");
  DEBUG_PRINT_V8C("F1", F1);
  DEBUG_PRINT_V8C("F2", F2);
  DEBUG_PRINT_V8C("V3", V3);
  DEBUG_PRINT_CFLOAT("COUP", COUP);

  const cfloat TMP9 = tmp9_calc(F1, F2, V3);
  const cfloat vertex = COUP * (-cI) * TMP9;
  
  DEBUG_PRINT_CFLOAT("vertex", vertex);
  return vertex;
}

// ============================================================================
// FFV1_1 core: Linear combinations for incoming fermion (F2,V3) -> F1
// ============================================================================

// Debug flag - set externally before calling FFV1_1
static bool g_ffv1_debug = false;

static inline
v4c ffv_linear_core(const v4c& f24, const v4c& f35,
                    const v4f& P, const v8c& V3,
                    cfloat denomci, float M,
                    const V3Comb& C)
{
  v4c dsgn;
  dsgn[0]=denomci; dsgn[1]=-denomci; dsgn[2]=-denomci; dsgn[3]=denomci;

#if defined(__X86SIM__) || defined(__AIESIM__)
  if (g_ffv1_debug) {
    printf("[FFV_CORE] denomci = (%.10e, %.10e)\n", (double)denomci.real, (double)denomci.imag);
    printf("[FFV_CORE] M = %.10e\n", (double)M);
    printf("[FFV_CORE] P = [%.10e, %.10e, %.10e, %.10e]\n", (double)P[0], (double)P[1], (double)P[2], (double)P[3]);
    for (int i = 0; i < 4; i++) { cfloat c = f24.get(i); printf("[FFV_CORE] f24[%d] = (%.10e, %.10e)\n", i, (double)c.real, (double)c.imag); }
    for (int i = 0; i < 4; i++) { cfloat c = f35.get(i); printf("[FFV_CORE] f35[%d] = (%.10e, %.10e)\n", i, (double)c.real, (double)c.imag); }
    printf("[FFV_CORE] V3Comb: p2p5=(%.6e,%.6e) m2p5=(%.6e,%.6e) p2m5=(%.6e,%.6e) m2m5=(%.6e,%.6e)\n",
           (double)C.p2p5.real, (double)C.p2p5.imag, (double)C.m2p5.real, (double)C.m2p5.imag,
           (double)C.p2m5.real, (double)C.p2m5.imag, (double)C.m2m5.real, (double)C.m2m5.imag);
    printf("[FFV_CORE] V3Comb: p3pci4=(%.6e,%.6e) m3pci4=(%.6e,%.6e) p3mci4=(%.6e,%.6e) m3mci4=(%.6e,%.6e)\n",
           (double)C.p3pci4.real, (double)C.p3pci4.imag, (double)C.m3pci4.real, (double)C.m3pci4.imag,
           (double)C.p3mci4.real, (double)C.p3mci4.imag, (double)C.m3mci4.real, (double)C.m3mci4.imag);
  }
#endif

  const v4c f42 = aie::reverse(f24);
  const v4c f53 = aie::reverse(f35);

  v4c t1, t2, t3, t4;

  // ---- acc1 = P0*t1 + P1*t2 + P2*t3 + P3*t4 (MATCHES BASELINE)
  t1[0]=C.m2p5;    t1[1]=C.m3pci4; t1[2]=C.p2p5;    t1[3]=C.m3pci4;
  t2[0]=C.p3mci4;  t2[1]=C.p2m5;   t2[2]=C.m3pci4;  t2[3]=C.p2p5;
  t3[0]=C.pci3p4;  t3[1]= cI*C.m2p5; t3[2]=C.mci3m4; t3[3]=C.mci2p5;
  t4[0]=C.m2p5;    t4[1]=C.p3mci4; t4[2]=C.m2m5;    t4[3]=C.m3pci4;

  auto acc1 = aie::mul(P[0], t1);
  acc1      = aie::mac(acc1, P[1], t2);
  acc1      = aie::mac(acc1, P[2], t3);
  acc1      = aie::mac(acc1, P[3], t4);

#if defined(__X86SIM__) || defined(__AIESIM__)
  if (g_ffv1_debug) {
    auto acc1_v = acc1.to_vector<cfloat>(0);
    for (int i = 0; i < 4; i++) { cfloat c = acc1_v.get(i); printf("[FFV_CORE] acc1[%d] = (%.10e, %.10e)\n", i, (double)c.real, (double)c.imag); }
  }
#endif

  // ---- acc2 (MATCHES BASELINE)
  t1[0]=C.p3pci4;  t1[1]=C.p2p5;   t1[2]=C.p3pci4;  t1[3]=C.m2p5;
  t2[0]=C.m2m5;    t2[1]=C.m3mci4; t2[2]=C.m2p5;    t2[3]=C.p3pci4;
  t3[0]=C.mci2p5;  t3[1]=C.pci3m4; t3[2]= cI*C.m2p5; t3[3]=C.mci3p4;
  t4[0]=C.p3pci4;  t4[1]=C.m2m5;   t4[2]=C.m3mci4;  t4[3]=C.m2p5;

  auto acc2 = aie::mul(P[0], t1);
  acc2      = aie::mac(acc2, P[1], t2);
  acc2      = aie::mac(acc2, P[2], t3);
  acc2      = aie::mac(acc2, P[3], t4);

#if defined(__X86SIM__) || defined(__AIESIM__)
  if (g_ffv1_debug) {
    auto acc2_v = acc2.to_vector<cfloat>(0);
    for (int i = 0; i < 4; i++) { cfloat c = acc2_v.get(i); printf("[FFV_CORE] acc2[%d] = (%.10e, %.10e)\n", i, (double)c.real, (double)c.imag); }
  }
#endif

  // acc3 (mass terms)
  t1[0]=C.p2p5;    t1[1]=C.m3pci4; t1[2]=C.m2p5;    t1[3]=C.m3pci4;
  t2[0]=C.p3pci4;  t2[1]=C.m2p5;   t2[2]=C.p3pci4;  t2[3]=C.p2p5;

  auto acc3 = aie::mul(aie::reverse(f24), t1);
  acc3      = aie::mac(acc3, aie::reverse(f35), t2);

#if defined(__X86SIM__) || defined(__AIESIM__)
  if (g_ffv1_debug) {
    auto acc3_v = acc3.to_vector<cfloat>(0);
    for (int i = 0; i < 4; i++) { cfloat c = acc3_v.get(i); printf("[FFV_CORE] acc3[%d] = (%.10e, %.10e)\n", i, (double)c.real, (double)c.imag); }
  }
#endif

  // Combine
  auto v    = acc1.to_vector<cfloat>(0);
  auto acc4 = aie::mul(f24, v);
  v         = acc2.to_vector<cfloat>(0);
  acc4      = aie::mac(acc4, f35, v);
  v         = acc3.to_vector<cfloat>(0);
  acc4      = aie::mac(acc4, M, v);

#if defined(__X86SIM__) || defined(__AIESIM__)
  if (g_ffv1_debug) {
    auto acc4_v = acc4.to_vector<cfloat>(0);
    for (int i = 0; i < 4; i++) { cfloat c = acc4_v.get(i); printf("[FFV_CORE] acc4[%d] = (%.10e, %.10e)\n", i, (double)c.real, (double)c.imag); }
  }
#endif

  auto result = aie::mul(dsgn, acc4.to_vector<cfloat>(0)).to_vector<cfloat>();
  
#if defined(__X86SIM__) || defined(__AIESIM__)
  if (g_ffv1_debug) {
    for (int i = 0; i < 4; i++) { cfloat c = result.get(i); printf("[FFV_CORE] result[%d] = (%.10e, %.10e)\n", i, (double)c.real, (double)c.imag); }
  }
#endif

  return result;
}

// ============================================================================
// FFV1_2 core: Linear combinations for outgoing fermion (F1,V3) -> F2
// Different coefficients from FFV1_1!
// ============================================================================
static inline
v4c ffv2_linear_core(const v4c& f24, const v4c& f35,
                     const v4f& P, const v8c& V3,
                     cfloat denomci, float M,
                     const V3Comb& C)
{
  v4c dsgn;
  dsgn[0]=denomci; dsgn[1]=-denomci; dsgn[2]=-denomci; dsgn[3]=denomci;

  v4c t1, t2, t3, t4;

  // acc1 for f24
  t1[0]=C.p2p5;    t1[1]=C.m3mci4;  t1[2]=C.m2p5;    t1[3]=C.m3mci4;
  t2[0]=C.m3mci4;  t2[1]=C.p2p5;    t2[2]=C.p3pci4;  t2[3]=C.p2m5;
  t3[0]=C.pci3m4;  t3[1]=cI*C.p2p5; t3[2]=C.mci3p4;  t3[3]=cI*C.p2m5;
  t4[0]=C.m2m5;    t4[1]=C.m3mci4;  t4[2]=C.m2p5;    t4[3]=C.p3pci4;

  auto acc1 = aie::mul(P[0], t1);
  acc1      = aie::mac(acc1, P[1], t2);
  acc1      = aie::mac(acc1, P[2], t3);
  acc1      = aie::mac(acc1, P[3], t4);

  // acc2 for f35
  t1[0]=C.p3mci4;  t1[1]=C.m2p5;    t1[2]=C.p3mci4;  t1[3]=C.p2p5;
  t2[0]=C.m2p5;    t2[1]=C.p3mci4;  t2[2]=C.m2m5;    t2[3]=C.m3pci4;
  t3[0]=cI*C.p2m5; t3[1]=C.pci3p4;  t3[2]=cI*C.p2p5; t3[3]=C.mci3m4;
  t4[0]=C.m3pci4;  t4[1]=C.m2p5;    t4[2]=C.p3mci4;  t4[3]=C.m2m5;

  auto acc2 = aie::mul(P[0], t1);
  acc2      = aie::mac(acc2, P[1], t2);
  acc2      = aie::mac(acc2, P[2], t3);
  acc2      = aie::mac(acc2, P[3], t4);

  // acc3 (mass terms - REVERSED pattern from FFV1_1)
  t1[0]=C.p2m5;    t1[1]=C.p3pci4;  t1[2]=C.m2m5;    t1[3]=C.p3pci4;
  t2[0]=C.m3pci4;  t2[1]=C.m2m5;    t2[2]=C.m3pci4;  t2[3]=C.p2m5;

  v4c f42_mass, f53_mass;
  f42_mass[0]=f24[2]; f42_mass[1]=f24[2]; f42_mass[2]=f24[0]; f42_mass[3]=f24[0];
  f53_mass[0]=f35[2]; f53_mass[1]=f35[2]; f53_mass[2]=f35[0]; f53_mass[3]=f35[0];

  auto acc3 = aie::mul(f42_mass, t1);
  acc3      = aie::mac(acc3, f53_mass, t2);

  // Combine
  auto v    = acc1.to_vector<cfloat>(0);
  auto acc4 = aie::mul(f24, v);
  v         = acc2.to_vector<cfloat>(0);
  acc4      = aie::mac(acc4, f35, v);
  v         = acc3.to_vector<cfloat>(0);
  acc4      = aie::mac(acc4, M, v);

  return aie::mul(dsgn, acc4.to_vector<cfloat>(0)).to_vector<cfloat>();
}

// ============================================================================
// FFV1_1: Off-shell incoming fermion from (F2, V3)
// PM: ~3-4KB compiled
// ============================================================================
inline v8c FFV1_1(v8c F2, v8c V3, cfloat COUP, float M1, float W1)
{
  DEBUG_HELAS_PRINT("  FFV1_1: Input F2, V3\n");
  DEBUG_PRINT_V8C("F2", F2);
  DEBUG_PRINT_V8C("V3", V3);
  
  v8c F1 = wf8_zero();
  F1[0] = F2[0] + V3[0];
  F1[1] = F2[1] + V3[1];

  const v4f    P1      = extractP_neg(F1);
  const cfloat denomci = denom(P1, M1, W1, COUP) * cI;
  const V3Comb C       = v3_make_combos(V3);

#if defined(__X86SIM__) || defined(__AIESIM__)
  if (g_ffv1_debug) {
    cfloat c0 = F1.get(0); cfloat c1 = F1.get(1);
    printf("[FFV1_1] F1[0]=(%.10e,%.10e) F1[1]=(%.10e,%.10e)\n", 
           (double)c0.real, (double)c0.imag, (double)c1.real, (double)c1.imag);
    printf("[FFV1_1] P1=[%.10e, %.10e, %.10e, %.10e]\n", (double)P1[0], (double)P1[1], (double)P1[2], (double)P1[3]);
    printf("[FFV1_1] COUP=(%.10e,%.10e) M1=%.10e W1=%.10e\n", (double)COUP.real, (double)COUP.imag, (double)M1, (double)W1);
    printf("[FFV1_1] denomci=(%.10e,%.10e)\n", (double)denomci.real, (double)denomci.imag);
    // Check if V3[6] and V3[7] have cached values or are zero
    cfloat v6 = V3.get(6); cfloat v7 = V3.get(7);
    cfloat v3 = V3.get(3); cfloat v4 = V3.get(4);
    cfloat expected_v6 = cI * v4;  // should be iV4
    cfloat expected_v7 = cI * v3;  // should be iV3
    printf("[FFV1_1] V3[6]=(%.10e,%.10e) expected_iV4=(%.10e,%.10e) MATCH=%d\n",
           (double)v6.real, (double)v6.imag, (double)expected_v6.real, (double)expected_v6.imag,
           (fabs(v6.real - expected_v6.real) < 1e-6 && fabs(v6.imag - expected_v6.imag) < 1e-6) ? 1 : 0);
    printf("[FFV1_1] V3[7]=(%.10e,%.10e) expected_iV3=(%.10e,%.10e) MATCH=%d\n",
           (double)v7.real, (double)v7.imag, (double)expected_v7.real, (double)expected_v7.imag,
           (fabs(v7.real - expected_v7.real) < 1e-6 && fabs(v7.imag - expected_v7.imag) < 1e-6) ? 1 : 0);
  }
#endif

  v4c f24, f35;
  f24[0]=F2[2]; f24[1]=F2[2]; f24[2]=F2[4]; f24[3]=F2[4];
  f35[0]=F2[3]; f35[1]=F2[3]; f35[2]=F2[5]; f35[3]=F2[5];

  const v4c pol = ffv_linear_core(f24, f35, P1, V3, denomci, M1, C);
  F1[2]=pol[0]; F1[3]=pol[1]; F1[4]=pol[2]; F1[5]=pol[3];
  
#if defined(__X86SIM__) || defined(__AIESIM__)
  if (g_ffv1_debug) {
    printf("[FFV1_1] OUTPUT F1:\n");
    for (int i = 0; i < 6; i++) { cfloat c = F1.get(i); printf("[FFV1_1] F1[%d] = (%.10e, %.10e)\n", i, (double)c.real, (double)c.imag); }
  }
#endif

  DEBUG_PRINT_V8C("F1", F1);
  return F1;
}

// ============================================================================
// FFV1_2: Off-shell outgoing fermion from (F1, V3)
// PM: ~3-4KB compiled
// ============================================================================
inline v8c FFV1_2(v8c F1, v8c V3, cfloat COUP, float M2, float W2)
{
  DEBUG_HELAS_PRINT("  FFV1_2: Input F1, V3\n");
  DEBUG_PRINT_V8C("F1", F1);
  DEBUG_PRINT_V8C("V3", V3);
  
  v8c F2 = wf8_zero();
  F2[0] = F1[0] + V3[0];
  F2[1] = F1[1] + V3[1];

  const v4f    P2      = extractP_neg(F2);
  const cfloat denomci = denom(P2, M2, W2, COUP) * cI;
  const V3Comb C       = v3_make_combos(V3);

  v4c f24, f35;
  f24[0]=F1[2]; f24[1]=F1[2]; f24[2]=F1[4]; f24[3]=F1[4];
  f35[0]=F1[3]; f35[1]=F1[3]; f35[2]=F1[5]; f35[3]=F1[5];

  const v4c pol = ffv2_linear_core(f24, f35, P2, V3, denomci, M2, C);
  F2[2]=pol[0]; F2[3]=pol[1]; F2[4]=pol[2]; F2[5]=pol[3];
  
  DEBUG_PRINT_V8C("F2", F2);
  return F2;
}

// ============================================================================
// FFV1P0_3: Off-shell vector from (F1, F2)
// PM: ~2-3KB compiled
// ============================================================================
inline v8c FFV1P0_3(v8c F1, v8c F2, cfloat COUP, float M3, float W3)
{
  DEBUG_HELAS_PRINT("  FFV1P0_3: Input F1, F2\n");
  DEBUG_PRINT_V8C("F1", F1);
  DEBUG_PRINT_V8C("F2", F2);
  
  v8c V3 = wf8_zero();

  V3[0] = F1[0] + F2[0];
  V3[1] = F1[1] + F2[1];

  const v4f P3         = extractP_neg(V3);
  const cfloat denomci = denom(P3, M3, W3, COUP) * cI;

  V3[2] = -denomci * ( F1[2]*F2[4] + F1[3]*F2[5] + F1[4]*F2[2] + F1[5]*F2[3] );
  V3[3] = -denomci * (-F1[2]*F2[5] - F1[3]*F2[4] + F1[4]*F2[3] + F1[5]*F2[2] );
  V3[4] = -denomci *  cI * (-(F1[2]*F2[5] + F1[5]*F2[2]) + (F1[3]*F2[4] + F1[4]*F2[3]) );
  V3[5] = -denomci * (-F1[2]*F2[4] - F1[5]*F2[3] + F1[3]*F2[5] + F1[4]*F2[2] );
  
  DEBUG_PRINT_V8C("V3", V3);
  stamp_vec_i_cache(V3);
  return V3;
}

#endif // PROCESSING_CORE_FFV_H
