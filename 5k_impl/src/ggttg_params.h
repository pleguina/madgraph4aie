// ============================================================================
// ggttg_params.h : Pre-loaded parameters for AIE kernels
// All constants are compile-time initialized to avoid parameter passing issues
// ============================================================================

#pragma once

#include <cstdint>

namespace ggttg {


    //------------------------------------------------------------------------------
// Compile-time map: bit width -> adf::plio_type
//------------------------------------------------------------------------------



// Physics constants
constexpr float MT = 173.0f;     // top mass (GeV)
constexpr float WT = 1.4915f;    // top width (GeV)

// Coupling constants (from MG5 param_card)
constexpr float GC_10_REAL = -1.217716f;  // Triple gluon coupling (real part)
constexpr float GC_11_REAL = 0.0f;        // QCD quark coupling (real part)
constexpr float GC_11_IMAG = 1.217716f;   // QCD quark coupling (imag part)
constexpr float GC_12_REAL = 0.0f;        // 4-gluon contact (real part)
constexpr float GC_12_IMAG = 1.48283f;    // 4-gluon contact (imag part)

// Helicity table (32 combinations for g g -> t t~ g)
// Order: g1, g2, t, tbar, g3 — matches MadGraph CPPProcess.cc helicities[ncomb][nexternal] exactly
// Helicity table for g g -> t tbar g (32 combinations of 5 external particles)
// MUST match MadGraph's ordering! Binary counting pattern:
//   Bit 0 (LSB, fastest): particle 4 (g3)
//   Bit 1:                particle 3 (tbar)
//   Bit 2:                particle 2 (t)
//   Bit 3:                particle 1 (g2)
//   Bit 4 (MSB, slowest): particle 0 (g1)
// Copied directly from MadGraph CPPProcess.cc helicities[][] array
// NOTE: We keep a compact int8 LUT (160B) for fast, branch-free decode.
/*
constexpr short HELICITY_TABLE[32][5] = {
    { -1, -1, -1, -1, -1 }, { -1, -1, -1, -1,  1 }, { -1, -1, -1,  1, -1 }, { -1, -1, -1,  1,  1 },
    { -1, -1,  1, -1, -1 }, { -1, -1,  1, -1,  1 }, { -1, -1,  1,  1, -1 }, { -1, -1,  1,  1,  1 },
    { -1,  1, -1, -1, -1 }, { -1,  1, -1, -1,  1 }, { -1,  1, -1,  1, -1 }, { -1,  1, -1,  1,  1 },
    { -1,  1,  1, -1, -1 }, { -1,  1,  1, -1,  1 }, { -1,  1,  1,  1, -1 }, { -1,  1,  1,  1,  1 },
    {  1, -1, -1, -1, -1 }, {  1, -1, -1, -1,  1 }, {  1, -1, -1,  1, -1 }, {  1, -1, -1,  1,  1 },
    {  1, -1,  1, -1, -1 }, {  1, -1,  1, -1,  1 }, {  1, -1,  1,  1, -1 }, {  1, -1,  1,  1,  1 },
    {  1,  1, -1, -1, -1 }, {  1,  1, -1, -1,  1 }, {  1,  1, -1,  1, -1 }, {  1,  1, -1,  1,  1 },
    {  1,  1,  1, -1, -1 }, {  1,  1,  1, -1,  1 }, {  1,  1,  1,  1, -1 }, {  1,  1,  1,  1,  1 }
};
*/

// Compact helicity LUT: 32 combos × 5 particles = 160 bytes
// Layout: [h][particle] with particle order {g1, g2, t, tbar, g3}
// This matches the GET_HEL_* bit decode and MG5 CPPProcess helicities[][] ordering.
constexpr int8_t HELICITY_LUT[32][5] = {
    { -1, -1, -1, -1, -1 }, { -1, -1, -1, -1,  1 }, { -1, -1, -1,  1, -1 }, { -1, -1, -1,  1,  1 },
    { -1, -1,  1, -1, -1 }, { -1, -1,  1, -1,  1 }, { -1, -1,  1,  1, -1 }, { -1, -1,  1,  1,  1 },
    { -1,  1, -1, -1, -1 }, { -1,  1, -1, -1,  1 }, { -1,  1, -1,  1, -1 }, { -1,  1, -1,  1,  1 },
    { -1,  1,  1, -1, -1 }, { -1,  1,  1, -1,  1 }, { -1,  1,  1,  1, -1 }, { -1,  1,  1,  1,  1 },
    {  1, -1, -1, -1, -1 }, {  1, -1, -1, -1,  1 }, {  1, -1, -1,  1, -1 }, {  1, -1, -1,  1,  1 },
    {  1, -1,  1, -1, -1 }, {  1, -1,  1, -1,  1 }, {  1, -1,  1,  1, -1 }, {  1, -1,  1,  1,  1 },
    {  1,  1, -1, -1, -1 }, {  1,  1, -1, -1,  1 }, {  1,  1, -1,  1, -1 }, {  1,  1, -1,  1,  1 },
    {  1,  1,  1, -1, -1 }, {  1,  1,  1, -1,  1 }, {  1,  1,  1,  1, -1 }, {  1,  1,  1,  1,  1 }
};

// Accessors (useful for branch-free decode in hot loops)
static inline int get_hel_g1(int h) { return (int)HELICITY_LUT[h][0]; }
static inline int get_hel_g2(int h) { return (int)HELICITY_LUT[h][1]; }
static inline int get_hel_t(int h)  { return (int)HELICITY_LUT[h][2]; }
static inline int get_hel_tb(int h) { return (int)HELICITY_LUT[h][3]; }
static inline int get_hel_g3(int h) { return (int)HELICITY_LUT[h][4]; }

// Helicity extraction: compute on-the-fly instead of table lookup
// This saves 320 bytes and produces EXACTLY the same physics
#define GET_HEL_G1(h)  (((h) >> 4) & 1 ? 1 : -1)
#define GET_HEL_G2(h)  (((h) >> 3) & 1 ? 1 : -1)
#define GET_HEL_T(h)   (((h) >> 2) & 1 ? 1 : -1)
#define GET_HEL_TB(h)  (((h) >> 1) & 1 ? 1 : -1)
#define GET_HEL_G3(h)  (((h) >> 0) & 1 ? 1 : -1)

// Color matrix (triangular form, raw cf values from MadGraph)
// These are the color flow matrix elements WITHOUT normalization
// The /9 denominator is applied during the color sum calculation
constexpr float COLOR_MATRIX[6][6] = {
    { 64.0f, -8.0f, -8.0f,  1.0f,  1.0f, 10.0f },
    {  0.0f, 64.0f,  1.0f, 10.0f, -8.0f,  1.0f },
    {  0.0f,  0.0f, 64.0f, -8.0f, 10.0f,  1.0f },
    {  0.0f,  0.0f,  0.0f, 64.0f,  1.0f, -8.0f },
    {  0.0f,  0.0f,  0.0f,  0.0f, 64.0f, -8.0f },
    {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 64.0f }
};

// Optimized color reduction constants (remove fdiv, use multiply)
constexpr float INV_COLOR_DENOM = 1.0f / 9.0f;  // Pre-compute 1/COLOR_DENOM

// Pre-normalized diagonal values: COLOR_MATRIX[i][i] * INV_COLOR_DENOM
constexpr float COLOR_DIAG_NORM[6] = {
    64.0f * INV_COLOR_DENOM,  // 7.111111...
    64.0f * INV_COLOR_DENOM,
    64.0f * INV_COLOR_DENOM,
    64.0f * INV_COLOR_DENOM,
    64.0f * INV_COLOR_DENOM,
    64.0f * INV_COLOR_DENOM
};

// Pre-normalized off-diagonal values: 2 * COLOR_MATRIX[i][j] * INV_COLOR_DENOM
// Stored in row-major triangular order: (0,1), (0,2), (0,3), (0,4), (0,5), (1,2), ...
constexpr float COLOR_OFFDIAG_NORM[15] = {
    2.0f * (-8.0f) * INV_COLOR_DENOM,   // (0,1) = -1.777777...
    2.0f * (-8.0f) * INV_COLOR_DENOM,   // (0,2)
    2.0f * ( 1.0f) * INV_COLOR_DENOM,   // (0,3) =  0.222222...
    2.0f * ( 1.0f) * INV_COLOR_DENOM,   // (0,4)
    2.0f * (10.0f) * INV_COLOR_DENOM,   // (0,5) =  2.222222...
    2.0f * ( 1.0f) * INV_COLOR_DENOM,   // (1,2)
    2.0f * (10.0f) * INV_COLOR_DENOM,   // (1,3)
    2.0f * (-8.0f) * INV_COLOR_DENOM,   // (1,4)
    2.0f * ( 1.0f) * INV_COLOR_DENOM,   // (1,5)
    2.0f * (-8.0f) * INV_COLOR_DENOM,   // (2,3)
    2.0f * (10.0f) * INV_COLOR_DENOM,   // (2,4)
    2.0f * ( 1.0f) * INV_COLOR_DENOM,   // (2,5)
    2.0f * ( 1.0f) * INV_COLOR_DENOM,   // (3,4)
    2.0f * (-8.0f) * INV_COLOR_DENOM,   // (3,5)
    2.0f * (-8.0f) * INV_COLOR_DENOM    // (4,5)
};

// Helper functions for complex numbers
inline cfloat make_complex(float real, float imag) {
    return {real, imag};
}

// // Forward declarations for split kernels
// struct WFPacket {
//     v4f momenta[5];      // 20 floats = 80 bytes
//     uint32_t good_mask;  // 1 uint32 = 4 bytes  
//     // Total: 84 bytes per PSP (much better than 640B full WF storage)
// };


// // Helper structure for helicity-optimized calculations
// struct HelicityInfo {
//     uint32_t mask;           // Which helicities to calculate
//     int active_count;        // Number of active helicities
//     int active_indices[32];  // Sparse list of active helicity indices
// };


// Coupling constants as complex numbers
inline cfloat get_gc10() { return make_complex(GC_10_REAL, 0.0f); }
inline cfloat get_gc11() { return make_complex(GC_11_REAL, GC_11_IMAG); }
inline cfloat get_gc12() { return make_complex(GC_12_REAL, GC_12_IMAG); }

// Constants for complex arithmetic
constexpr cfloat CI = {0.0f, 1.0f};
constexpr cfloat MCI = {0.0f, -1.0f};

// Spin-color averaging denominator
constexpr float COLOR_DENOM = 9.0f;        // Color denominator per flow
constexpr float SPINCOLOR_AVG = 1.0f / (256.0f * COLOR_DENOM);  // 1/(256*9) = 1/2304 for g g -> t t~ g

} // namespace ggttg