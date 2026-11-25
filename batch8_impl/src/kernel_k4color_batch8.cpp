// ============================================================================
// kernel_k4color_batch8.cpp
//
// Batched K4-equivalent: D16 (VVVV1P0_1/3P0_1/4P0_1 + FFV1_0_batch8, 3
// sub-diagrams) + color reduction -> ME2, batched across 8 PSPs. Reads
// the 5wf-batch+jamp token from K3-batch. Color reduction is pure real
// (v8f) arithmetic.
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
void kernel_k4color_batch8(
    ::input_cascade<caccfloat>* __restrict token_in,
    ::output_stream<float>* __restrict me2_out)
{
    const cfloat gc11 = get_gc11();
    const cfloat gc12 = get_gc12();

    v8f me2_sum = aie::zeros<float,8>();

    for (int h = 0; h < 32; h++) {
        WFBatch8 w0,w1,w2,w3,w4;
        v8c jamp[6];
        token_batch8::read_token_5wf_batch8_jamp(token_in, w0,w1,w2,w3,w4, jamp);

        WFBatch8 wv1 = VVVV1P0_1_batch8(w0, w1, w4, gc12, 0.0f, 0.0f);
        WFBatch8 wv3 = VVVV3P0_1_batch8(w0, w1, w4, gc12, 0.0f, 0.0f);
        WFBatch8 wv4 = VVVV4P0_1_batch8(w0, w1, w4, gc12, 0.0f, 0.0f);

        v8c amp;
        amp = FFV1_0_batch8(w3, w2, wv1, gc11);
        jamp[0] = cadd8(jamp[0], amp); jamp[1] = csub8(jamp[1], amp);
        jamp[3] = csub8(jamp[3], amp); jamp[5] = cadd8(jamp[5], amp);

        amp = FFV1_0_batch8(w3, w2, wv3, gc11);
        jamp[1] = csub8(jamp[1], amp); jamp[2] = cadd8(jamp[2], amp);
        jamp[3] = csub8(jamp[3], amp); jamp[4] = cadd8(jamp[4], amp);

        amp = FFV1_0_batch8(w3, w2, wv4, gc11);
        jamp[0] = csub8(jamp[0], amp); jamp[2] = cadd8(jamp[2], amp);
        jamp[4] = cadd8(jamp[4], amp); jamp[5] = csub8(jamp[5], amp);

        // Color reduction, pure real (v8f) arithmetic across 8 PSPs.
        v8f j_re[6], j_im[6];
        for (int i = 0; i < 6; i++) { j_re[i] = aie::real(jamp[i]); j_im[i] = aie::imag(jamp[i]); }

        v8f mag2[6];
        for (int i = 0; i < 6; i++)
            mag2[i] = aie::add(aie::mul(j_re[i],j_re[i]).template to_vector<float>(),
                                 aie::mul(j_im[i],j_im[i]).template to_vector<float>());

        auto dot = [&](int a, int b) {
            return aie::add(aie::mul(j_re[a],j_re[b]).template to_vector<float>(),
                              aie::mul(j_im[a],j_im[b]).template to_vector<float>());
        };
        v8f dot_01=dot(0,1), dot_02=dot(0,2), dot_03=dot(0,3), dot_04=dot(0,4), dot_05=dot(0,5);
        v8f dot_12=dot(1,2), dot_13=dot(1,3), dot_14=dot(1,4), dot_15=dot(1,5);
        v8f dot_23=dot(2,3), dot_24=dot(2,4), dot_25=dot(2,5);
        v8f dot_34=dot(3,4), dot_35=dot(3,5);
        v8f dot_45=dot(4,5);

        v8f hel_me2 = aie::zeros<float,8>();
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_DIAG_NORM[0]), mag2[0]).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_DIAG_NORM[1]), mag2[1]).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_DIAG_NORM[2]), mag2[2]).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_DIAG_NORM[3]), mag2[3]).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_DIAG_NORM[4]), mag2[4]).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_DIAG_NORM[5]), mag2[5]).template to_vector<float>());

        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[0]),  dot_01).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[1]),  dot_02).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[2]),  dot_03).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[3]),  dot_04).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[4]),  dot_05).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[5]),  dot_12).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[6]),  dot_13).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[7]),  dot_14).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[8]),  dot_15).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[9]),  dot_23).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[10]), dot_24).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[11]), dot_25).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[12]), dot_34).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[13]), dot_35).template to_vector<float>());
        hel_me2 = aie::add(hel_me2, aie::mul(bcastf8(COLOR_OFFDIAG_NORM[14]), dot_45).template to_vector<float>());

        me2_sum = aie::add(me2_sum, hel_me2);
    }

    me2_sum = aie::mul(me2_sum, bcastf8(1.0f/256.0f)).template to_vector<float>();

    for (int p = 0; p < NPSP; p++) writeincr(me2_out, me2_sum[p]);
}

} // namespace ggttg
