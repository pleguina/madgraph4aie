// ============================================================================
// kernel_k2a_amps_batch8.cpp
//
// Third of the three K2a split tiles: reads the full 14wf-equivalent token
// (all 6 propagators already computed upstream), computes D2, D3, D4
// (FFV1_0_batch8, measured cost ~3718 B standalone -- by far the cheapest
// of the three split tiles). Forwards the 14wf-equivalent token onward to
// K2b (still needs all 6 propagators for D5-D14).
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
void kernel_k2a_amps_batch8(
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
        amp = FFV1_0_batch8(w3, w7, w5_01, gc11);
        jamp[4] = cadd8(jamp[4], cmul8(bcast8(cI), amp));
        jamp[5] = csub8(jamp[5], cmul8(bcast8(cI), amp));

        amp = FFV1_0_batch8(w8, w2, w5_01, gc11);
        jamp[0] = cadd8(jamp[0], cmul8(bcast8(cI), amp));
        jamp[2] = csub8(jamp[2], cmul8(bcast8(cI), amp));

        amp = FFV1_0_batch8(w9, w5_20, w4, gc11);
        jamp[1] = csub8(jamp[1], amp);

        token_batch8::write_token_14wf_batch8(token_out, w0,w1,w2,w3,w4, w5_01,w10,w5_04,
                                                w7,w8,w9,w5_20,w5_30,w11, jamp);
    }
}

} // namespace ggttg
