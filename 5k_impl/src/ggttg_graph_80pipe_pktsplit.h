// ============================================================================
// ggttg_graph_80pipe_pktsplit.h
//
// Full-scale: 10-group, 80-pipeline packet-switched architecture
//
// Layout: 10 column-groups × 8 rows = 80 pipelines
//   Each group occupies 5 consecutive columns and all 8 rows.
//   PLIO is placed at the center column of each group (max 2 hops to any k1).
//
//   Group g → cols [g*5 .. g*5+4], PLIO at col g*5+2
//   Row r   → cascade direction: even rows L→R, odd rows R→L
//
//   10 input PLIOs × pktsplit<8> → 80 k1 kernels
//   80 k4 kernels → pktmerge<8> × 10 output PLIOs
//
// Packet IDs 0-7 address rows 0-7 within each group.
// ============================================================================

#ifndef GGTTG_GRAPH_80PIPE_PKTSPLIT_H
#define GGTTG_GRAPH_80PIPE_PKTSPLIT_H

#include <string>
#include <adf.h>
#include <aie_api/aie.hpp>
#include "ggttg_config.h"
#include "ggttg_params.h"

namespace ggttg {

// Forward declarations
template<int BATCH> void kernel_k1_wfgen_pkt(::input_pktstream*, ::output_cascade<caccfloat>*);
template<int BATCH> void kernel_k2a_ff_diag(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int BATCH> void kernel_k2b_ff_diag(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int BATCH> void kernel_k3_vvv_amp(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
template<int BATCH> void kernel_k4_vvvv_color_pkt(::input_cascade<caccfloat>*, ::output_pktstream*);

// ============================================================================
// ADF Graph
// ============================================================================
#if !defined(__AIENGINE__) && !defined(__chess__)

class Graph_80Pipeline_PktSplit : public adf::graph {
public:
    // 10 column-groups × 8 rows = 80 pipelines
    // Each group: 5 consecutive columns (one pipeline per row)
    static constexpr int NUM_GROUPS      = 10;  // column groups (one PLIO each)
    static constexpr int ROWS_PER_GROUP  = 8;   // rows 0-7, one pipeline per row
    static constexpr int COLS_PER_PIPE   = 5;   // k1,k2a,k2b,k3,k4
    static constexpr int TOTAL_PIPES     = NUM_GROUPS * ROWS_PER_GROUP;  // 80

    // 10 PLIO pairs (32-bit for packet switching)
    adf::input_plio  plio_in [NUM_GROUPS];
    adf::output_plio plio_out[NUM_GROUPS];

    // 10 packet switching nodes (pktsplit/pktmerge over 8 rows)
    adf::pktsplit<ROWS_PER_GROUP> splitter[NUM_GROUPS];
    adf::pktmerge<ROWS_PER_GROUP> merger  [NUM_GROUPS];

    // 80 worker pipelines (5 kernels each)
    adf::kernel k1 [TOTAL_PIPES];
    adf::kernel k2a[TOTAL_PIPES];
    adf::kernel k2b[TOTAL_PIPES];
    adf::kernel k3 [TOTAL_PIPES];
    adf::kernel k4 [TOTAL_PIPES];

    Graph_80Pipeline_PktSplit() {
        // ========================================
        // PLIO Configuration: 10 groups
        // PLIOs placed at the center column of each group so that every k1
        // kernel in the group is at most 2 stream-switch hops away.
        //   Group g center column: g*5 + 2
        // ========================================
        for (int g = 0; g < NUM_GROUPS; ++g) {
            std::string in_name  = "psp_in_"  + std::to_string(g);
            std::string out_name = "me2_out_" + std::to_string(g);
            std::string in_file  = "data/psp_in_"  + std::to_string(g) + "_streamed.txt";
            std::string out_file = "data/me2_out_" + std::to_string(g) + ".txt";

            plio_in[g] = adf::input_plio::create(
                in_name, adf::plio_32_bits, in_file, 500);
            plio_out[g] = adf::output_plio::create(
                out_name, adf::plio_32_bits, out_file, 500);

            // Pin each PLIO to a valid shim column spread evenly across the device.
            // xcvc1902: columns 0-5 and 44+ are NOT valid PLIO shim sites.
            // Formula g*4+6 gives: 6,10,14,18,22,26,30,34,38,42 — all valid.
            // Each PLIO is at most ~7 hops from any k1 in its group (far better
            // than the original ~22 hops when all 8 PLIOs clustered at cols 22-29).
            int plio_col = g * 4 + 6;
            adf::location<adf::PLIO>(plio_in[g])  = adf::shim(plio_col);
            adf::location<adf::PLIO>(plio_out[g]) = adf::shim(plio_col);
        }

        // ========================================
        // Packet Switching Infrastructure
        // ========================================
        for (int g = 0; g < NUM_GROUPS; ++g) {
            splitter[g] = adf::pktsplit<ROWS_PER_GROUP>::create();
            merger[g]   = adf::pktmerge<ROWS_PER_GROUP>::create();

            adf::connect<adf::pktstream>(plio_in[g].out[0],  splitter[g].in[0]);
            adf::connect<adf::pktstream>(merger[g].out[0],   plio_out[g].in[0]);
        }

        // ========================================
        // 80 Worker Pipelines
        //
        // Pipeline index: p = g * ROWS_PER_GROUP + r
        //   g = column group (0-9), r = row (0-7)
        //
        // Cascade direction per row:
        //   Even rows (0,2,4,6): L→R  → k1 at col_base+0, k4 at col_base+4
        //   Odd  rows (1,3,5,7): R→L  → k1 at col_base+4, k4 at col_base+0
        //
        // Packet ID for splitter/merger port = row index r (0-7)
        // ========================================
        for (int g = 0; g < NUM_GROUPS; ++g) {
            int col_base = g * COLS_PER_PIPE;

            for (int r = 0; r < ROWS_PER_GROUP; ++r) {
                int p = g * ROWS_PER_GROUP + r;

                // --- Instantiate kernels ---
                k1 [p] = adf::kernel::create(::ggttg::kernel_k1_wfgen_pkt<1>);
                k2a[p] = adf::kernel::create(::ggttg::kernel_k2a_ff_diag<1>);
                k2b[p] = adf::kernel::create(::ggttg::kernel_k2b_ff_diag<1>);
                k3 [p] = adf::kernel::create(::ggttg::kernel_k3_vvv_amp<1>);
                k4 [p] = adf::kernel::create(::ggttg::kernel_k4_vvvv_color_pkt<1>);

                adf::source(k1 [p]) = "kernel_k1_wfgen_pkt.cpp";
                adf::source(k2a[p]) = "kernel_k2a_ff_diag.cpp";
                adf::source(k2b[p]) = "kernel_k2b_ff_diag.cpp";
                adf::source(k3 [p]) = "kernel_k3_vvv_amp.cpp";
                adf::source(k4 [p]) = "kernel_k4_vvvv_color_pkt.cpp";

                adf::runtime<adf::ratio>(k1 [p]) = 1.0;
                adf::runtime<adf::ratio>(k2a[p]) = 1.0;
                adf::runtime<adf::ratio>(k2b[p]) = 1.0;
                adf::runtime<adf::ratio>(k3 [p]) = 1.0;
                adf::runtime<adf::ratio>(k4 [p]) = 1.0;

                // --- Cascade connections (intra-pipeline) ---
                adf::connect<adf::cascade>(k1 [p].out[0], k2a[p].in[0]);
                adf::connect<adf::cascade>(k2a[p].out[0], k2b[p].in[0]);
                adf::connect<adf::cascade>(k2b[p].out[0], k3 [p].in[0]);
                adf::connect<adf::cascade>(k3 [p].out[0], k4 [p].in[0]);

                // --- Placement: align with hardware cascade direction ---
                // Even rows cascade L→R: k1 leftmost, k4 rightmost
                // Odd  rows cascade R→L: k1 rightmost, k4 leftmost
                bool l2r = (r % 2 == 0);
                if (l2r) {
                    adf::location<adf::kernel>(k1 [p]) = adf::tile(col_base + 0, r);
                    adf::location<adf::kernel>(k2a[p]) = adf::tile(col_base + 1, r);
                    adf::location<adf::kernel>(k2b[p]) = adf::tile(col_base + 2, r);
                    adf::location<adf::kernel>(k3 [p]) = adf::tile(col_base + 3, r);
                    adf::location<adf::kernel>(k4 [p]) = adf::tile(col_base + 4, r);
                } else {
                    adf::location<adf::kernel>(k1 [p]) = adf::tile(col_base + 4, r);
                    adf::location<adf::kernel>(k2a[p]) = adf::tile(col_base + 3, r);
                    adf::location<adf::kernel>(k2b[p]) = adf::tile(col_base + 2, r);
                    adf::location<adf::kernel>(k3 [p]) = adf::tile(col_base + 1, r);
                    adf::location<adf::kernel>(k4 [p]) = adf::tile(col_base + 0, r);
                }

                // --- Stream connections: splitter port = row index ---
                adf::connect<>(splitter[g].out[r], k1[p].in[0]);
                adf::connect<>(k4[p].out[0],       merger[g].in[r]);
            }
        }
    }
};

#endif // !defined(__AIENGINE__) && !defined(__chess__)

} // namespace ggttg

#endif // GGTTG_GRAPH_80PIPE_PKTSPLIT_H
