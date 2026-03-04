// ============================================================================
// ggttg_config.h : Central compile-time configuration for gg->tt~g AIE design
// ----------------------------------------------------------------------------
// Adjust these knobs to explore performance trade-offs without touching core
// kernel source code.
// ============================================================================

// ============================================================================
// ggttg_config.h : Central compile-time configuration for gg->tt~g AIE design
// Fixed for Vitis 2024.1 with proper PLIO width definitions
// ============================================================================

#pragma once

// Batch size (phase-space points per kernel invocation per packet)
// IMPORTANT: BATCH is the number of PSPs per packet!
//   - BATCH=1:    Each packet contains 1 PSP (1 header + 20 floats)
//   - BATCH=1000: Each packet contains 1000 PSPs (1 header + 20,000 floats)
//
// For initial testing with 1 PSP per pipeline, use BATCH=1
// For production with multiple PSPs per pipeline, increase BATCH
#ifndef GGTTG_BATCH
#define GGTTG_BATCH 1  // *** CHANGED: 1 PSP per packet for testing ***
#endif

// Number of spatially replicated pipelines (cores)
#ifndef GGTTG_NCORES
#define GGTTG_NCORES 1  
#endif

// Enable multi-stage pipeline (1 = split into stages, 0 = monolithic kernel)
#ifndef GGTTG_ENABLE_PIPELINE
#define GGTTG_ENABLE_PIPELINE 0
#endif

#if GGTTG_ENABLE_PIPELINE
#ifndef USE_TWO_STAGE_PIPELINE
#define USE_TWO_STAGE_PIPELINE
#endif
#endif

// Numeric type selection:
// 0 = float32 (current fully supported path)
// 1 = fixed-point (not implemented)
#ifndef GGTTG_NUM_TYPE
#define GGTTG_NUM_TYPE 0
#endif

// Unroll factor for helicity loop (1,2,4,8)
#ifndef GGTTG_HEL_UNROLL
#define GGTTG_HEL_UNROLL 2
#endif

// Helicity handling optimization mode for K1
// 0 = baseline (GET_HEL_* + recompute externals per helicity)
// 1 = safe LUT decode (HELICITY_LUT + recompute externals per helicity)
// 2 = aggressive reuse (cache external WFs for hel=±1, select by bit; still computes VVV1P0_1 per helicity)
#ifndef GGTTG_HEL_MODE
#define GGTTG_HEL_MODE 2
#endif

// Whether to recompute helicity mask per PSP (1) or per batch (0)
#ifndef GGTTG_MASK_PER_PSP
#define GGTTG_MASK_PER_PSP 0
#endif

// PLIO data widths (bits) - MODIFIED FOR PACKET SWITCHING
// IMPORTANT: Stream switches are 32-bit native. Packet switching requires 32-bit PLIO.
#ifndef GGTTG_IN_PLIO_BITS
#define GGTTG_IN_PLIO_BITS 32  // Valid: 32, 64, 128, 256 - SET TO 32 FOR PACKET SWITCHING
#endif
#ifndef GGTTG_OUT_PLIO_BITS
#define GGTTG_OUT_PLIO_BITS 32  // Valid: 32, 64, 128, 256 (MUST match output stream word size)
#endif

// ---------------------------------------------------------------------------
// Type traits for potential future fixed-point support
// ---------------------------------------------------------------------------

struct Float32Traits {
    using scalar = float;
    struct cscalar { 
        float real; 
        float imag; 
        cscalar() : real(0.0f), imag(0.0f) {}
        cscalar(float r, float i) : real(r), imag(i) {}
    };
    static inline cscalar make(float r, float i) { 
        return cscalar(r, i); 
    }
};

#if GGTTG_NUM_TYPE == 0
using ActiveTraits = Float32Traits;
#else
// Placeholder for future fixed-point implementation
using ActiveTraits = Float32Traits;
#pragma message("[ggttg] FIXED-POINT MODE NOT YET IMPLEMENTED: using float32 fallback.")
#endif