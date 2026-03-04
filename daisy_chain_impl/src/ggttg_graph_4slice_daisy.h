// ============================================================================
// ggttg_graph_4slice_daisy.h
//
// ADF Graph for 4-Way Helicity-Sliced Daisy-Chain Architecture
//
// ARCHITECTURE:
//   - 4 parallel helicity slices (8 helicities each)
//   - PSP broadcast via stream daisy-chain (K1_0 → K1_1 → K1_2 → K1_3)
//   - ME² reduction via stream daisy-chain (K4_0 → K4_1 → K4_2 → K4_3)
//   - Cascade within each slice (K1 → K2a → K2b → K3 → K4)
//
// TILE LAYOUT (per unit, 4 rows × 5 columns = 20 tiles):
//
//   Row 0 (L→R): [K1_0]─casc─[K2a_0]─casc─[K2b_0]─casc─[K3_0]─casc─[K4_0]
//                  │                                                  │
//                stream                                            stream
//                  ↓                                                  ↓
//   Row 1 (R←L): [K4_1]─casc─[K3_1]─casc─[K2b_1]─casc─[K2a_1]─casc─[K1_1]
//                  │                                                  │
//                stream                                            stream
//                  ↓                                                  ↓
//   Row 2 (L→R): [K1_2]─casc─[K2a_2]─casc─[K2b_2]─casc─[K3_2]─casc─[K4_2]
//                  │                                                  │
//                stream                                            stream
//                  ↓                                                  ↓
//   Row 3 (R←L): [K4_3]─casc─[K3_3]─casc─[K2b_3]─casc─[K2a_3]─casc─[K1_3]
//                  │
//               PLIO_out
//
// RESOURCES PER UNIT:
//   - Tiles: 20 (4 slices × 5 kernels)
//   - PLIO:  2 (1 input, 1 output)
//   - Streams per kernel: ≤2 (within AIE limit)
//
// SCALABILITY:
//   - Max units: 16 (limited by 32 PLIO)
//   - Total tiles: 16 × 20 = 320 (80% of 400)
// ============================================================================

#pragma once
#include <string>
#include <adf.h>
#include <aie_api/aie.hpp>
#include "ggttg_config.h"
#include "ggttg_params.h"

namespace ggttg {

// ============================================================================
// Forward declarations for daisy-chain kernels
// ============================================================================

// K1: PSP forwarding + helicity slicing
template<int BATCH, int HEL_START, int HEL_END, bool FORWARD>
void kernel_k1_wfgen_daisy(
    ::input_stream<float>*,
    ::output_stream<float>*,
    ::output_cascade<caccfloat>*);

// K2a: Helicity slicing only
template<int BATCH, int HEL_START, int HEL_END>
void kernel_k2a_ff_diag_hslice(
    ::input_cascade<caccfloat>*,
    ::output_cascade<caccfloat>*);

// K2b: Helicity slicing only
template<int BATCH, int HEL_START, int HEL_END>
void kernel_k2b_ff_diag_hslice(
    ::input_cascade<caccfloat>*,
    ::output_cascade<caccfloat>*);

// K3: Helicity slicing only
template<int BATCH, int HEL_START, int HEL_END>
void kernel_k3_vvv_amp_hslice(
    ::input_cascade<caccfloat>*,
    ::output_cascade<caccfloat>*);

// K4: ME² accumulation + helicity slicing
template<int BATCH, int HEL_START, int HEL_END, bool IS_FIRST, bool IS_LAST>
void kernel_k4_vvvv_color_daisy(
    ::input_cascade<caccfloat>*,
    ::input_stream<float>*,
    ::output_stream<float>*);

// ============================================================================
// ADF Graph
// ============================================================================
#if !defined(__AIENGINE__) && !defined(__chess__)

template <int Bits>
static constexpr adf::plio_type bits_to_plio_type_daisy() {
    if  (Bits == 32)  return adf::plio_32_bits;
    if  (Bits == 64)  return adf::plio_64_bits;
    if  (Bits == 128) return adf::plio_128_bits;
    static_assert(Bits == 32 || Bits == 64 || Bits == 128, "PLIO width must be 32/64/128");
    return adf::plio_32_bits;
}

// ============================================================================
// Single Unit: 4 Helicity Slices with Daisy-Chain Streams
// ============================================================================
template<int BATCH, int UNIT_ID, int BASE_COL, int BASE_ROW>
class DaisyChainUnit : public adf::graph {
public:
    adf::input_plio  psp_in;
    adf::output_plio me2_out;

private:
    // 4 slices × 5 kernels = 20 kernels per unit
    adf::kernel k1[4];
    adf::kernel k2a[4];
    adf::kernel k2b[4];
    adf::kernel k3[4];
    adf::kernel k4[4];

public:
    DaisyChainUnit() {
        constexpr auto IN_TYPE  = bits_to_plio_type_daisy<GGTTG_IN_PLIO_BITS>();
        constexpr auto OUT_TYPE = bits_to_plio_type_daisy<GGTTG_OUT_PLIO_BITS>();
        
        // ====================================================================
        // Create PLIO
        // ====================================================================
        const std::string in_name  = "psp_in_"  + std::to_string(UNIT_ID);
        const std::string out_name = "me2_out_" + std::to_string(UNIT_ID);
        
    #ifdef GGTTG_PLIO_FREQ_MHZ
        const double freq = static_cast<double>(GGTTG_PLIO_FREQ_MHZ);
        psp_in  = adf::input_plio::create(in_name,  IN_TYPE,  "data/" + in_name + ".txt", freq);
        me2_out = adf::output_plio::create(out_name, OUT_TYPE, "data/" + out_name + ".txt", freq);
    #else
        psp_in  = adf::input_plio::create(in_name,  IN_TYPE,  "data/" + in_name + ".txt");
        me2_out = adf::output_plio::create(out_name, OUT_TYPE, "data/" + out_name + ".txt");
    #endif
        
        // ====================================================================
        // Create Kernels for each slice
        // ====================================================================
        
        // Slice 0: helicities 0-7, forwards PSP, first in ME² chain
        k1[0]  = adf::kernel::create(kernel_k1_wfgen_daisy<BATCH, 0, 8, true>);
        k2a[0] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 0, 8>);
        k2b[0] = adf::kernel::create(kernel_k2b_ff_diag_hslice<BATCH, 0, 8>);
        k3[0]  = adf::kernel::create(kernel_k3_vvv_amp_hslice<BATCH, 0, 8>);
        k4[0]  = adf::kernel::create(kernel_k4_vvvv_color_daisy<BATCH, 0, 8, true, false>);
        
