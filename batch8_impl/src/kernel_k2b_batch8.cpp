// ============================================================================
// kernel_k2b_batch8.cpp
//
// Batched K2b: D5, D6, D7, D8, D9, D10, D11, D13, D14 -- 9 diagrams, all
// using propagators forwarded from K2a-propB (no recompute). Reads the
// 14wf-batch token,
// writes the standard 8wf-batch token to K34 (still needs w5_01,w10,w5_04
// for K34's D1,D12,D15).
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
void kernel_k2b_batch8(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc11 = get_gc11();

    for (int h = 0; h < 32; h++) {
        WFBatch8 w0,w1,w2,w3,w4,w5_01,w10,w5_04,w7,w8,w9,w5_20,w5_30,w11;
        v8c jamp[6];
        token_batch8::read_token_14wf_batch8(token_in, w0,w1,w2,w3,w4, w5_01,w10,w5_04,
                                               w7,w8,w9,w5_20,w5_30,w11, jamp);

        v8c amp;

        // D5: FFV1_0(w3, w5_20, w10)
        amp = FFV1_0_batch8(w3, w5_20, w10, gc11);
        jamp[0] = cadd8(jamp[0], cmul8(bcast8(cI), amp));
        jamp[1] = csub8(jamp[1], cmul8(bcast8(cI), amp));

        // D6: FFV1_0(w8, w5_20, w1)
        amp = FFV1_0_batch8(w8, w5_20, w1, gc11);
        jamp[0] = csub8(jamp[0], amp);

        // D7: FFV1_0(w5_30, w11, w4)
        amp = FFV1_0_batch8(w5_30, w11, w4, gc11);
        jamp[3] = csub8(jamp[3], amp);

        // D8: FFV1_0(w5_30, w2, w10)
        amp = FFV1_0_batch8(w5_30, w2, w10, gc11);
        jamp[3] = cadd8(jamp[3], cmul8(bcast8(cI), amp));
        jamp[5] = csub8(jamp[5], cmul8(bcast8(cI), amp));

        // D9: FFV1_0(w5_30, w7, w1)
        amp = FFV1_0_batch8(w5_30, w7, w1, gc11);
        jamp[5] = csub8(jamp[5], amp);

        // D10: FFV1_0(w3, w11, w5_04)
        amp = FFV1_0_batch8(w3, w11, w5_04, gc11);
        jamp[2] = cadd8(jamp[2], cmul8(bcast8(cI), amp));
        jamp[3] = csub8(jamp[3], cmul8(bcast8(cI), amp));

        // D11: FFV1_0(w9, w2, w5_04)
        amp = FFV1_0_batch8(w9, w2, w5_04, gc11);
        jamp[1] = cadd8(jamp[1], cmul8(bcast8(cI), amp));
        jamp[4] = csub8(jamp[4], cmul8(bcast8(cI), amp));

        // D13: FFV1_0(w8, w11, w0)
        amp = FFV1_0_batch8(w8, w11, w0, gc11);
        jamp[2] = csub8(jamp[2], amp);

        // D14: FFV1_0(w9, w7, w0)
        amp = FFV1_0_batch8(w9, w7, w0, gc11);
        jamp[4] = csub8(jamp[4], amp);

        token_batch8::write_token_8wf_batch8(token_out, w0,w1,w2,w3,w4, w5_01,w10,w5_04, jamp);
    }
}

} // namespace ggttg
