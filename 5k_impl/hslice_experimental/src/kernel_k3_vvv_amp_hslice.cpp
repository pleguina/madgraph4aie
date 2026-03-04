// ============================================================================
// kernel_k3_vvv_amp_hslice.cpp
//
// K3 HELICITY-SLICED VERSION: VVV Amplitude Computations
//
// Changes from baseline:
//   - Template parameters: HEL_START, HEL_END added
//   - Helicity loop: [HEL_START, HEL_END) instead of [0, 32)
//
// Computes diagrams: D1, D12, D15 (all using VVV1_0 amplitude)
//
// HELAS Functions Used:
//   - VVV1_0    (amplitude ONLY)
//   - FFV1P0_3  (computed locally for D1, D12, D15)
//
// Token (unchanged):
//   Input:  Extended token (8 WFs + JAMP) from K2b
//   Output: Standard token (5 WFs + JAMP) to K4
// ============================================================================

#ifndef KERNEL_K3_VVV_AMP_HSLICE_CPP
#define KERNEL_K3_VVV_AMP_HSLICE_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

// Selective HELAS includes
#include "helas/processing_core_ffv.h"     // FFV1P0_3
#include "helas/processing_core_vvv1_0.h"  // VVV1_0 amplitude ONLY

#include "kernels_4k_token.h"
#include "ggttg_params.h"
#include "ggttg_config.h"

namespace ggttg {

#if defined(__X86SIM__) || defined(__AIESIM__)
  #define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
#endif

// ============================================================================
// K3 HELICITY-SLICED: VVV Amplitudes D1, D12, D15
//
// Template Parameters:
//   BATCH:     Number of PSPs per kernel invocation
//   HEL_START: First helicity index (inclusive)
//   HEL_END:   Last helicity index (exclusive)
// ============================================================================
template<int BATCH, int HEL_START, int HEL_END>
void kernel_k3_vvv_amp_hslice(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc10 = get_gc10();  // VVV coupling
    const cfloat gc11 = get_gc11();  // FFV coupling
    
    DEBUG_PRINT("[K3_HSLICE] Starting: BATCH=%d, HEL=[%d,%d)\n", BATCH, HEL_START, HEL_END);
    
    for (int b = 0; b < BATCH; ++b) {
        // ====================================================================
        // Helicity loop: process only [HEL_START, HEL_END)
        // ====================================================================
        for (int h = HEL_START; h < HEL_END; ++h) {
            // Read token: 8 WFs + JAMP
            v8c w0, w1, w2, w3, w4;
            v8c w5_01, w10, w5_04;
            cfloat jamp[6];
            
            token_ext::read_token_8wf_k1(token_in,
                w0, w1, w2, w3, w4,
                w5_01, w10, w5_04,
                jamp);
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) {
                DEBUG_PRINT("[K3_HSLICE_READ] PSP %d, Hel %d (first in slice) - Received from K2B:\n", b, h);
                cfloat c;
                c = w0.get(0); DEBUG_PRINT("[K3_HSLICE_READ] w0[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w5_01.get(0); DEBUG_PRINT("[K3_HSLICE_READ] w5_01[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                DEBUG_PRINT("[K3_HSLICE_READ] JAMP from K2B:\n");
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
            }
#endif
            
            // Compute w6 locally (FFV1P0_3)
            v8c w6 = FFV1P0_3(w3, w2, gc11, 0.0f, 0.0f);

#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) {
                cfloat c = w6.get(0);
                DEBUG_PRINT("[K3_HSLICE_W6] w6[0]=FFV1P0_3(w3,w2) = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
            }
#endif
            
            cfloat amp;
            
            // D1: VVV1_0(w5_01, w6, w4)
            amp = VVV1_0(w5_01, w6, w4, gc10);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) DEBUG_PRINT("[K3_HSLICE_D1_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[0] -= amp;
            jamp[2] += amp;
            jamp[4] += amp;
            jamp[5] -= amp;
            
            // D12: VVV1_0(w5_04, w1, w6)
            amp = VVV1_0(w5_04, w1, w6, gc10);
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) DEBUG_PRINT("[K3_HSLICE_D12_AMP] amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
            jamp[1] += amp;
            jamp[2] -= amp;
            jamp[3] += amp;
            jamp[4] -= amp;
            
            // D15: VVV1_0(w0, w10, w6)
            amp = VVV1_0(w0, w10, w6, gc10);
            jamp[0] += amp;
            jamp[1] -= amp;
            jamp[3] -= amp;
            jamp[5] += amp;
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && h == HEL_START) {
                DEBUG_PRINT("[K3_HSLICE] PSP %d, Hel %d - JAMP after D1,D12,D15:\n", b, h);
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
            }
#endif
            
            // Forward standard token to K4
            token_ext::write_token_5wf(token_out,
                w0, w1, w2, w3, w4,
                jamp);
        }
    }
    
    DEBUG_PRINT("[K3_HSLICE] Done\n");
}

// ============================================================================
// Explicit instantiations for 4-way helicity slicing
// ============================================================================

// BATCH=1
template void kernel_k3_vvv_amp_hslice<1, 0, 8>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<1, 8, 16>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<1, 16, 24>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<1, 24, 32>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

// BATCH=2
template void kernel_k3_vvv_amp_hslice<2, 0, 8>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<2, 8, 16>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<2, 16, 24>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<2, 24, 32>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

// BATCH=4
template void kernel_k3_vvv_amp_hslice<4, 0, 8>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<4, 8, 16>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<4, 16, 24>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template void kernel_k3_vvv_amp_hslice<4, 24, 32>(
    ::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

} // namespace ggttg

#endif // KERNEL_K3_VVV_AMP_HSLICE_CPP
