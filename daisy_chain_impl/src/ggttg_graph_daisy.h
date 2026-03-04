//===----------------------------------------------------------------------===//
// Daisy-Chain Graph for 4-Way Helicity Slicing
//===----------------------------------------------------------------------===//
//
// Architecture:
//
//   PLIO_in ────────────────────────────────────────────────────────────┐
//       │                                                               │
//       ▼                                                               │
//   ┌─────────────────────────────────────────────────────────────────┐ │
//   │ K1_0 ══stream══> K1_1 ══stream══> K1_2 ══stream══> K1_3         │ │
//   │   │                │                │                │          │ │
//   │ cascade          cascade          cascade          cascade      │ │
//   │   ▼                ▼                ▼                ▼          │ │
//   │ K2a_0            K2a_1            K2a_2            K2a_3        │ │
//   │   │                │                │                │          │ │
//   │ cascade          cascade          cascade          cascade      │ │
//   │   ▼                ▼                ▼                ▼          │ │
//   │ K2b_0            K2b_1            K2b_2            K2b_3        │ │
//   │   │                │                │                │          │ │
//   │ cascade          cascade          cascade          cascade      │ │
//   │   ▼                ▼                ▼                ▼          │ │
//   │ K3_0             K3_1             K3_2             K3_3         │ │
//   │   │                │                │                │          │ │
//   │ cascade          cascade          cascade          cascade      │ │
//   │   ▼                ▼                ▼                ▼          │ │
//   │ K4_0 ══stream══> K4_1 ══stream══> K4_2 ══stream══> K4_3 ────────┼─┼──> PLIO_out
//   └─────────────────────────────────────────────────────────────────┘ │
//                                                                       │
// Tile Placement (example for Unit 0, rows 0-3):                        │
//                                                                       │
//   Row 0 (L→R): [K1_0]──[K2a_0]──[K2b_0]──[K3_0]──[K4_0]              │
//   Row 1 (R→L): [K4_1]──[K3_1]──[K2b_1]──[K2a_1]──[K1_1]              │
//   Row 2 (L→R): [K1_2]──[K2a_2]──[K2b_2]──[K3_2]──[K4_2]              │
//   Row 3 (R→L): [K4_3]──[K3_3]──[K2b_3]──[K2a_3]──[K1_3]              │
//                                                                       │
// Stream routing:                                                       │
//   K1_0 (row 0) ───stream───> K1_1 (row 1) [vertical]                 │
//   K1_1 (row 1) ───stream───> K1_2 (row 2) [vertical]                 │
//   ...etc                                                              │
//                                                                       │
//===----------------------------------------------------------------------===//

#ifndef GGTTG_GRAPH_DAISY_H
#define GGTTG_GRAPH_DAISY_H

#include <adf.h>
#include "kernel_k1_daisy.h"
#include "kernel_k4_daisy.h"
// Include your K2a, K2b, K3 headers (need helicity-sliced versions)

using namespace adf;

