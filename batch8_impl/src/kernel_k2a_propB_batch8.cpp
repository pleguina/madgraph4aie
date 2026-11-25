// ============================================================================
// kernel_k2a_propB_batch8.cpp
//
// Second of the three K2a split tiles: computes the FFV1_2-family
// propagators (w8, w9, w5_30 -- built on FFV1_2_batch8/ffv2_linear_core_batch,
// measured cost ~8998 B), forwards the full 14wf-equivalent token.
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
void kernel_k2a_propB_batch8(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc11 = get_gc11();

    for (int h = 0; h < 32; h++) {
        WFBatch8 w0,w1,w2,w3,w4,w5_01,w10,w5_04,w7,w5_20,w11;
        v8c jamp[6];
        token_batch8::read_token_11wf_batch8(token_in, w0,w1,w2,w3,w4, w5_01,w10,w5_04, w7,w5_20,w11, jamp);

        WFBatch8 w8    = FFV1_2_batch8(w3, w4, gc11, MT, WT);
        WFBatch8 w9    = FFV1_2_batch8(w3, w1, gc11, MT, WT);
        WFBatch8 w5_30 = FFV1_2_batch8(w3, w0, gc11, MT, WT);

        token_batch8::write_token_14wf_batch8(token_out, w0,w1,w2,w3,w4, w5_01,w10,w5_04,
                                                w7,w8,w9,w5_20,w5_30,w11, jamp);
    }
}

} // namespace ggttg
