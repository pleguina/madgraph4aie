// ============================================================================
// ggttg_graph_multilane.h
//
// Multi-Lane ADF Graph for PSP-Level Parallelism
//
// ARCHITECTURE: N parallel 5-kernel cascade pipelines processing independent PSPs
//
// IMPORTANT: Cascade direction alternates by row!
//   - Even rows (0,2,4,6): Cascade flows LEFT → RIGHT ✓
//   - Odd rows (1,3,5,7):  Cascade flows RIGHT → LEFT (reversed column order!)
//
// This graph handles cascade direction automatically based on row placement.
//
// TILE LAYOUT (4-lane example):
//   Row 7: [K4]←[K3]←[K2b]←[K2a]←[K1]  (R→L cascade, cols reversed)
//   Row 6: [K1]→[K2a]→[K2b]→[K3]→[K4]  (L→R cascade)
//   Row 5: [K4]←[K3]←[K2b]←[K2a]←[K1]  (R→L cascade, cols reversed)
//   Row 4: [K1]→[K2a]→[K2b]→[K3]→[K4]  (L→R cascade)
// ============================================================================

#pragma once
#include <string>
#include <adf.h>
#include <aie_api/aie.hpp>
#include "ggttg_config.h"
#include "ggttg_params.h"

namespace ggttg {

// Forward declarations
template<int BATCH> void kernel_k1_wfgen(::input_stream<float>*, ::output_cascade<caccfloat>*);
template<int BATCH> void kernel_k2a_ff_diag(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int BATCH> void kernel_k2b_ff_diag(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int BATCH> void kernel_k3_vvv_amp(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int BATCH> void kernel_k4_vvvv_color(::input_cascade<caccfloat>*, ::output_stream<float>*);

#if !defined(__AIENGINE__) && !defined(__chess__)

template <int Bits>
static constexpr adf::plio_type bits_to_plio_type_ml() {
    if  (Bits == 32)  return adf::plio_32_bits;
    if  (Bits == 64)  return adf::plio_64_bits;
    if  (Bits == 128) return adf::plio_128_bits;
    static_assert(Bits == 32 || Bits == 64 || Bits == 128, "PLIO width must be 32/64/128");
    return adf::plio_32_bits;
}

// ============================================================================
// Multi-Lane Graph with Cascade Direction Awareness
// ============================================================================
template <int BATCH, int NLANES>
class GraphGGTTG_MultiLane : public adf::graph {
public:
    static_assert(NLANES >= 1 && NLANES <= 8, "NLANES must be 1-8");
    
    adf::input_plio  in_plio [NLANES];
    adf::output_plio out_plio[NLANES];

    adf::kernel k_wfgen     [NLANES];
    adf::kernel k_ff_diag_a [NLANES];
    adf::kernel k_ff_diag_b [NLANES];
    adf::kernel k_vvv_amp   [NLANES];
    adf::kernel k_vvvv_color[NLANES];

    GraphGGTTG_MultiLane() {
        static constexpr int IN_SAMPLES  = BATCH * 5 * 4;  // 5 particles × 4 floats
        static constexpr int OUT_SAMPLES = BATCH;          // 1 ME2 result

        constexpr auto IN_TYPE  = bits_to_plio_type_ml<GGTTG_IN_PLIO_BITS>();
        constexpr auto OUT_TYPE = bits_to_plio_type_ml<GGTTG_OUT_PLIO_BITS>();

        // Base row and column offsets
        constexpr int BASE_ROW = 4;   // Start at row 4
        constexpr int COL_SPACING = 6; // 6 columns between lanes (5 kernels + 1 gap)

        for (int l = 0; l < NLANES; ++l) {
            // ================================================================
            // PLIO Configuration
            // ================================================================
            const std::string in_name  = "psp_in_"  + std::to_string(l);
            const std::string out_name = "me2_out_" + std::to_string(l);

        #ifdef GGTTG_PLIO_FREQ_MHZ
            const double freq = static_cast<double>(GGTTG_PLIO_FREQ_MHZ);
            in_plio [l] = adf::input_plio ::create(in_name , IN_TYPE , "data/" + in_name  + ".txt", freq);
            out_plio[l] = adf::output_plio::create(out_name, OUT_TYPE, "data/" + out_name + ".txt", freq);
        #else
            in_plio [l] = adf::input_plio ::create(in_name , IN_TYPE , "data/" + in_name  + ".txt");
            out_plio[l] = adf::output_plio::create(out_name, OUT_TYPE, "data/" + out_name + ".txt");
        #endif

            // ================================================================
            // Kernel instantiation (same for all rows)
            // ================================================================
            k_wfgen     [l] = adf::kernel::create(::ggttg::kernel_k1_wfgen<BATCH>);
            k_ff_diag_a [l] = adf::kernel::create(::ggttg::kernel_k2a_ff_diag<BATCH>);
            k_ff_diag_b [l] = adf::kernel::create(::ggttg::kernel_k2b_ff_diag<BATCH>);
            k_vvv_amp   [l] = adf::kernel::create(::ggttg::kernel_k3_vvv_amp<BATCH>);
            k_vvvv_color[l] = adf::kernel::create(::ggttg::kernel_k4_vvvv_color<BATCH>);

            adf::runtime<adf::ratio>(k_wfgen     [l]) = 1.0;
            adf::runtime<adf::ratio>(k_ff_diag_a [l]) = 1.0;
            adf::runtime<adf::ratio>(k_ff_diag_b [l]) = 1.0;
            adf::runtime<adf::ratio>(k_vvv_amp   [l]) = 1.0;
            adf::runtime<adf::ratio>(k_vvvv_color[l]) = 1.0;

            adf::source(k_wfgen     [l]) = "kernel_k1_wfgen.cpp";
            adf::source(k_ff_diag_a [l]) = "kernel_k2a_ff_diag.cpp";
            adf::source(k_ff_diag_b [l]) = "kernel_k2b_ff_diag.cpp";
            adf::source(k_vvv_amp   [l]) = "kernel_k3_vvv_amp.cpp";
            adf::source(k_vvvv_color[l]) = "kernel_k4_vvvv_color.cpp";

            // ================================================================
            // PLIO -> K1 connection
            // ================================================================
            auto conn_psp = adf::connect<adf::stream>(in_plio[l].out[0], k_wfgen[l].in[0]);
            adf::fifo_depth(conn_psp) = 256;

            // ================================================================
            // CASCADE connections (direction-independent - hardware handles it)
            // ================================================================
            adf::connect<adf::cascade>(k_wfgen[l].out[0], k_ff_diag_a[l].in[0]);
            adf::connect<adf::cascade>(k_ff_diag_a[l].out[0], k_ff_diag_b[l].in[0]);
            adf::connect<adf::cascade>(k_ff_diag_b[l].out[0], k_vvv_amp[l].in[0]);
            adf::connect<adf::cascade>(k_vvv_amp[l].out[0], k_vvvv_color[l].in[0]);

            // ================================================================
            // K4 -> PLIO connection
            // ================================================================
            adf::connect<adf::stream>(k_vvvv_color[l].out[0], out_plio[l].in[0]);

            // ================================================================
            // TILE PLACEMENT with cascade direction awareness
            //
            // Even rows (4, 6): cascade L→R, kernels at cols [0,1,2,3,4]
            // Odd rows (5, 7):  cascade R→L, kernels at cols [4,3,2,1,0] (reversed!)
            // ================================================================
            const int row = BASE_ROW + l;
            const int base_col = (l / 2) * COL_SPACING; // Each pair of rows shares column space
            
            const bool is_odd_row = (row & 1);
            
            if (!is_odd_row) {
                // Even row: cascade flows L→R, normal column order
                adf::location<adf::kernel>(k_wfgen     [l]) = adf::tile(base_col,     row);
                adf::location<adf::kernel>(k_ff_diag_a [l]) = adf::tile(base_col + 1, row);
                adf::location<adf::kernel>(k_ff_diag_b [l]) = adf::tile(base_col + 2, row);
                adf::location<adf::kernel>(k_vvv_amp   [l]) = adf::tile(base_col + 3, row);
                adf::location<adf::kernel>(k_vvvv_color[l]) = adf::tile(base_col + 4, row);
            } else {
                // Odd row: cascade flows R→L, REVERSED column order!
                // K1 must be at highest column, K4 at lowest
                adf::location<adf::kernel>(k_wfgen     [l]) = adf::tile(base_col + 4, row);
                adf::location<adf::kernel>(k_ff_diag_a [l]) = adf::tile(base_col + 3, row);
                adf::location<adf::kernel>(k_ff_diag_b [l]) = adf::tile(base_col + 2, row);
                adf::location<adf::kernel>(k_vvv_amp   [l]) = adf::tile(base_col + 1, row);
                adf::location<adf::kernel>(k_vvvv_color[l]) = adf::tile(base_col,     row);
            }
        }
    }
};

// ============================================================================
// Convenience type aliases
// ============================================================================
using Graph_1Lane  = GraphGGTTG_MultiLane<1, 1>;  // Current baseline
using Graph_2Lanes = GraphGGTTG_MultiLane<1, 2>;  // 2× throughput
using Graph_4Lanes = GraphGGTTG_MultiLane<1, 4>;  // 4× throughput
using Graph_8Lanes = GraphGGTTG_MultiLane<1, 8>;  // 8× throughput (max)

#endif  // !defined(__AIENGINE__) && !defined(__chess__)
}  // namespace ggttg

// ============================================================================
// RESOURCE SUMMARY
// ============================================================================
//
// NLANES | Tiles | PLIO In | PLIO Out | Rows Used     | Columns Used
// -------|-------|---------|----------|---------------|-------------
//   1    |   5   |    1    |    1     | 4             | 0-4
//   2    |  10   |    2    |    2     | 4-5           | 0-4
//   4    |  20   |    4    |    4     | 4-7           | 0-4, 6-10
//   8    |  40   |    8    |    8     | 4-7 (×2 cols) | 0-4, 6-10, 12-16, 18-22
//
// EXPECTED THROUGHPUT (@ 1 GHz, ~33k cycles/PSP):
//   1 lane:  ~30 kPSP/s
//   2 lanes: ~60 kPSP/s
//   4 lanes: ~120 kPSP/s
//   8 lanes: ~240 kPSP/s
//
// PLIO BUDGET CHECK (max 32):
//   8 lanes = 16 PLIO (8 in + 8 out) = 50% utilization ✓
// ============================================================================
