// ============================================================================
// kernel_k2a_propA_batch8.cpp
//
// Split-tile architecture: the monolithic K2a_batch8 (27188 B) is split
// into three tiles. This one computes the FFV1_1-family propagators
// (w7, w5_20, w11 -- all built on FFV1_1_batch8/ffv_linear_core_batch,
// measured cost ~8854 B), forwards the 11wf-equivalent token.
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
void kernel_k2a_propA_batch8(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_cascade<caccfloat>* __restrict token_out)
{
    const cfloat gc11 = get_gc11();

    for (int h = 0; h < 32; h++) {
        WFBatch8 w0,w1,w2,w3,w4,w5_01,w10,w5_04;
        v8c jamp[6];
        token_batch8::read_token_8wf_batch8(token_in, w0,w1,w2,w3,w4,w5_01,w10,w5_04, jamp);

        WFBatch8 w7    = FFV1_1_batch8(w2, w4, gc11, MT, WT);
        WFBatch8 w5_20 = FFV1_1_batch8(w2, w0, gc11, MT, WT);
        WFBatch8 w11   = FFV1_1_batch8(w2, w1, gc11, MT, WT);

        token_batch8::write_token_11wf_batch8(token_out, w0,w1,w2,w3,w4, w5_01,w10,w5_04, w7, w5_20, w11, jamp);
    }
}

} // namespace ggttg
