// ============================================================================
// kernel_k1b_vvvprop_batch8.cpp
//
// Second half of the split K1: reads w0-w4 from K1a, computes the 3
// VVV1P0_1 propagators (w5_01, w10, w5_04), forwards the 8wf-equivalent
// batched token (jamp initialized to zero) to K2a-propA.
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
void kernel_k1b_vvvprop_batch8(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc10 = get_gc10();

    for (int h = 0; h < 32; h++) {
        WFBatch8 w0, w1, w2, w3, w4;
        token_batch8::read_token_5wf_batch8(token_in, w0, w1, w2, w3, w4);

        WFBatch8 w5_01 = VVV1P0_1_batch8(w0, w1, gc10, 0.0f, 0.0f);
        WFBatch8 w10   = VVV1P0_1_batch8(w1, w4, gc10, 0.0f, 0.0f);
        WFBatch8 w5_04 = VVV1P0_1_batch8(w0, w4, gc10, 0.0f, 0.0f);

        v8c jamp[6];
        for (int i = 0; i < 6; i++) jamp[i] = zero8();

        token_batch8::write_token_8wf_batch8(token_out, w0, w1, w2, w3, w4, w5_01, w10, w5_04, jamp);
    }
}

} // namespace ggttg
