// ============================================================================
// helas_batch8.h
//
// PSP-batched HELAS library: each v8f/v8c register holds the same quantity
// for 8 different phase-space points, so every HELAS call evaluates 8 events.
// ============================================================================
#pragma once

#include "helas/processing_core_common.h"
#include "ggttg_params.h"

namespace ggttg {

using v8f = aie::vector<float, 8>;
struct WFBatch8 { v8c c[8]; };

static inline v8f safe_inv8(const v8f& x) {
    v8f invx = aie::inv(x);
    v8f z = aie::zeros<float,8>();
    auto m = aie::neq(x, z);
    return aie::select(z, invx, m);
}
static inline v8f safe_sqrt8(const v8f& x) { return aie::sqrt(aie::max(x, aie::zeros<float,8>())); }
static inline v8f safe_rsqrt8(const v8f& x) {
    v8f z = aie::zeros<float,8>();
    v8f cl = aie::max(x, z);
    v8f r = aie::inv(aie::sqrt(cl));
    auto m = aie::gt(cl, z);
    return aie::select(z, r, m);
}
static inline v8f bcastf8(float s) { return aie::broadcast<float, 8>(s); }
static inline v8c bcast8(cfloat s) { return aie::broadcast<cfloat, 8>(s); }
static inline v8c zero8() { return aie::zeros<cfloat, 8>(); }
static inline v8c mkc8(const v8f& re, const v8f& im) { v8c o; for (int i=0;i<8;i++) o.set(cfloat{re[i],im[i]},i); return o; }

static inline v8c cinv8(const v8c& z) {
    aie::vector<float,8> re = aie::real(z), im = aie::imag(z);
    aie::vector<float,8> norm2 = aie::add(aie::mul(re,re).template to_vector<float>(),
                                            aie::mul(im,im).template to_vector<float>());
    aie::vector<float,8> inv_norm2 = aie::inv(norm2);
    v8c out;
    aie::vector<float,8> out_re = aie::mul(re, inv_norm2).template to_vector<float>();
    aie::vector<float,8> out_im = aie::mul(aie::neg(im), inv_norm2).template to_vector<float>();
    for (int i = 0; i < 8; i++) out.set(cfloat{out_re[i], out_im[i]}, i);
    return out;
}
static inline v8c cmul8(const v8c& a, const v8c& b) { return aie::mul(a, b).template to_vector<cfloat>(); }
static inline v8c cadd8(const v8c& a, const v8c& b) { return aie::add(a, b); }
static inline v8c csub8(const v8c& a, const v8c& b) { return aie::sub(a, b); }
static inline v8c realmul8(const aie::vector<float,8>& r, const v8c& cvec) {
    v8c out;
    aie::vector<float,8> re = aie::real(cvec), im = aie::imag(cvec);
    aie::vector<float,8> orr = aie::mul(r, re).template to_vector<float>();
    aie::vector<float,8> ori = aie::mul(r, im).template to_vector<float>();
    for (int i = 0; i < 8; i++) out.set(cfloat{orr[i], ori[i]}, i);
    return out;
}
// Accumulates real*complex into a (re,im) v8f pair WITHOUT going through
// mkc8's per-lane reconstruction on every term -- only the final caller
// converts to v8c once, instead of once per addend. Cuts mkc8 calls in
// dot-product-style accumulations (e.g. 4 realmul8 terms summed) from 4
// down to 1.
static inline void realmul8_acc(const aie::vector<float,8>& r, const v8c& cvec,
                                  aie::vector<float,8>& acc_re, aie::vector<float,8>& acc_im, bool first) {
    aie::vector<float,8> re = aie::real(cvec), im = aie::imag(cvec);
    aie::vector<float,8> mre = aie::mul(r, re).template to_vector<float>();
    aie::vector<float,8> mim = aie::mul(r, im).template to_vector<float>();
    if (first) { acc_re = mre; acc_im = mim; }
    else { acc_re = aie::add(acc_re, mre); acc_im = aie::add(acc_im, mim); }
}
static inline aie::vector<float,8> minkowski_p2_batch(const aie::vector<float,8> P[4]) {
    auto sq0 = aie::mul(P[0], P[0]).template to_vector<float>();
    auto sq1 = aie::mul(P[1], P[1]).template to_vector<float>();
    auto sq2 = aie::mul(P[2], P[2]).template to_vector<float>();
    auto sq3 = aie::mul(P[3], P[3]).template to_vector<float>();
    return aie::sub(aie::sub(aie::sub(sq0, sq1), sq2), sq3);
}
static inline v8c denom_batch(const aie::vector<float,8> P[4], float M, float W, cfloat COUP) {
    aie::vector<float,8> p2 = minkowski_p2_batch(P);
    cfloat mm_iMw = {M*M, -M*W};
    v8c den = csub8(mkc8(p2, aie::zeros<float,8>()), bcast8(mm_iMw));
    return cmul8(bcast8(COUP), cinv8(den));
}
static inline void extractP_batch(const v8c& c0, const v8c& c1, aie::vector<float,8> P[4]) {
    P[0] = aie::real(c0); P[1] = aie::real(c1); P[2] = aie::imag(c1); P[3] = aie::imag(c0);
}
static inline void extractP_neg_batch(const v8c& c0, const v8c& c1, aie::vector<float,8> P[4]) {
    extractP_batch(c0, c1, P);
    for (int i = 0; i < 4; i++) P[i] = aie::neg(P[i]);
}

struct V3CombBatch {
    v8c p2p5, m2p5, p2m5, m2m5, mci2p5;
    v8c p3pci4, m3pci4, p3mci4, m3mci4;
    v8c pci3p4, mci3p4, pci3m4, mci3m4;
};
static inline V3CombBatch v3_make_combos_batch(const WFBatch8& V3) {
    V3CombBatch c;
    c.p2p5   = cadd8(V3.c[2], V3.c[5]);
    c.m2p5   = csub8(V3.c[5], V3.c[2]);
    c.p2m5   = csub8(zero8(), c.m2p5);
    c.m2m5   = csub8(zero8(), c.p2p5);
    c.mci2p5 = cmul8(bcast8(-cI), c.p2p5);
    v8c iV4 = cmul8(bcast8(cI), V3.c[4]);
    v8c iV3 = cmul8(bcast8(cI), V3.c[3]);
    c.p3pci4 = cadd8(V3.c[3], iV4);
    c.m3pci4 = csub8(iV4, V3.c[3]);
    c.pci3p4 = cadd8(iV3, V3.c[4]);
    c.mci3p4 = csub8(V3.c[4], iV3);
    c.p3mci4 = csub8(zero8(), c.m3pci4);
    c.m3mci4 = csub8(zero8(), c.p3pci4);
    c.pci3m4 = csub8(zero8(), c.mci3p4);
    c.mci3m4 = csub8(zero8(), c.pci3p4);
    return c;
}
static inline void pol4_batch(const WFBatch8& v, v8c out[4]) { out[0]=v.c[2]; out[1]=v.c[3]; out[2]=v.c[4]; out[3]=v.c[5]; }

// ---- FFV1_1 / FFV1_2 ----
static AIE_NOINLINE void ffv_linear_core_batch(const v8c f24[4], const v8c f35[4],
    const aie::vector<float,8> P[4], const V3CombBatch& C, const v8c& denomci, float M, v8c result[4])
{
    v8c dsgn[4] = { denomci, csub8(zero8(),denomci), csub8(zero8(),denomci), denomci };
    v8c t1_[4] = { C.m2p5, C.m3pci4, C.p2p5, C.m3pci4 };
    v8c t2_[4] = { C.p3mci4, C.p2m5, C.m3pci4, C.p2p5 };
    v8c t3_[4] = { C.pci3p4, cmul8(bcast8(cI), C.m2p5), C.mci3m4, C.mci2p5 };
    v8c t4_[4] = { C.m2p5, C.p3mci4, C.m2m5, C.m3pci4 };
    v8c acc1[4];
    for (int j = 0; j < 4; j++) {
        aie::vector<float,8> re, im;
        realmul8_acc(P[0],t1_[j], re, im, true);
        realmul8_acc(P[1],t2_[j], re, im, false);
        realmul8_acc(P[2],t3_[j], re, im, false);
        realmul8_acc(P[3],t4_[j], re, im, false);
        acc1[j] = mkc8(re, im);
    }

    v8c t1b_[4] = { C.p3pci4, C.p2p5, C.p3pci4, C.m2p5 };
    v8c t2b_[4] = { C.m2m5, C.m3mci4, C.m2p5, C.p3pci4 };
    v8c t3b_[4] = { C.mci2p5, C.pci3m4, cmul8(bcast8(cI), C.m2p5), C.mci3p4 };
    v8c t4b_[4] = { C.p3pci4, C.m2m5, C.m3mci4, C.m2p5 };
    v8c acc2[4];
    for (int j = 0; j < 4; j++) {
        aie::vector<float,8> re, im;
        realmul8_acc(P[0],t1b_[j], re, im, true);
        realmul8_acc(P[1],t2b_[j], re, im, false);
        realmul8_acc(P[2],t3b_[j], re, im, false);
        realmul8_acc(P[3],t4b_[j], re, im, false);
        acc2[j] = mkc8(re, im);
    }

    v8c t1c_[4] = { C.p2p5, C.m3pci4, C.m2p5, C.m3pci4 };
    v8c t2c_[4] = { C.p3pci4, C.m2p5, C.p3pci4, C.p2p5 };
    v8c f24r[4] = { f24[3], f24[2], f24[1], f24[0] };
    v8c f35r[4] = { f35[3], f35[2], f35[1], f35[0] };
    v8c acc3[4];
    for (int j = 0; j < 4; j++) acc3[j] = cadd8(cmul8(f24r[j], t1c_[j]), cmul8(f35r[j], t2c_[j]));

    for (int j = 0; j < 4; j++) {
        v8c mterm = realmul8(bcastf8(M), acc3[j]);
        v8c acc4 = cadd8(cadd8(cmul8(f24[j], acc1[j]), cmul8(f35[j], acc2[j])), mterm);
        result[j] = cmul8(dsgn[j], acc4);
    }
}
static AIE_NOINLINE void ffv2_linear_core_batch(const v8c f24[4], const v8c f35[4],
    const aie::vector<float,8> P[4], const V3CombBatch& C, const v8c& denomci, float M, v8c result[4])
{
    v8c dsgn[4] = { denomci, csub8(zero8(),denomci), csub8(zero8(),denomci), denomci };
    v8c t1_[4] = { C.p2p5, C.m3mci4, C.m2p5, C.m3mci4 };
    v8c t2_[4] = { C.m3mci4, C.p2p5, C.p3pci4, C.p2m5 };
    v8c t3_[4] = { C.pci3m4, cmul8(bcast8(cI), C.p2p5), C.mci3p4, cmul8(bcast8(cI), C.p2m5) };
    v8c t4_[4] = { C.m2m5, C.m3mci4, C.m2p5, C.p3pci4 };
    v8c acc1[4];
    for (int j = 0; j < 4; j++) {
        aie::vector<float,8> re, im;
        realmul8_acc(P[0],t1_[j], re, im, true);
        realmul8_acc(P[1],t2_[j], re, im, false);
        realmul8_acc(P[2],t3_[j], re, im, false);
        realmul8_acc(P[3],t4_[j], re, im, false);
        acc1[j] = mkc8(re, im);
    }

    v8c t1b_[4] = { C.p3mci4, C.m2p5, C.p3mci4, C.p2p5 };
    v8c t2b_[4] = { C.m2p5, C.p3mci4, C.m2m5, C.m3pci4 };
    v8c t3b_[4] = { cmul8(bcast8(cI), C.p2m5), C.pci3p4, cmul8(bcast8(cI), C.p2p5), C.mci3m4 };
    v8c t4b_[4] = { C.m3pci4, C.m2p5, C.p3mci4, C.m2m5 };
    v8c acc2[4];
    for (int j = 0; j < 4; j++) {
        aie::vector<float,8> re, im;
        realmul8_acc(P[0],t1b_[j], re, im, true);
        realmul8_acc(P[1],t2b_[j], re, im, false);
        realmul8_acc(P[2],t3b_[j], re, im, false);
        realmul8_acc(P[3],t4b_[j], re, im, false);
        acc2[j] = mkc8(re, im);
    }

    v8c t1c_[4] = { C.p2m5, C.p3pci4, C.m2m5, C.p3pci4 };
    v8c t2c_[4] = { C.m3pci4, C.m2m5, C.m3pci4, C.p2m5 };
    v8c f42m[4] = { f24[2], f24[2], f24[0], f24[0] };
    v8c f53m[4] = { f35[2], f35[2], f35[0], f35[0] };
    v8c acc3[4];
    for (int j = 0; j < 4; j++) acc3[j] = cadd8(cmul8(f42m[j], t1c_[j]), cmul8(f53m[j], t2c_[j]));

    for (int j = 0; j < 4; j++) {
        v8c mterm = realmul8(bcastf8(M), acc3[j]);
        v8c acc4 = cadd8(cadd8(cmul8(f24[j], acc1[j]), cmul8(f35[j], acc2[j])), mterm);
        result[j] = cmul8(dsgn[j], acc4);
    }
}
static AIE_NOINLINE WFBatch8 FFV1_1_batch8(const WFBatch8& F2, const WFBatch8& V3, cfloat COUP, float M1, float W1) {
    WFBatch8 F1;
    F1.c[0] = cadd8(F2.c[0], V3.c[0]);
    F1.c[1] = cadd8(F2.c[1], V3.c[1]);
    aie::vector<float,8> P[4];
    extractP_neg_batch(F1.c[0], F1.c[1], P);
    v8c denomci = cmul8(denom_batch(P, M1, W1, COUP), bcast8(cI));
    V3CombBatch C = v3_make_combos_batch(V3);
    v8c f24[4] = { F2.c[2], F2.c[2], F2.c[4], F2.c[4] };
    v8c f35[4] = { F2.c[3], F2.c[3], F2.c[5], F2.c[5] };
    v8c pol[4];
    ffv_linear_core_batch(f24, f35, P, C, denomci, M1, pol);
    F1.c[2]=pol[0]; F1.c[3]=pol[1]; F1.c[4]=pol[2]; F1.c[5]=pol[3];
    F1.c[6]=zero8(); F1.c[7]=zero8();
    return F1;
}
static AIE_NOINLINE WFBatch8 FFV1_2_batch8(const WFBatch8& F1, const WFBatch8& V3, cfloat COUP, float M2, float W2) {
    WFBatch8 F2;
    F2.c[0] = cadd8(F1.c[0], V3.c[0]);
    F2.c[1] = cadd8(F1.c[1], V3.c[1]);
    aie::vector<float,8> P[4];
    extractP_neg_batch(F2.c[0], F2.c[1], P);
    v8c denomci = cmul8(denom_batch(P, M2, W2, COUP), bcast8(cI));
    V3CombBatch C = v3_make_combos_batch(V3);
    v8c f24[4] = { F1.c[2], F1.c[2], F1.c[4], F1.c[4] };
    v8c f35[4] = { F1.c[3], F1.c[3], F1.c[5], F1.c[5] };
    v8c pol[4];
    ffv2_linear_core_batch(f24, f35, P, C, denomci, M2, pol);
    F2.c[2]=pol[0]; F2.c[3]=pol[1]; F2.c[4]=pol[2]; F2.c[5]=pol[3];
    F2.c[6]=zero8(); F2.c[7]=zero8();
    return F2;
}
static AIE_NOINLINE v8c FFV1_0_batch8(const WFBatch8& F1, const WFBatch8& F2, const WFBatch8& V3, cfloat COUP) {
    v8c f42[4] = { F2.c[4], F2.c[4], F2.c[2], F2.c[2] };
    v8c f53[4] = { F2.c[5], F2.c[5], F2.c[3], F2.c[3] };
    v8c F1v[4] = { F1.c[2], F1.c[3], F1.c[4], F1.c[5] };
    V3CombBatch C = v3_make_combos_batch(V3);
    v8c t1_[4] = { C.p2p5, C.p3mci4, C.p2m5, C.m3pci4 };
    v8c t2_[4] = { C.p3pci4, C.p2m5, C.m3mci4, C.p2p5 };
    v8c acc[4];
    for (int j = 0; j < 4; j++) acc[j] = cadd8(cmul8(f42[j], t1_[j]), cmul8(f53[j], t2_[j]));
    v8c mul[4];
    for (int j = 0; j < 4; j++) mul[j] = cmul8(F1v[j], acc[j]);
    v8c TMP9 = cadd8(cadd8(mul[0], mul[1]), cadd8(mul[2], mul[3]));
    return cmul8(cmul8(bcast8(COUP), bcast8(-cI)), TMP9);
}

// ---- VVV1P0_1 ----
// AIE_NOINLINE: called 5x (VVV1P0_1), 9x (VVV1_0), and 6x total across the
// 3 VVVV variants -- left as plain `inline`, the compiler duplicated the
// full body at every call site (VVV1_0_batch8 grows to 10488 B, far bigger
// than VVV1P0_1_batch8 despite structurally similar work).
static AIE_NOINLINE v8c tmp_calc_vv_batch(const WFBatch8& v1, const WFBatch8& v2) {
    v8c a[4], b[4]; pol4_batch(v1, a); pol4_batch(v2, b);
    v8c prod[4];
    for (int i = 0; i < 4; i++) prod[i] = csub8(zero8(), cmul8(a[i], b[i]));
    prod[0] = csub8(zero8(), prod[0]);
    return cadd8(cadd8(prod[0], prod[1]), cadd8(prod[2], prod[3]));
}
static AIE_NOINLINE v8c tmp_calc_vp_batch(const WFBatch8& v, const aie::vector<float,8> p[4]) {
    // Minkowski dot product (lane-0 sign flip): -p0*a0 -p1*a1 -p2*a2 -p3*a3,
    // then the scalar version's extra lane-0 negation cancels the first
    // term's sign back to +p0*a0. Accumulate re/im directly instead of
    // building 4 intermediate v8c via realmul8+mkc8 and summing those.
    v8c a[4]; pol4_batch(v, a);
    aie::vector<float,8> re, im;
    realmul8_acc(p[0], a[0], re, im, true);   // will be sign-flipped back below
    aie::vector<float,8> re1, im1;
    realmul8_acc(p[1], a[1], re1, im1, true);
    realmul8_acc(p[2], a[2], re1, im1, false);
    realmul8_acc(p[3], a[3], re1, im1, false);
    // total = p0*a0 - (p1*a1+p2*a2+p3*a3)
    return mkc8(aie::sub(re, re1), aie::sub(im, im1));
}
static AIE_NOINLINE void vvv_calc_batch(const WFBatch8& V2, const WFBatch8& V3,
    const aie::vector<float,8> P2[4], const aie::vector<float,8> P3[4],
    v8c mt0pt2, v8c pt4mt5, v8c TMP6, v8c denomci, v8c out[4])
{
    v8c a[4], b[4]; pol4_batch(V2, a); pol4_batch(V3, b);
    aie::vector<float,8> p[4] = { aie::sub(P3[0],P2[0]), aie::sub(P3[1],P2[1]), aie::sub(P3[2],P2[2]), aie::sub(P3[3],P2[3]) };
    for (int j = 0; j < 4; j++) {
        v8c acc = cadd8(cmul8(a[j], mt0pt2), cmul8(b[j], pt4mt5));
        acc = cadd8(acc, realmul8(p[j], TMP6));
        out[j] = cmul8(acc, denomci);
    }
}
static AIE_NOINLINE WFBatch8 VVV1P0_1_batch8(const WFBatch8& V2, const WFBatch8& V3, cfloat COUP, float M1, float W1) {
    WFBatch8 V1;
    V1.c[0] = cadd8(V2.c[0], V3.c[0]);
    V1.c[1] = cadd8(V2.c[1], V3.c[1]);
    aie::vector<float,8> P1[4], P2[4], P3[4];
    extractP_neg_batch(V1.c[0], V1.c[1], P1);
    extractP_batch(V2.c[0], V2.c[1], P2);
    extractP_batch(V3.c[0], V3.c[1], P3);
    v8c denomci = cmul8(denom_batch(P1, M1, W1, COUP), bcast8(cI));
    v8c TMP0 = tmp_calc_vp_batch(V3, P1);
    v8c TMP2 = tmp_calc_vp_batch(V3, P2);
    v8c TMP4 = tmp_calc_vp_batch(V2, P3);
    v8c TMP6 = tmp_calc_vv_batch(V3, V2);
    v8c TMP1 = tmp_calc_vp_batch(V2, P1);
    v8c mt0pt2 = csub8(TMP2, TMP0);
    v8c pt4mt5 = csub8(TMP1, TMP4);
    v8c pol[4];
    vvv_calc_batch(V2, V3, P2, P3, mt0pt2, pt4mt5, TMP6, denomci, pol);
    V1.c[2]=pol[0]; V1.c[3]=pol[1]; V1.c[4]=pol[2]; V1.c[5]=pol[3];
    V1.c[6]=zero8(); V1.c[7]=zero8();
    return V1;
}

// ---- VVV1_0 (amplitude) ----
static AIE_NOINLINE v8c VVV1_0_batch8(const WFBatch8& V1, const WFBatch8& V2, const WFBatch8& V3, cfloat COUP) {
    aie::vector<float,8> P1[4], P2[4], P3[4];
    extractP_batch(V1.c[0], V1.c[1], P1);
    extractP_batch(V2.c[0], V2.c[1], P2);
    extractP_batch(V3.c[0], V3.c[1], P3);

    v8c TMP0 = tmp_calc_vp_batch(V3, P1);
    v8c TMP1 = tmp_calc_vp_batch(V2, P1);
    v8c TMP2 = tmp_calc_vp_batch(V3, P2);
    v8c TMP3 = tmp_calc_vv_batch(V3, V1);
    v8c TMP4 = tmp_calc_vp_batch(V2, P3);
    v8c TMP5 = tmp_calc_vp_batch(V1, P2);
    v8c TMP6 = tmp_calc_vv_batch(V3, V2);
    v8c TMP7 = tmp_calc_vp_batch(V1, P3);
    v8c TMP8 = tmp_calc_vv_batch(V2, V1);

    v8c term1 = cmul8(TMP8, cmul8(bcast8(cI), csub8(TMP2, TMP0)));
    v8c term2 = cmul8(TMP3, cmul8(bcast8(cI), csub8(TMP1, TMP4)));
    v8c term3 = cmul8(TMP6, cmul8(bcast8(cI), csub8(TMP7, TMP5)));
    return cmul8(bcast8(COUP), cadd8(cadd8(term1, term2), term3));
}

// ---- FFV1P0_3 ----
static AIE_NOINLINE v8c FFV1P0_3_batch8(const WFBatch8& F1, const WFBatch8& F2, cfloat COUP, float M3, float W3, v8c out_c2345[4]) {
    v8c c0 = cadd8(F1.c[0], F2.c[0]);
    v8c c1 = cadd8(F1.c[1], F2.c[1]);
    aie::vector<float,8> P[4];
    extractP_neg_batch(c0, c1, P);
    v8c denomci = cmul8(denom_batch(P, M3, W3, COUP), bcast8(cI));
    v8c ndenomci = csub8(zero8(), denomci);

    v8c t0 = cadd8(cadd8(cmul8(F1.c[2],F2.c[4]), cmul8(F1.c[3],F2.c[5])), cadd8(cmul8(F1.c[4],F2.c[2]), cmul8(F1.c[5],F2.c[3])));
    v8c t1 = cadd8(csub8(zero8(),cadd8(cmul8(F1.c[2],F2.c[5]), cmul8(F1.c[3],F2.c[4]))), cadd8(cmul8(F1.c[4],F2.c[3]), cmul8(F1.c[5],F2.c[2])));
    v8c inner2 = cadd8(csub8(zero8(), cadd8(cmul8(F1.c[2],F2.c[5]), cmul8(F1.c[5],F2.c[2]))), cadd8(cmul8(F1.c[3],F2.c[4]), cmul8(F1.c[4],F2.c[3])));
    v8c t2 = cmul8(bcast8(cI), inner2);
    v8c t3 = cadd8(csub8(zero8(),cadd8(cmul8(F1.c[2],F2.c[4]), cmul8(F1.c[5],F2.c[3]))), cadd8(cmul8(F1.c[3],F2.c[5]), cmul8(F1.c[4],F2.c[2])));

    out_c2345[0] = cmul8(ndenomci, t0);
    out_c2345[1] = cmul8(ndenomci, t1);
    out_c2345[2] = cmul8(ndenomci, t2);
    out_c2345[3] = cmul8(ndenomci, t3);
    return c0;
}

// ---- VVVV1P0_1 / 3P0_1 / 4P0_1 ----
static AIE_NOINLINE void vvvv_calc_batch(const WFBatch8& v1, const WFBatch8& v2, const v8c& tmpx1, const v8c& tmpx2, const v8c& denomci, v8c out[4]) {
    v8c a[4], b[4]; pol4_batch(v1, a); pol4_batch(v2, b);
    for (int j = 0; j < 4; j++) out[j] = cmul8(cadd8(cmul8(a[j],tmpx1), cmul8(b[j],tmpx2)), denomci);
}
static AIE_NOINLINE WFBatch8 VVVV_common_batch(const WFBatch8& V2, const WFBatch8& V3, const WFBatch8& V4,
    cfloat COUP, float M1, float W1, const WFBatch8& A, const WFBatch8& B, const v8c& tmpX, const v8c& tmpY)
{
    WFBatch8 V1;
    V1.c[0] = cadd8(cadd8(V2.c[0], V3.c[0]), V4.c[0]);
    V1.c[1] = cadd8(cadd8(V2.c[1], V3.c[1]), V4.c[1]);
    aie::vector<float,8> P1[4];
    extractP_neg_batch(V1.c[0], V1.c[1], P1);
    v8c denomci = cmul8(denom_batch(P1, M1, W1, COUP), bcast8(cI));
    v8c pol[4];
    vvvv_calc_batch(A, B, tmpX, tmpY, denomci, pol);
    V1.c[2]=pol[0]; V1.c[3]=pol[1]; V1.c[4]=pol[2]; V1.c[5]=pol[3];
    V1.c[6]=zero8(); V1.c[7]=zero8();
    return V1;
}
static AIE_NOINLINE WFBatch8 VVVV1P0_1_batch8(const WFBatch8& V2, const WFBatch8& V3, const WFBatch8& V4, cfloat COUP, float M1, float W1) {
    v8c TMP10 = tmp_calc_vv_batch(V2, V4);
    v8c TMP6  = tmp_calc_vv_batch(V3, V2);
    return VVVV_common_batch(V2, V3, V4, COUP, M1, W1, V4, V3, csub8(zero8(),TMP6), TMP10);
}
static AIE_NOINLINE WFBatch8 VVVV3P0_1_batch8(const WFBatch8& V2, const WFBatch8& V3, const WFBatch8& V4, cfloat COUP, float M1, float W1) {
    v8c TMP11 = tmp_calc_vv_batch(V3, V4);
    v8c TMP6  = tmp_calc_vv_batch(V3, V2);
    return VVVV_common_batch(V2, V3, V4, COUP, M1, W1, V4, V2, csub8(zero8(),TMP6), TMP11);
}
static AIE_NOINLINE WFBatch8 VVVV4P0_1_batch8(const WFBatch8& V2, const WFBatch8& V3, const WFBatch8& V4, cfloat COUP, float M1, float W1) {
    v8c TMP10 = tmp_calc_vv_batch(V2, V4);
    v8c TMP11 = tmp_calc_vv_batch(V3, V4);
    return VVVV_common_batch(V2, V3, V4, COUP, M1, W1, V3, V2, csub8(zero8(),TMP10), TMP11);
}

// ---- external generators (process-specific specialization for this
//      process's momentum and helicity convention) ----
static AIE_NOINLINE WFBatch8 vxxxxx_beam_batch8(const v8f P[4], int nhel, int nsv) {
    WFBatch8 vc; for (int i=0;i<8;i++) vc.c[i]=zero8();
    const v8f p0 = P[0], p3 = P[3];
    const float hel = (float)nhel;
    vc.c[0] = mkc8(aie::mul(bcastf8((float)nsv),p0).template to_vector<float>(), aie::mul(bcastf8((float)nsv),p3).template to_vector<float>());
    vc.c[1] = zero8();
    vc.c[2] = zero8();
    vc.c[3] = mkc8(bcastf8(-hel*SQH), bcastf8(0.f));
    v8f zero_v = aie::zeros<float,8>();
    auto neg_mask = aie::lt(p3, zero_v);
    v8f sign = aie::select(bcastf8(SQH), bcastf8(-SQH), neg_mask);
    vc.c[4] = mkc8(bcastf8(0.f), aie::mul(bcastf8((float)nsv), sign).template to_vector<float>());
    vc.c[5] = mkc8(bcastf8(1.f - fabsf(hel)), bcastf8(0.f));
    return vc;
}
static AIE_NOINLINE WFBatch8 vxxxxx_generic_batch8(const v8f P[4], int nhel, int nsv) {
    WFBatch8 vc; for (int i=0;i<8;i++) vc.c[i]=zero8();
    const v8f p0=P[0], p1=P[1], p2=P[2], p3=P[3];
    const float hel = (float)nhel;
    v8f pt = safe_sqrt8(aie::add(aie::mul(p1,p1).template to_vector<float>(), aie::mul(p2,p2).template to_vector<float>()));
    v8f pp = p0;
    v8f rpp = safe_inv8(pp);
    v8f rpt = safe_inv8(pt);
    vc.c[0] = mkc8(aie::mul(bcastf8((float)nsv),p0).template to_vector<float>(), aie::mul(bcastf8((float)nsv),p3).template to_vector<float>());
    vc.c[1] = mkc8(aie::mul(bcastf8((float)nsv),p1).template to_vector<float>(), aie::mul(bcastf8((float)nsv),p2).template to_vector<float>());
    vc.c[2] = zero8();
    v8f v5re = aie::mul(bcastf8(hel*SQH), aie::mul(pt,rpp).template to_vector<float>()).template to_vector<float>();
    vc.c[5] = mkc8(v5re, bcastf8(0.f));
    v8f pzpt = aie::mul(aie::mul(p3, aie::mul(rpp,rpt).template to_vector<float>()).template to_vector<float>(), bcastf8(SQH*hel)).template to_vector<float>();
    v8f negp1pzpt = aie::neg(aie::mul(p1,pzpt).template to_vector<float>());
    v8f v3im = aie::mul(bcastf8((float)(-nsv)), aie::mul(p2,rpt).template to_vector<float>()).template to_vector<float>();
    v3im = aie::mul(v3im, bcastf8(SQH)).template to_vector<float>();
    vc.c[3] = mkc8(negp1pzpt, v3im);
    v8f negp2pzpt = aie::neg(aie::mul(p2,pzpt).template to_vector<float>());
    v8f v4im = aie::mul(bcastf8((float)(+nsv)), aie::mul(p1,rpt).template to_vector<float>()).template to_vector<float>();
    v4im = aie::mul(v4im, bcastf8(SQH)).template to_vector<float>();
    vc.c[4] = mkc8(negp2pzpt, v4im);
    return vc;
}
static AIE_NOINLINE WFBatch8 ixxxxx_generic_batch8(const v8f P[4], float fmass, int nhel, int nsf) {
    WFBatch8 fi; for (int i=0;i<8;i++) fi.c[i]=zero8();
    const v8f p0=P[0], p1=P[1], p2=P[2], p3=P[3];
    const int nh = nhel * nsf;
    fi.c[0] = mkc8(aie::mul(bcastf8((float)(-nsf)),p0).template to_vector<float>(), aie::mul(bcastf8((float)(-nsf)),p3).template to_vector<float>());
    fi.c[1] = mkc8(aie::mul(bcastf8((float)(-nsf)),p1).template to_vector<float>(), aie::mul(bcastf8((float)(-nsf)),p2).template to_vector<float>());
    v8f pt2 = aie::add(aie::mul(p1,p1).template to_vector<float>(), aie::mul(p2,p2).template to_vector<float>());
    v8f pabs = safe_sqrt8(aie::add(pt2, aie::mul(p3,p3).template to_vector<float>()));
    v8f pp = pabs;
    float sf0 = 0.5f*(1.f+nsf+(1-nsf)*nh);
    float sf1 = 0.5f*(1.f+nsf-(1-nsf)*nh);
    v8f omega0 = safe_sqrt8(aie::add(p0, pp));
    v8f omega1 = aie::mul(bcastf8(fmass), safe_inv8(omega0)).template to_vector<float>();
    const int ip = (1+nh)>>1, im = (1-nh)>>1;
    v8f sfomega0 = aie::mul(bcastf8(sf0), ip? omega1 : omega0).template to_vector<float>();
    v8f sfomega1 = aie::mul(bcastf8(sf1), im? omega1 : omega0).template to_vector<float>();
    v8f pp3 = aie::max(aie::add(pp,p3), aie::zeros<float,8>());
    v8f invpp = safe_inv8(pp);
    v8f rs = safe_rsqrt8(aie::mul(bcastf8(2.f), aie::mul(pp,pp3).template to_vector<float>()).template to_vector<float>());
    v8f chi0_re = safe_sqrt8(aie::mul(bcastf8(0.5f), aie::mul(pp3,invpp).template to_vector<float>()).template to_vector<float>());
    v8f chi1_re = aie::mul(bcastf8((float)nh), aie::mul(p1,rs).template to_vector<float>()).template to_vector<float>();
    v8f chi1_im = aie::mul(p2, rs).template to_vector<float>();
    v8f chi_im_re = im? chi1_re : chi0_re, chi_im_im = im? chi1_im : bcastf8(0.f);
    v8f chi_ip_re = ip? chi1_re : chi0_re, chi_ip_im = ip? chi1_im : bcastf8(0.f);
    fi.c[2] = mkc8(aie::mul(chi_im_re,sfomega0).template to_vector<float>(), aie::mul(chi_im_im,sfomega0).template to_vector<float>());
    fi.c[3] = mkc8(aie::mul(chi_ip_re,sfomega0).template to_vector<float>(), aie::mul(chi_ip_im,sfomega0).template to_vector<float>());
    fi.c[4] = mkc8(aie::mul(chi_im_re,sfomega1).template to_vector<float>(), aie::mul(chi_im_im,sfomega1).template to_vector<float>());
    fi.c[5] = mkc8(aie::mul(chi_ip_re,sfomega1).template to_vector<float>(), aie::mul(chi_ip_im,sfomega1).template to_vector<float>());
    return fi;
}
static AIE_NOINLINE WFBatch8 oxxxxx_generic_batch8(const v8f P[4], float fmass, int nhel, int nsf) {
    WFBatch8 fo; for (int i=0;i<8;i++) fo.c[i]=zero8();
    const v8f p0=P[0], p1=P[1], p2=P[2], p3=P[3];
    const int nh = nhel * nsf;
    fo.c[0] = mkc8(aie::mul(bcastf8((float)nsf),p0).template to_vector<float>(), aie::mul(bcastf8((float)nsf),p3).template to_vector<float>());
    fo.c[1] = mkc8(aie::mul(bcastf8((float)nsf),p1).template to_vector<float>(), aie::mul(bcastf8((float)nsf),p2).template to_vector<float>());
    v8f pt2 = aie::add(aie::mul(p1,p1).template to_vector<float>(), aie::mul(p2,p2).template to_vector<float>());
    v8f pabs = safe_sqrt8(aie::add(pt2, aie::mul(p3,p3).template to_vector<float>()));
    v8f pp = pabs;
    float sf0 = 0.5f*(1.f+nsf+(1-nsf)*nh);
    float sf1 = 0.5f*(1.f+nsf-(1-nsf)*nh);
    v8f omega0 = safe_sqrt8(aie::add(p0, pp));
    v8f omega1 = aie::mul(bcastf8(fmass), safe_inv8(omega0)).template to_vector<float>();
    const int ip = (1+nh)>>1, im = (1-nh)>>1;
    v8f sfomeg0 = aie::mul(bcastf8(sf0), ip? omega1 : omega0).template to_vector<float>();
    v8f sfomeg1 = aie::mul(bcastf8(sf1), im? omega1 : omega0).template to_vector<float>();
    v8f pp3 = aie::max(aie::add(pp,p3), aie::zeros<float,8>());
    v8f invpp = safe_inv8(pp);
    v8f rs = safe_rsqrt8(aie::mul(bcastf8(2.f), aie::mul(pp,pp3).template to_vector<float>()).template to_vector<float>());
    v8f chi0_re = safe_sqrt8(aie::mul(bcastf8(0.5f), aie::mul(pp3,invpp).template to_vector<float>()).template to_vector<float>());
    v8f chi1_re = aie::mul(bcastf8((float)nh), aie::mul(p1,rs).template to_vector<float>()).template to_vector<float>();
    v8f chi1_im = aie::neg(aie::mul(p2, rs).template to_vector<float>());
    v8f chi_im_re = im? chi1_re : chi0_re, chi_im_im = im? chi1_im : bcastf8(0.f);
    v8f chi_ip_re = ip? chi1_re : chi0_re, chi_ip_im = ip? chi1_im : bcastf8(0.f);
    fo.c[2] = mkc8(aie::mul(chi_im_re,sfomeg1).template to_vector<float>(), aie::mul(chi_im_im,sfomeg1).template to_vector<float>());
    fo.c[3] = mkc8(aie::mul(chi_ip_re,sfomeg1).template to_vector<float>(), aie::mul(chi_ip_im,sfomeg1).template to_vector<float>());
    fo.c[4] = mkc8(aie::mul(chi_im_re,sfomeg0).template to_vector<float>(), aie::mul(chi_im_im,sfomeg0).template to_vector<float>());
    fo.c[5] = mkc8(aie::mul(chi_ip_re,sfomeg0).template to_vector<float>(), aie::mul(chi_ip_im,sfomeg0).template to_vector<float>());
    return fo;
}

} // namespace ggttg
