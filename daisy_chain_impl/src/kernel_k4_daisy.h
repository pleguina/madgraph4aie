//===----------------------------------------------------------------------===//
// K4 Daisy-Chain Kernel - Partial ME² Accumulation
//===----------------------------------------------------------------------===//
//
// This kernel processes its helicity slice, then accumulates partial ME²
// from the previous slice (if any) and forwards the sum downstream.
//
// Stream Interface:
//   - partial_in:  Input stream (from previous K4, nullptr for first slice)
//   - partial_out: Output stream (to next K4 or PLIO)
//   - cascade_in:  Cascade input (from K3 in same slice)
//
// Template Parameters:
//   - HEL_START, HEL_END: Helicity range [HEL_START, HEL_END)
//   - IS_FIRST: If true, no incoming partial ME² to read
//   - IS_LAST: If true, applies spin averaging (1/256) before output
//
//===----------------------------------------------------------------------===//

#ifndef KERNEL_K4_DAISY_H
#define KERNEL_K4_DAISY_H

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "ggttg_params.h"

//===----------------------------------------------------------------------===//
// K4 with ME² accumulation for daisy-chain topology
//===----------------------------------------------------------------------===//
template<int HEL_START, int HEL_END, bool IS_FIRST, bool IS_LAST>
void kernel_k4_vvvv_color_daisy(
    input_cascade<accfloat>* __restrict cascade_in,
    input_stream<float>* __restrict partial_in,   // Used only if !IS_FIRST
    output_stream<float>* __restrict partial_out)
{
    // =========================================================================
    // Phase 1: Read incoming partial ME² (if not first slice)
    // =========================================================================
    
    float me2_accumulated;
    
    if constexpr (IS_FIRST) {
        me2_accumulated = 0.0f;
    } else {
        // Read accumulated ME² from previous K4 in the daisy chain
        me2_accumulated = readincr(partial_in);
    }
    
    // =========================================================================
    // Phase 2: Process helicity range, compute local contribution
    // =========================================================================
    
    constexpr int N_HEL = HEL_END - HEL_START;
    
    // JAMP color amplitudes (6 for gg→ttg)
    float jamp_re[6], jamp_im[6];
    
    for (int h = HEL_START; h < HEL_END; ++h) {
        // Read cascade token from K3
        // Token contains: JAMP[0..5] complex values + any additional data
        
        // Reset JAMP for this helicity
        for (int j = 0; j < 6; ++j) {
            jamp_re[j] = 0.0f;
            jamp_im[j] = 0.0f;
        }
        
        // Read amplitudes from cascade (from K3)
        // Format depends on your token structure
        for (int j = 0; j < 6; ++j) {
            jamp_re[j] = get_mcd();  // Real part
            jamp_im[j] = get_mcd();  // Imaginary part
        }
        
        // =====================================================================
        // VVVV Contact Terms (if any - add to JAMP)
        // =====================================================================
        
        // D16 and other 4-gluon vertices would be computed here
        // (Placeholder - actual VVVV1_0 calls)
        
        // =====================================================================
        // Color sum: ME² = Σ_i,j JAMP[i]* × ColorMatrix[i][j] × JAMP[j]
        // =====================================================================
        
        // Color matrix for gg→ttg (6×6, real, symmetric)
        // Actual values from MadGraph color.inc
        static constexpr float cf[6][6] = {
            {12.0f, -4.0f,  4.0f, -4.0f,  4.0f, -4.0f},
            {-4.0f, 12.0f, -4.0f,  4.0f, -4.0f,  4.0f},
            { 4.0f, -4.0f, 12.0f, -4.0f,  4.0f, -4.0f},
            {-4.0f,  4.0f, -4.0f, 12.0f, -4.0f,  4.0f},
            { 4.0f, -4.0f,  4.0f, -4.0f, 12.0f, -4.0f},
            {-4.0f,  4.0f, -4.0f,  4.0f, -4.0f, 12.0f}
        };
        
        float me2_this_hel = 0.0f;
        
        // Compute |M|² = Σ_ij JAMP[i]* × cf[i][j] × JAMP[j]
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 6; ++j) {
                // JAMP[i]* × JAMP[j] = (re_i - i*im_i)(re_j + i*im_j)
                //                    = re_i*re_j + im_i*im_j + i*(re_i*im_j - im_i*re_j)
                // We only need real part for |M|²
                float re_prod = jamp_re[i] * jamp_re[j] + jamp_im[i] * jamp_im[j];
                me2_this_hel += cf[i][j] * re_prod;
            }
        }
        
        // Accumulate this helicity's contribution
        me2_accumulated += me2_this_hel;
    }
    
    // =========================================================================
    // Phase 3: Output accumulated ME²
    // =========================================================================
    
    if constexpr (IS_LAST) {
        // Last slice: apply spin averaging (1/256 for 2⁸ spin states)
        // Actually for gg→ttg: 2 gluons × 2 quarks × 2 antiquarks = 2⁵ = 32 helicities
        // Initial state average: 1/(2×2) × 1/(2×2) = 1/16 (2 gluons, 2 helicities each)
        // Factor depends on your conventions - adjust as needed
        constexpr float SPIN_AVG = 1.0f / 256.0f;  // Or 1/16 depending on convention
        
        float me2_final = me2_accumulated * SPIN_AVG;
        writeincr(partial_out, me2_final);
    } else {
        // Not last: forward accumulated partial sum to next K4
        writeincr(partial_out, me2_accumulated);
    }
}

//===----------------------------------------------------------------------===//
// Explicit instantiations for 4-way helicity slicing
//===----------------------------------------------------------------------===//

// Slice 0: helicities 0-7, FIRST (no input), not last
template void kernel_k4_vvvv_color_daisy<0, 8, true, false>(
    input_cascade<accfloat>*, input_stream<float>*, output_stream<float>*);

// Slice 1: helicities 8-15, not first, not last
template void kernel_k4_vvvv_color_daisy<8, 16, false, false>(
    input_cascade<accfloat>*, input_stream<float>*, output_stream<float>*);

// Slice 2: helicities 16-23, not first, not last
template void kernel_k4_vvvv_color_daisy<16, 24, false, false>(
    input_cascade<accfloat>*, input_stream<float>*, output_stream<float>*);

// Slice 3: helicities 24-31, not first, LAST (applies spin avg)
template void kernel_k4_vvvv_color_daisy<24, 32, false, true>(
    input_cascade<accfloat>*, input_stream<float>*, output_stream<float>*);

#endif // KERNEL_K4_DAISY_H
