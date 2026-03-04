// ============================================================================
// kernel_k2a_ff_diag_hslice.cpp
//
// K2a HELICITY-SLICED VERSION: FFV Diagram Computations (Part 1)
//
// Changes from baseline:
//   - Template parameters: HEL_START, HEL_END added
//   - Helicity loop: [HEL_START, HEL_END) instead of [0, 32)
//
// Computes diagrams: D2, D3, D4, D5, D6, D7 (6 diagrams)
//
// Token (unchanged):
//   Input:  Extended token (8 WFs + JAMP) from K1
//   Output: Extended token (8 WFs + JAMP) to K2b
// ============================================================================

#ifndef KERNEL_K2A_FF_DIAG_HSLICE_CPP
#define KERNEL_K2A_FF_DIAG_HSLICE_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "helas/processing_core_ffv.h"
#include "kernels_4k_token.h"
#include "ggttg_params.h"
#include "ggttg_config.h"

#if defined(__X86SIM__) || defined(__AIESIM__)
  #include <cstdio>
  #define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
#endif

namespace ggttg {

// ============================================================================
// K2a HELICITY-SLICED: FFV Diagrams D2-D7
//
// Template Parameters:
//   BATCH:     Number of PSPs per kernel invocation
//   HEL_START: First helicity index (inclusive)
//   HEL_END:   Last helicity index (exclusive)
// ============================================================================
template<int BATCH, int HEL_START, int HEL_END>
void kernel_k2a_ff_diag_hslice(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc11 = get_gc11();
    
    DEBUG_PRINT("[K2A_HSLICE] Starting: BATCH=%d, HEL=[%d,%d)\n", BATCH, HEL_START, HEL_END);
    
    for (int b = 0; b < BATCH; ++b) {
        // ====================================================================
        // Helicity loop: process only [HEL_START, HEL_END)
        // ====================================================================
        for (int h = HEL_START; h < HEL_END; ++h) {
            v8c w0, w1, w2, w3, w4, w5_01, w10, w5_04;
            cfloat jamp[6];
            
            token_ext::read_token_8wf_k1(token_in, w0, w1, w2, w3, w4, w5_01, w10, w5_04, jamp);

#if defined(__X86SIM__) || defined(__AIESIM__)
            // Print first 3 helicities of slice to verify cascade token transfer
            bool debug_this = (b == 0 && h == HEL_START);
            bool debug_next = (b == 0 && h == HEL_START + 1 && HEL_START + 1 < HEL_END);
            
            if (debug_this || debug_next) {
                const char* label = "K2A_HSLICE_READ";
                cfloat c;
                printf("[%s] PSP %d, Hel %d - Received from K1:\n", label, b, h);
                for (int k = 0; k < 6; k++) { c = w0.get(k); printf("[%s] w0[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w1.get(k); printf("[%s] w1[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
                for (int k = 0; k < 6; k++) { c = w5_01.get(k); printf("[%s] w5_01[%d] = (%.10e, %.10e)\n", label, k, (double)c.real, (double)c.imag); }
            }
#endif
            
            // Compute propagators needed for K2a diagrams (D2-D7)
            v8c w7    = FFV1_1(w2, w4, gc11, MT, WT);  // for D2
            v8c w8    = FFV1_2(w3, w4, gc11, MT, WT);  // for D3, D6
            v8c w9    = FFV1_2(w3, w1, gc11, MT, WT);  // for D4
            v8c w5_20 = FFV1_1(w2, w0, gc11, MT, WT);  // for D4, D5, D6
            v8c w5_30 = FFV1_2(w3, w0, gc11, MT, WT);  // for D7
            v8c w11   = FFV1_1(w2, w1, gc11, MT, WT);  // for D7

            cfloat amp;
            
            // D2: FFV1_0(w3, w7, w5_01)
            amp = FFV1_0(w3, w7, w5_01, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) DEBUG_PRINT("[K2A_HSLICE_D2_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[4] += CI * amp; jamp[5] -= CI * amp;
            
            // D3: FFV1_0(w8, w2, w5_01)
            amp = FFV1_0(w8, w2, w5_01, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) DEBUG_PRINT("[K2A_HSLICE_D3_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[0] += CI * amp; jamp[2] -= CI * amp;
            
            // D4: FFV1_0(w9, w5_20, w4)
            amp = FFV1_0(w9, w5_20, w4, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) DEBUG_PRINT("[K2A_HSLICE_D4_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[1] -= amp;
            
            // D5: FFV1_0(w3, w5_20, w10)
            amp = FFV1_0(w3, w5_20, w10, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) DEBUG_PRINT("[K2A_HSLICE_D5_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[0] += CI * amp; jamp[1] -= CI * amp;
            
            // D6: FFV1_0(w8, w5_20, w1)
            amp = FFV1_0(w8, w5_20, w1, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) DEBUG_PRINT("[K2A_HSLICE_D6_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[0] -= amp;
            
            // D7: FFV1_0(w5_30, w11, w4)
            amp = FFV1_0(w5_30, w11, w4, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) DEBUG_PRINT("[K2A_HSLICE_D7_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[3] -= amp;

#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) {
                cfloat c;
                c = w7.get(0); DEBUG_PRINT("[K2A_HSLICE_PROP] w7[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w8.get(0); DEBUG_PRINT("[K2A_HSLICE_PROP] w8[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w5_20.get(0); DEBUG_PRINT("[K2A_HSLICE_PROP] w5_20[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                
                DEBUG_PRINT("[K2A_HSLICE] PSP %d, Hel %d - JAMP after D2-D7:\n", b, h);
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
            }
#endif
            
            // Forward to K2b
            token_ext::write_token_8wf_k1(token_out,
                w0, w1, w2, w3, w4, w5_01, w10, w5_04,
                jamp);
        }
    }
    
    DEBUG_PRINT("[K2A_HSLICE] Done\n");
}

// ============================================================================
// Explicit instantiations for 4-way helicity slicing
// ============================================================================

// BATCH=1
template void kernel_k2a_ff_diag_hslice<1, 0, 8>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<1, 8, 16>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<1, 16, 24>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<1, 24, 32>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

// BATCH=2
template void kernel_k2a_ff_diag_hslice<2, 0, 8>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<2, 8, 16>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<2, 16, 24>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<2, 24, 32>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

// BATCH=4
template void kernel_k2a_ff_diag_hslice<4, 0, 8>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<4, 8, 16>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<4, 16, 24>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k2a_ff_diag_hslice<4, 24, 32>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

} // namespace ggttg
#endif
