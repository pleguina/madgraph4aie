// ============================================================================
// graph_batch8_grid.h
//
// Full-chip replication of the 8-tile batched pipeline (K1a/K1b/K2a-propA/
// K2a-propB/K2a-amps/K2b/K3/K4color, 8 PSPs per invocation, every tile under
// the 16384 B program-memory budget), templatized on the number of lanes.
//
// LAYOUT (serpentine): the 50-column x 8-row array is one cascade chain of
// 400 tiles. Even rows run from low to high column, odd rows from high to
// low, and the chain continues vertically at the array edge (column 49
// between rows 0-1, 2-3, 4-5, 6-7; column 0 between rows 1-2, 3-4, 5-6).
// Stage s of lane l sits at chain position 8*l + s:
//   row = (8*l + s) / 50
//   col = row even ? (8*l + s) % 50 : 49 - (8*l + s) % 50
// 50 lanes x 8 tiles = 400 tiles (100 %). Lanes 6, 12, 18, 31, 37 and 43
// span two rows; the other 44 lanes stay within one row. The cascade
// direction on each row is fixed by the silicon: a chain declared against
// it is rejected by the mapper (AIE-PRE-MAPPER-46).
// ============================================================================
#pragma once
#include <string>
#include <adf.h>
#include <aie_api/aie.hpp>
#include "ggttg_config.h"
#include "ggttg_params.h"

namespace ggttg {

template<int NPSP> void kernel_k1a_wfgen_batch8(::input_stream<float>*, ::output_cascade<caccfloat>*);
template<int NPSP> void kernel_k1b_vvvprop_batch8(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int NPSP> void kernel_k2a_propA_batch8(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int NPSP> void kernel_k2a_propB_batch8(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int NPSP> void kernel_k2a_amps_batch8(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int NPSP> void kernel_k2b_batch8(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int NPSP> void kernel_k3_batch8(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int NPSP> void kernel_k4color_batch8(::input_cascade<caccfloat>*, ::output_stream<float>*);

#if !defined(__AIENGINE__) && !defined(__chess__)

template <int NLANES>
class GraphBatch8Grid : public adf::graph {
public:
    static constexpr int TILES_PER_LANE = 8;
    static constexpr int ARRAY_COLS     = 50;
    static constexpr int ARRAY_ROWS     = 8;
    static constexpr int MAX_LANES      = ARRAY_COLS * ARRAY_ROWS / TILES_PER_LANE; // 50
    static_assert(NLANES >= 1 && NLANES <= MAX_LANES,
                  "NLANES must fit the 50-col x 8-row array at 8 tiles/lane (max 50)");

    adf::input_plio  in_plio [NLANES];
    adf::output_plio out_plio[NLANES];

    adf::kernel k1a      [NLANES];
    adf::kernel k1b      [NLANES];
    adf::kernel k2a_propA[NLANES];
    adf::kernel k2a_propB[NLANES];
    adf::kernel k2a_amps [NLANES];
    adf::kernel k2b      [NLANES];
    adf::kernel k3       [NLANES];
    adf::kernel k4color  [NLANES];

    GraphBatch8Grid() {
        for (int l = 0; l < NLANES; ++l) {
            const std::string in_name  = "psp_in_" + std::to_string(l);
            const std::string out_name = "out_"    + std::to_string(l);

            in_plio [l] = adf::input_plio ::create(in_name , adf::plio_32_bits, "data/" + in_name  + ".txt");
            out_plio[l] = adf::output_plio::create(out_name, adf::plio_32_bits, "data/" + out_name + ".txt");

            k1a      [l] = adf::kernel::create(::ggttg::kernel_k1a_wfgen_batch8<8>);
            k1b      [l] = adf::kernel::create(::ggttg::kernel_k1b_vvvprop_batch8<8>);
            k2a_propA[l] = adf::kernel::create(::ggttg::kernel_k2a_propA_batch8<8>);
            k2a_propB[l] = adf::kernel::create(::ggttg::kernel_k2a_propB_batch8<8>);
            k2a_amps [l] = adf::kernel::create(::ggttg::kernel_k2a_amps_batch8<8>);
            k2b      [l] = adf::kernel::create(::ggttg::kernel_k2b_batch8<8>);
            k3       [l] = adf::kernel::create(::ggttg::kernel_k3_batch8<8>);
            k4color  [l] = adf::kernel::create(::ggttg::kernel_k4color_batch8<8>);

            adf::source(k1a      [l]) = "kernel_k1a_wfgen_batch8.cpp";
            adf::source(k1b      [l]) = "kernel_k1b_vvvprop_batch8.cpp";
            adf::source(k2a_propA[l]) = "kernel_k2a_propA_batch8.cpp";
            adf::source(k2a_propB[l]) = "kernel_k2a_propB_batch8.cpp";
            adf::source(k2a_amps [l]) = "kernel_k2a_amps_batch8.cpp";
            adf::source(k2b      [l]) = "kernel_k2b_batch8.cpp";
            adf::source(k3       [l]) = "kernel_k3_batch8.cpp";
            adf::source(k4color  [l]) = "kernel_k4color_batch8.cpp";

            for (adf::kernel* k : {&k1a[l], &k1b[l], &k2a_propA[l], &k2a_propB[l],
                                    &k2a_amps[l], &k2b[l], &k3[l], &k4color[l]})
                adf::runtime<adf::ratio>(*k) = 1.0;

            auto conn_psp = adf::connect<adf::stream>(in_plio[l].out[0], k1a[l].in[0]);
            adf::fifo_depth(conn_psp) = 256;

            adf::connect<adf::cascade>(k1a[l].out[0],       k1b[l].in[0]);
            adf::connect<adf::cascade>(k1b[l].out[0],       k2a_propA[l].in[0]);
            adf::connect<adf::cascade>(k2a_propA[l].out[0], k2a_propB[l].in[0]);
            adf::connect<adf::cascade>(k2a_propB[l].out[0], k2a_amps[l].in[0]);
            adf::connect<adf::cascade>(k2a_amps[l].out[0],  k2b[l].in[0]);
            adf::connect<adf::cascade>(k2b[l].out[0],       k3[l].in[0]);
            adf::connect<adf::cascade>(k3[l].out[0],        k4color[l].in[0]);
            adf::connect<adf::stream>(k4color[l].out[0],    out_plio[l].in[0]);

            // Serpentine placement, see LAYOUT above.
            auto tile_of = [&](int stage) {
                const int idx = l * TILES_PER_LANE + stage;
                const int row = idx / ARRAY_COLS;
                const int off = idx % ARRAY_COLS;
                const int col = (row % 2 == 0) ? off : (ARRAY_COLS - 1 - off);
                return adf::tile(col, row);
            };

            adf::location<adf::kernel>(k1a      [l]) = tile_of(0);
            adf::location<adf::kernel>(k1b      [l]) = tile_of(1);
            adf::location<adf::kernel>(k2a_propA[l]) = tile_of(2);
            adf::location<adf::kernel>(k2a_propB[l]) = tile_of(3);
            adf::location<adf::kernel>(k2a_amps [l]) = tile_of(4);
            adf::location<adf::kernel>(k2b      [l]) = tile_of(5);
            adf::location<adf::kernel>(k3       [l]) = tile_of(6);
            adf::location<adf::kernel>(k4color  [l]) = tile_of(7);
        }
    }
};

#endif // !defined(__AIENGINE__) && !defined(__chess__)

} // namespace ggttg
