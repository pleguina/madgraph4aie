// ============================================================================
// ggttg_graph_helicity_parallel.h
//
// Helicity-Parallel ADF Graph for gg→tt̄g
//
// CONCEPT: Split 32 helicities across multiple tile chains, then reduce
//
// ARCHITECTURE (4-way helicity split example):
//
//                    ┌─► [K1_h0-7]─[K2a]─[K2b]─[K3]─[K4] ─► partial_ME²_0 ─┐
//                    │                                                      │
// PSP (broadcast) ───┼─► [K1_h8-15]─[K2a]─[K2b]─[K3]─[K4] ─► partial_ME²_1 ─┼──► [Reducer] ─► ME²
//                    │                                                      │
//                    ├─► [K1_h16-23]─[K2a]─[K2b]─[K3]─[K4] ─► partial_ME²_2 ─┤
//                    │                                                      │
//                    └─► [K1_h24-31]─[K2a]─[K2b]─[K3]─[K4] ─► partial_ME²_3 ─┘
//
// HELICITY MATH:
//   Each slice computes: partial_ME²[s] = Σ_{h∈slice} hel_me2[h]
//   Reducer computes:    ME² = (1/256) × Σ_s partial_ME²[s]
//
// ADVANTAGES:
//   - 4× reduction in latency (8 helicities per chain vs 32)
//   - Same PLIO as single lane (1 in, 1 out) with internal parallelism
//
// DISADVANTAGES:
//   - Requires broadcast of PSP to 4 chains (stream fanout)
//   - Requires reducer kernel (adds one tile)
//   - More complex routing
// ============================================================================

#pragma once
#include <string>
#include <adf.h>
#include <aie_api/aie.hpp>
#include "ggttg_config.h"
#include "ggttg_params.h"

