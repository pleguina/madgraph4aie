// ============================================================================
// kernel_k4_vvvv_color.cpp
//
// K4 Kernel: VVVV Contact + Color Reduction
//
// Computes diagram D16 (3 sub-diagrams using VVVV contact vertices)
// + Color matrix reduction → |M|²
//
// HELAS Functions Used:
//   - VVVV1P0_1  (4-gluon contact, form 1)
//   - VVVV3P0_1  (4-gluon contact, form 3)
//   - VVVV4P0_1  (4-gluon contact, form 4)
//   - FFV1_0     (amplitude, to connect VVVV to fermion line)
//
// HELAS Functions NOT Included (compile hygiene):
//   - VVV1P0_1  (in K1 only, exclusion principle!)
//   - VVV1_0    (in K3 only, exclusion principle!)
//   - FFV1_1    (in K2 only)
//   - FFV1_2    (in K2 only)
//   - FFV1P0_3  (in K1 only)
//
// Token:
//   Input:  Standard token (5 WFs + JAMP) from K3
//   Output: Stream of ME² values (one float per PSP)
// ============================================================================

#ifndef KERNEL_K4_VVVV_COLOR_CPP
#define KERNEL_K4_VVVV_COLOR_CPP

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

// Selective HELAS includes - ONLY what K4 needs
#include "helas/processing_core_vvvv.h"  // VVVV1P0_1, VVVV3P0_1, VVVV4P0_1
#include "helas/processing_core_ffv.h"   // FFV1_0 only (FFV1_1, FFV1_2 not used)

// Token helpers and config
#include "kernels_4k_token.h"
#include "ggttg_params.h"
#include "ggttg_config.h"

