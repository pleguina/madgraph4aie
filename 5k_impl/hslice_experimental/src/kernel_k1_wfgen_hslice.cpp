// ============================================================================
// kernel_k1_wfgen_hslice.cpp
//
// Helicity-Sliced K1 Kernel with VVV Precomputation
//
// This kernel processes only a subset of helicities [HEL_START, HEL_END)
// Combined with VVV precomputation for maximum performance.
//
// For 4-way split: each instance handles 8 helicities
// For 8-way split: each instance handles 4 helicities
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
// Helicity-Sliced K1 with VVV Precomputation
//
// Template parameters:
//   BATCH:     Number of PSPs per invocation
//   HEL_START: First helicity index (inclusive)
//   HEL_END:   Last helicity index (exclusive)
// ============================================================================
template<int BATCH, int HEL_START, int HEL_END>
void kernel_k1_wfgen_hslice(
    ::input_stream<float>* __restrict psp_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    static_assert(HEL_START < HEL_END, "Invalid helicity range");
    static_assert(HEL_END <= 32, "Helicity index out of bounds");
    constexpr int N_HEL = HEL_END - HEL_START;
    
    DEBUG_PRINT("[K1_HSLICE] Starting: BATCH=%d, HEL=[%d,%d)\n", BATCH, HEL_START, HEL_END);
    
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
        // ====================================================================
        const v8c g1_m = vxxxxx(Pg1,  0.0f, -1, -1);
        const v8c g1_p = vxxxxx(Pg1,  0.0f, +1, -1);
        const v8c g2_m = vxxxxx(Pg2,  0.0f, -1, -1);
        const v8c g2_p = vxxxxx(Pg2,  0.0f, +1, -1);
        const v8c t_m  = oxxxxx(Pt,   MT,   -1, +1);
        const v8c t_p  = oxxxxx(Pt,   MT,   +1, +1);
        const v8c tb_m = ixxxxx(Ptbar, MT,  -1, -1);
        const v8c tb_p = ixxxxx(Ptbar, MT,  +1, -1);
        const v8c g3_m = vxxxxx(Pg3,  0.0f, -1, +1);
        const v8c g3_p = vxxxxx(Pg3,  0.0f, +1, +1);

        // ====================================================================
        // Step 3: PRECOMPUTE VVV products for all gluon combinations
        //
        // Only compute the VVV products that are actually needed for this
        // helicity slice. This is an optimization over computing all 12.
        //
        // Helicity bit layout: [b_g1(4) b_g2(3) b_t(2) b_tb(1) b_g3(0)]
        // ====================================================================
        
        // Determine which gluon combinations appear in this helicity slice
        // For small slices, we might not need all 4 combinations of each VVV
        
        // For simplicity and correctness, precompute all 4 combinations
        // (12 VVV calls total, still 8× better than 96)
        
        // w5_01[4]: VVV1P0_1(g1, g2)
        v8c w5_01_cache[4];
        w5_01_cache[0] = VVV1P0_1(g1_m, g2_m, gc10, 0.0f, 0.0f);  // b_g1=0, b_g2=0
        w5_01_cache[1] = VVV1P0_1(g1_m, g2_p, gc10, 0.0f, 0.0f);  // b_g1=0, b_g2=1
        w5_01_cache[2] = VVV1P0_1(g1_p, g2_m, gc10, 0.0f, 0.0f);  // b_g1=1, b_g2=0
        w5_01_cache[3] = VVV1P0_1(g1_p, g2_p, gc10, 0.0f, 0.0f);  // b_g1=1, b_g2=1

        // w10[4]: VVV1P0_1(g2, g3)
        v8c w10_cache[4];
        w10_cache[0] = VVV1P0_1(g2_m, g3_m, gc10, 0.0f, 0.0f);
        w10_cache[1] = VVV1P0_1(g2_m, g3_p, gc10, 0.0f, 0.0f);
        w10_cache[2] = VVV1P0_1(g2_p, g3_m, gc10, 0.0f, 0.0f);
        w10_cache[3] = VVV1P0_1(g2_p, g3_p, gc10, 0.0f, 0.0f);

        // w5_04[4]: VVV1P0_1(g1, g3)
        v8c w5_04_cache[4];
        w5_04_cache[0] = VVV1P0_1(g1_m, g3_m, gc10, 0.0f, 0.0f);
        w5_04_cache[1] = VVV1P0_1(g1_m, g3_p, gc10, 0.0f, 0.0f);
        w5_04_cache[2] = VVV1P0_1(g1_p, g3_m, gc10, 0.0f, 0.0f);
        w5_04_cache[3] = VVV1P0_1(g1_p, g3_p, gc10, 0.0f, 0.0f);

        // ====================================================================
        // Step 4: Process only assigned helicities [HEL_START, HEL_END)
        // ====================================================================
        for (int h = HEL_START; h < HEL_END; ++h) {
            // Extract helicity bits
            const int b_g1 = (h >> 4) & 1;
            const int b_g2 = (h >> 3) & 1;
            const int b_t  = (h >> 2) & 1;
            const int b_tb = (h >> 1) & 1;
            const int b_g3 = (h >> 0) & 1;

            // Select external wavefunctions
            const v8c& w0 = b_g1 ? g1_p : g1_m;
            const v8c& w1 = b_g2 ? g2_p : g2_m;
            const v8c& w2 = b_t  ? t_p  : t_m;
            const v8c& w3 = b_tb ? tb_p : tb_m;
            const v8c& w4 = b_g3 ? g3_p : g3_m;

            // Index into precomputed VVV products
            const int idx_01 = (b_g1 << 1) | b_g2;
            const int idx_24 = (b_g2 << 1) | b_g3;
            const int idx_04 = (b_g1 << 1) | b_g3;

            const v8c& w5_01 = w5_01_cache[idx_01];
            const v8c& w10   = w10_cache[idx_24];
            const v8c& w5_04 = w5_04_cache[idx_04];

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
    
    DEBUG_PRINT("[K1_HSLICE] Done: HEL=[%d,%d)\n", HEL_START, HEL_END);
}

// ============================================================================
// Explicit Instantiations for 4-Way Helicity Split (8 helicities each)
// ============================================================================
template void kernel_k1_wfgen_hslice<1, 0, 8>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 8, 16>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 16, 24>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 24, 32>(::input_stream<float>*, ::output_cascade<caccfloat>*);

// ============================================================================
// Explicit Instantiations for 8-Way Helicity Split (4 helicities each)
// ============================================================================
template void kernel_k1_wfgen_hslice<1, 0, 4>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 4, 8>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 8, 12>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 12, 16>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 16, 20>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 20, 24>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 24, 28>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 28, 32>(::input_stream<float>*, ::output_cascade<caccfloat>*);

// ============================================================================
// Explicit Instantiations for 2-Way Helicity Split (16 helicities each)
// ============================================================================
template void kernel_k1_wfgen_hslice<1, 0, 16>(::input_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_hslice<1, 16, 32>(::input_stream<float>*, ::output_cascade<caccfloat>*);

} // namespace ggttg
