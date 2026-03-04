//===----------------------------------------------------------------------===//
// K1 Daisy-Chain Kernel - PSP Forward + Helicity Slice Processing
//===----------------------------------------------------------------------===//
//
// This kernel receives PSP data, optionally forwards it to the next slice,
// then processes its assigned helicity range.
//
// Stream Interface:
//   - psp_in:      Input stream (from PLIO or previous K1)
//   - psp_forward: Output stream (to next K1, nullptr for last slice)
//   - cascade_out: Cascade output (to K2a in same slice)
//
// Template Parameters:
//   - HEL_START, HEL_END: Helicity range [HEL_START, HEL_END)
//   - FORWARD: If true, forwards PSP to next K1 via stream
//
//===----------------------------------------------------------------------===//

#ifndef KERNEL_K1_DAISY_H
#define KERNEL_K1_DAISY_H

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "ggttg_params.h"

// PSP data structure (5 particles × 4-momentum)
constexpr int N_PSP_FLOATS = 20;  // 5 particles × 4 components
constexpr int N_PSP_V4 = 5;       // 5 × v4float

//===----------------------------------------------------------------------===//
// K1 with PSP forwarding for daisy-chain topology
//===----------------------------------------------------------------------===//
template<int HEL_START, int HEL_END, bool FORWARD>
void kernel_k1_wfgen_daisy(
    input_stream<float>* __restrict psp_in,
    output_stream<float>* __restrict psp_forward,  // Used only if FORWARD=true
    output_cascade<accfloat>* __restrict cascade_out)
{
    // =========================================================================
    // Phase 1: Read PSP and optionally forward
    // =========================================================================
    
    // PSP momenta storage
    alignas(32) float p[5][4];  // p[particle][mu]
    
    // Read PSP from input stream
    for (int i = 0; i < 5; ++i) {
        v4float pmu = readincr_v4(psp_in);
        
        // Forward to next slice if not the last one
        if constexpr (FORWARD) {
            writeincr(psp_forward, pmu[0]);
            writeincr(psp_forward, pmu[1]);
            writeincr(psp_forward, pmu[2]);
            writeincr(psp_forward, pmu[3]);
        }
        
        // Store locally
        p[i][0] = pmu[0];  // E
        p[i][1] = pmu[1];  // px
        p[i][2] = pmu[2];  // py
        p[i][3] = pmu[3];  // pz
    }
    
    // =========================================================================
    // Phase 2: Compute external wavefunctions (helicity-independent)
    // =========================================================================
    
    // Wavefunction storage: v8c = 8 complex floats (real/imag interleaved)
    // Using float pairs for simplicity in this template
    alignas(32) float w1[2][6];   // u-bar spinor for initial quark (2 helicities)
    alignas(32) float w2[2][6];   // v spinor for initial antiquark (2 helicities)  
    alignas(32) float w3[2][6];   // u spinor for final quark (2 helicities)
    alignas(32) float w4[2][6];   // v-bar spinor for final antiquark (2 helicities)
    alignas(32) float w5[2][6];   // gluon polarization (2 helicities)
    alignas(32) float w6[2][6];   // gluon polarization (2 helicities)
    alignas(32) float w10[2][6];  // gluon polarization (2 helicities)
    
    // Compute all external wavefunctions for both helicity states
    // (Actual implementation would call ixxxxx, oxxxxx, vxxxxx)
    // For now, placeholder that shows the structure
    
    // =========================================================================
    // Phase 3: VVV Precomputation (gluon helicity combinations only)
    // =========================================================================
    
    // VVV products depend only on gluon helicities (w5, w6, w10)
    // 2³ = 8 combinations, but VVV1_0 is symmetric, so some reuse
    // Precompute all 12 unique VVV calls here (instead of 96 in loop)
    
    alignas(32) float vvv_cache[12][6];  // 12 unique VVV combinations
    
    // VVV indices used in amplitude calculation:
    // vvv1_0(w5[h5], w6[h6], vxx)     - h5,h6 ∈ {0,1}
    // vvv1_0(w5[h5], w10[h10], vxx)   - h5,h10 ∈ {0,1}  
    // vvv1_0(w6[h6], w10[h10], vxx)   - h6,h10 ∈ {0,1}
    
    // Precompute (actual VVV1_0 calls would go here)
    // ...
    
    // =========================================================================
    // Phase 4: Process assigned helicity range
    // =========================================================================
    
    constexpr int N_HEL = HEL_END - HEL_START;
    
    for (int h = HEL_START; h < HEL_END; ++h) {
        // Decode helicity bits: h = h1 + 2*h2 + 4*h3 + 8*h4 + 16*h5
        // Wait, for gg→ttg we have: g g → t tbar g
        // 5 external particles, but quarks have 2 helicities each (up/down)
        // and gluons have 2 helicities (±1)
        
        int h1 = (h >> 0) & 1;  // Initial gluon 1
        int h2 = (h >> 1) & 1;  // Initial gluon 2
        int h3 = (h >> 2) & 1;  // Final top
        int h4 = (h >> 3) & 1;  // Final anti-top
        int h5 = (h >> 4) & 1;  // Final gluon
        
        // Select wavefunctions for this helicity combination
        // Build token with 8 wavefunctions + VVV cache pointers
        
        // =====================================================================
        // Write cascade token to K2a
        // =====================================================================
        
        // Token format (example - adjust to match your actual format):
        // [helicity_index][wf1][wf2][wf3][wf4][wf5][wf6][wf10][vvv_indices]
        
        // Write helicity index
        put_mcd(h);
        
        // Write selected wavefunctions (6 complex components each)
        // Using cascade float interface
        for (int c = 0; c < 6; ++c) {
            put_mcd(w1[h1][c]);
        }
        for (int c = 0; c < 6; ++c) {
            put_mcd(w2[h2][c]);
        }
        for (int c = 0; c < 6; ++c) {
            put_mcd(w3[h3][c]);
        }
        for (int c = 0; c < 6; ++c) {
            put_mcd(w4[h4][c]);
        }
        for (int c = 0; c < 6; ++c) {
            put_mcd(w5[h5][c]);
        }
        for (int c = 0; c < 6; ++c) {
            put_mcd(w6[h2][c]);  // Depends on initial gluon 2
        }
        for (int c = 0; c < 6; ++c) {
            put_mcd(w10[h5][c]); // Depends on final gluon
        }
        
        // Write VVV cache indices for this helicity combination
        // (K2/K3 will use these to look up precomputed VVV results)
        // Actually, better to pass the VVV results directly:
        int vvv_idx_56 = h5 * 2 + h2;   // Index for VVV(w5,w6,...)
        int vvv_idx_510 = h5 * 2 + h5;  // Index for VVV(w5,w10,...)
        int vvv_idx_610 = h2 * 2 + h5;  // Index for VVV(w6,w10,...)
        
        for (int c = 0; c < 6; ++c) {
            put_mcd(vvv_cache[vvv_idx_56][c]);
        }
        for (int c = 0; c < 6; ++c) {
            put_mcd(vvv_cache[vvv_idx_510][c]);
        }
        for (int c = 0; c < 6; ++c) {
            put_mcd(vvv_cache[vvv_idx_610][c]);
        }
    }
}

//===----------------------------------------------------------------------===//
// Explicit instantiations for 4-way helicity slicing
//===----------------------------------------------------------------------===//

// Slice 0: helicities 0-7, forwards PSP
template void kernel_k1_wfgen_daisy<0, 8, true>(
    input_stream<float>*, output_stream<float>*, output_cascade<accfloat>*);

// Slice 1: helicities 8-15, forwards PSP  
template void kernel_k1_wfgen_daisy<8, 16, true>(
    input_stream<float>*, output_stream<float>*, output_cascade<accfloat>*);

// Slice 2: helicities 16-23, forwards PSP
template void kernel_k1_wfgen_daisy<16, 24, true>(
    input_stream<float>*, output_stream<float>*, output_cascade<accfloat>*);

// Slice 3: helicities 24-31, NO forward (last in chain)
template void kernel_k1_wfgen_daisy<24, 32, false>(
    input_stream<float>*, output_stream<float>*, output_cascade<accfloat>*);

#endif // KERNEL_K1_DAISY_H