namespace ggttg {

#if defined(__X86SIM__) || defined(__AIESIM__)
  // #define DEBUG_PRINT(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)  // Disabled for 1000 events
  #define DEBUG_PRINT(...)
#else
  #define DEBUG_PRINT(...)
#endif

// ============================================================================
// K4: VVVV_COLOR Kernel
//
// Computes diagram D16 (3 sub-diagrams) + color reduction → |M|²
//
// D16 consists of 3 4-gluon contact interactions:
//   D16a: VVVV1P0_1(w0, w1, w4) → FFV1_0(w3, w2, ...)
//   D16b: VVVV3P0_1(w0, w1, w4) → FFV1_0(w3, w2, ...)
//   D16c: VVVV4P0_1(w0, w1, w4) → FFV1_0(w3, w2, ...)
// ============================================================================
template<int BATCH>
void kernel_k4_vvvv_color(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_stream<float>* __restrict me2_out)
{
    const cfloat gc11 = get_gc11();  // FFV coupling
    const cfloat gc12 = get_gc12();  // VVVV coupling
    
    DEBUG_PRINT("[K4_VVVV] Starting: BATCH=%d\n", BATCH);
    
    for (int b = 0; b < BATCH; ++b) {
        float me2_sum = 0.0f;
        
        for (int h = 0; h < 32; ++h) {
            // Read standard token: 5 WFs + JAMP (use scalar jamp variables)
            v8c w0, w1, w2, w3, w4;
            cfloat j0, j1, j2, j3, j4, j5;
            {
                cfloat jamp_tmp[6];
                token_ext::read_token_5wf(token_in,
                    w0, w1, w2, w3, w4,
                    jamp_tmp);
                j0 = jamp_tmp[0];
                j1 = jamp_tmp[1];
                j2 = jamp_tmp[2];
                j3 = jamp_tmp[3];
                j4 = jamp_tmp[4];
                j5 = jamp_tmp[5];
            }
            
#if 0  // DISABLED: Expensive debug block causes AIE hang
            if (b == 0 && h == 0) {  // Print COMPLETE token for PSP 0, Hel 0
                DEBUG_PRINT("================== K4 COMPLETE CASCADE TOKEN (PSP %d, Hel %d) ==================\n", b, h);
                cfloat c;
                
                // Print w0 (ALL 8 components)
                DEBUG_PRINT("[K4_READ] w0[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w0.get(i);
                    DEBUG_PRINT("w0[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                // Print w1 (ALL 8 components)
                DEBUG_PRINT("[K4_READ] w1[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w1.get(i);
                    DEBUG_PRINT("w1[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                // Print w2 (ALL 8 components)
                DEBUG_PRINT("[K4_READ] w2[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w2.get(i);
                    DEBUG_PRINT("w2[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                // Print w3 (ALL 8 components)
                DEBUG_PRINT("[K4_READ] w3[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w3.get(i);
                    DEBUG_PRINT("w3[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                // Print w4 (ALL 8 components)
                DEBUG_PRINT("[K4_READ] w4[8]: ");
                for (int i = 0; i < 8; i++) {
                    c = w4.get(i);
                    DEBUG_PRINT("w4[%d]=(%.10e,%.10e) ", i, (double)c.real, (double)c.imag);
                }
                DEBUG_PRINT("\n");
                
                // Print COMPLETE JAMP array (all 6 values)
                DEBUG_PRINT("[K4_READ] JAMP[6]: ");
                DEBUG_PRINT("jamp[0]=(%.10e,%.10e) ", (double)j0.real, (double)j0.imag);
                DEBUG_PRINT("jamp[1]=(%.10e,%.10e) ", (double)j1.real, (double)j1.imag);
                DEBUG_PRINT("jamp[2]=(%.10e,%.10e) ", (double)j2.real, (double)j2.imag);
                DEBUG_PRINT("jamp[3]=(%.10e,%.10e) ", (double)j3.real, (double)j3.imag);
                DEBUG_PRINT("jamp[4]=(%.10e,%.10e) ", (double)j4.real, (double)j4.imag);
                DEBUG_PRINT("jamp[5]=(%.10e,%.10e) ", (double)j5.real, (double)j5.imag);
                DEBUG_PRINT("\n");
                DEBUG_PRINT("================================================================================\n");
            }
#endif
            
            cfloat amp;
            
            // ================================================================
            // D16: 4-gluon contact vertices (3 sub-diagrams)
            // ================================================================
            
            // Compute off-shell gluons from 4-gluon vertices
            v8c w_vvvv1 = VVVV1P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            v8c w_vvvv3 = VVVV3P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            v8c w_vvvv4 = VVVV4P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
            
            // D16a: FFV1_0(w3, w2, VVVV1P0_1)
            amp = FFV1_0(w3, w2, w_vvvv1, gc11);
            j0 += amp;
            j1 -= amp;
            j3 -= amp;
            j5 += amp;
            
            // D16b: FFV1_0(w3, w2, VVVV3P0_1)
            amp = FFV1_0(w3, w2, w_vvvv3, gc11);
            j1 -= amp;
            j2 += amp;
            j3 -= amp;
            j4 += amp;
            
            // D16c: FFV1_0(w3, w2, VVVV4P0_1)
            amp = FFV1_0(w3, w2, w_vvvv4, gc11);
            j0 -= amp;
            j2 += amp;
            j4 += amp;
            j5 -= amp;
            
            // ================================================================
            // Optimized color reduction: fully unrolled 6×6
            // Avoid fdiv: use multiply by INV_COLOR_DENOM (pre-baked)
            // Keep jamp in registers (j0..j5 scalars instead of array)
            // ================================================================
            
            // Compute magnitude squared for each jamp
            const float mag2_0 = j0.real*j0.real + j0.imag*j0.imag;
            const float mag2_1 = j1.real*j1.real + j1.imag*j1.imag;
            const float mag2_2 = j2.real*j2.real + j2.imag*j2.imag;
            const float mag2_3 = j3.real*j3.real + j3.imag*j3.imag;
            const float mag2_4 = j4.real*j4.real + j4.imag*j4.imag;
            const float mag2_5 = j5.real*j5.real + j5.imag*j5.imag;
            
            // Compute dot products (Re(conj(ji)*jj)) for off-diagonal terms
            const float dot_01 = j0.real*j1.real + j0.imag*j1.imag;
            const float dot_02 = j0.real*j2.real + j0.imag*j2.imag;
            const float dot_03 = j0.real*j3.real + j0.imag*j3.imag;
            const float dot_04 = j0.real*j4.real + j0.imag*j4.imag;
            const float dot_05 = j0.real*j5.real + j0.imag*j5.imag;
            const float dot_12 = j1.real*j2.real + j1.imag*j2.imag;
            const float dot_13 = j1.real*j3.real + j1.imag*j3.imag;
            const float dot_14 = j1.real*j4.real + j1.imag*j4.imag;COLOR_OFFDIAG_NORM
            const float dot_15 = j1.real*j5.real + j1.imag*j5.imag;
            const float dot_23 = j2.real*j3.real + j2.imag*j3.imag;
            const float dot_24 = j2.real*j4.real + j2.imag*j4.imag;
            const float dot_25 = j2.real*j5.real + j2.imag*j5.imag;
            const float dot_34 = j3.real*j4.real + j3.imag*j4.imag;
            const float dot_35 = j3.real*j5.real + j3.imag*j5.imag;
            const float dot_45 = j4.real*j5.real + j4.imag*j5.imag;
            
            // Fully unrolled color sum using pre-normalized constants
            float hel_me2 = 0.0f;
            
            // Diagonal terms: COLOR_DIAG_NORM[i] * mag2_i
            hel_me2 += COLOR_DIAG_NORM[0] * mag2_0;
            hel_me2 += COLOR_DIAG_NORM[1] * mag2_1;
            hel_me2 += COLOR_DIAG_NORM[2] * mag2_2;
            hel_me2 += COLOR_DIAG_NORM[3] * mag2_3;
            hel_me2 += COLOR_DIAG_NORM[4] * mag2_4;
            hel_me2 += COLOR_DIAG_NORM[5] * mag2_5;
            
            // Off-diagonal terms: COLOR_OFFDIAG_NORM[k] * dot_ij
            hel_me2 += COLOR_OFFDIAG_NORM[0]  * dot_01;
            hel_me2 += COLOR_OFFDIAG_NORM[1]  * dot_02;
            hel_me2 += COLOR_OFFDIAG_NORM[2]  * dot_03;
            hel_me2 += COLOR_OFFDIAG_NORM[3]  * dot_04;
            hel_me2 += COLOR_OFFDIAG_NORM[4]  * dot_05;
            hel_me2 += COLOR_OFFDIAG_NORM[5]  * dot_12;
            hel_me2 += COLOR_OFFDIAG_NORM[6]  * dot_13;
            hel_me2 += COLOR_OFFDIAG_NORM[7]  * dot_14;
            hel_me2 += COLOR_OFFDIAG_NORM[8]  * dot_15;
            hel_me2 += COLOR_OFFDIAG_NORM[9]  * dot_23;
            hel_me2 += COLOR_OFFDIAG_NORM[10] * dot_24;
            hel_me2 += COLOR_OFFDIAG_NORM[11] * dot_25;
            hel_me2 += COLOR_OFFDIAG_NORM[12] * dot_34;
            hel_me2 += COLOR_OFFDIAG_NORM[13] * dot_35;
            hel_me2 += COLOR_OFFDIAG_NORM[14] * dot_45;
            
            me2_sum += hel_me2;
            
#if defined(__X86SIM__) || defined(__AIESIM__)
            if (b == 0 && (h == 0 || h == 16 || h == 31)) {
                DEBUG_PRINT("[K4_HEL] PSP %d, Hel %d: hel_me2 = %.10e, running sum = %.10e\n", 
                            b, h, (double)hel_me2, (double)me2_sum);
            }
#endif
        }
        
        DEBUG_PRINT("[K4_SUM] PSP %d: me2_sum (before /256) = %.10e\n", b, (double)me2_sum);
        
        // Spin averaging: divide by 256 (= 4 × 4 × 4 × 4 for 4 external particles)
        // Note: One gluon only contributes factor of 2, but we sum over all 32 helicities
        me2_sum *= (1.0f / 256.0f);
        
        DEBUG_PRINT("[K4_FINAL] PSP %d: me2_sum (after /256) = %.10e\n", b, (double)me2_sum);
        
        // Write ME² result
        writeincr(me2_out, me2_sum);
        
        DEBUG_PRINT("[K4_WRITE] PSP %d: Written %.10e to output\n", b, (double)me2_sum);
    }
    
    DEBUG_PRINT("[K4_VVVV] Done\n");
}

} // namespace ggttg

#endif // KERNEL_K4_VVVV_COLOR_CPP

