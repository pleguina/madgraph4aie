// ============================================================================
// kernel_k1a_wfgen_batch8.cpp
//
// Split-tile architecture. A monolithic K1 (22532 B, over the 16384 B PM
// budget) is split into two tiles:
// K1a generates all 5 external batched wavefunctions (measured cost
// ~10086 B: 2x vxxxxx_beam_batch8 + vxxxxx_generic_batch8 +
// oxxxxx_generic_batch8 + ixxxxx_generic_batch8), K1b (kernel_k1b_
// vvvprop_batch8.cpp) computes the 3 VVV1P0_1 propagators (~8730 B).
// ============================================================================

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "helas_batch8.h"
#include "token_batch8.h"
#include "ggttg_config.h"
#include "ggttg_params.h"

namespace ggttg {

template<int NPSP>
void kernel_k1a_wfgen_batch8(
    ::input_stream<float>* __restrict psp_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    v8f Pg1[4], Pg2[4], Pt[4], Ptbar[4], Pg3[4];

    for (int p = 0; p < NPSP; p++) {
        float row[20];
        for (int i = 0; i < 20; i++) row[i] = readincr(psp_in);
        for (int k = 0; k < 4; k++) {
            Pg1[k][p]   = row[0+k];
            Pg2[k][p]   = row[4+k];
            Pt[k][p]    = row[8+k];
            Ptbar[k][p] = row[12+k];
            Pg3[k][p]   = row[16+k];
        }
    }

    for (int h = 0; h < 32; h++) {
        const int b_g1 = (h >> 4) & 1;
        const int b_g2 = (h >> 3) & 1;
        const int b_t  = (h >> 2) & 1;
        const int b_tb = (h >> 1) & 1;
        const int b_g3 = (h >> 0) & 1;

        WFBatch8 w0 = vxxxxx_beam_batch8(Pg1, b_g1 ? +1 : -1, -1);
        WFBatch8 w1 = vxxxxx_beam_batch8(Pg2, b_g2 ? +1 : -1, -1);
        WFBatch8 w2 = oxxxxx_generic_batch8(Pt, MT, b_t ? +1 : -1, +1);
        WFBatch8 w3 = ixxxxx_generic_batch8(Ptbar, MT, b_tb ? +1 : -1, -1);
        WFBatch8 w4 = vxxxxx_generic_batch8(Pg3, b_g3 ? +1 : -1, +1);

        token_batch8::write_token_5wf_batch8(token_out, w0, w1, w2, w3, w4);
    }
}

} // namespace ggttg