        // Slice 1: helicities 8-15, forwards PSP, middle of ME² chain
        k1[1]  = adf::kernel::create(kernel_k1_wfgen_daisy<BATCH, 8, 16, true>);
        k2a[1] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 8, 16>);
        k2b[1] = adf::kernel::create(kernel_k2b_ff_diag_hslice<BATCH, 8, 16>);
        k3[1]  = adf::kernel::create(kernel_k3_vvv_amp_hslice<BATCH, 8, 16>);
        k4[1]  = adf::kernel::create(kernel_k4_vvvv_color_daisy<BATCH, 8, 16, false, false>);
        
        // Slice 2: helicities 16-23, forwards PSP, middle of ME² chain
        k1[2]  = adf::kernel::create(kernel_k1_wfgen_daisy<BATCH, 16, 24, true>);
        k2a[2] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 16, 24>);
        k2b[2] = adf::kernel::create(kernel_k2b_ff_diag_hslice<BATCH, 16, 24>);
        k3[2]  = adf::kernel::create(kernel_k3_vvv_amp_hslice<BATCH, 16, 24>);
        k4[2]  = adf::kernel::create(kernel_k4_vvvv_color_daisy<BATCH, 16, 24, false, false>);
        
        // Slice 3: helicities 24-31, NO forward (last), last in ME² chain
        k1[3]  = adf::kernel::create(kernel_k1_wfgen_daisy<BATCH, 24, 32, false>);
        k2a[3] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 24, 32>);
        k2b[3] = adf::kernel::create(kernel_k2b_ff_diag_hslice<BATCH, 24, 32>);
        k3[3]  = adf::kernel::create(kernel_k3_vvv_amp_hslice<BATCH, 24, 32>);
        k4[3]  = adf::kernel::create(kernel_k4_vvvv_color_daisy<BATCH, 24, 32, false, true>);
        
        // ====================================================================
        // Set source files
        // ====================================================================
        for (int s = 0; s < 4; ++s) {
            adf::source(k1[s])  = "kernel_k1_wfgen_daisy.cpp";
            adf::source(k2a[s]) = "kernel_k2a_ff_diag_hslice.cpp";
            adf::source(k2b[s]) = "kernel_k2b_ff_diag_hslice.cpp";
            adf::source(k3[s])  = "kernel_k3_vvv_amp_hslice.cpp";
            adf::source(k4[s])  = "kernel_k4_vvvv_color_daisy.cpp";
            
            adf::runtime<adf::ratio>(k1[s])  = 0.9;
            adf::runtime<adf::ratio>(k2a[s]) = 0.9;
            adf::runtime<adf::ratio>(k2b[s]) = 0.9;
            adf::runtime<adf::ratio>(k3[s])  = 0.9;
            adf::runtime<adf::ratio>(k4[s])  = 0.9;
        }
        
        // ====================================================================
        // Tile Placement
        // 
        // Each slice uses one row, cascade direction alternates:
        //   Even rows (0,2): cascade L→R, K1 at left, K4 at right
        //   Odd rows (1,3):  cascade R→L, K1 at right, K4 at left
        // ====================================================================
        for (int s = 0; s < 4; ++s) {
            int row = BASE_ROW + s;
            
            if (row % 2 == 0) {
                // Even row: cascade flows Left→Right
                adf::location<adf::kernel>(k1[s])  = adf::tile(BASE_COL + 0, row);
                adf::location<adf::kernel>(k2a[s]) = adf::tile(BASE_COL + 1, row);
                adf::location<adf::kernel>(k2b[s]) = adf::tile(BASE_COL + 2, row);
                adf::location<adf::kernel>(k3[s])  = adf::tile(BASE_COL + 3, row);
                adf::location<adf::kernel>(k4[s])  = adf::tile(BASE_COL + 4, row);
            } else {
                // Odd row: cascade flows Right→Left
                adf::location<adf::kernel>(k1[s])  = adf::tile(BASE_COL + 4, row);
                adf::location<adf::kernel>(k2a[s]) = adf::tile(BASE_COL + 3, row);
                adf::location<adf::kernel>(k2b[s]) = adf::tile(BASE_COL + 2, row);
                adf::location<adf::kernel>(k3[s])  = adf::tile(BASE_COL + 1, row);
                adf::location<adf::kernel>(k4[s])  = adf::tile(BASE_COL + 0, row);
            }
        }
        
        // ====================================================================
        // CASCADE connections: Within each slice (K1 → K2a → K2b → K3 → K4)
        // ====================================================================
        for (int s = 0; s < 4; ++s) {
            adf::connect<adf::cascade>(k1[s].out[0],  k2a[s].in[0]);
            adf::connect<adf::cascade>(k2a[s].out[0], k2b[s].in[0]);
            adf::connect<adf::cascade>(k2b[s].out[0], k3[s].in[0]);
            adf::connect<adf::cascade>(k3[s].out[0],  k4[s].in[0]);
        }
        
        // ====================================================================
        // STREAM connections: PSP daisy-chain (PLIO → K1_0 → K1_1 → K1_2 → K1_3)
        // ====================================================================
        
        // PLIO → K1_0 (stream input port 0)
        adf::connect<adf::stream>(psp_in.out[0], k1[0].in[0]);
        
        // K1_0 → K1_1 → K1_2 → K1_3 (PSP forwarding via stream output port 1)
        // Note: K1 has out[0]=cascade, out[1]=stream for PSP forward
        adf::connect<adf::stream>(k1[0].out[1], k1[1].in[0]);
        adf::connect<adf::stream>(k1[1].out[1], k1[2].in[0]);
        adf::connect<adf::stream>(k1[2].out[1], k1[3].in[0]);
        // K1_3 has FORWARD=false, so no out[1] connection
        
        // ====================================================================
        // STREAM connections: ME² daisy-chain (K4_0 → K4_1 → K4_2 → K4_3 → PLIO)
        // ====================================================================
        
        // K4_0 → K4_1 (partial ME²)
        // Note: K4 has in[0]=cascade, in[1]=stream for partial ME²
        adf::connect<adf::stream>(k4[0].out[0], k4[1].in[1]);
        
        // K4_1 → K4_2
        adf::connect<adf::stream>(k4[1].out[0], k4[2].in[1]);
        
        // K4_2 → K4_3
        adf::connect<adf::stream>(k4[2].out[0], k4[3].in[1]);
        
        // K4_3 → PLIO (final ME²)
        adf::connect<adf::stream>(k4[3].out[0], me2_out.in[0]);
    }
};

