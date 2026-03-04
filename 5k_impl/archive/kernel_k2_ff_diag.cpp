// ============================================================================
// kernel_k2_ff_diag.cpp
//
// K2 Kernel: FFV Diagram Computations
//
// Computes diagrams: D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D13, D14
// (All diagrams using FFV1_0 amplitude)
//
// HELAS Functions Used:
//   - FFV1_1  (off-shell fermion propagator, F = F + V)
//   - FFV1_2  (off-shell fermion propagator, F = V + F)
//   - FFV1_0  (amplitude)
//
// HELAS Functions NOT Included (compile hygiene):
//   - VVV1P0_1  (in K1 only, exclusion principle!)
//   - VVV1_0    (in K3 only, exclusion principle!)
//   - FFV1P0_3  (in K1 only)
//   - VVVV*     (in K4 only)
//
// Token:
//   Input:  Extended token (9 WFs + JAMP) from K1
//   Output: Extended token (9 WFs + JAMP) to K3
// ============================================================================

#ifndef KERNEL_K2_FF_DIAG_CPP
#define KERNEL_K2_FF_DIAG_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

// Selective HELAS includes - ONLY what K2 needs
#include "helas/processing_core_ffv.h"  // FFV1_0, FFV1_1, FFV1_2 only

// Token helpers and config
#include "kernels_4k_token.h"
#include "ggttg_params.h"
#include "ggttg_config.h"

namespace ggttg {

// ============================================================================
// K2: FF_DIAG Kernel
//
// Computes diagrams D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D13, D14
//
// Uses precomputed from K1:
//   - w5_01 = VVV1P0_1(w0, w1)  for D2, D3
//   - w10   = VVV1P0_1(w1, w4)  for D5, D8
//   - w5_04 = VVV1P0_1(w0, w4)  for D10, D11
// ============================================================================
template<int BATCH>
void kernel_k2_ff_diag(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc11 = get_gc11();  // FFV coupling
    
    for (int b = 0; b < BATCH; ++b) {
        for (int h = 0; h < 32; ++h) {
            // Read extended token: 9 WFs + JAMP
            v8c w0, w1, w2, w3, w4;
            v8c w5_01, w10, w5_04, w6;
            cfloat jamp[6];
            
            token_ext::read_token_9wf(token_in,
                w0, w1, w2, w3, w4,
                w5_01, w10, w5_04, w6,
                jamp);
            
            cfloat amp;
            
            // ================================================================
            // Compute off-shell fermion propagators (FFV1_1, FFV1_2)
            // ================================================================
            v8c w7    = FFV1_1(w2, w4, gc11, MT, WT);  // for D2, D9, D14
            v8c w8    = FFV1_2(w3, w4, gc11, MT, WT);  // for D3, D6, D13
            v8c w9    = FFV1_2(w3, w1, gc11, MT, WT);  // for D4, D11, D14
            v8c w5_20 = FFV1_1(w2, w0, gc11, MT, WT);  // for D4, D5, D6
            v8c w5_30 = FFV1_2(w3, w0, gc11, MT, WT);  // for D7, D8, D9
            v8c w11   = FFV1_1(w2, w1, gc11, MT, WT);  // for D7, D10, D13
            
            // ================================================================
            // D2: FFV1_0(w3, w7, w5_01)
            // Uses: w5_01 = VVV1P0_1(w0,w1) from K1
            // ================================================================
            amp = FFV1_0(w3, w7, w5_01, gc11);
            jamp[4] += CI * amp;
            jamp[5] -= CI * amp;
            
            // ================================================================
            // D3: FFV1_0(w8, w2, w5_01)
            // Uses: w5_01 = VVV1P0_1(w0,w1) from K1
            // ================================================================
            amp = FFV1_0(w8, w2, w5_01, gc11);
            jamp[0] += CI * amp;
            jamp[2] -= CI * amp;
            
            // ================================================================
            // D4: FFV1_0(w9, w5_20, w4)
            // ================================================================
            amp = FFV1_0(w9, w5_20, w4, gc11);
            jamp[1] -= amp;
            
            // ================================================================
            // D5: FFV1_0(w3, w5_20, w10)
            // Uses: w10 = VVV1P0_1(w1,w4) from K1
            // ================================================================
            amp = FFV1_0(w3, w5_20, w10, gc11);
            jamp[0] += CI * amp;
            jamp[1] -= CI * amp;
            
            // ================================================================
            // D6: FFV1_0(w8, w5_20, w1)
            // ================================================================
            amp = FFV1_0(w8, w5_20, w1, gc11);
            jamp[0] -= amp;
            
            // ================================================================
            // D7: FFV1_0(w5_30, w11, w4)
            // ================================================================
            amp = FFV1_0(w5_30, w11, w4, gc11);
            jamp[3] -= amp;
            
            // ================================================================
            // D8: FFV1_0(w5_30, w2, w10)
            // Uses: w10 = VVV1P0_1(w1,w4) from K1
            // ================================================================
            amp = FFV1_0(w5_30, w2, w10, gc11);
            jamp[3] += CI * amp;
            jamp[5] -= CI * amp;
            
            // ================================================================
            // D9: FFV1_0(w5_30, w7, w1)
            // ================================================================
            amp = FFV1_0(w5_30, w7, w1, gc11);
            jamp[5] -= amp;
            
            // ================================================================
            // D10: FFV1_0(w3, w11, w5_04)
            // Uses: w5_04 = VVV1P0_1(w0,w4) from K1
            // ================================================================
            amp = FFV1_0(w3, w11, w5_04, gc11);
            jamp[2] += CI * amp;
            jamp[3] -= CI * amp;
            
            // ================================================================
            // D11: FFV1_0(w9, w2, w5_04)
            // Uses: w5_04 = VVV1P0_1(w0,w4) from K1
            // ================================================================
            amp = FFV1_0(w9, w2, w5_04, gc11);
            jamp[1] += CI * amp;
            jamp[4] -= CI * amp;
            
            // ================================================================
            // D13: FFV1_0(w8, w11, w0)
            // ================================================================
            amp = FFV1_0(w8, w11, w0, gc11);
            jamp[2] -= amp;
            
            // ================================================================
            // D14: FFV1_0(w9, w7, w0)
            // ================================================================
            amp = FFV1_0(w9, w7, w0, gc11);
            jamp[4] -= amp;
            
            // Forward token to K3
            // K3 needs: w0, w1, w5_01, w10, w5_04, w6 for VVV1_0 diagrams
            token_ext::write_token_9wf(token_out,
                w0, w1, w2, w3, w4,
                w5_01, w10, w5_04, w6,
                jamp);
        }
    }
}

} // namespace ggttg

#endif // KERNEL_K2_FF_DIAG_CPP
