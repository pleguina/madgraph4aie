// ============================================================================
// kernels_4k_token.h
// 
// Token format helpers for 4K kernel architecture
// 
// Token formats:
//   K1→K2→K3: Extended token (9 WFs + JAMP)
//   K3→K4:    Standard token (5 WFs + JAMP)
// ============================================================================

#ifndef KERNELS_4K_TOKEN_H
#define KERNELS_4K_TOKEN_H

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "helas/processing_core_common.h"

namespace ggttg {
namespace token_ext {

// ============================================================================
// Helper: write single wavefunction (lanes 0..5, 2 cascade beats)
// ============================================================================
inline void write_wf(::output_cascade<caccfloat>* out, const v8c& wf) {
    // First beat: lanes 0..3
    aie::vector<cfloat, 4> vec1;
    vec1[0] = wf.get(0);
    vec1[1] = wf.get(1);
    vec1[2] = wf.get(2);
    vec1[3] = wf.get(3);
    aie::accum<caccfloat, 4> acc1(vec1);
    writeincr(out, acc1);
    
    // Second beat: lanes 4..5 + padding
    aie::vector<cfloat, 4> vec2;
    vec2[0] = wf.get(4);
    vec2[1] = wf.get(5);
    vec2[2] = cfloat{0.f, 0.f};
    vec2[3] = cfloat{0.f, 0.f};
    aie::accum<caccfloat, 4> acc2(vec2);
    writeincr(out, acc2);
}

// ============================================================================
// Helper: read single wavefunction from cascade
// ============================================================================
inline v8c read_wf(::input_cascade<caccfloat>* in) {
    v8c wf;
    
    // First beat: lanes 0..3
    aie::accum<caccfloat, 4> acc1 = readincr_v<4>(in);
    aie::vector<cfloat, 4> vec1 = acc1.to_vector<cfloat>();
    wf.set(vec1[0], 0);
    wf.set(vec1[1], 1);
    wf.set(vec1[2], 2);
    wf.set(vec1[3], 3);
    
    // Second beat: lanes 4..5
    aie::accum<caccfloat, 4> acc2 = readincr_v<4>(in);
    aie::vector<cfloat, 4> vec2 = acc2.to_vector<cfloat>();
    wf.set(vec2[0], 4);
    wf.set(vec2[1], 5);
    wf.set(cfloat{0.f, 0.f}, 6);
    wf.set(cfloat{0.f, 0.f}, 7);
    
    return wf;
}

// ============================================================================
// Helper: write JAMP[6]
// ============================================================================
inline void write_jamp(::output_cascade<caccfloat>* out, const cfloat j[6]) {
    aie::vector<cfloat, 4> vec1;
    vec1[0] = j[0];
    vec1[1] = j[1];
    vec1[2] = j[2];
    vec1[3] = j[3];
    aie::accum<caccfloat, 4> acc1(vec1);
    writeincr(out, acc1);

    aie::vector<cfloat, 4> vec2;
    vec2[0] = j[4];
    vec2[1] = j[5];
    vec2[2] = cfloat{0.f, 0.f};
    vec2[3] = cfloat{0.f, 0.f};
    aie::accum<caccfloat, 4> acc2(vec2);
    writeincr(out, acc2);
}

// ============================================================================
// Helper: read JAMP[6]
// ============================================================================
inline void read_jamp(::input_cascade<caccfloat>* in, cfloat j[6]) {
    aie::accum<caccfloat, 4> acc1 = readincr_v<4>(in);
    aie::vector<cfloat, 4> vec1 = acc1.to_vector<cfloat>();
    j[0] = vec1[0];
    j[1] = vec1[1];
    j[2] = vec1[2];
    j[3] = vec1[3];

    aie::accum<caccfloat, 4> acc2 = readincr_v<4>(in);
    aie::vector<cfloat, 4> vec2 = acc2.to_vector<cfloat>();
    j[4] = vec2[0];
    j[5] = vec2[1];
}

// ============================================================================
// Extended token K1→K2a→K2b→K3: 9 wavefunctions + JAMP[6]
// Standard format for 5k split architecture
// ============================================================================
inline void write_token_9wf(
    ::output_cascade<caccfloat>* out,
    const v8c& w0, const v8c& w1, const v8c& w2, const v8c& w3, const v8c& w4,
    const v8c& w5_01, const v8c& w10, const v8c& w5_04, const v8c& w6,
    const cfloat jamp[6])
{
    write_wf(out, w0);
    write_wf(out, w1);
    write_wf(out, w2);
    write_wf(out, w3);
    write_wf(out, w4);
    write_wf(out, w5_01);
    write_wf(out, w10);
    write_wf(out, w5_04);
    write_wf(out, w6);
    write_jamp(out, jamp);
}

inline void read_token_9wf(
    ::input_cascade<caccfloat>* in,
    v8c& w0, v8c& w1, v8c& w2, v8c& w3, v8c& w4,
    v8c& w5_01, v8c& w10, v8c& w5_04, v8c& w6,
    cfloat jamp[6])
{
    w0 = read_wf(in);
    w1 = read_wf(in);
    w2 = read_wf(in);
    w3 = read_wf(in);
    w4 = read_wf(in);
    w5_01 = read_wf(in);
    w10 = read_wf(in);
    w5_04 = read_wf(in);
    w6 = read_wf(in);
    read_jamp(in, jamp);
}

// ============================================================================
// Extended token K2a→K2b: 15 wavefunctions + JAMP[6]
// Original 9 WFs + 6 propagators (w7, w8, w9, w5_20, w5_30, w11)
// ============================================================================
inline void write_token_15wf(
    ::output_cascade<caccfloat>* out,
    const v8c& w0, const v8c& w1, const v8c& w2, const v8c& w3, const v8c& w4,
    const v8c& w5_01, const v8c& w10, const v8c& w5_04, const v8c& w6,
    const v8c& w7, const v8c& w8, const v8c& w9,
    const v8c& w5_20, const v8c& w5_30, const v8c& w11,
    const cfloat jamp[6])
{
    write_wf(out, w0);
    write_wf(out, w1);
    write_wf(out, w2);
    write_wf(out, w3);
    write_wf(out, w4);
    write_wf(out, w5_01);
    write_wf(out, w10);
    write_wf(out, w5_04);
    write_wf(out, w6);
    write_wf(out, w7);
    write_wf(out, w8);
    write_wf(out, w9);
    write_wf(out, w5_20);
    write_wf(out, w5_30);
    write_wf(out, w11);
    write_jamp(out, jamp);
}

inline void read_token_15wf(
    ::input_cascade<caccfloat>* in,
    v8c& w0, v8c& w1, v8c& w2, v8c& w3, v8c& w4,
    v8c& w5_01, v8c& w10, v8c& w5_04, v8c& w6,
    v8c& w7, v8c& w8, v8c& w9,
    v8c& w5_20, v8c& w5_30, v8c& w11,
    cfloat jamp[6])
{
    w0 = read_wf(in);
    w1 = read_wf(in);
    w2 = read_wf(in);
    w3 = read_wf(in);
    w4 = read_wf(in);
    w5_01 = read_wf(in);
    w10 = read_wf(in);
    w5_04 = read_wf(in);
    w6 = read_wf(in);
    w7 = read_wf(in);
    w8 = read_wf(in);
    w9 = read_wf(in);
    w5_20 = read_wf(in);
    w5_30 = read_wf(in);
    w11 = read_wf(in);
    read_jamp(in, jamp);
}

// ============================================================================
// Extended token from K1→K2: 8 wavefunctions + JAMP[6]
// Removed w6 (FFV1P0_3 moved to K3)
// ============================================================================
inline void write_token_8wf_k1(
    ::output_cascade<caccfloat>* out,
    const v8c& w0, const v8c& w1, const v8c& w2, const v8c& w3, const v8c& w4,
    const v8c& w5_01, const v8c& w10, const v8c& w5_04,
    const cfloat jamp[6])
{
    write_wf(out, w0);
    write_wf(out, w1);
    write_wf(out, w2);
    write_wf(out, w3);
    write_wf(out, w4);
    write_wf(out, w5_01);
    write_wf(out, w10);
    write_wf(out, w5_04);
    write_jamp(out, jamp);
}

inline void read_token_8wf_k1(
    ::input_cascade<caccfloat>* in,
    v8c& w0, v8c& w1, v8c& w2, v8c& w3, v8c& w4,
    v8c& w5_01, v8c& w10, v8c& w5_04,
    cfloat jamp[6])
{
    w0 = read_wf(in);
    w1 = read_wf(in);
    w2 = read_wf(in);
    w3 = read_wf(in);
    w4 = read_wf(in);
    w5_01 = read_wf(in);
    w10 = read_wf(in);
    w5_04 = read_wf(in);
    read_jamp(in, jamp);
}

// ============================================================================
// Extended token from K2→K3: 14 wavefunctions + JAMP[6]
// External WFs + VVV propagators + FFV propagators
// ============================================================================
inline void write_token_14wf_k2(
    ::output_cascade<caccfloat>* out,
    const v8c& w0, const v8c& w1, const v8c& w2, const v8c& w3, const v8c& w4,
    const v8c& w5_01, const v8c& w10, const v8c& w5_04,
    const v8c& w7, const v8c& w8, const v8c& w9,
    const v8c& w5_20, const v8c& w5_30, const v8c& w11,
    const cfloat jamp[6])
{
    write_wf(out, w0);
    write_wf(out, w1);
    write_wf(out, w2);
    write_wf(out, w3);
    write_wf(out, w4);
    write_wf(out, w5_01);
    write_wf(out, w10);
    write_wf(out, w5_04);
    write_wf(out, w7);
    write_wf(out, w8);
    write_wf(out, w9);
    write_wf(out, w5_20);
    write_wf(out, w5_30);
    write_wf(out, w11);
    write_jamp(out, jamp);
}

inline void read_token_14wf_k2(
    ::input_cascade<caccfloat>* in,
    v8c& w0, v8c& w1, v8c& w2, v8c& w3, v8c& w4,
    v8c& w5_01, v8c& w10, v8c& w5_04,
    v8c& w7, v8c& w8, v8c& w9,
    v8c& w5_20, v8c& w5_30, v8c& w11,
    cfloat jamp[6])
{
    w0 = read_wf(in);
    w1 = read_wf(in);
    w2 = read_wf(in);
    w3 = read_wf(in);
    w4 = read_wf(in);
    w5_01 = read_wf(in);
    w10 = read_wf(in);
    w5_04 = read_wf(in);
    w7 = read_wf(in);
    w8 = read_wf(in);
    w9 = read_wf(in);
    w5_20 = read_wf(in);
    w5_30 = read_wf(in);
    w11 = read_wf(in);
    read_jamp(in, jamp);
}

// ============================================================================
// Standard token: 5 wavefunctions + JAMP[6]
// Used between K3→K4
// ============================================================================
inline void write_token_5wf(
    ::output_cascade<caccfloat>* out,
    const v8c& w0, const v8c& w1, const v8c& w2, const v8c& w3, const v8c& w4,
    const cfloat jamp[6])
{
    write_wf(out, w0);
    write_wf(out, w1);
    write_wf(out, w2);
    write_wf(out, w3);
    write_wf(out, w4);
    write_jamp(out, jamp);
}

inline void read_token_5wf(
    ::input_cascade<caccfloat>* in,
    v8c& w0, v8c& w1, v8c& w2, v8c& w3, v8c& w4,
    cfloat jamp[6])
{
    w0 = read_wf(in);
    w1 = read_wf(in);
    w2 = read_wf(in);
    w3 = read_wf(in);
    w4 = read_wf(in);
    read_jamp(in, jamp);
}

// ============================================================================
// 11 WF token: 8 base + 3 VVV + 2 propagators (w5_20, w11) - SIMPLER
// Used K2→K3 after reducing moved diagrams to just 2
// ============================================================================
inline void write_token_11wf(
    ::output_cascade<caccfloat>* out,
    const v8c& w0, const v8c& w1, const v8c& w2, const v8c& w3, const v8c& w4,
    const v8c& w5_01, const v8c& w10, const v8c& w5_04,
    const v8c& w5_20, const v8c& w11,
    const cfloat jamp[6])
{
    write_wf(out, w0);
    write_wf(out, w1);
    write_wf(out, w2);
    write_wf(out, w3);
    write_wf(out, w4);
    write_wf(out, w5_01);
    write_wf(out, w10);
    write_wf(out, w5_04);
    write_wf(out, w5_20);
    write_wf(out, w11);
    write_jamp(out, jamp);
}

inline void read_token_11wf(
    ::input_cascade<caccfloat>* in,
    v8c& w0, v8c& w1, v8c& w2, v8c& w3, v8c& w4,
    v8c& w5_01, v8c& w10, v8c& w5_04,
    v8c& w5_20, v8c& w11,
    cfloat jamp[6])
{
    w0 = read_wf(in);
    w1 = read_wf(in);
    w2 = read_wf(in);
    w3 = read_wf(in);
    w4 = read_wf(in);
    w5_01 = read_wf(in);
    w10 = read_wf(in);
    w5_04 = read_wf(in);
    w5_20 = read_wf(in);
    w11 = read_wf(in);
    read_jamp(in, jamp);
}

} // namespace token_ext
} // namespace ggttg

#endif // KERNELS_4K_TOKEN_H