// ============================================================================
// Multi-Unit Graph: Multiple Independent Daisy-Chain Units
// ============================================================================
template<int BATCH, int N_UNITS>
class GraphGGTTG_4Slice_Daisy : public adf::graph {
public:
    // For simplicity, define units statically (up to 8 units shown)
    // Each unit uses 5 columns and 4 rows
    
    DaisyChainUnit<BATCH, 0, 0, 0> unit0;
    
    // Uncomment to add more units (each needs BASE_COL += 6 for spacing)
    // DaisyChainUnit<BATCH, 1, 6, 0> unit1;
    // DaisyChainUnit<BATCH, 2, 12, 0> unit2;
    // DaisyChainUnit<BATCH, 3, 18, 0> unit3;
    // DaisyChainUnit<BATCH, 4, 24, 0> unit4;
    // DaisyChainUnit<BATCH, 5, 30, 0> unit5;
    // DaisyChainUnit<BATCH, 6, 36, 0> unit6;
    // DaisyChainUnit<BATCH, 7, 42, 0> unit7;
    
    // Second row of units (rows 4-7)
    // DaisyChainUnit<BATCH, 8, 0, 4> unit8;
    // ...
    
    GraphGGTTG_4Slice_Daisy() {
        // Constructor - units are auto-constructed
    }
};

#endif  // !defined(__AIENGINE__) && !defined(__chess__)

}  // namespace ggttg
