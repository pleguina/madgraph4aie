// ============================================================================
// graph_4slice_daisy.cpp
//
// Test Graph instantiation for 4-Slice Daisy-Chain Architecture
//
// To compile and test:
//   1. Update CMakeLists.txt to use this graph
//   2. Ensure data/psp_in_0.txt exists (same format as before)
//   3. Build with: cmake --build build/hw
//   4. Simulate with: aiesimulator (or x86simulator for x86sim)
// ============================================================================

#include "ggttg_graph_4slice_daisy.h"

// Instantiate a single unit for testing
// BATCH=1: Process 1 PSP per invocation
// N_UNITS=1: Single unit (4 slices, 20 tiles)
ggttg::GraphGGTTG_4Slice_Daisy<1, 1> myGraph;

#if defined(__AIESIM__) || defined(__X86SIM__)
int main() {
    myGraph.init();
    myGraph.run(1);  // Run once (processes BATCH PSPs)
    myGraph.end();
    return 0;
}
#endif
