// ============================================================================
// kernel_k1_wfgen_daisy.cpp
// 
// K1 DAISY-CHAIN VERSION: External wavefunctions + VVV1P0_1 + PSP forwarding
// 
// Changes from baseline:
//   - Template parameters: HEL_START, HEL_END, FORWARD
//   - Helicity loop: [HEL_START, HEL_END) instead of [0, 32)
//   - PSP forwarding: If FORWARD=true, forwards PSP to next K1 via stream
//   - Stream output port: psp_forward (used when FORWARD=true)
//
// HELAS functions used (unchanged):
//   - vxxxxx, ixxxxx, oxxxxx (external WF generators)
//   - VVV1P0_1 (off-shell triple gluon current)
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
  #define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
#endif

namespace ggttg {

// ============================================================================
// K1 DAISY-CHAIN: WFGEN Kernel with PSP forwarding
// 
// Template Parameters:
//   BATCH:     Number of PSPs per kernel invocation
//   HEL_START: First helicity index (inclusive)
//   HEL_END:   Last helicity index (exclusive)
//   FORWARD:   If true, forward PSP to next K1 via stream
//
// Inputs:
//   psp_in:      PSP stream (5 × 4 floats per PSP)
//   
// Outputs:
//   psp_forward: Stream to next K1 (only used if FORWARD=true)
//   token_out:   Cascade token with 8 wavefunctions + JAMP
// ============================================================================
template<int BATCH, int HEL_START, int HEL_END, bool FORWARD>
void kernel_k1_wfgen_daisy(
    ::input_stream<float>* __restrict psp_in,
    ::output_stream<float>* __restrict psp_forward,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    DEBUG_PRINT("[K1_DAISY] Starting: BATCH=%d, HEL=[%d,%d), FORWARD=%d\n", 
                BATCH, HEL_START, HEL_END, FORWARD ? 1 : 0);
    
    // Couplings
    const cfloat gc10 = get_gc10();  // VVV coupling
    
    for (int b = 0; b < BATCH; ++b) {
        // ====================================================================
        // Phase 1: Load PSP and optionally forward to next K1
        // 
        // Stream read and write can overlap - both use stream switch
        // PSP forwarding happens BEFORE compute, so downstream K1s start early
        // ====================================================================
        v4f Pg1, Pg2, Pt, Ptbar, Pg3;
        
        // Read and forward particle 1 (gluon 1)
        for (int k = 0; k < 4; ++k) {
            float val = readincr(psp_in);
            Pg1[k] = val;
            if constexpr (FORWARD) writeincr(psp_forward, val);
        }
        
        // Read and forward particle 2 (gluon 2)
        for (int k = 0; k < 4; ++k) {
            float val = readincr(psp_in);
            Pg2[k] = val;
            if constexpr (FORWARD) writeincr(psp_forward, val);
        }
        
        // Read and forward particle 3 (top)
        for (int k = 0; k < 4; ++k) {
            float val = readincr(psp_in);
            Pt[k] = val;
            if constexpr (FORWARD) writeincr(psp_forward, val);
        }
        
        // Read and forward particle 4 (anti-top)
        for (int k = 0; k < 4; ++k) {
            float val = readincr(psp_in);
            Ptbar[k] = val;
            if constexpr (FORWARD) writeincr(psp_forward, val);
        }
        
        // Read and forward particle 5 (gluon 3)
        for (int k = 0; k < 4; ++k) {
            float val = readincr(psp_in);
            Pg3[k] = val;
            if constexpr (FORWARD) writeincr(psp_forward, val);
        }

        // ====================================================================
        // Phase 2: Cache external wavefunctions for both helicity states
        // (helicity-independent computation, done once per PSP)
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
        // Print cached wavefunctions for PSP 0 to verify they match baseline
        if (b == 0) {
            cfloat c0, c1, c2, c3, c4, c5;
            c0 = g1_m.get(0); c1 = g1_p.get(0);
            c2 = g2_m.get(0); c3 = g2_p.get(0);
            c4 = t_m.get(0);  c5 = t_p.get(0);
            printf("[K1_DAISY_CACHE] PSP 0, Slice [%d,%d) cached WFs:\n", HEL_START, HEL_END);
            printf("  g1_m[0] = (%.10e, %.10e)\n", (double)c0.real, (double)c0.imag);
            printf("  g1_p[0] = (%.10e, %.10e)\n", (double)c1.real, (double)c1.imag);
            printf("  g2_m[0] = (%.10e, %.10e)\n", (double)c2.real, (double)c2.imag);
            printf("  g2_p[0] = (%.10e, %.10e)\n", (double)c3.real, (double)c3.imag);
            printf("  t_m[0]  = (%.10e, %.10e)\n", (double)c4.real, (double)c4.imag);
            printf("  t_p[0]  = (%.10e, %.10e)\n", (double)c5.real, (double)c5.imag);
        }
#endif

        // ====================================================================
        // Phase 3: Helicity loop - process only [HEL_START, HEL_END)
        // ====================================================================
        for (int h = HEL_START; h < HEL_END; ++h) {
            // Bit layout: bit4=g1, bit3=g2, bit2=t, bit1=tbar, bit0=g3
            const int b_g1 = (h >> 4) & 1;
            const int b_g2 = (h >> 3) & 1;
            const int b_t  = (h >> 2) & 1;
            const int b_tb = (h >> 1) & 1;
            const int b_g3 = (h >> 0) & 1;

            // Select cached wavefunctions based on helicity bits
            const v8c w0 = b_g1 ? g1_p : g1_m;
            const v8c w1 = b_g2 ? g2_p : g2_m;
            const v8c w2 = b_t  ? t_p  : t_m;
            const v8c w3 = b_tb ? tb_p : tb_m;
            const v8c w4 = b_g3 ? g3_p : g3_m;

            // VVV1P0_1 products (depend on gluon polarizations)
            const v8c w5_01 = VVV1P0_1(w0, w1, gc10, 0.0f, 0.0f);
            const v8c w10   = VVV1P0_1(w1, w4, gc10, 0.0f, 0.0f);
            const v8c w5_04 = VVV1P0_1(w0, w4, gc10, 0.0f, 0.0f);

#if defined(__X86SIM__) || defined(__AIESIM__)
            // Print first 3 helicities of each slice to verify values match baseline
            bool debug_first = (b == 0 && h == HEL_START);
            bool debug_second = (b == 0 && h == HEL_START + 1 && HEL_START + 1 < HEL_END);
            bool debug_third = (b == 0 && h == HEL_START + 2 && HEL_START + 2 < HEL_END);
            
            if (debug_first || debug_second || debug_third) {
                const char* label = "K1_DAISY_DEBUG";
                cfloat c;
                
                printf("[%s] PSP %d, Hel %d - bits=(%d%d%d%d%d):\n", label, b, h, b_g1, b_g2, b_t, b_tb, b_g3);
                
                // Print external wavefunctions w0-w4 (all 6 components)
                for (int k = 0; k < 6; k++) { c = w0.get(k); printf("[%s] w0[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w1.get(k); printf("[%s] w1[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w2.get(k); printf("[%s] w2[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w3.get(k); printf("[%s] w3[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w4.get(k); printf("[%s] w4[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                
                // Print VVV propagators
                for (int k = 0; k < 6; k++) { c = w5_01.get(k); printf("[%s] w5_01[%d]=VVV1P0_1(w0,w1) = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w10.get(k); printf("[%s] w10[%d]=VVV1P0_1(w1,w4) = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w5_04.get(k); printf("[%s] w5_04[%d]=VVV1P0_1(w0,w4) = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
            }
#endif

            // Initialize JAMP to zero
            cfloat jamp[6];
            for (int i = 0; i < 6; ++i) jamp[i] = {0.0f, 0.0f};

            // Write token to cascade (to K2a)
            token_ext::write_token_8wf_k1(token_out,
                w0, w1, w2, w3, w4,
                w5_01, w10, w5_04,
                jamp);
        }
    }
    
    DEBUG_PRINT("[K1_DAISY] Done\n");
}

// ============================================================================
// Explicit instantiations for 4-way helicity slicing
//
// Slice 0: helicities  0-7,  forwards PSP
// Slice 1: helicities  8-15, forwards PSP
// Slice 2: helicities 16-23, forwards PSP
// Slice 3: helicities 24-31, NO forward (last in chain)
// ============================================================================

// BATCH=1 instantiations
template void kernel_k1_wfgen_daisy<1, 0, 8, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<1, 8, 16, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<1, 16, 24, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<1, 24, 32, false>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);

// BATCH=2 instantiations
template void kernel_k1_wfgen_daisy<2, 0, 8, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<2, 8, 16, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<2, 16, 24, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<2, 24, 32, false>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);

// BATCH=4 instantiations
template void kernel_k1_wfgen_daisy<4, 0, 8, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<4, 8, 16, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<4, 16, 24, true>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);
template void kernel_k1_wfgen_daisy<4, 24, 32, false>(
    ::input_stream<float>*, ::output_stream<float>*, ::output_cascade<caccfloat>*);

} // namespace ggttg
