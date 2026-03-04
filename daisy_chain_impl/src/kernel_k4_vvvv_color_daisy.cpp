// ============================================================================
// kernel_k4_vvvv_color_daisy.cpp
//
// K4 DAISY-CHAIN VERSION: VVVV Contact + Color Reduction + ME² Accumulation
//
// Changes from baseline:
//   - Template parameters: HEL_START, HEL_END, IS_FIRST, IS_LAST added
//   - Helicity loop: [HEL_START, HEL_END) instead of [0, 32)
//   - ME² accumulation: Reads partial ME² from previous K4 (if !IS_FIRST)
//   - Spin averaging: Applied only by last K4 (if IS_LAST)
//   - Stream I/O: partial_in (from prev K4), me2_out (to next K4 or PLIO)
//
// HELAS Functions Used (unchanged):
//   - VVVV1P0_1, VVVV3P0_1, VVVV4P0_1
//   - FFV1_0
//
// Token:
//   Input:  Standard token (5 WFs + JAMP) from K3 via CASCADE
//   Stream: partial_in (partial ME² from previous K4)
//   Output: me2_out (accumulated ME² to next K4 or final to PLIO)
// ============================================================================

#ifndef KERNEL_K4_VVVV_COLOR_DAISY_CPP
#define KERNEL_K4_VVVV_COLOR_DAISY_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

