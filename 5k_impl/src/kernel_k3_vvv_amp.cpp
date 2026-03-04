// ============================================================================
// kernel_k3_vvv_amp.cpp
//
// K3 Kernel: VVV Amplitude Computations (VVV1_0 ONLY!)
//
// Computes diagrams: D1, D12, D15
// (All diagrams using VVV1_0 amplitude - the final VVV contraction)
//
// HELAS Functions Used:
//   - VVV1_0    (amplitude ONLY - no off-shell propagator)
//   - FFV1P0_3  (computed locally for D1, D12, D15)
//
// HELAS Functions NOT Included (compile hygiene + VVV EXCLUSION PRINCIPLE):
//   - VVV1P0_1  *** CRITICAL: NEVER include in this kernel! ***
//   - FFV1_0    (in K2/K4)
//   - FFV1_1    (in K2)
//   - FFV1_2    (in K2)
//   - VVVV*     (in K4)
//
// VVV EXCLUSION PRINCIPLE:
//   VVV1P0_1 (~10KB) + VVV1_0 (~8KB) > 16KB PM limit
//   These two functions MUST be in separate kernels!
//
// Token:
//   Input:  Extended token (8 WFs + JAMP) from K2b
//   Output: Standard token (5 WFs + JAMP) to K4
// ============================================================================

#ifndef KERNEL_K3_VVV_AMP_CPP
#define KERNEL_K3_VVV_AMP_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

// Selective HELAS includes - VVV1_0 and FFV1P0_3
// *** DO NOT ADD processing_core_vvv1p0_1.h - VVV EXCLUSION PRINCIPLE ***
#include "helas/processing_core_ffv.h"     // FFV1P0_3 (computed locally)
#include "helas/processing_core_vvv1_0.h"  // VVV1_0 amplitude ONLY

// Token helpers and config
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

// ============================================================================
// K3: VVV_AMP Kernel
//
// Computes diagrams D1, D12, D15 (all using VVV1_0 amplitude)
//
// Uses precomputed from K1 (via token):
//   - w5_01 = VVV1P0_1(w0, w1)  for D1
//   - w10   = VVV1P0_1(w1, w4)  for D15
//   - w5_04 = VVV1P0_1(w0, w4)  for D12
// Computes locally:
//   - w6    = FFV1P0_3(w3, w2)  for D1, D12, D15 (moved from K1)
//
// CRITICAL: This kernel contains VVV1_0 ONLY
//           All VVV1P0_1 products are received via token from K1
// ============================================================================
template<int BATCH>
void kernel_k3_vvv_amp(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc10 = get_gc10();  // VVV coupling
    const cfloat gc11 = get_gc11();  // FFV coupling
    
    DEBUG_PRINT("[K3_VVV] Starting: BATCH=%d\n", BATCH);
    
    for (int b = 0; b < BATCH; ++b) {
        for (int h = 0; h < 32; ++h) {
            // Read token: 8 WFs + JAMP (w6 computed locally)
            v8c w0, w1, w2, w3, w4;
            v8c w5_01, w10, w5_04;
            cfloat jamp[6];
            
            token_ext::read_token_8wf_k1(token_in,
                w0, w1, w2, w3, w4,
                w5_01, w10, w5_04,
                jamp);
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                DEBUG_PRINT("[K3_READ] PSP %d, Hel %d - Received from K2B:\n", b, h);
                cfloat c;
                c = w0.get(0); DEBUG_PRINT("[K3_READ] w0[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w5_01.get(0); DEBUG_PRINT("[K3_READ] w5_01[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
            }
#endif
            
            // Compute w6 locally (moved from K1 for memory optimization)
            // w6 = FFV1P0_3(w3, w2) used in D1, D12, D15
            v8c w6 = FFV1P0_3(w3, w2, gc11, 0.0f, 0.0f);
            
            cfloat amp;
            
            // ================================================================
            // D1: VVV1_0(w5_01, w6, w4)
            //
            // w5_01 = VVV1P0_1(w0,w1) from K1
            // w6    = FFV1P0_3(w3,w2) from K1
            // w4    = external gluon
            // ================================================================
            amp = VVV1_0(w5_01, w6, w4, gc10);
            jamp[0] -= amp;
            jamp[2] += amp;
            jamp[4] += amp;
            jamp[5] -= amp;
            
            // ================================================================
            // D12: VVV1_0(w5_04, w1, w6)
            //
            // w5_04 = VVV1P0_1(w0,w4) from K1
            // w1    = external gluon
            // w6    = FFV1P0_3(w3,w2) from K1
            // ================================================================
            amp = VVV1_0(w5_04, w1, w6, gc10);
            jamp[1] += amp;
            jamp[2] -= amp;
            jamp[3] += amp;
            jamp[4] -= amp;
            
            // ================================================================
            // D15: VVV1_0(w0, w10, w6)
            //
            // w0  = external gluon
            // w10 = VVV1P0_1(w1,w4) from K1
            // w6  = FFV1P0_3(w3,w2) from K1
            // ================================================================
            amp = VVV1_0(w0, w10, w6, gc10);
            jamp[0] += amp;
            jamp[1] -= amp;
            jamp[3] -= amp;
            jamp[5] += amp;
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == 0) {
                DEBUG_PRINT("================== K3 SENDING TO K4 (PSP %d, Hel %d) ==================\n", b, h);
                cfloat c;
                
                // Print ALL 8 components of each wavefunction being sent
                DEBUG_PRINT("[K3_WRITE] w0[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w0.get(i);
                    DEBUG_PRINT("w0[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                DEBUG_PRINT("[K3_WRITE] w1[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w1.get(i);
                    DEBUG_PRINT("w1[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                DEBUG_PRINT("[K3_WRITE] w2[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w2.get(i);
                    DEBUG_PRINT("w2[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                DEBUG_PRINT("[K3_WRITE] w3[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w3.get(i);
                    DEBUG_PRINT("w3[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                DEBUG_PRINT("[K3_WRITE] w4[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w4.get(i);
                    DEBUG_PRINT("w4[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                // Print COMPLETE JAMP array being sent to K4
                DEBUG_PRINT("[K3_WRITE] JAMP[6]: ");
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("jamp[%d]=(%.10e,%.10e) ", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
                DEBUG_PRINT("\n");
                DEBUG_PRINT("========================================================================\n");
            }
#endif
            
            // Forward standard token to K4
            token_ext::write_token_5wf(token_out,
                w0, w1, w2, w3, w4,
                jamp);
        }
    }
    
    DEBUG_PRINT("[K3_VVV] Done\n");
}

} // namespace ggttg

#endif // KERNEL_K3_VVV_AMP_CPP