namespace ggttg {

// ============================================================================
// Helicity-Templated Kernel Forward Declarations
// These kernels only process helicities in range [HEL_START, HEL_END)
// ============================================================================
template<int BATCH, int HEL_START, int HEL_END>
void kernel_k1_wfgen_hslice(::input_stream<float>*, ::output_cascade<caccfloat>*);

template<int BATCH, int HEL_START, int HEL_END>
void kernel_k2a_ff_diag_hslice(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

template<int BATCH, int HEL_START, int HEL_END>
void kernel_k2b_ff_diag_hslice(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

template<int BATCH, int HEL_START, int HEL_END>
void kernel_k3_vvv_amp_hslice(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);

template<int BATCH, int HEL_START, int HEL_END>
void kernel_k4_vvvv_color_hslice(::input_cascade<caccfloat>*, ::output_stream<float>*);

// Reducer kernel: sums partial ME² from N_SLICES streams
template<int BATCH, int N_SLICES>
void kernel_reducer(::input_stream<float>* in[N_SLICES], ::output_stream<float>* out);

// ============================================================================
// Reducer Kernel Implementation
// ============================================================================
template<int BATCH, int N_SLICES>
void kernel_reducer(::input_stream<float>* in[N_SLICES], ::output_stream<float>* out) {
    for (int b = 0; b < BATCH; ++b) {
        float sum = 0.0f;
        for (int s = 0; s < N_SLICES; ++s) {
            sum += readincr(in[s]);
        }
        writeincr(out, sum);
    }
}

#if !defined(__AIENGINE__) && !defined(__chess__)

template <int Bits>
static constexpr adf::plio_type bits_to_plio_type_hp() {
    if  (Bits == 32)  return adf::plio_32_bits;
    if  (Bits == 64)  return adf::plio_64_bits;
    if  (Bits == 128) return adf::plio_128_bits;
    static_assert(Bits == 32 || Bits == 64 || Bits == 128, "PLIO width must be 32/64/128");
    return adf::plio_32_bits;
}

// ============================================================================
// Helicity-Parallel Graph (4-way split)
// ============================================================================
template <int BATCH>
class GraphGGTTG_HelicityParallel : public adf::graph {
public:
    static constexpr int N_SLICES = 4;
    static constexpr int HEL_PER_SLICE = 32 / N_SLICES;  // 8
    
    adf::input_plio  in_plio;
    adf::output_plio out_plio;
    
    // 4 parallel pipeline chains
    adf::kernel k_wfgen     [N_SLICES];
    adf::kernel k_ff_diag_a [N_SLICES];
    adf::kernel k_ff_diag_b [N_SLICES];
    adf::kernel k_vvv_amp   [N_SLICES];
    adf::kernel k_vvvv_color[N_SLICES];
    
    // Reducer
    adf::kernel k_reducer;

    GraphGGTTG_HelicityParallel() {
        constexpr auto IN_TYPE  = bits_to_plio_type_hp<GGTTG_IN_PLIO_BITS>();
        constexpr auto OUT_TYPE = bits_to_plio_type_hp<GGTTG_OUT_PLIO_BITS>();

        // Single PSP input
        in_plio = adf::input_plio::create("psp_in_0", IN_TYPE, "data/psp_in_0.txt");
        out_plio = adf::output_plio::create("me2_out_0", OUT_TYPE, "data/me2_out_0.txt");

        // ====================================================================
        // Kernel Instantiation with Helicity Ranges
        // ====================================================================
        
        // Slice 0: helicities 0-7
        k_wfgen     [0] = adf::kernel::create(kernel_k1_wfgen_hslice<BATCH, 0, 8>);
        k_ff_diag_a [0] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 0, 8>);
        k_ff_diag_b [0] = adf::kernel::create(kernel_k2b_ff_diag_hslice<BATCH, 0, 8>);
        k_vvv_amp   [0] = adf::kernel::create(kernel_k3_vvv_amp_hslice<BATCH, 0, 8>);
        k_vvvv_color[0] = adf::kernel::create(kernel_k4_vvvv_color_hslice<BATCH, 0, 8>);

        // Slice 1: helicities 8-15
        k_wfgen     [1] = adf::kernel::create(kernel_k1_wfgen_hslice<BATCH, 8, 16>);
        k_ff_diag_a [1] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 8, 16>);
        k_ff_diag_b [1] = adf::kernel::create(kernel_k2b_ff_diag_hslice<BATCH, 8, 16>);
        k_vvv_amp   [1] = adf::kernel::create(kernel_k3_vvv_amp_hslice<BATCH, 8, 16>);
        k_vvvv_color[1] = adf::kernel::create(kernel_k4_vvvv_color_hslice<BATCH, 8, 16>);

        // Slice 2: helicities 16-23
        k_wfgen     [2] = adf::kernel::create(kernel_k1_wfgen_hslice<BATCH, 16, 24>);
        k_ff_diag_a [2] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 16, 24>);
        k_ff_diag_b [2] = adf::kernel::create(kernel_k2b_ff_diag_hslice<BATCH, 16, 24>);
        k_vvv_amp   [2] = adf::kernel::create(kernel_k3_vvv_amp_hslice<BATCH, 16, 24>);
        k_vvvv_color[2] = adf::kernel::create(kernel_k4_vvvv_color_hslice<BATCH, 16, 24>);

        // Slice 3: helicities 24-31
        k_wfgen     [3] = adf::kernel::create(kernel_k1_wfgen_hslice<BATCH, 24, 32>);
        k_ff_diag_a [3] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 24, 32>);
        k_ff_diag_b [3] = adf::kernel::create(kernel_k2b_ff_diag_hslice<BATCH, 24, 32>);
        k_vvv_amp   [3] = adf::kernel::create(kernel_k3_vvv_amp_hslice<BATCH, 24, 32>);
        k_vvvv_color[3] = adf::kernel::create(kernel_k4_vvvv_color_hslice<BATCH, 24, 32>);

        // Reducer
        k_reducer = adf::kernel::create(kernel_reducer<BATCH, N_SLICES>);

        // ====================================================================
        // Source Files and Runtime
        // ====================================================================
        for (int s = 0; s < N_SLICES; ++s) {
            adf::source(k_wfgen     [s]) = "kernel_k1_wfgen_hslice.cpp";
            adf::source(k_ff_diag_a [s]) = "kernel_k2a_ff_diag_hslice.cpp";
            adf::source(k_ff_diag_b [s]) = "kernel_k2b_ff_diag_hslice.cpp";
            adf::source(k_vvv_amp   [s]) = "kernel_k3_vvv_amp_hslice.cpp";
            adf::source(k_vvvv_color[s]) = "kernel_k4_vvvv_color_hslice.cpp";
            
            adf::runtime<adf::ratio>(k_wfgen     [s]) = 1.0;
            adf::runtime<adf::ratio>(k_ff_diag_a [s]) = 1.0;
            adf::runtime<adf::ratio>(k_ff_diag_b [s]) = 1.0;
            adf::runtime<adf::ratio>(k_vvv_amp   [s]) = 1.0;
            adf::runtime<adf::ratio>(k_vvvv_color[s]) = 1.0;
        }
        adf::source(k_reducer) = "kernel_reducer.cpp";
        adf::runtime<adf::ratio>(k_reducer) = 1.0;

        // ====================================================================
        // PSP BROADCAST: Connect single input to all 4 K1 kernels
        // This requires stream fanout in the AXI-Stream network
        // ====================================================================
        for (int s = 0; s < N_SLICES; ++s) {
            adf::connect<adf::stream>(in_plio.out[0], k_wfgen[s].in[0]);
        }

