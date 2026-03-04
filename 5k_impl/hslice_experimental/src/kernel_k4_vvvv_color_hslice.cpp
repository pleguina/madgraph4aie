// ============================================================================
// kernel_k4_vvvv_color_hslice.cpp
//
// Helicity-Sliced K4 Kernel: VVVV Contact + Partial Color Reduction
//
// IMPORTANT: This kernel outputs PARTIAL ME² sum (without /256 normalization)
// The reducer kernel will sum all slices and apply the final normalization.
//
// Template parameters:
//   BATCH:     Number of PSPs per invocation
//   HEL_START: First helicity index (inclusive)  
//   HEL_END:   Last helicity index (exclusive)
// ============================================================================

#ifndef KERNEL_K4_VVVV_COLOR_HSLICE_CPP
#define KERNEL_K4_VVVV_COLOR_HSLICE_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

#include "helas/processing_core_vvvv.h"
#include "helas/processing_core_ffv.h"

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
// K4 Helicity-Sliced: VVVV + Partial Color Sum
//
// Outputs: Partial ME² sum for helicities [HEL_START, HEL_END)
//          WITHOUT the /256 spin averaging factor
// ============================================================================
template<int BATCH, int HEL_START, int HEL_END>
void kernel_k4_vvvv_color_hslice(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_stream<float>* __restrict partial_me2_out)
{
    static_assert(HEL_START < HEL_END, "Invalid helicity range");
    static_assert(HEL_END <= 32, "Helicity index out of bounds");
    constexpr int N_HEL = HEL_END - HEL_START;
    
    const cfloat gc11 = get_gc11();  // FFV coupling
    const cfloat gc12 = get_gc12();  // VVVV coupling
    
    DEBUG_PRINT("[K4_HSLICE] Starting: BATCH=%d, HEL=[%d,%d)\n", BATCH, HEL_START, HEL_END);
    
    for (int b = 0; b < BATCH; ++b) {
        float partial_me2 = 0.0f;  // Accumulator for this slice's contribution
        
        // Process only assigned helicities
        for (int h = HEL_START; h < HEL_END; ++h) {
            // Read standard token: 5 WFs + JAMP
            v8c w0, w1, w2, w3, w4;
            cfloat jamp[6];
            
            token_ext::read_token_5wf(token_in,
                w0, w1, w2, w3, w4,
                jamp);
            
            cfloat amp;
            
            // ================================================================
            // D16: 4-gluon contact vertices (3 sub-diagrams)
            // ================================================================
            v8c w_vvvv1 = VVVV1P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            v8c w_vvvv3 = VVVV3P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            v8c w_vvvv4 = VVVV4P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            
            // D16a
            amp = FFV1_0(w3, w2, w_vvvv1, gc11);
            jamp[0] += amp; jamp[1] -= amp; jamp[3] -= amp; jamp[5] += amp;
            
            // D16b
            amp = FFV1_0(w3, w2, w_vvvv3, gc11);
            jamp[1] -= amp; jamp[2] += amp; jamp[3] -= amp; jamp[4] += amp;
            
            // D16c
            amp = FFV1_0(w3, w2, w_vvvv4, gc11);
            jamp[0] -= amp; jamp[2] += amp; jamp[4] += amp; jamp[5] -= amp;
            
            // ================================================================
            // Color reduction: compute partial |M|² for this helicity
            // NOTE: NO /256 here - reducer will do final normalization
            // ================================================================
            float hel_me2 = 0.0f;
            
            for (int i = 0; i < 6; ++i) {
                // Diagonal term
                hel_me2 += COLOR_MATRIX[i][i] * 
                    (jamp[i].real * jamp[i].real + jamp[i].imag * jamp[i].imag) 
                    / COLOR_DENOM;
                
                // Off-diagonal terms (upper triangle, factor of 2)
                for (int j = i + 1; j < 6; ++j) {
                    const float c = COLOR_MATRIX[i][j];
                    hel_me2 += 2.0f * c * 
                        (jamp[i].real * jamp[j].real + jamp[i].imag * jamp[j].imag) 
                        / COLOR_DENOM;
                }
            }
            
            partial_me2 += hel_me2;
        }
        
        // Output PARTIAL sum (reducer will sum slices and divide by 256)
        writeincr(partial_me2_out, partial_me2);
        
        DEBUG_PRINT("[K4_HSLICE] PSP %d, HEL=[%d,%d): partial_me2 = %.10e\n", 
                    b, HEL_START, HEL_END, (double)partial_me2);
    }
    
    DEBUG_PRINT("[K4_HSLICE] Done\n");
}

// ============================================================================
// Explicit Instantiations for 4-Way Helicity Split
// ============================================================================
template void kernel_k4_vvvv_color_hslice<1, 0, 8>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 8, 16>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 16, 24>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 24, 32>(::input_cascade<caccfloat>*, ::output_stream<float>*);

// ============================================================================
// Explicit Instantiations for 8-Way Helicity Split
// ============================================================================
template void kernel_k4_vvvv_color_hslice<1, 0, 4>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 4, 8>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 8, 12>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 12, 16>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 16, 20>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 20, 24>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 24, 28>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 28, 32>(::input_cascade<caccfloat>*, ::output_stream<float>*);

// ============================================================================
// Explicit Instantiations for 2-Way Helicity Split
// ============================================================================
template void kernel_k4_vvvv_color_hslice<1, 0, 16>(::input_cascade<caccfloat>*, ::output_stream<float>*);
template void kernel_k4_vvvv_color_hslice<1, 16, 32>(::input_cascade<caccfloat>*, ::output_stream<float>*);

} // namespace ggttg

#endif // KERNEL_K4_VVVV_COLOR_HSLICE_CPP
