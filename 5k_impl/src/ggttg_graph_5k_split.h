// ============================================================================
// ggttg_graph_5k_split.h
//
// ADF graph for 5-Kernel Token Cascade Pipeline with SPLIT HELAS compilation
//
// ARCHITECTURE (5 kernels to resolve program memory overflow):
//   K1  (WFGEN)      : External WFs + VVV1P0_1 + FFV1P0_3
//   K2a (FF_DIAG_A)  : FFV1_1, FFV1_2, FFV1_0 (diagrams D2-D7, all propagators)
//   K2b (FF_DIAG_B)  : FFV1_0 (diagrams D8-D14)
//   K3  (VVV_AMP)    : VVV1_0 ONLY
//   K4  (VVVV_COLOR) : VVVV + FFV1_0 + color reduction
//
// PM BUDGET (expected):
//   K1:  ~17KB (same, still overflows by ~1.4KB)
//   K2a: ~9-10KB (6 diagrams + propagator computation)
//   K2b: ~9-10KB (6 diagrams using propagators)
//   K3:  ~7KB (VVV1_0 only)
//   K4:  ~10KB (VVVV + color)
//
// PIPELINE LAYOUT (5 tiles per lane):
//   Col:   base    base+1    base+2    base+3    base+4
//          K1       K2a       K2b        K3        K4
//       (WFGEN) (FF_DIAG_A)(FF_DIAG_B)(VVV_AMP)(VVVV_COLOR)
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

// ============================================================================
// ADF Graph
// ============================================================================
#if !defined(__AIENGINE__) && !defined(__chess__)

template <int Bits>
static constexpr adf::plio_type bits_to_plio_type_5k_split() {
    if  (Bits == 32)  return adf::plio_32_bits;
    if  (Bits == 64)  return adf::plio_64_bits;
    if  (Bits == 128) return adf::plio_128_bits;
    static_assert(Bits == 32 || Bits == 64 || Bits == 128, "PLIO width must be 32/64/128");
    return adf::plio_32_bits;
}

template <int BATCH, int NLANES>
class GraphGGTTG_5K_Split : public adf::graph {
public:
    adf::input_plio  in_plio [NLANES];
    adf::output_plio out_plio[NLANES];

    adf::kernel k_wfgen     [NLANES];
    adf::kernel k_ff_diag_a [NLANES];
    adf::kernel k_ff_diag_b [NLANES];
    adf::kernel k_vvv_amp   [NLANES];
    adf::kernel k_vvvv_color[NLANES];

    GraphGGTTG_5K_Split() {
        static constexpr int IN_SAMPLES  = BATCH * 5 * 4;  // 5 particles × 4 floats per PSP
        static constexpr int OUT_SAMPLES = BATCH;          // 1 ME2 result per PSP

        constexpr auto IN_TYPE  = bits_to_plio_type_5k_split<GGTTG_IN_PLIO_BITS>();
        constexpr auto OUT_TYPE = bits_to_plio_type_5k_split<GGTTG_OUT_PLIO_BITS>();

        for (int l = 0; l < NLANES; ++l) {
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
            // Kernel instantiation
            // ================================================================
            k_wfgen     [l] = adf::kernel::create(::ggttg::kernel_k1_wfgen<BATCH>);
            k_ff_diag_a [l] = adf::kernel::create(::ggttg::kernel_k2a_ff_diag<BATCH>);
            k_ff_diag_b [l] = adf::kernel::create(::ggttg::kernel_k2b_ff_diag<BATCH>);
            k_vvv_amp   [l] = adf::kernel::create(::ggttg::kernel_k3_vvv_amp<BATCH>);
            k_vvvv_color[l] = adf::kernel::create(::ggttg::kernel_k4_vvvv_color<BATCH>);

            // Runtime ratio
            adf::runtime<adf::ratio>(k_wfgen     [l]) = 1.0;
            adf::runtime<adf::ratio>(k_ff_diag_a [l]) = 1.0;
            adf::runtime<adf::ratio>(k_ff_diag_b [l]) = 1.0;
            adf::runtime<adf::ratio>(k_vvv_amp   [l]) = 1.0;
            adf::runtime<adf::ratio>(k_vvvv_color[l]) = 1.0;

            // ================================================================
            // SPLIT SOURCE FILES
            // ================================================================
            adf::source(k_wfgen     [l]) = "kernel_k1_wfgen.cpp";
            adf::source(k_ff_diag_a [l]) = "kernel_k2a_ff_diag.cpp";
            adf::source(k_ff_diag_b [l]) = "kernel_k2b_ff_diag.cpp";
            adf::source(k_vvv_amp   [l]) = "kernel_k3_vvv_amp.cpp";
            adf::source(k_vvvv_color[l]) = "kernel_k4_vvvv_color.cpp";

            // ================================================================
            // PLIO -> K1 connection (stream)
            // ================================================================
            auto conn_psp = adf::connect<adf::stream>(in_plio[l].out[0], k_wfgen[l].in[0]);
            adf::fifo_depth(conn_psp) = 256;

            // ================================================================
            // CASCADE connections: WF-Token pipeline
            //   K1 → K2a → K2b → K3 → K4
            //
            // Token sizes (optimized for minimal cascade traffic):
            //   K1→K2a:  Extended (9 WFs + JAMP) = 9×2 + 2 = 20 cascade writes
            //   K2a→K2b: Extended (9 WFs + JAMP) = 20 cascade writes
            //   K2b→K3:  Extended (9 WFs + JAMP) = 20 cascade writes
            //   K3→K4:   Standard (5 WFs + JAMP) = 5×2 + 2 = 12 cascade writes
            //
            // Note: K2a and K2b each recompute needed propagators from external WFs
            //       instead of passing them via token (trades computation for bandwidth)
            // ================================================================
            adf::connect<adf::cascade>(k_wfgen[l].out[0], k_ff_diag_a[l].in[0]);
            adf::connect<adf::cascade>(k_ff_diag_a[l].out[0], k_ff_diag_b[l].in[0]);
            adf::connect<adf::cascade>(k_ff_diag_b[l].out[0], k_vvv_amp[l].in[0]);
            adf::connect<adf::cascade>(k_vvv_amp[l].out[0], k_vvvv_color[l].in[0]);

            // ================================================================
            // K4 -> PLIO connection (stream output)
            // ================================================================
            adf::connect<adf::stream> k4_to_PLIO(k_vvvv_color[l].out[0], out_plio[l].in[0]);

            // ================================================================
            // TILE PLACEMENT CONSTRAINTS
            // 5 tiles per pipeline (consecutive columns for cascade)
            // ================================================================
            const int base_col = l * 6;  // 6 column spacing for placement flexibility
            const int row = 4;

            adf::location<adf::kernel>(k_wfgen     [l]) = adf::tile(base_col,     row);
            adf::location<adf::kernel>(k_ff_diag_a [l]) = adf::tile(base_col + 1, row);
            adf::location<adf::kernel>(k_ff_diag_b [l]) = adf::tile(base_col + 2, row);
            adf::location<adf::kernel>(k_vvv_amp   [l]) = adf::tile(base_col + 3, row);
            adf::location<adf::kernel>(k_vvvv_color[l]) = adf::tile(base_col + 4, row);

            // ================================================================
            // OPTIMIZATION FLAGS (already using -Oz for maximum size reduction)
            // ================================================================
        }
    }
};

#endif  // !defined(__AIENGINE__) && !defined(__chess__)
}  // namespace ggttg
