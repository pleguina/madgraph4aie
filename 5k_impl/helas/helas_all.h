// ============================================================================
// helas_all.h
//
// Convenience header that includes ALL HELAS functions.
// WARNING: Do NOT use this in kernel code! It will cause PM overflow.
//
// For kernels, include only the specific headers you need:
//   - processing_core_wfgen.h    : vxxxxx, ixxxxx, oxxxxx
//   - processing_core_ffv.h      : FFV1_0, FFV1_1, FFV1_2, FFV1P0_3
//   - processing_core_vvv1_0.h   : VVV1_0 (amplitude only)
//   - processing_core_vvv1p0_1.h : VVV1P0_1 (off-shell current only)
//   - processing_core_vvvv.h     : VVVV1P0_1, VVVV3P0_1, VVVV4P0_1
//
// CRITICAL: Never include both vvv1_0.h and vvv1p0_1.h in the same kernel!
// ============================================================================
#ifndef HELAS_ALL_H
#define HELAS_ALL_H

#include "processing_core_common.h"
#include "processing_core_wfgen.h"
#include "processing_core_ffv.h"
#include "processing_core_vvv1_0.h"
#include "processing_core_vvv1p0_1.h"
#include "processing_core_vvvv.h"

#endif // HELAS_ALL_H
