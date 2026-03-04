// ============================================================================
// processing_core_vvv1_0.h
//
// VVV1_0: Triple gluon AMPLITUDE function (returns scalar cfloat)
// 
// This is separate from VVV1P0_1 because they CANNOT coexist in the same kernel.
// Combined PM would exceed 16KB tile limit.
// ============================================================================
#ifndef PROCESSING_CORE_VVV1_0_H
#define PROCESSING_CORE_VVV1_0_H

#include "processing_core_vvv.h"

// ============================================================================
// VVV1_0: Triple gluon amplitude
// Returns: scalar amplitude (cfloat)
// PM: ~2KB compiled
// ============================================================================
inline cfloat VVV1_0(v8c V1, v8c V2, v8c V3, cfloat COUP)
{
  DEBUG_HELAS_PRINT("  VVV1_0: Computing triple gluon amplitude\n");
  DEBUG_PRINT_V8C("V1", V1);
  DEBUG_PRINT_V8C("V2", V2);
  DEBUG_PRINT_V8C("V3", V3);
  DEBUG_PRINT_CFLOAT("COUP", COUP);

  const v4f P1 = extractP(V1);
  const v4f P2 = extractP(V2);
  const v4f P3 = extractP(V3);

  // Metric contractions
  const cfloat TMP0 = tmp_calc_vp(V3, P1);
  const cfloat TMP1 = tmp_calc_vp(V2, P1);
  const cfloat TMP2 = tmp_calc_vp(V3, P2);
  const cfloat TMP3 = tmp_calc_vv(V3, V1);
  const cfloat TMP4 = tmp_calc_vp(V2, P3);
  const cfloat TMP5 = tmp_calc_vp(V1, P2);
  const cfloat TMP6 = tmp_calc_vv(V3, V2);
  const cfloat TMP7 = tmp_calc_vp(V1, P3);
  const cfloat TMP8 = tmp_calc_vv(V2, V1);

  // Build vertex amplitude
  cfloat vertex = COUP * (
      TMP8 * (-cI * TMP0 + cI * TMP2) +
      TMP3 * (-cI * TMP4 + cI * TMP1) +
      TMP6 * (-cI * TMP5 + cI * TMP7)
  );

  DEBUG_PRINT_CFLOAT("vertex", vertex);
  return vertex;
}

#endif // PROCESSING_CORE_VVV1_0_H
