// ============================================================================
// kernel_k3_batch8.cpp
//
// Batched K3: D1, D12, D15 via VVV1_0_batch8, w6 via FFV1P0_3_batch8
// (separate from K4 because VVV1_0 + FFV1P0_3 + VVVV + colour do not fit
// one tile's program memory). Reads the 8wf-batch token, writes a 5wf-batch
// token (w0-w4 + jamp) to K4.
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
void kernel_k3_batch8(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc10 = get_gc10();
    const cfloat gc11 = get_gc11();

    for (int h = 0; h < 32; h++) {
        WFBatch8 w0,w1,w2,w3,w4,w5_01,w10,w5_04;
        v8c jamp[6];
        token_batch8::read_token_8wf_batch8(token_in, w0,w1,w2,w3,w4,w5_01,w10,w5_04, jamp);

        WFBatch8 w6;
        v8c w6_c2345[4];
        w6.c[0] = FFV1P0_3_batch8(w3, w2, gc11, 0.0f, 0.0f, w6_c2345);
        w6.c[1] = cadd8(w3.c[1], w2.c[1]);
        w6.c[2]=w6_c2345[0]; w6.c[3]=w6_c2345[1]; w6.c[4]=w6_c2345[2]; w6.c[5]=w6_c2345[3];

        v8c amp;

        // D1: VVV1_0(w5_01, w6, w4)
        amp = VVV1_0_batch8(w5_01, w6, w4, gc10);
        jamp[0] = csub8(jamp[0], amp);
        jamp[2] = cadd8(jamp[2], amp);
        jamp[4] = cadd8(jamp[4], amp);
        jamp[5] = csub8(jamp[5], amp);

        // D12: VVV1_0(w5_04, w1, w6)
        amp = VVV1_0_batch8(w5_04, w1, w6, gc10);
        jamp[1] = cadd8(jamp[1], amp);
        jamp[2] = csub8(jamp[2], amp);
        jamp[3] = cadd8(jamp[3], amp);
        jamp[4] = csub8(jamp[4], amp);

        // D15: VVV1_0(w0, w10, w6)
        amp = VVV1_0_batch8(w0, w10, w6, gc10);
        jamp[0] = cadd8(jamp[0], amp);
        jamp[1] = csub8(jamp[1], amp);
        jamp[3] = csub8(jamp[3], amp);
        jamp[5] = cadd8(jamp[5], amp);

        token_batch8::write_token_5wf_batch8_jamp(token_out, w0, w1, w2, w3, w4, jamp);
    }
}

} // namespace ggttg
