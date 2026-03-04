// ============================================================================
// kernel_k4_vvvv_color_pkt.cpp
//
// K4 VVVV_COLOR Kernel - Packet Stream Version for pktmerge
//
// Reads input from cascade (same as regular version)
// Outputs to output_pktstream (handles packet headers/TLAST)
// ============================================================================

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

// Selective HELAS includes - ONLY what K4 needs
#include "helas/processing_core_vvvv.h"  // VVVV1P0_1, VVVV3P0_1, VVVV4P0_1
#include "helas/processing_core_ffv.h"   // FFV1_0 only

// Token helpers and config
#include "kernels_4k_token.h"
#include "ggttg_params.h"
#include "ggttg_config.h"

namespace ggttg {

#if defined(__X86SIM__) || defined(__AIESIM__)
  #define DEBUG_PRINT(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)
#else
  #define DEBUG_PRINT(...)
#endif

// ============================================================================
// K4 Packet Stream Version: For use with pktmerge
//
// Input: Cascade token (5 wavefunctions + JAMP)
// Output: output_pktstream (writes packet header, ME² data, TLAST)
// ============================================================================
template<int BATCH>
void kernel_k4_vvvv_color_pkt(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_pktstream* __restrict me2_out)
{
    const cfloat gc11 = get_gc11();  // FFV coupling
    const cfloat gc12 = get_gc12();  // VVVV coupling
    
    DEBUG_PRINT("[K4_VVVV_PKT] Starting: BATCH=%d\n", BATCH);
    
    // Get packet ID and write packet header
    const uint32 pktType = 0;
    uint32 ID = getPacketid(me2_out, 0);
    DEBUG_PRINT("[K4_PKT] Received from cascade, packet ID=%u\n", ID);
    writeHeader(me2_out, pktType, ID);
    DEBUG_PRINT("[K4_PKT] Wrote packet header for ID=%u\n", ID);
    
    for (int b = 0; b < BATCH; ++b) {
        float me2_sum = 0.0f;
        
        for (int h = 0; h < 32; ++h) {
            // Read standard token: 5 WFs + JAMP
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
            const float dot_14 = j1.real*j4.real + j1.imag*j4.imag;
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
        }
        
        // Spin averaging
        me2_sum *= (1.0f / 256.0f);
        
        DEBUG_PRINT("[K4_PKT] Packet ID=%u, PSP %d: me2 = %.10e\n", ID, b, (double)me2_sum);
        
        // Write ME² result to packet stream (as float, with TLAST on last word of packet)
        bool is_last = (b == BATCH - 1);
        writeincr(me2_out, me2_sum, is_last);
    }
    
    DEBUG_PRINT("[K4_PKT] Completed packet ID=%u, sent to pktmerge\n", ID);
    DEBUG_PRINT("[K4_VVVV_PKT] Done\n");
}

} // namespace ggttg