        // ====================================================================
        // CASCADE Connections within each slice
        // ====================================================================
        for (int s = 0; s < N_SLICES; ++s) {
            adf::connect<adf::cascade>(k_wfgen[s].out[0], k_ff_diag_a[s].in[0]);
            adf::connect<adf::cascade>(k_ff_diag_a[s].out[0], k_ff_diag_b[s].in[0]);
            adf::connect<adf::cascade>(k_ff_diag_b[s].out[0], k_vvv_amp[s].in[0]);
            adf::connect<adf::cascade>(k_vvv_amp[s].out[0], k_vvvv_color[s].in[0]);
        }

        // ====================================================================
        // REDUCER Connections: All K4 outputs → Reducer → Output
        // ====================================================================
        for (int s = 0; s < N_SLICES; ++s) {
            adf::connect<adf::stream>(k_vvvv_color[s].out[0], k_reducer.in[s]);
        }
        adf::connect<adf::stream>(k_reducer.out[0], out_plio.in[0]);

        // ====================================================================
        // TILE PLACEMENT
        //
        // Use 4 rows, one per helicity slice
        // Reducer on a separate tile
        //
        //   Row 7: [K1_s3][K2a][K2b][K3][K4]  (hel 24-31, R→L)
        //   Row 6: [K1_s2][K2a][K2b][K3][K4]  (hel 16-23, L→R)
        //   Row 5: [K1_s1][K2a][K2b][K3][K4]  (hel 8-15,  R→L)
        //   Row 4: [K1_s0][K2a][K2b][K3][K4]  (hel 0-7,   L→R)
        //                              [Reducer] @ (5, 3)
        // ====================================================================
        const int BASE_ROW = 4;
        
        for (int s = 0; s < N_SLICES; ++s) {
            const int row = BASE_ROW + s;
            const bool is_odd_row = (row & 1);
            
            if (!is_odd_row) {
                // L→R cascade
                adf::location<adf::kernel>(k_wfgen     [s]) = adf::tile(0, row);
                adf::location<adf::kernel>(k_ff_diag_a [s]) = adf::tile(1, row);
                adf::location<adf::kernel>(k_ff_diag_b [s]) = adf::tile(2, row);
                adf::location<adf::kernel>(k_vvv_amp   [s]) = adf::tile(3, row);
                adf::location<adf::kernel>(k_vvvv_color[s]) = adf::tile(4, row);
            } else {
                // R→L cascade (reversed)
                adf::location<adf::kernel>(k_wfgen     [s]) = adf::tile(4, row);
                adf::location<adf::kernel>(k_ff_diag_a [s]) = adf::tile(3, row);
                adf::location<adf::kernel>(k_ff_diag_b [s]) = adf::tile(2, row);
                adf::location<adf::kernel>(k_vvv_amp   [s]) = adf::tile(1, row);
                adf::location<adf::kernel>(k_vvvv_color[s]) = adf::tile(0, row);
            }
        }
        
        // Reducer on column 5, row 4
        adf::location<adf::kernel>(k_reducer) = adf::tile(5, 4);
    }
};

#endif  // !defined(__AIENGINE__) && !defined(__chess__)
}  // namespace ggttg

// ============================================================================
// HELICITY-SLICED KERNEL TEMPLATE
//
// To implement the _hslice kernels, modify the helicity loop:
//
// ORIGINAL (in kernel_k1_wfgen.cpp):
//   for (int h = 0; h < 32; ++h) { ... }
//
// SLICED (in kernel_k1_wfgen_hslice.cpp):
//   for (int h = HEL_START; h < HEL_END; ++h) { ... }
//
// The color sum in K4 must also be modified:
//   - Don't divide by 256 (reducer does final normalization)
//   - Output partial sum for this slice's helicities
// ============================================================================

// ============================================================================
// PERFORMANCE ANALYSIS
// ============================================================================
//
// CURRENT (32 helicities, single chain):
//   - Latency: ~33,000 cycles/PSP
//   - Tiles: 5
//
// HELICITY-PARALLEL (4 slices × 8 helicities):
//   - Latency per slice: ~33,000/4 = ~8,250 cycles (for 8 helicities)
//   - Reducer overhead: ~100 cycles (4 reads + 1 write)
//   - Total latency: ~8,350 cycles/PSP
//   - Tiles: 4×5 + 1 = 21
//
// SPEEDUP: 33,000 / 8,350 = ~4× latency reduction
//
// COMBINED with PSP-parallelism:
//   - 4 PSP lanes × 4 helicity slices = 16× theoretical speedup
//   - Tiles: 16 × 5 + 4 reducers = 84 tiles (21% of 400)
//   - Throughput: ~480 kPSP/s
// ============================================================================
