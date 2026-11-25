// ============================================================================
// token_batch8.h
//
// Batched cascade token helpers. Each WF component (6 meaningful lanes,
// no padding needed -- unlike the per-PSP scheme, which pads 6 components
// into 2x4-lane beats) is sent as ONE full 8-lane beat (8 PSPs of that
// component, fully utilized). A WF costs 6 beats here vs. the per-PSP
// scheme's 2 beats/WF -- but each beat carries 8 PSPs, so total floats
// moved per PSP is actually LOWER (no padding waste): 6 beats x 8 lanes /
// 8 PSPs = 6 complex/PSP/WF, vs. today's 2 beats x 4 lanes / 1 PSP = 8
// complex-slots/PSP/WF (6 meaningful + 2 wasted padding).
// ============================================================================
#pragma once

#include "helas_batch8.h"

namespace ggttg {
namespace token_batch8 {

// Single-beat aie::accum<caccfloat,8> encoding: one cascade beat per
// wavefunction component. A two-beat accum<caccfloat,4> encoding roughly
// doubles the inlined size of read_wf/write_wf and of every
// read_token_*/write_token_* call site, which pushes tiles over the 16 KB
// program-memory budget.
inline void write_wf(::output_cascade<caccfloat>* out, const WFBatch8& wf) {
    for (int k = 0; k < 6; k++) {
        aie::accum<caccfloat, 8> acc(wf.c[k]);
        writeincr(out, acc);
    }
}
inline WFBatch8 read_wf(::input_cascade<caccfloat>* in) {
    WFBatch8 wf;
    for (int k = 0; k < 6; k++) {
        aie::accum<caccfloat, 8> acc = readincr_v<8>(in);
        wf.c[k] = acc.to_vector<cfloat>();
    }
    wf.c[6] = zero8(); wf.c[7] = zero8();
    return wf;
}

// JAMP: 6 complex accumulators, each batched across 8 PSPs -- one beat each.
inline void write_jamp(::output_cascade<caccfloat>* out, const v8c jamp[6]) {
    for (int k = 0; k < 6; k++) {
        aie::accum<caccfloat, 8> acc(jamp[k]);
        writeincr(out, acc);
    }
}
inline void read_jamp(::input_cascade<caccfloat>* in, v8c jamp[6]) {
    for (int k = 0; k < 6; k++) {
        aie::accum<caccfloat, 8> acc = readincr_v<8>(in);
        jamp[k] = acc.to_vector<cfloat>();
    }
}

// 8wf-equivalent token: w0-w4, w5_01, w10, w5_04 + jamp[6], all batched.
inline void write_token_8wf_batch8(::output_cascade<caccfloat>* out,
    const WFBatch8& w0, const WFBatch8& w1, const WFBatch8& w2, const WFBatch8& w3, const WFBatch8& w4,
    const WFBatch8& w5_01, const WFBatch8& w10, const WFBatch8& w5_04, const v8c jamp[6])
{
    write_wf(out,w0); write_wf(out,w1); write_wf(out,w2); write_wf(out,w3); write_wf(out,w4);
    write_wf(out,w5_01); write_wf(out,w10); write_wf(out,w5_04);
    write_jamp(out, jamp);
}
inline void read_token_8wf_batch8(::input_cascade<caccfloat>* in,
    WFBatch8& w0, WFBatch8& w1, WFBatch8& w2, WFBatch8& w3, WFBatch8& w4,
    WFBatch8& w5_01, WFBatch8& w10, WFBatch8& w5_04, v8c jamp[6])
{
    w0=read_wf(in); w1=read_wf(in); w2=read_wf(in); w3=read_wf(in); w4=read_wf(in);
    w5_01=read_wf(in); w10=read_wf(in); w5_04=read_wf(in);
    read_jamp(in, jamp);
}

// 5wf-equivalent token: just w0-w4, no jamp (used K1a->K1b, before any
// diagram contributions exist yet).
inline void write_token_5wf_batch8(::output_cascade<caccfloat>* out,
    const WFBatch8& w0, const WFBatch8& w1, const WFBatch8& w2, const WFBatch8& w3, const WFBatch8& w4)
{
    write_wf(out,w0); write_wf(out,w1); write_wf(out,w2); write_wf(out,w3); write_wf(out,w4);
}
inline void read_token_5wf_batch8(::input_cascade<caccfloat>* in,
    WFBatch8& w0, WFBatch8& w1, WFBatch8& w2, WFBatch8& w3, WFBatch8& w4)
{
    w0=read_wf(in); w1=read_wf(in); w2=read_wf(in); w3=read_wf(in); w4=read_wf(in);
}

// 5wf-equivalent token WITH jamp: w0-w4 + jamp[6] (K3-batch -> K4-batch,
// matching the scalar write_token_5wf shape).
inline void write_token_5wf_batch8_jamp(::output_cascade<caccfloat>* out,
    const WFBatch8& w0, const WFBatch8& w1, const WFBatch8& w2, const WFBatch8& w3, const WFBatch8& w4,
    const v8c jamp[6])
{
    write_wf(out,w0); write_wf(out,w1); write_wf(out,w2); write_wf(out,w3); write_wf(out,w4);
    write_jamp(out, jamp);
}
inline void read_token_5wf_batch8_jamp(::input_cascade<caccfloat>* in,
    WFBatch8& w0, WFBatch8& w1, WFBatch8& w2, WFBatch8& w3, WFBatch8& w4,
    v8c jamp[6])
{
    w0=read_wf(in); w1=read_wf(in); w2=read_wf(in); w3=read_wf(in); w4=read_wf(in);
    read_jamp(in, jamp);
}

// 14wf-equivalent token: 8wf + 6 propagators (w7,w8,w9,w5_20,w5_30,w11) + jamp.
inline void write_token_14wf_batch8(::output_cascade<caccfloat>* out,
    const WFBatch8& w0, const WFBatch8& w1, const WFBatch8& w2, const WFBatch8& w3, const WFBatch8& w4,
    const WFBatch8& w5_01, const WFBatch8& w10, const WFBatch8& w5_04,
    const WFBatch8& w7, const WFBatch8& w8, const WFBatch8& w9,
    const WFBatch8& w5_20, const WFBatch8& w5_30, const WFBatch8& w11,
    const v8c jamp[6])
{
    write_wf(out,w0); write_wf(out,w1); write_wf(out,w2); write_wf(out,w3); write_wf(out,w4);
    write_wf(out,w5_01); write_wf(out,w10); write_wf(out,w5_04);
    write_wf(out,w7); write_wf(out,w8); write_wf(out,w9);
    write_wf(out,w5_20); write_wf(out,w5_30); write_wf(out,w11);
    write_jamp(out, jamp);
}
inline void read_token_14wf_batch8(::input_cascade<caccfloat>* in,
    WFBatch8& w0, WFBatch8& w1, WFBatch8& w2, WFBatch8& w3, WFBatch8& w4,
    WFBatch8& w5_01, WFBatch8& w10, WFBatch8& w5_04,
    WFBatch8& w7, WFBatch8& w8, WFBatch8& w9,
    WFBatch8& w5_20, WFBatch8& w5_30, WFBatch8& w11,
    v8c jamp[6])
{
    w0=read_wf(in); w1=read_wf(in); w2=read_wf(in); w3=read_wf(in); w4=read_wf(in);
    w5_01=read_wf(in); w10=read_wf(in); w5_04=read_wf(in);
    w7=read_wf(in); w8=read_wf(in); w9=read_wf(in);
    w5_20=read_wf(in); w5_30=read_wf(in); w11=read_wf(in);
    read_jamp(in, jamp);
}

// 9wf-equivalent token: 8wf + w6 (K3a -> K3b).
inline void write_token_9wf_batch8(::output_cascade<caccfloat>* out,
    const WFBatch8& w0, const WFBatch8& w1, const WFBatch8& w2, const WFBatch8& w3, const WFBatch8& w4,
    const WFBatch8& w5_01, const WFBatch8& w10, const WFBatch8& w5_04, const WFBatch8& w6,
    const v8c jamp[6])
{
    write_wf(out,w0); write_wf(out,w1); write_wf(out,w2); write_wf(out,w3); write_wf(out,w4);
    write_wf(out,w5_01); write_wf(out,w10); write_wf(out,w5_04); write_wf(out,w6);
    write_jamp(out, jamp);
}
inline void read_token_9wf_batch8(::input_cascade<caccfloat>* in,
    WFBatch8& w0, WFBatch8& w1, WFBatch8& w2, WFBatch8& w3, WFBatch8& w4,
    WFBatch8& w5_01, WFBatch8& w10, WFBatch8& w5_04, WFBatch8& w6,
    v8c jamp[6])
{
    w0=read_wf(in); w1=read_wf(in); w2=read_wf(in); w3=read_wf(in); w4=read_wf(in);
    w5_01=read_wf(in); w10=read_wf(in); w5_04=read_wf(in); w6=read_wf(in);
    read_jamp(in, jamp);
}

// K4a -> K4b token: w2, w3 (needed for FFV1_0's F1,F2) + the 3 VVVV
// off-shell vectors + jamp.
inline void write_token_vvvv_batch8(::output_cascade<caccfloat>* out,
    const WFBatch8& w2, const WFBatch8& w3,
    const WFBatch8& wv1, const WFBatch8& wv3, const WFBatch8& wv4,
    const v8c jamp[6])
{
    write_wf(out,w2); write_wf(out,w3); write_wf(out,wv1); write_wf(out,wv3); write_wf(out,wv4);
    write_jamp(out, jamp);
}
inline void read_token_vvvv_batch8(::input_cascade<caccfloat>* in,
    WFBatch8& w2, WFBatch8& w3, WFBatch8& wv1, WFBatch8& wv3, WFBatch8& wv4,
    v8c jamp[6])
{
    w2=read_wf(in); w3=read_wf(in); wv1=read_wf(in); wv3=read_wf(in); wv4=read_wf(in);
    read_jamp(in, jamp);
}

// 11wf-equivalent token: 8wf + 3 FFV1_1-family propagators (w7,w5_20,w11).
inline void write_token_11wf_batch8(::output_cascade<caccfloat>* out,
    const WFBatch8& w0, const WFBatch8& w1, const WFBatch8& w2, const WFBatch8& w3, const WFBatch8& w4,
    const WFBatch8& w5_01, const WFBatch8& w10, const WFBatch8& w5_04,
    const WFBatch8& w7, const WFBatch8& w5_20, const WFBatch8& w11,
    const v8c jamp[6])
{
    write_wf(out,w0); write_wf(out,w1); write_wf(out,w2); write_wf(out,w3); write_wf(out,w4);
    write_wf(out,w5_01); write_wf(out,w10); write_wf(out,w5_04);
    write_wf(out,w7); write_wf(out,w5_20); write_wf(out,w11);
    write_jamp(out, jamp);
}
inline void read_token_11wf_batch8(::input_cascade<caccfloat>* in,
    WFBatch8& w0, WFBatch8& w1, WFBatch8& w2, WFBatch8& w3, WFBatch8& w4,
    WFBatch8& w5_01, WFBatch8& w10, WFBatch8& w5_04,
    WFBatch8& w7, WFBatch8& w5_20, WFBatch8& w11,
    v8c jamp[6])
{
    w0=read_wf(in); w1=read_wf(in); w2=read_wf(in); w3=read_wf(in); w4=read_wf(in);
    w5_01=read_wf(in); w10=read_wf(in); w5_04=read_wf(in);
    w7=read_wf(in); w5_20=read_wf(in); w11=read_wf(in);
    read_jamp(in, jamp);
}

} // namespace token_batch8
} // namespace ggttg
