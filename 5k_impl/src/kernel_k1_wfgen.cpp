// ============================================================================
// kernel_k1_wfgen.cpp
// 
// K1: WFGEN Kernel - External wavefunctions + VVV1P0_1
// 
// HELAS functions used:
//   - vxxxxx, ixxxxx, oxxxxx (external WF generators)
//   - VVV1P0_1 (off-shell triple gluon current)
// 
// Does NOT use: VVV1_0, FFV1_0, FFV1_1, FFV1_2, FFV1P0_3, VVVV*
// (FFV1P0_3 moved to K3 for memory optimization)
// ============================================================================

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

// Include ONLY the HELAS functions this kernel needs
#include "helas/processing_core_wfgen.h"
#include "helas/processing_core_vvv1p0_1.h"

#include "ggttg_params.h"
#include "ggttg_config.h"
#include "kernels_4k_token.h"

#if defined(__X86SIM__) || defined(__AIESIM__)
  #include <cstdio>
  // #define DEBUG_PRINT(...) printf(__VA_ARGS__)  // Disabled for 1000 events
  #define DEBUG_PRINT(...)
#else
  #define DEBUG_PRINT(...)
#endif

namespace ggttg {

// ============================================================================
// K1: WFGEN Kernel
// 
// Inputs: PSP stream (5 × 4 floats per PSP)
// Outputs: Cascade token with 9 wavefunctions + JAMP (all zeros initially)
// 
// Contains: vxxxxx, oxxxxx, ixxxxx, VVV1P0_1, FFV1P0_3
// Does NOT contain: VVV1_0, FFV1_1, FFV1_2, FFV1_0, VVVV
// ============================================================================
template<int BATCH>
void kernel_k1_wfgen(
    ::input_stream<float>* __restrict psp_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    DEBUG_PRINT("[K1_WFGEN] Starting: BATCH=%d\n", BATCH);
    
    // Couplings
    const cfloat gc10 = get_gc10();  // VVV coupling
    const cfloat gc11 = get_gc11();  // FFV coupling
    
    for (int b = 0; b < BATCH; ++b) {
        // Load PSP (5 × 4 floats)
        v4f Pg1, Pg2, Pt, Ptbar, Pg3;
        for (int k = 0; k < 4; ++k) Pg1[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pg2[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pt[k]    = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Ptbar[k] = readincr(psp_in);
        for (int k = 0; k < 4; ++k) Pg3[k]   = readincr(psp_in);
        

      #if GGTTG_HEL_MODE == 2
        // ====================================================================
        // Variant 2 (Aggressive reuse): cache external wavefunctions for hel=±1
        // - Avoids recomputing vxxxxx/ixxxxx/oxxxxx 32× per PSP
        // - Helicity selection is done by bit indexing (branch-free)
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

#if defined(__X86SIM__) || defined(__AIESIM__)
        // Print cached wavefunctions for PSP 0 to verify they differ by helicity
        if (b == 0) {
            cfloat c0, c1, c2, c3, c4, c5;
            c0 = g1_m.get(0); c1 = g1_p.get(0);
            c2 = g2_m.get(0); c3 = g2_p.get(0);
            c4 = t_m.get(0);  c5 = t_p.get(0);
            printf("[K1_CACHE_DEBUG] PSP 0 cached WFs:\n");
            printf("  g1_m[0] = (%.10e, %.10e)\n", (double)c0.real, (double)c0.imag);
            printf("  g1_p[0] = (%.10e, %.10e)\n", (double)c1.real, (double)c1.imag);
            printf("  g2_m[0] = (%.10e, %.10e)\n", (double)c2.real, (double)c2.imag);
            printf("  g2_p[0] = (%.10e, %.10e)\n", (double)c3.real, (double)c3.imag);
            printf("  t_m[0]  = (%.10e, %.10e)\n", (double)c4.real, (double)c4.imag);
            printf("  t_p[0]  = (%.10e, %.10e)\n", (double)c5.real, (double)c5.imag);
        }
#endif

        for (int h = 0; h < 32; ++h) {
            // Bit layout matches ggttg_params.h GET_HEL_* macros:
            // bit4=g1, bit3=g2, bit2=t, bit1=tbar, bit0=g3
            const int b_g1 = (h >> 4) & 1;
            const int b_g2 = (h >> 3) & 1;
            const int b_t  = (h >> 2) & 1;
            const int b_tb = (h >> 1) & 1;
            const int b_g3 = (h >> 0) & 1;

            const v8c w0 = b_g1 ? g1_p : g1_m;
            const v8c w1 = b_g2 ? g2_p : g2_m;
            const v8c w2 = b_t  ? t_p  : t_m;
            const v8c w3 = b_tb ? tb_p : tb_m;
            const v8c w4 = b_g3 ? g3_p : g3_m;

            // VVV1P0_1 products (still per-helicity: depends on gluon polarizations)
            const v8c w5_01 = VVV1P0_1(w0, w1, gc10, 0.0f, 0.0f);
            const v8c w10   = VVV1P0_1(w1, w4, gc10, 0.0f, 0.0f);
            const v8c w5_04 = VVV1P0_1(w0, w4, gc10, 0.0f, 0.0f);

#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                cfloat c;
                c = w10.get(0); DEBUG_PRINT("[K1_W10_COMPUTED] w10[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w10.get(5); DEBUG_PRINT("[K1_W10_COMPUTED] w10[5] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
            }
#endif

#if defined(__X86SIM__) || defined(__AIESIM__)
            // Print first 3 helicities to verify values change (match baseline pattern)
            bool debug_this = (b == 0 && h == 0);
            bool debug_hel1 = (b == 0 && h == 1);
            bool debug_hel2 = (b == 0 && h == 2);
            
            if (debug_this || debug_hel1 || debug_hel2) {
                const char* label = debug_this ? "K1_DEBUG" : (debug_hel1 ? "K1_HEL1_WF" : "K1_HEL2_WF");
                cfloat c0, c1, c2, c3, c4, c5, c6, c7;
                
                // Print external wavefunctions w0-w4 (all 6 lanes like baseline)
                printf("[%s] PSP %d, Hel %d - Helicities: bits=(%d%d%d%d%d)\n", label, b, h, b_g1, b_g2, b_t, b_tb, b_g3);
                for (int k = 0; k < 6; k++) { c0 = w0.get(k); printf("[%s] w0[%d] = (%.10e, %.10e)\n", label, k, (double)c0.real, (double)c0.imag); }
                for (int k = 0; k < 6; k++) { c1 = w1.get(k); printf("[%s] w1[%d] = (%.10e, %.10e)\n", label, k, (double)c1.real, (double)c1.imag); }
                for (int k = 0; k < 6; k++) { c2 = w2.get(k); printf("[%s] w2[%d] = (%.10e, %.10e)\n", label, k, (double)c2.real, (double)c2.imag); }
                for (int k = 0; k < 6; k++) { c3 = w3.get(k); printf("[%s] w3[%d] = (%.10e, %.10e)\n", label, k, (double)c3.real, (double)c3.imag); }
                for (int k = 0; k < 6; k++) { c4 = w4.get(k); printf("[%s] w4[%d] = (%.10e, %.10e)\n", label, k, (double)c4.real, (double)c4.imag); }
                
                // Print VVV propagators
                for (int k = 0; k < 6; k++) { c5 = w5_01.get(k); printf("[%s] w5_01[%d]=VVV1P0_1(w0,w1) = (%.10e, %.10e)\n", label, k, (double)c5.real, (double)c5.imag); }
                for (int k = 0; k < 6; k++) { c6 = w10.get(k); printf("[%s] w10[%d]=VVV1P0_1(w1,w4) = (%.10e, %.10e)\n", label, k, (double)c6.real, (double)c6.imag); }
                for (int k = 0; k < 6; k++) { c7 = w5_04.get(k); printf("[%s] w5_04[%d]=VVV1P0_1(w0,w4) = (%.10e, %.10e)\n", label, k, (double)c7.real, (double)c7.imag); }
            }
#endif

            cfloat jamp[6];
            for (int i = 0; i < 6; ++i) jamp[i] = {0.0f, 0.0f};

            token_ext::write_token_8wf_k1(token_out,
          w0, w1, w2, w3, w4,
          w5_01, w10, w5_04,
          jamp);
        }

      #elif GGTTG_HEL_MODE == 1
        // ====================================================================
        // Variant 1 (Safe + simple): LUT-based helicity decode
        // - Removes GET_HEL_* shift/mask logic from the hot loop
        // - Keeps the same per-helicity wavefunction generation
        // ====================================================================
        for (int h = 0; h < 32; ++h) {
          const int hel_g1 = get_hel_g1(h);
          const int hel_g2 = get_hel_g2(h);
          const int hel_t  = get_hel_t(h);
          const int hel_tb = get_hel_tb(h);
          const int hel_g3 = get_hel_g3(h);

            v8c w0 = vxxxxx(Pg1,  0.0f, hel_g1, -1);
            v8c w1 = vxxxxx(Pg2,  0.0f, hel_g2, -1);
            v8c w2 = oxxxxx(Pt,   MT,   hel_t,  +1);
            v8c w3 = ixxxxx(Ptbar, MT,  hel_tb, -1);
            v8c w4 = vxxxxx(Pg3,  0.0f, hel_g3, +1);

            v8c w5_01 = VVV1P0_1(w0, w1, gc10, 0.0f, 0.0f);
            v8c w10   = VVV1P0_1(w1, w4, gc10, 0.0f, 0.0f);
            v8c w5_04 = VVV1P0_1(w0, w4, gc10, 0.0f, 0.0f);

            cfloat jamp[6];
            for (int i = 0; i < 6; ++i) jamp[i] = {0.0f, 0.0f};

            token_ext::write_token_8wf_k1(token_out,
          w0, w1, w2, w3, w4,
          w5_01, w10, w5_04,
          jamp);
        }

      #else
        // ====================================================================
        // Baseline: on-the-fly macro helicity decode + recompute externals
        // ====================================================================
        for (int h = 0; h < 32; ++h) {
            const int hel_g1 = GET_HEL_G1(h);
            const int hel_g2 = GET_HEL_G2(h);
            const int hel_t  = GET_HEL_T(h);
            const int hel_tb = GET_HEL_TB(h);
            const int hel_g3 = GET_HEL_G3(h);

            v8c w0 = vxxxxx(Pg1,  0.0f, hel_g1, -1);
            v8c w1 = vxxxxx(Pg2,  0.0f, hel_g2, -1);
            v8c w2 = oxxxxx(Pt,   MT,   hel_t,  +1);
            v8c w3 = ixxxxx(Ptbar, MT,  hel_tb, -1);
            v8c w4 = vxxxxx(Pg3,  0.0f, hel_g3, +1);

            v8c w5_01 = VVV1P0_1(w0, w1, gc10, 0.0f, 0.0f);
            v8c w10   = VVV1P0_1(w1, w4, gc10, 0.0f, 0.0f);
            v8c w5_04 = VVV1P0_1(w0, w4, gc10, 0.0f, 0.0f);

            cfloat jamp[6];
            for (int i = 0; i < 6; ++i) jamp[i] = {0.0f, 0.0f};

            token_ext::write_token_8wf_k1(token_out,
          w0, w1, w2, w3, w4,
          w5_01, w10, w5_04,
          jamp);
        }
      #endif
    }
    
    DEBUG_PRINT("[K1_WFGEN] Done\n");
}

} // namespace ggttg

