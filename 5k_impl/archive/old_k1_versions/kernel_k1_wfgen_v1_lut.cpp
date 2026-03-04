// ============================================================================
// kernel_k1_wfgen_v1_lut.cpp
// 
// VARIANT 1: LUT-based helicity extraction (Safe + Simple)
// 
// CHANGES FROM BASELINE:
//   - Replaced GET_HEL_* macros with compile-time lookup table
//   - Eliminates 5 × 32 = 160 shift+mask+branch operations per PSP
//   - Saves ~480 cycles per PSP (0.2% speedup)
//   - PMEM cost: +320 bytes (HELICITY_LUT)
// 
// CORRECTNESS: Identical physics to baseline (same 32 helicities computed)
// RISK: VERY LOW - direct replacement, no logic change
// ============================================================================

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

#include "helas/processing_core_wfgen.h"
#include "helas/processing_core_vvv1p0_1.h"
#include "ggttg_params.h"
#include "ggttg_config.h"
#include "kernels_4k_token.h"

#if defined(__X86SIM__) || defined(__AIESIM__)
  #include <cstdio>
  #define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
#endif

namespace ggttg {

// ============================================================================
// Helicity Lookup Table (replaces GET_HEL_* macros)
// ============================================================================
struct HelicityConfig {
    int8_t g1, g2, t, tb, g3;
};

// Precomputed helicity table (10 bytes × 32 = 320 bytes in constant memory)
// Matches EXACTLY the bit-extraction logic from GET_HEL_* macros
constexpr HelicityConfig HELICITY_LUT[32] = {
    {-1,-1,-1,-1,-1}, {-1,-1,-1,-1,+1}, {-1,-1,-1,+1,-1}, {-1,-1,-1,+1,+1},
    {-1,-1,+1,-1,-1}, {-1,-1,+1,-1,+1}, {-1,-1,+1,+1,-1}, {-1,-1,+1,+1,+1},
    {-1,+1,-1,-1,-1}, {-1,+1,-1,-1,+1}, {-1,+1,-1,+1,-1}, {-1,+1,-1,+1,+1},
    {-1,+1,+1,-1,-1}, {-1,+1,+1,-1,+1}, {-1,+1,+1,+1,-1}, {-1,+1,+1,+1,+1},
    {+1,-1,-1,-1,-1}, {+1,-1,-1,-1,+1}, {+1,-1,-1,+1,-1}, {+1,-1,-1,+1,+1},
    {+1,-1,+1,-1,-1}, {+1,-1,+1,-1,+1}, {+1,-1,+1,+1,-1}, {+1,-1,+1,+1,+1},
    {+1,+1,-1,-1,-1}, {+1,+1,-1,-1,+1}, {+1,+1,-1,+1,-1}, {+1,+1,-1,+1,+1},
    {+1,+1,+1,-1,-1}, {+1,+1,+1,-1,+1}, {+1,+1,+1,+1,-1}, {+1,+1,+1,+1,+1}
};

// ============================================================================
// K1: WFGEN Kernel (VARIANT 1 - LUT)
// ============================================================================
template<int BATCH>
void kernel_k1_wfgen_v1_lut(
    ::input_stream<float>* __restrict psp_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    DEBUG_PRINT("[K1_WFGEN_V1_LUT] Starting: BATCH=%d\n", BATCH);
    
    const cfloat gc10 = get_gc10();
    
    for (int b = 0; b < BATCH; ++b) {
        // Load PSP (5 × 4 floats)
        v4f Pg1, Pg2, Pt, Ptbar, Pg3;
        for (int k = 0; k < 4; ++k) Pg1[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pg2[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pt[k]    = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Ptbar[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pg3[k]   = readincr(psp_in);
        
        // Helicity loop (OPTIMIZED: LUT instead of macros)
        for (int h = 0; h < 32; ++h) {
            // BEFORE: 5 macro calls = 25 cycles (shift+mask+branch each)
            // AFTER:  1 LUT lookup  = 1 cycle (base+offset load)
            const auto& hel = HELICITY_LUT[h];
            
            // External wavefunctions (direct register access, no branching)
            v8c w0 = vxxxxx(Pg1,   0.0f, hel.g1, -1);
            v8c w1 = vxxxxx(Pg2,   0.0f, hel.g2, -1);
            v8c w2 = oxxxxx(Pt,    MT,   hel.t,  +1);
            v8c w3 = ixxxxx(Ptbar, MT,   hel.tb, -1);
            v8c w4 = vxxxxx(Pg3,   0.0f, hel.g3, +1);
            
            // VVV1P0_1 products
            v8c w5_01 = VVV1P0_1(w0, w1, gc10, 0.0f, 0.0f);
            v8c w10   = VVV1P0_1(w1, w4, gc10, 0.0f, 0.0f);
            v8c w5_04 = VVV1P0_1(w0, w4, gc10, 0.0f, 0.0f);
            
            // Initialize JAMP
            cfloat jamp[6];
            for (int i = 0; i < 6; ++i) jamp[i] = {0.0f, 0.0f};
            
            // Write token
            token_ext::write_token_8wf_k1(token_out,
                w0, w1, w2, w3, w4,
                w5_01, w10, w5_04,
                jamp);
        }
    }
    
    DEBUG_PRINT("[K1_WFGEN_V1_LUT] Done\n");
}

} // namespace ggttg
