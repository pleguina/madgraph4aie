// ============================================================================
// processing_core_vvv.h
//
// VVV vertex functions:
//   - VVV1_0:   Triple gluon amplitude (scalar result)
//   - VVV1P0_1: Off-shell triple gluon current (vector result)
//
// IMPORTANT: VVV1_0 and VVV1P0_1 should NOT be in the same kernel!
//            Combined PM exceeds 16KB tile limit.
// ============================================================================
#ifndef PROCESSING_CORE_VVV_H
#define PROCESSING_CORE_VVV_H

#include "processing_core_common.h"

// ============================================================================
// VVV helper: build output polarization vector
// ============================================================================
AIE_ALWAYS_INLINE v4c vvv_calc(const v8c& V2, const v8c& V3,
                               const v4f& P2, const v4f& P3,
                               cfloat mt0pt2, cfloat pt4mt5, cfloat TMP6, cfloat denomci) {
  const v4c a = pol4(V2), b = pol4(V3);
  const v4f p = aie::sub(P3, P2);
  auto acc = aie::mul(a, mt0pt2);
  acc      = aie::mac(acc, b, pt4mt5);
  acc      = aie::mac(acc, p, TMP6);
  return aie::mul(acc.to_vector<cfloat>(0), denomci).to_vector<cfloat>();
}

#endif // PROCESSING_CORE_VVV_H
