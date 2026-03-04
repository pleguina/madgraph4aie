// ============================================================================
// kernel_k1_wfgen_v3_vvv_precompute.cpp
//
// K1: WFGEN Kernel - OPTIMIZED VERSION with VVV precomputation
//
// OPTIMIZATION: Precompute all 4 gluon helicity combinations for VVV products
// before the helicity loop, reducing VVV calls from 96 to 12 per PSP.
//
// VVV products depend ONLY on gluon polarizations (g1, g2 for w5_01, etc.)
// Since gluons have 2 polarization states each, there are only 2²=4 combinations.
//
// Helicity bits: [b_g1 b_g2 b_t b_tb b_g3]
//   - w5_01 = VVV1P0_1(w0, w1) depends on (b_g1, b_g2) → 4 combos
//   - w10   = VVV1P0_1(w1, w4) depends on (b_g2, b_g3) → 4 combos
//   - w5_04 = VVV1P0_1(w0, w4) depends on (b_g1, b_g3) → 4 combos
//
// Savings: 3 VVV × 32 helicities = 96 calls → 3 VVV × 4 combos = 12 calls
//          = 8× reduction in VVV computation!
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
// K1 OPTIMIZED: VVV Precomputation Version
// ============================================================================
template<int BATCH>
void kernel_k1_wfgen_v3(
    ::input_stream<float>* __restrict psp_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    DEBUG_PRINT("[K1_WFGEN_V3] Starting with VVV precomputation: BATCH=%d\n", BATCH);
    
    const cfloat gc10 = get_gc10();  // VVV coupling
    
    for (int b = 0; b < BATCH; ++b) {
        // ====================================================================
        // Step 1: Load PSP momenta (5 × 4 floats)
        // ====================================================================
        v4f Pg1, Pg2, Pt, Ptbar, Pg3;
        for (int k = 0; k < 4; ++k) Pg1[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pg2[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pt[k]    = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Ptbar[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pg3[k]   = readincr(psp_in);

        // ====================================================================
        // Step 2: Cache external wavefunctions (10 WFs for 5 particles × 2 hel)
        // This is the same as GGTTG_HEL_MODE=2
        // ====================================================================
        const v8c g1_m = vxxxxx(Pg1,  0.0f, -1, -1);  // g1, hel=-1
        const v8c g1_p = vxxxxx(Pg1,  0.0f, +1, -1);  // g1, hel=+1
        const v8c g2_m = vxxxxx(Pg2,  0.0f, -1, -1);  // g2, hel=-1
        const v8c g2_p = vxxxxx(Pg2,  0.0f, +1, -1);  // g2, hel=+1
        const v8c t_m  = oxxxxx(Pt,   MT,   -1, +1);  // t,  hel=-1
        const v8c t_p  = oxxxxx(Pt,   MT,   +1, +1);  // t,  hel=+1
        const v8c tb_m = ixxxxx(Ptbar, MT,  -1, -1);  // tbar, hel=-1
        const v8c tb_p = ixxxxx(Ptbar, MT,  +1, -1);  // tbar, hel=+1
        const v8c g3_m = vxxxxx(Pg3,  0.0f, -1, +1);  // g3, hel=-1
        const v8c g3_p = vxxxxx(Pg3,  0.0f, +1, +1);  // g3, hel=+1

        // ====================================================================
        // Step 3: PRECOMPUTE VVV products for all gluon combinations
        //
        // w5_01 = VVV1P0_1(g1, g2): 4 combos indexed by (b_g1, b_g2)
        // w10   = VVV1P0_1(g2, g3): 4 combos indexed by (b_g2, b_g3)
        // w5_04 = VVV1P0_1(g1, g3): 4 combos indexed by (b_g1, b_g3)
        //
        // Index encoding: idx = b_first * 2 + b_second
        //   00 → idx=0 (both -1), 01 → idx=1 (-1,+1), 10 → idx=2 (+1,-1), 11 → idx=3 (both +1)
        // ====================================================================
        
        // w5_01[4]: VVV1P0_1(g1, g2)
        v8c w5_01_cache[4];
        w5_01_cache[0] = VVV1P0_1(g1_m, g2_m, gc10, 0.0f, 0.0f);  // g1=-1, g2=-1
        w5_01_cache[1] = VVV1P0_1(g1_m, g2_p, gc10, 0.0f, 0.0f);  // g1=-1, g2=+1
        w5_01_cache[2] = VVV1P0_1(g1_p, g2_m, gc10, 0.0f, 0.0f);  // g1=+1, g2=-1
        w5_01_cache[3] = VVV1P0_1(g1_p, g2_p, gc10, 0.0f, 0.0f);  // g1=+1, g2=+1

        // w10[4]: VVV1P0_1(g2, g3)
        v8c w10_cache[4];
        w10_cache[0] = VVV1P0_1(g2_m, g3_m, gc10, 0.0f, 0.0f);  // g2=-1, g3=-1
        w10_cache[1] = VVV1P0_1(g2_m, g3_p, gc10, 0.0f, 0.0f);  // g2=-1, g3=+1
        w10_cache[2] = VVV1P0_1(g2_p, g3_m, gc10, 0.0f, 0.0f);  // g2=+1, g3=-1
        w10_cache[3] = VVV1P0_1(g2_p, g3_p, gc10, 0.0f, 0.0f);  // g2=+1, g3=+1

        // w5_04[4]: VVV1P0_1(g1, g3)
        v8c w5_04_cache[4];
        w5_04_cache[0] = VVV1P0_1(g1_m, g3_m, gc10, 0.0f, 0.0f);  // g1=-1, g3=-1
        w5_04_cache[1] = VVV1P0_1(g1_m, g3_p, gc10, 0.0f, 0.0f);  // g1=-1, g3=+1
        w5_04_cache[2] = VVV1P0_1(g1_p, g3_m, gc10, 0.0f, 0.0f);  // g1=+1, g3=-1
        w5_04_cache[3] = VVV1P0_1(g1_p, g3_p, gc10, 0.0f, 0.0f);  // g1=+1, g3=+1

        DEBUG_PRINT("[K1_V3] PSP %d: Precomputed 12 VVV products\n", b);

        // ====================================================================
        // Step 4: Helicity loop with INDEXED LOOKUP (no VVV computation!)
        // ====================================================================
        for (int h = 0; h < 32; ++h) {
            // Extract helicity bits
            const int b_g1 = (h >> 4) & 1;  // bit 4
            const int b_g2 = (h >> 3) & 1;  // bit 3
            const int b_t  = (h >> 2) & 1;  // bit 2
            const int b_tb = (h >> 1) & 1;  // bit 1
            const int b_g3 = (h >> 0) & 1;  // bit 0

            // Select external wavefunctions (simple ternary, no compute)
            const v8c& w0 = b_g1 ? g1_p : g1_m;
            const v8c& w1 = b_g2 ? g2_p : g2_m;
            const v8c& w2 = b_t  ? t_p  : t_m;
            const v8c& w3 = b_tb ? tb_p : tb_m;
            const v8c& w4 = b_g3 ? g3_p : g3_m;

            // INDEX INTO PRECOMPUTED VVV PRODUCTS (no VVV computation!)
            const int idx_01 = (b_g1 << 1) | b_g2;  // index for w5_01
            const int idx_24 = (b_g2 << 1) | b_g3;  // index for w10 (g2, g3)
            const int idx_04 = (b_g1 << 1) | b_g3;  // index for w5_04

            const v8c& w5_01 = w5_01_cache[idx_01];
            const v8c& w10   = w10_cache[idx_24];
            const v8c& w5_04 = w5_04_cache[idx_04];

#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                DEBUG_PRINT("[K1_V3] h=%d: idx_01=%d, idx_24=%d, idx_04=%d\n", 
                            h, idx_01, idx_24, idx_04);
            }
#endif

            // Initialize JAMP (unchanged)
            cfloat jamp[6];
            for (int i = 0; i < 6; ++i) jamp[i] = {0.0f, 0.0f};

            // Write token (unchanged)
            token_ext::write_token_8wf_k1(token_out,
                w0, w1, w2, w3, w4,
                w5_01, w10, w5_04,
                jamp);
        }
    }
    
    DEBUG_PRINT("[K1_WFGEN_V3] Done\n");
}

// Explicit instantiation
template void kernel_k1_wfgen_v3<1>(::input_stream<float>*, ::output_cascade<caccfloat>*);

} // namespace ggttg

// ============================================================================
// PERFORMANCE ANALYSIS
// ============================================================================
//
// BEFORE (current kernel_k1_wfgen.cpp with GGTTG_HEL_MODE=2):
//   - External WF gen: 10 calls (cached)
//   - VVV calls in loop: 3 × 32 = 96 calls
//   - Total VVV cycles: ~277 cycles/call × 96 = 26,592 cycles
//
// AFTER (this version):
//   - External WF gen: 10 calls (same)
//   - VVV calls before loop: 3 × 4 = 12 calls
//   - VVV lookup in loop: 3 × 32 = 96 lookups (essentially FREE)
//   - Total VVV cycles: ~277 cycles/call × 12 = 3,324 cycles
//
// SAVINGS: 26,592 - 3,324 = 23,268 cycles per PSP
//          = ~71% reduction in K1 compute time!
//
// MEMORY COST: 12 × v8c × 8 bytes/cfloat × 8 lanes = 768 bytes in data memory
//              (well within 32 KB budget)
// ============================================================================
