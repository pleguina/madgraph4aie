// ============================================================================
// processing_core_ffv1p0_3.h
//
// Minimal FFV header containing ONLY FFV1P0_3 function
// Used by K1 to avoid pulling in FFV1_0, FFV1_1, FFV1_2
// ============================================================================
#ifndef PROCESSING_CORE_FFV1P0_3_H
#define PROCESSING_CORE_FFV1P0_3_H

#include "processing_core_common.h"

// ============================================================================
// FFV1P0_3: Off-shell vector from (F1, F2)
// PM: ~2.6KB compiled
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

#endif // PROCESSING_CORE_FFV1P0_3_H