// Selective HELAS includes
#include "helas/processing_core_vvvv.h"  // VVVV1P0_1, VVVV3P0_1, VVVV4P0_1
#include "helas/processing_core_ffv.h"   // FFV1_0

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
// K4 DAISY-CHAIN: VVVV + Color + ME² Accumulation
//
// Template Parameters:
//   BATCH:     Number of PSPs per kernel invocation
//   HEL_START: First helicity index (inclusive)
//   HEL_END:   Last helicity index (exclusive)
//   IS_FIRST:  If true, no incoming partial ME² (start accumulation at 0)
//   IS_LAST:   If true, apply spin averaging (1/256) before output
//
// Stream Interface:
//   partial_in:  From previous K4 (ignored if IS_FIRST)
//   me2_out:     To next K4 or PLIO
// ============================================================================
template<int BATCH, int HEL_START, int HEL_END, bool IS_FIRST, bool IS_LAST>
void kernel_k4_vvvv_color_daisy(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::input_stream<float>* __restrict partial_in,
    ::output_stream<float>* __restrict me2_out)
{
    const cfloat gc11 = get_gc11();  // FFV coupling
    const cfloat gc12 = get_gc12();  // VVVV coupling
    
    DEBUG_PRINT("[K4_DAISY] Starting: BATCH=%d, HEL=[%d,%d), IS_FIRST=%d, IS_LAST=%d\n", 
                BATCH, HEL_START, HEL_END, IS_FIRST ? 1 : 0, IS_LAST ? 1 : 0);
    
    for (int b = 0; b < BATCH; ++b) {
        // ====================================================================
        // Phase 1: Read incoming partial ME² (if not first slice)
        // ====================================================================
        float me2_sum;
        
        if constexpr (IS_FIRST) {
            me2_sum = 0.0f;
            DEBUG_PRINT("[K4_DAISY] PSP %d: Starting accumulation at 0 (IS_FIRST)\n", b);
        } else {
            me2_sum = readincr(partial_in);
            DEBUG_PRINT("[K4_DAISY] PSP %d: Received partial ME² = %.10e from previous K4\n", 
                        b, (double)me2_sum);
        }
        
        // ====================================================================
        // Phase 2: Helicity loop - process only [HEL_START, HEL_END)
        // ====================================================================
        for (int h = HEL_START; h < HEL_END; ++h) {
            // Read standard token: 5 WFs + JAMP
            v8c w0, w1, w2, w3, w4;
            cfloat jamp[6];
            
            token_ext::read_token_5wf(token_in,
                w0, w1, w2, w3, w4,
                jamp);
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && (h == HEL_START || h == HEL_END - 1)) {
                DEBUG_PRINT("[K4_DAISY_READ] PSP %d, Hel %d - Received from K3:\n", b, h);
                cfloat c;
                c = w0.get(0); DEBUG_PRINT("[K4_DAISY_READ] w0[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                c = w1.get(0); DEBUG_PRINT("[K4_DAISY_READ] w1[0] = (%.10e, %.10e)\n", (double)c.real, (double)c.imag);
                DEBUG_PRINT("[K4_DAISY_READ] JAMP from K3:\n");
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
            }
#endif
            
            cfloat amp;
            
            // ================================================================
            // D16: 4-gluon contact vertices (3 sub-diagrams)
            // ================================================================
            v8c w_vvvv1 = VVVV1P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            v8c w_vvvv3 = VVVV3P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            v8c w_vvvv4 = VVVV4P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            
            // D16a: FFV1_0(w3, w2, VVVV1P0_1)
            amp = FFV1_0(w3, w2, w_vvvv1, gc11);
            jamp[0] += amp;
            jamp[1] -= amp;
            jamp[3] -= amp;
            jamp[5] += amp;
            
            // D16b: FFV1_0(w3, w2, VVVV3P0_1)
            amp = FFV1_0(w3, w2, w_vvvv3, gc11);
            jamp[1] -= amp;
            jamp[2] += amp;
            jamp[3] -= amp;
            jamp[4] += amp;
            
            // D16c: FFV1_0(w3, w2, VVVV4P0_1)
            amp = FFV1_0(w3, w2, w_vvvv4, gc11);
            jamp[0] -= amp;
            jamp[2] += amp;
            jamp[4] += amp;
            jamp[5] -= amp;
            
            // ================================================================
            // Color reduction: compute |M|² for this helicity
            // ================================================================
            float hel_me2 = 0.0f;
            
            for (int i = 0; i < 6; ++i) {
                // Diagonal term
                hel_me2 += COLOR_MATRIX[i][i] * 
                    (jamp[i].real * jamp[i].real + jamp[i].imag * jamp[i].imag) 
                    / COLOR_DENOM;
                
                // Off-diagonal terms
                for (int j = i + 1; j < 6; ++j) {
                    const float c = COLOR_MATRIX[i][j];
                    hel_me2 += 2.0f * c * 
                        (jamp[i].real * jamp[j].real + jamp[i].imag * jamp[j].imag) 
                        / COLOR_DENOM;
                }
            }
            
            me2_sum += hel_me2;

#if defined(__X86SIM__) || defined(__AIESIM__)
            // Print final JAMP for each helicity (after D16)
            if (b == 0 && h == HEL_START) {
                DEBUG_PRINT("[K4_DAISY_JAMP] PSP %d, Hel %d - JAMP after D16:\n", b, h);
                for (int i = 0; i < 6; i++) {
                    DEBUG_PRINT("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
                }
            }
#endif
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && (h == HEL_START || h == HEL_END - 1)) {
                DEBUG_PRINT("[K4_DAISY] PSP %d, Hel %d: hel_me2 = %.10e, running sum = %.10e\n", 
                            b, h, (double)hel_me2, (double)me2_sum);
            }
#endif
        }
        
        // ====================================================================
        // Phase 3: Output accumulated ME²
        // ====================================================================
        DEBUG_PRINT("[K4_DAISY_SUM] PSP %d, Slice [%d,%d): me2_sum (before spin avg) = %.10e\n", 
                    b, HEL_START, HEL_END, (double)me2_sum);
        
        if constexpr (IS_LAST) {
            // Last slice: apply spin averaging
            me2_sum *= (1.0f / 256.0f);
            DEBUG_PRINT("[K4_DAISY_FINAL] PSP %d: Final ME² (after /256) = %.10e\n", b, (double)me2_sum);
        } else {
            DEBUG_PRINT("[K4_DAISY_FWD] PSP %d: Forwarding partial ME² = %.10e to next K4\n", 
                        b, (double)me2_sum);
        }
        
        writeincr(me2_out, me2_sum);
        DEBUG_PRINT("[K4_DAISY_WRITE] PSP %d: Written %.10e to output\n", b, (double)me2_sum);
    }
    
    DEBUG_PRINT("[K4_DAISY] Done\n");
}

// ============================================================================
// Explicit instantiations for 4-way helicity slicing
//
// Slice 0: helicities  0-7,  IS_FIRST=true,  IS_LAST=false
// Slice 1: helicities  8-15, IS_FIRST=false, IS_LAST=false
// Slice 2: helicities 16-23, IS_FIRST=false, IS_LAST=false
// Slice 3: helicities 24-31, IS_FIRST=false, IS_LAST=true
// ============================================================================

// BATCH=1
template void kernel_k4_vvvv_color_daisy<1, 0, 8, true, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<1, 8, 16, false, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<1, 16, 24, false, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<1, 24, 32, false, true>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);

// BATCH=2
template void kernel_k4_vvvv_color_daisy<2, 0, 8, true, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<2, 8, 16, false, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<2, 16, 24, false, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<2, 24, 32, false, true>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);

// BATCH=4
template void kernel_k4_vvvv_color_daisy<4, 0, 8, true, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<4, 8, 16, false, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<4, 16, 24, false, false>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_daisy<4, 24, 32, false, true>(
    ::input_cascade<caccfloat>*, ::input_stream<float>*, ::output_stream<float>*);

} // namespace ggttg

#endif // KERNEL_K4_VVVV_COLOR_DAISY_CPP
