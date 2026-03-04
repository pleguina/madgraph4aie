// ============================================================================
// kernel_k2b_ff_diag.cpp
//
// K2b Kernel: FFV Diagram Computations (Part 2)
//
// Computes diagrams: D8, D9, D10, D11, D13, D14 (6 diagrams)
// Receives propagators from K2a
//
// Token:
//   Input:  Extended token (8 WFs + JAMP) from K2a
//   Output: Extended token (8 WFs + JAMP) to K3
// ============================================================================

#ifndef KERNEL_K2B_FF_DIAG_CPP
#define KERNEL_K2B_FF_DIAG_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "helas/processing_core_ffv.h"
#include "kernels_4k_token.h"
#include "ggttg_params.h"
#include "ggttg_config.h"

namespace ggttg {

#if defined(__X86SIM__) || defined(__AIESIM__)
  // #define DEBUG_PRINT(...) printf(__VA_ARGS__)  // Disabled for 1000 events
  #define DEBUG_PRINT(...)
#else
  #define DEBUG_PRINT(...)
#endif

template<int BATCH>
void kernel_k2b_ff_diag(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc11 = get_gc11();
    
    DEBUG_PRINT("[K2B_FF] Starting: BATCH=%d\n", BATCH);
    
    for (int b = 0; b < BATCH; ++b) {
        for (int h = 0; h < 32; ++h) {
            v8c w0, w1, w2, w3, w4, w5_01, w10, w5_04;
            cfloat jamp[6];
            
            token_ext::read_token_8wf_k1(token_in,
                w0, w1, w2, w3, w4, w5_01, w10, w5_04,
                jamp);
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                DEBUG_PRINT("[K2B_RECV] PSP %d, Hel %d - Received JAMP from K2A:\n", b, h);
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
            }
#endif
            
            // Compute propagators needed for K2b diagrams (D8-D14)
            v8c w7    = FFV1_1(w2, w4, gc11, MT, WT);  // for D9, D14
            v8c w8    = FFV1_2(w3, w4, gc11, MT, WT);  // for D13
            v8c w9    = FFV1_2(w3, w1, gc11, MT, WT);  // for D11, D14
            v8c w5_30 = FFV1_2(w3, w0, gc11, MT, WT);  // for D8, D9
            v8c w11   = FFV1_1(w2, w1, gc11, MT, WT);  // for D10, D13
            
            cfloat amp;
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                cfloat c;
                c = w10.get(0); DEBUG_PRINT("[K2B_W10] w10[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w10.get(5); DEBUG_PRINT("[K2B_W10] w10[5] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w2.get(0); DEBUG_PRINT("[K2B_W2] w2[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
            }
#endif
            
            // D8: FFV1_0(w5_30, w2, w10)
            amp = FFV1_0(w5_30, w2, w10, gc11);
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                cfloat c;
                c = w5_30.get(0); DEBUG_PRINT("[K2B_W5_30] w5_30[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                DEBUG_PRINT("[K2B_AMP] D8 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
            }
#endif
            
            jamp[3] += CI * amp; jamp[5] -= CI * amp;
            
            // D9: FFV1_0(w5_30, w7, w1)
            amp = FFV1_0(w5_30, w7, w1, gc11);
            jamp[5] -= amp;
            
            // D10: FFV1_0(w3, w11, w5_04)
            amp = FFV1_0(w3, w11, w5_04, gc11);
            jamp[2] += CI * amp; jamp[3] -= CI * amp;
            
            // D11: FFV1_0(w9, w2, w5_04)
            amp = FFV1_0(w9, w2, w5_04, gc11);
            jamp[1] += CI * amp; jamp[4] -= CI * amp;
            
            // D13: FFV1_0(w8, w11, w0)
            amp = FFV1_0(w8, w11, w0, gc11);
            jamp[2] -= amp;
            
            // D14: FFV1_0(w9, w7, w0)
            amp = FFV1_0(w9, w7, w0, gc11);
            jamp[4] -= amp;
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                DEBUG_PRINT("[K2B_AFTER_D14] PSP %d, Hel %d - JAMP after D8-D14:\n", b, h);
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
            }
#endif
            
            // Forward to K3 (8 WFs, w6 removed)
            token_ext::write_token_8wf_k1(token_out,
                w0, w1, w2, w3, w4, w5_01, w10, w5_04,
                jamp);
        }
    }
    
    DEBUG_PRINT("[K2B_FF] Done\n");
}

} // namespace ggttg
#endif
