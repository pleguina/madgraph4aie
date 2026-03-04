// ============================================================================
// kernels_hslice_adapters.h
//
// Header-only adapters for converting existing K2a, K2b, K3 kernels to 
// helicity-sliced versions.
//
// APPROACH: The existing K2a, K2b, K3 kernels work on tokens that contain
// data for ONE helicity at a time. The helicity loop is in each kernel.
// For helicity-sliced versions, we simply parameterize the loop bounds.
//
// IMPLEMENTATION NOTE:
// Rather than duplicating all the kernel code, we use template wrappers
// that call the original kernel body with different loop bounds.
// ============================================================================

#pragma once

namespace ggttg {

// ============================================================================
// Helicity Range Traits
// ============================================================================

// 4-way split: 8 helicities per slice
struct HSlice4Way {
    static constexpr int N_SLICES = 4;
    static constexpr int HEL_PER_SLICE = 8;
    
    static constexpr int start(int slice) { return slice * 8; }
    static constexpr int end(int slice) { return (slice + 1) * 8; }
};

// 8-way split: 4 helicities per slice
struct HSlice8Way {
    static constexpr int N_SLICES = 8;
    static constexpr int HEL_PER_SLICE = 4;
    
    static constexpr int start(int slice) { return slice * 4; }
    static constexpr int end(int slice) { return (slice + 1) * 4; }
};

// 2-way split: 16 helicities per slice
struct HSlice2Way {
    static constexpr int N_SLICES = 2;
    static constexpr int HEL_PER_SLICE = 16;
    
    static constexpr int start(int slice) { return slice * 16; }
    static constexpr int end(int slice) { return (slice + 1) * 16; }
};

// ============================================================================
// NOTE ON IMPLEMENTATION
// ============================================================================
//
// The existing kernels (K2a, K2b, K3) have this structure:
//
//   template<int BATCH>
//   void kernel_k2a_ff_diag(...) {
//       for (int b = 0; b < BATCH; ++b) {
//           for (int h = 0; h < 32; ++h) {        // <-- This loop
//               // Process helicity h
//           }
//       }
//   }
//
// For helicity-sliced versions, we change to:
//
//   template<int BATCH, int HEL_START, int HEL_END>
//   void kernel_k2a_ff_diag_hslice(...) {
//       for (int b = 0; b < BATCH; ++b) {
//           for (int h = HEL_START; h < HEL_END; ++h) {  // <-- Parameterized
//               // Process helicity h (same body)
//           }
//       }
//   }
//
// Since K2a, K2b, K3 process tokens in the SAME ORDER as K1 produces them,
// and K4 consumes them, the helicity slicing "just works" as long as:
//
// 1. K1 produces tokens for helicities [HEL_START, HEL_END) only
// 2. K2a, K2b, K3 process the same range
// 3. K4 consumes the same range and outputs partial ME²
// 4. Reducer sums all partial ME² values
//
// ============================================================================
// QUICK MODIFICATION GUIDE
// ============================================================================
//
// To convert kernel_k2a_ff_diag.cpp to helicity-sliced:
//
// 1. Change template signature:
//    FROM: template<int BATCH>
//    TO:   template<int BATCH, int HEL_START, int HEL_END>
//
// 2. Change inner loop:
//    FROM: for (int h = 0; h < 32; ++h)
//    TO:   for (int h = HEL_START; h < HEL_END; ++h)
//
// 3. Add explicit instantiations for each helicity range:
//    template void kernel_k2a_ff_diag<1, 0, 8>(...);
//    template void kernel_k2a_ff_diag<1, 8, 16>(...);
//    template void kernel_k2a_ff_diag<1, 16, 24>(...);
//    template void kernel_k2a_ff_diag<1, 24, 32>(...);
//
// The same modification applies to K2b and K3.
//
// ============================================================================

} // namespace ggttg

// ============================================================================
// FORWARD DECLARATIONS FOR GRAPH
// ============================================================================
//
// Use these in ggttg_graph_max_throughput.h:
//
// template<int BATCH, int HEL_START, int HEL_END>
// void kernel_k2a_ff_diag_hslice(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
//
// template<int BATCH, int HEL_START, int HEL_END>
// void kernel_k2b_ff_diag_hslice(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
//
// template<int BATCH, int HEL_START, int HEL_END>
// void kernel_k3_vvv_amp_hslice(::input_cascade<caccfloat>*, ::output_cascade<caccfloat>*);
//
// Then in the graph:
//   k_ff_diag_a[u][0] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 0, 8>);
//   k_ff_diag_a[u][1] = adf::kernel::create(kernel_k2a_ff_diag_hslice<BATCH, 8, 16>);
//   ...etc
// ============================================================================
