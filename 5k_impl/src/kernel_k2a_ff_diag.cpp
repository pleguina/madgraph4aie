// ============================================================================
// kernel_k2a_ff_diag.cpp
//
// K2a Kernel: FFV Diagram Computations (Part 1)
//
// Computes diagrams: D2, D3, D4, D5, D6, D7 (6 diagrams)
// Computes all propagators and passes them to K2b
//
// Token:
//   Input:  Extended token (8 WFs + JAMP) from K1
//   Output: Extended token (8 WFs + JAMP) to K2b
// ============================================================================

#ifndef KERNEL_K2A_FF_DIAG_CPP
#define KERNEL_K2A_FF_DIAG_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "helas/processing_core_ffv.h"
#include "kernels_4k_token.h"
#include "ggttg_params.h"
#include "ggttg_config.h"

#if defined(__X86SIM__) || defined(__AIESIM__)
  #include <cstdio>
  // #define DEBUG_PRINT(...) printf(__VA_ARGS__)  // Disabled for 1000 events
  #define DEBUG_PRINT(...)
#else
  #define DEBUG_PRINT(...)
#endif

namespace ggttg {

template<int BATCH>
void kernel_k2a_ff_diag(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc11 = get_gc11();
    
    for (int b = 0; b < BATCH; ++b) {
        for (int h = 0; h < 32; ++h) {
            v8c w0, w1, w2, w3, w4, w5_01, w10, w5_04;
            cfloat jamp[6];
            
            token_ext::read_token_8wf_k1(token_in, w0, w1, w2, w3, w4, w5_01, w10, w5_04, jamp);

#if defined(__X86SIM__) || defined(__AIESIM__)
            // Print what K2a receives for first 3 helicities to compare with K1
            bool debug_this = (b == 0 && h == 0);
            bool debug_hel1 = (b == 0 && h == 1);
            bool debug_hel2 = (b == 0 && h == 2);
            
            if (debug_this || debug_hel1 || debug_hel2) {
                const char* label = debug_this ? "K2A_READ" : (debug_hel1 ? "K2A_HEL1_READ" : "K2A_HEL2_READ");
                cfloat c;
                printf("[%s] PSP %d, Hel %d - Received from K1:\n", label, b, h);
                for (int k = 0; k < 6; k++) { c = w0.get(k); printf("[%s] w0[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w1.get(k); printf("[%s] w1[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w5_01.get(k); printf("[%s] w5_01[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
            }
#endif

#if defined(__X86SIM__) || defined(__AIESIM__)
            // Enable detailed FFV1_1 debug for helicity 0 only
            if (b == 0 && h == 0) {
                g_ffv1_debug = true;
                printf("[K2A_DEBUG] === Computing w7 = FFV1_1(w2, w4) ===\n");
            }
#endif
            
            // Compute propagators needed for K2a diagrams (D2-D7)
            v8c w7    = FFV1_1(w2, w4, gc11, MT, WT);  // for D2

#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                g_ffv1_debug = false;  // Turn off after first call
            }
#endif

            v8c w8    = FFV1_2(w3, w4, gc11, MT, WT);  // for D3, D6
            v8c w9    = FFV1_2(w3, w1, gc11, MT, WT);  // for D4
            v8c w5_20 = FFV1_1(w2, w0, gc11, MT, WT);  // for D4, D5, D6
            v8c w5_30 = FFV1_2(w3, w0, gc11, MT, WT);  // for D7
            v8c w11   = FFV1_1(w2, w1, gc11, MT, WT);  // for D7

#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                cfloat c;
                c = w7.get(0); DEBUG_PRINT("[K2A_PROP] w7[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w8.get(0); DEBUG_PRINT("[K2A_PROP] w8[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w5_20.get(0); DEBUG_PRINT("[K2A_PROP] w5_20[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
            }
#endif
            
            cfloat amp;
            
            // D2: FFV1_0(w3, w7, w5_01)
            amp = FFV1_0(w3, w7, w5_01, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) DEBUG_PRINT("[K2A_D2_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[4] += CI * amp; jamp[5] -= CI * amp;
            
            // D3: FFV1_0(w8, w2, w5_01)
            amp = FFV1_0(w8, w2, w5_01, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) DEBUG_PRINT("[K2A_D3_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[0] += CI * amp; jamp[2] -= CI * amp;
            
            // D4: FFV1_0(w9, w5_20, w4)
            amp = FFV1_0(w9, w5_20, w4, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) DEBUG_PRINT("[K2A_D4_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[1] -= amp;
            
            // D5: FFV1_0(w3, w5_20, w10)
            amp = FFV1_0(w3, w5_20, w10, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) DEBUG_PRINT("[K2A_D5_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[0] += CI * amp; jamp[1] -= CI * amp;
            
            // D6: FFV1_0(w8, w5_20, w1)
            amp = FFV1_0(w8, w5_20, w1, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) DEBUG_PRINT("[K2A_D6_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[0] -= amp;
            
            // D7: FFV1_0(w5_30, w11, w4)
            amp = FFV1_0(w5_30, w11, w4, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) DEBUG_PRINT("[K2A_D7_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[3] -= amp;
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                DEBUG_PRINT("[K2A_AFTER_D7] PSP %d, Hel %d - JAMP after D2-D7:\n", b, h);
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
            }
#endif
            
            // Pass to K2b: 8 WFs (w6 removed, will be computed in K3)
            token_ext::write_token_8wf_k1(token_out,
                w0, w1, w2, w3, w4, w5_01, w10, w5_04,
                jamp);
        }
    }
}

} // namespace ggttg
#endif
