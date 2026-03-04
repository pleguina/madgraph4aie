// ============================================================================
// processing_core_vvvv.h
//
// VVVV vertex functions (4-gluon contact):
//   - VVVV1P0_1, VVVV3P0_1, VVVV4P0_1: Off-shell vector from 3 input vectors
// ============================================================================
#ifndef PROCESSING_CORE_VVVV_H
#define PROCESSING_CORE_VVVV_H

#include "processing_core_common.h"

// ============================================================================
// VVVV helper: build output polarization vector
// ============================================================================
AIE_ALWAYS_INLINE v4c vvvv_calc(const v8c& v1, const v8c& v2,
                                cfloat tmpx1, cfloat tmpx2, cfloat denomci) {
  const v4c a = pol4(v1), b = pol4(v2);
  auto acc = aie::mul(a, tmpx1);
  acc      = aie::mac(acc, b, tmpx2);
  return aie::mul(acc.to_vector<cfloat>(0), denomci).to_vector<cfloat>();
}

// ============================================================================
// VVVV common core
// ============================================================================
static inline v8c VVVV_common(v8c V2, v8c V3, v8c V4,
                              cfloat COUP, float M1, float W1,
                              const v8c& A, const v8c& B,
                              cfloat tmpX, cfloat tmpY)
{
  v8c V1 = wf8_zero();
  V1[0] = V2[0] + V3[0] + V4[0];
  V1[1] = V2[1] + V3[1] + V4[1];

  const v4f    P1      = extractP_neg(V1);
  const cfloat denomci = denom(P1, M1, W1, COUP) * cI;

  const v4c pol = vvvv_calc(A, B, tmpX, tmpY, denomci);
  V1[2]=pol[0]; V1[3]=pol[1]; V1[4]=pol[2]; V1[5]=pol[3];
  stamp_vec_i_cache(V1);
  return V1;
}

// ============================================================================
// VVVV1P0_1: 4-gluon contact vertex (form 1)
// ============================================================================
AIE_ALWAYS_INLINE v8c VVVV1P0_1(v8c V2, v8c V3, v8c V4, cfloat COUP, float M1, float W1)
{
  DEBUG_HELAS_PRINT("  VVVV1P0_1: Input V2, V3, V4\n");
  DEBUG_PRINT_V8C("V2", V2);
  DEBUG_PRINT_V8C("V3", V3);
  DEBUG_PRINT_V8C("V4", V4);
  
  const cfloat TMP10 = tmp_calc_vv(V2, V4);
  const cfloat TMP6  = tmp_calc_vv(V3, V2);
  v8c result = VVVV_common(V2, V3, V4, COUP, M1, W1, V4, V3, -TMP6, TMP10);
  
  DEBUG_PRINT_V8C("V1", result);
  return result;
}

// ============================================================================
// VVVV3P0_1: 4-gluon contact vertex (form 3)
// ============================================================================
AIE_ALWAYS_INLINE v8c VVVV3P0_1(v8c V2, v8c V3, v8c V4, cfloat COUP, float M1, float W1)
{
  DEBUG_HELAS_PRINT("  VVVV3P0_1: Input V2, V3, V4\n");
  DEBUG_PRINT_V8C("V2", V2);
  DEBUG_PRINT_V8C("V3", V3);
  DEBUG_PRINT_V8C("V4", V4);
  
  const cfloat TMP11 = tmp_calc_vv(V3, V4);
  const cfloat TMP6  = tmp_calc_vv(V3, V2);
  v8c result = VVVV_common(V2, V3, V4, COUP, M1, W1, V4, V2, -TMP6, TMP11);
  
  DEBUG_PRINT_V8C("V1", result);
  return result;
}

// ============================================================================
// VVVV4P0_1: 4-gluon contact vertex (form 4)
// ============================================================================
AIE_ALWAYS_INLINE v8c VVVV4P0_1(v8c V2, v8c V3, v8c V4, cfloat COUP, float M1, float W1)
{
  DEBUG_HELAS_PRINT("  VVVV4P0_1: Input V2, V3, V4\n");
  DEBUG_PRINT_V8C("V2", V2);
  DEBUG_PRINT_V8C("V3", V3);
  DEBUG_PRINT_V8C("V4", V4);
  
  const cfloat TMP10 = tmp_calc_vv(V2, V4);
  const cfloat TMP11 = tmp_calc_vv(V3, V4);
  v8c result = VVVV_common(V2, V3, V4, COUP, M1, W1, V3, V2, -TMP10, TMP11);
  
  DEBUG_PRINT_V8C("V1", result);
  return result;
}

#endif // PROCESSING_CORE_VVVV_H
