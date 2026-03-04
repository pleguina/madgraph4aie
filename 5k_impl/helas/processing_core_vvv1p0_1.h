// ============================================================================
// processing_core_vvv1p0_1.h
//
// VVV1P0_1: Off-shell triple gluon CURRENT function (returns v8c wavefunction)
// 
// This is separate from VVV1_0 because they CANNOT coexist in the same kernel.
// Combined PM would exceed 16KB tile limit.
// ============================================================================
#ifndef PROCESSING_CORE_VVV1P0_1_H
#define PROCESSING_CORE_VVV1P0_1_H

#include "processing_core_vvv.h"

// ============================================================================
// VVV1P0_1: Off-shell triple gluon current
// Returns: off-shell vector wavefunction (v8c)
// PM: ~2KB compiled
// ============================================================================
inline v8c VVV1P0_1(v8c V2, v8c V3, cfloat COUP, float M1, float W1)
{
  DEBUG_HELAS_PRINT("  VVV1P0_1: Computing off-shell triple gluon current\n");
  DEBUG_PRINT_V8C("V2", V2);
  DEBUG_PRINT_V8C("V3", V3);
  DEBUG_PRINT_CFLOAT("COUP", COUP);
  DEBUG_HELAS_PRINT("  VVV1P0_1: M1=%e, W1=%e\n", (double)M1, (double)W1);

  v8c V1 = wf8_zero();

  // Pack output momentum
  V1[0] = V2[0] + V3[0];
  V1[1] = V2[1] + V3[1];

  const v4f P1 = extractP_neg(V1);
  const v4f P2 = extractP(V2);
  const v4f P3 = extractP(V3);

  DEBUG_PRINT_V4F("P1", P1);
  DEBUG_PRINT_V4F("P2", P2);
  DEBUG_PRINT_V4F("P3", P3);

  // Denominator
  const cfloat denomci = denom(P1, M1, W1, COUP) * cI;
  DEBUG_PRINT_CFLOAT("denom*cI", denomci);

  // Metric contractions
  const cfloat TMP0 = tmp_calc_vp(V3, P1);
  const cfloat TMP2 = tmp_calc_vp(V3, P2);
  const cfloat TMP4 = tmp_calc_vp(V2, P3);
  const cfloat TMP6 = tmp_calc_vv(V3, V2);

  // Build output polarization
  const cfloat mt0pt2 = -TMP0 + TMP2;
  const cfloat pt4mt5 = tmp_calc_vp(V2, P1) - TMP4;

  const v4c pol = vvv_calc(V2, V3, P2, P3, mt0pt2, pt4mt5, TMP6, denomci);
  V1[2] = pol[0]; V1[3] = pol[1]; V1[4] = pol[2]; V1[5] = pol[3];

  DEBUG_PRINT_V8C("V1", V1);

  // Stamp cache for downstream use
  stamp_vec_i_cache(V1);
  return V1;
}

#endif // PROCESSING_CORE_VVV1P0_1_H