//===----------------------------------------------------------------------===//
// Single Unit: 4 Slices with Daisy-Chain Streams
//===----------------------------------------------------------------------===//
template<int UNIT_ID, int BASE_COL>
class DaisyChainUnit : public graph {
public:
    // External ports
    input_plio  psp_in;
    output_plio me2_out;
    
private:
    // Kernels for each slice (4 slices × 5 kernels = 20 kernels)
    kernel k1[4];
    kernel k2a[4];
    kernel k2b[4];
    kernel k3[4];
    kernel k4[4];
    
public:
    DaisyChainUnit() {
        // =====================================================================
        // Create PLIO
        // =====================================================================
        std::string in_name = "psp_in_" + std::to_string(UNIT_ID);
        std::string out_name = "me2_out_" + std::to_string(UNIT_ID);
        
        psp_in  = input_plio::create(in_name, plio_32_bits, "data/psp_in_" + std::to_string(UNIT_ID) + ".txt");
        me2_out = output_plio::create(out_name, plio_32_bits, "data/me2_out_" + std::to_string(UNIT_ID) + ".txt");
        
        // =====================================================================
        // Create Kernels
        // =====================================================================
        
        // K1 kernels with PSP forwarding
        k1[0] = kernel::create(kernel_k1_wfgen_daisy<0, 8, true>);
        k1[1] = kernel::create(kernel_k1_wfgen_daisy<8, 16, true>);
        k1[2] = kernel::create(kernel_k1_wfgen_daisy<16, 24, true>);
        k1[3] = kernel::create(kernel_k1_wfgen_daisy<24, 32, false>);
        
        // K2a, K2b, K3 kernels (using your existing helicity-sliced versions)
        // TODO: Replace with actual helicity-sliced kernel functions
        for (int s = 0; s < 4; ++s) {
            // k2a[s] = kernel::create(kernel_k2a_ff_diag_hslice<s*8, (s+1)*8>);
            // k2b[s] = kernel::create(kernel_k2b_ff_diag_hslice<s*8, (s+1)*8>);
            // k3[s]  = kernel::create(kernel_k3_vvv_amp_hslice<s*8, (s+1)*8>);
        }
        
        // K4 kernels with ME² accumulation
        k4[0] = kernel::create(kernel_k4_vvvv_color_daisy<0, 8, true, false>);
        k4[1] = kernel::create(kernel_k4_vvvv_color_daisy<8, 16, false, false>);
        k4[2] = kernel::create(kernel_k4_vvvv_color_daisy<16, 24, false, false>);
        k4[3] = kernel::create(kernel_k4_vvvv_color_daisy<24, 32, false, true>);
        
        // =====================================================================
        // Set Source Files
        // =====================================================================
        for (int s = 0; s < 4; ++s) {
            source(k1[s])  = "src/kernel_k1_daisy.cpp";
            // source(k2a[s]) = "src/kernel_k2a_ff_diag_hslice.cpp";
            // source(k2b[s]) = "src/kernel_k2b_ff_diag_hslice.cpp";
            // source(k3[s])  = "src/kernel_k3_vvv_amp_hslice.cpp";
            source(k4[s])  = "src/kernel_k4_daisy.cpp";
        }
        
        // =====================================================================
        // Tile Placement
        // =====================================================================
        // Each slice uses one row, cascade flows horizontally
        // Slices on different rows to use 4 independent cascade paths
        
        for (int s = 0; s < 4; ++s) {
            int row = s;  // Slice s uses row s (relative to unit's row allocation)
            
            if (row % 2 == 0) {
                // Even row: cascade flows Left→Right
                // K1 at left, K4 at right
                location<kernel>(k1[s])  = tile(BASE_COL + 0, row);
                location<kernel>(k2a[s]) = tile(BASE_COL + 1, row);
                location<kernel>(k2b[s]) = tile(BASE_COL + 2, row);
                location<kernel>(k3[s])  = tile(BASE_COL + 3, row);
                location<kernel>(k4[s])  = tile(BASE_COL + 4, row);
            } else {
                // Odd row: cascade flows Right→Left
                // K1 at right, K4 at left
                location<kernel>(k1[s])  = tile(BASE_COL + 4, row);
                location<kernel>(k2a[s]) = tile(BASE_COL + 3, row);
                location<kernel>(k2b[s]) = tile(BASE_COL + 2, row);
                location<kernel>(k3[s])  = tile(BASE_COL + 1, row);
                location<kernel>(k4[s])  = tile(BASE_COL + 0, row);
            }
        }
        
        // =====================================================================
        // Connect Cascade (within each slice)
        // =====================================================================
        for (int s = 0; s < 4; ++s) {
            connect<cascade>(k1[s].out[0],  k2a[s].in[0]);
            connect<cascade>(k2a[s].out[0], k2b[s].in[0]);
            connect<cascade>(k2b[s].out[0], k3[s].in[0]);
            connect<cascade>(k3[s].out[0],  k4[s].in[0]);
        }
        
        // =====================================================================
        // Connect PSP Daisy Chain (K1_0 → K1_1 → K1_2 → K1_3)
        // =====================================================================
        
        // PLIO → K1_0
        connect<stream>(psp_in.out[0], k1[0].in[0]);
        
        // K1_0 → K1_1 → K1_2 → K1_3 (stream forwarding)
        connect<stream>(k1[0].out[0], k1[1].in[0]);
        connect<stream>(k1[1].out[0], k1[2].in[0]);
        connect<stream>(k1[2].out[0], k1[3].in[0]);
        
        // =====================================================================
        // Connect ME² Daisy Chain (K4_0 → K4_1 → K4_2 → K4_3)
        // =====================================================================
        
        // K4_0 → K4_1 → K4_2 → K4_3 (partial ME² accumulation)
        connect<stream>(k4[0].out[0], k4[1].in[1]);  // in[1] is partial_in
        connect<stream>(k4[1].out[0], k4[2].in[1]);
        connect<stream>(k4[2].out[0], k4[3].in[1]);
        
        // K4_3 → PLIO
        connect<stream>(k4[3].out[0], me2_out.in[0]);
        
        // =====================================================================
        // Runtime Ratios
        // =====================================================================
        for (int s = 0; s < 4; ++s) {
            runtime<ratio>(k1[s])  = 0.9;
            runtime<ratio>(k2a[s]) = 0.9;
            runtime<ratio>(k2b[s]) = 0.9;
            runtime<ratio>(k3[s])  = 0.9;
            runtime<ratio>(k4[s])  = 0.9;
        }
    }
};

//===----------------------------------------------------------------------===//
// Multi-Unit Graph: Multiple Independent Daisy-Chain Units
//===----------------------------------------------------------------------===//
template<int N_UNITS>
class MultiUnitDaisyGraph : public graph {
public:
    // Array of units
    DaisyChainUnit<0, 0> unit0;
    // Add more units as needed, each with different BASE_COL
    // DaisyChainUnit<1, 10> unit1;  // Unit 1 starts at column 10
    // ...
    
    // With 4 rows per unit and 5 columns, we can fit:
    // 50 columns / 5 columns per unit = 10 units across
    // 8 rows / 4 rows per unit = 2 units vertically
    // Total: 20 units max (but limited by 32 PLIO → 16 units)
};

#endif // GGTTG_GRAPH_DAISY_H
