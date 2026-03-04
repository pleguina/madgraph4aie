// ============================================================================
// kernel_k1_wfgen_pkt.cpp
// 
// K1 WFGEN Kernel - Packet Stream Version for pktsplit
// 
// Reads input from input_pktstream (handles packet headers/TLAST)
// Outputs to cascade (same as regular version)
// ============================================================================

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>

// Include ONLY the HELAS functions this kernel needs
#include "helas/processing_core_wfgen.h"
#include "helas/processing_core_vvv1p0_1.h"

#include "ggttg_params.h"
#include "ggttg_config.h"
#include "kernels_4k_token.h"

#if defined(__X86SIM__) || defined(__AIESIM__)
  #include <cstdio>
  #define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
#endif

namespace ggttg {

// ============================================================================
// K1 Packet Stream Version: For use with pktsplit
// 
// Input: input_pktstream (reads packet header, then PSP data, handles TLAST)
// Output: Cascade token (9 wavefunctions + JAMP)
// ============================================================================
template<int BATCH>
void kernel_k1_wfgen_pkt(
    ::input_pktstream* __restrict psp_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    DEBUG_PRINT("[K1_WFGEN_PKT] Starting: BATCH=%d\n", BATCH);
    
    // Read packet header and extract ID for tracing
    bool hdr_tlast;
    uint32_t pkt_header = readincr(psp_in, hdr_tlast);
    uint32_t pkt_id = pkt_header & 0x1F;  // Bits [4:0]
    DEBUG_PRINT("[K1_PKT] Received packet ID=%u (header=0x%08X)\n", pkt_id, pkt_header);
    
    // Couplings
    const cfloat gc10 = get_gc10();  // VVV coupling
    const cfloat gc11 = get_gc11();  // FFV coupling
    
    for (int b = 0; b < BATCH; ++b) {
        // Load PSP (5 × 4 floats) - read as int32 and reinterpret as float
        v4f Pg1, Pg2, Pt, Ptbar, Pg3;
        bool tlast;
        
        for (int k = 0; k < 4; ++k) {
            int32_t tmp = readincr(psp_in, tlast);
            Pg1[k] = reinterpret_cast<float&>(tmp);
        }
        for (int k = 0; k < 4; ++k) {
            int32_t tmp = readincr(psp_in, tlast);
            Pg2[k] = reinterpret_cast<float&>(tmp);
        }
        for (int k = 0; k < 4; ++k) {
            int32_t tmp = readincr(psp_in, tlast);
            Pt[k] = reinterpret_cast<float&>(tmp);
        }
        for (int k = 0; k < 4; ++k) {
            int32_t tmp = readincr(psp_in, tlast);
            Ptbar[k] = reinterpret_cast<float&>(tmp);
        }
        for (int k = 0; k < 4; ++k) {
            int32_t tmp = readincr(psp_in, tlast);
            Pg3[k] = reinterpret_cast<float&>(tmp);
        }

      #if GGTTG_HEL_MODE == 2
        // Cache external wavefunctions for hel=±1
        const v8c g1_m = vxxxxx(Pg1,  0.0f, -1, -1);
        const v8c g1_p = vxxxxx(Pg1,  0.0f, +1, -1);
        const v8c g2_m = vxxxxx(Pg2,  0.0f, -1, -1);
        const v8c g2_p = vxxxxx(Pg2,  0.0f, +1, -1);
        const v8c t_m  = oxxxxx(Pt,   MT,   -1, +1);
        const v8c t_p  = oxxxxx(Pt,   MT,   +1, +1);
        const v8c tb_m = ixxxxx(Ptbar, MT,  -1, -1);
        const v8c tb_p = ixxxxx(Ptbar, MT,  +1, -1);
        const v8c g3_m = vxxxxx(Pg3,  0.0f, -1, +1);
        const v8c g3_p = vxxxxx(Pg3,  0.0f, +1, +1);

        for (int h = 0; h < 32; ++h) {
            const int b_g1 = (h >> 4) & 1;
            const int b_g2 = (h >> 3) & 1;
            const int b_t  = (h >> 2) & 1;
            const int b_tb = (h >> 1) & 1;
            const int b_g3 = (h >> 0) & 1;

            const v8c w0 = b_g1 ? g1_p : g1_m;
            const v8c w1 = b_g2 ? g2_p : g2_m;
            const v8c w2 = b_t  ? t_p  : t_m;
            const v8c w3 = b_tb ? tb_p : tb_m;
            const v8c w4 = b_g3 ? g3_p : g3_m;

            const v8c w5_01 = VVV1P0_1(w0, w1, gc10, 0.0f, 0.0f);
            const v8c w10   = VVV1P0_1(w1, w4, gc10, 0.0f, 0.0f);
            const v8c w5_04 = VVV1P0_1(w0, w4, gc10, 0.0f, 0.0f);

            cfloat jamp[6];
            for (int i = 0; i < 6; ++i) jamp[i] = {0.0f, 0.0f};

            token_ext::write_token_8wf_k1(token_out,
                                           w0, w1, w2, w3, w4,
                                           w5_01, w10, w5_04,
                                           jamp);
        }
      #else
        // Baseline mode - recompute per helicity
        for (int h = 0; h < 32; ++h) {
            const int hel_g1 = GET_HEL_G1(h);
            const int hel_g2 = GET_HEL_G2(h);
            const int hel_t  = GET_HEL_T(h);
            const int hel_tb = GET_HEL_TB(h);
            const int hel_g3 = GET_HEL_G3(h);

            v8c w0 = vxxxxx(Pg1,  0.0f, hel_g1, -1);
            v8c w1 = vxxxxx(Pg2,  0.0f, hel_g2, -1);
            v8c w2 = oxxxxx(Pt,   MT,   hel_t,  +1);
            v8c w3 = ixxxxx(Ptbar, MT,  hel_tb, -1);
            v8c w4 = vxxxxx(Pg3,  0.0f, hel_g3, +1);

            v8c w5_01 = VVV1P0_1(w0, w1, gc10, 0.0f, 0.0f);
            v8c w10   = VVV1P0_1(w1, w4, gc10, 0.0f, 0.0f);
            v8c w5_04 = VVV1P0_1(w0, w4, gc10, 0.0f, 0.0f);

            cfloat jamp[6];
            for (int i = 0; i < 6; ++i) jamp[i] = {0.0f, 0.0f};

            token_ext::write_token_8wf_k1(token_out,
                                           w0, w1, w2, w3, w4,
                                           w5_01, w10, w5_04,
                                           jamp);
        }
      #endif
    }
    
    DEBUG_PRINT("[K1_PKT] Completed packet ID=%u, passed to cascade\n", pkt_id);
    DEBUG_PRINT("[K1_WFGEN_PKT] Done\n");
}

} // namespace ggttg
