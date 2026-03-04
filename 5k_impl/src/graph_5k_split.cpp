// ============================================================================
// graph_5k_split.cpp
//
// Entry point for AIE 5-Kernel Token Cascade Pipeline with SPLIT HELAS
//
// ARCHITECTURE:
//   K1  (WFGEN)      : External WFs + VVV1P0_1 + FFV1P0_3
//   K2a (FF_DIAG_A)  : FFV1_1, FFV1_2, FFV1_0 (diagrams D2-D7)
//   K2b (FF_DIAG_B)  : FFV1_0 (diagrams D8-D14)
//   K3  (VVV_AMP)    : VVV1_0 ONLY
//   K4  (VVVV_COLOR) : VVVV + FFV1_0 + color reduction
//
// PM BUDGET (expected):
//   K1:  ~17KB (same as before)
//   K2a: ~9-10KB (6 diagrams + propagators)
//   K2b: ~9-10KB (6 diagrams)
//   K3:  ~7KB (VVV1_0 only)
//   K4:  ~10KB (VVVV + color)
// ============================================================================

#include "ggttg_graph_5k_split.h"
#include "ggttg_config.h"
#include "ggttg_params.h"

#if defined(__AIESIM__) || defined(__X86SIM__)
  #include <cstdio>
  // #define GRAPH_DEBUG_PRINT(...) printf(__VA_ARGS__)  // Disabled for 1000 events
  #define GRAPH_DEBUG_PRINT(...)
#else
  #define GRAPH_DEBUG_PRINT(...)
#endif

static constexpr int BATCH  = GGTTG_BATCH;
static constexpr int NCORES = GGTTG_NCORES;

using namespace ggttg;

// Instantiate the 5-Kernel Split Graph
GraphGGTTG_5K_Split<BATCH, NCORES> ggttg_graph_5k_split;

#if defined(__AIESIM__) || defined(__X86SIM__)
int main() {
    GRAPH_DEBUG_PRINT("\n========================================================\n");
    GRAPH_DEBUG_PRINT("GGTTG 5-Kernel Token Cascade Pipeline (SPLIT HELAS)\n");
    GRAPH_DEBUG_PRINT("========================================================\n");
    GRAPH_DEBUG_PRINT("Configuration:\n");
    GRAPH_DEBUG_PRINT("  BATCH  = %d\n", BATCH);
    GRAPH_DEBUG_PRINT("  NCORES = %d (pipelines)\n", NCORES);
    GRAPH_DEBUG_PRINT("========================================================\n");
    GRAPH_DEBUG_PRINT("5-Kernel Architecture (SPLIT COMPILATION):\n");
    GRAPH_DEBUG_PRINT("  K1:  kernel_k1_wfgen.cpp       - vxxxxx, VVV1P0_1, FFV1P0_3\n");
    GRAPH_DEBUG_PRINT("  K2a: kernel_k2a_ff_diag.cpp    - FFV1_1, FFV1_2, FFV1_0 (D2-D7)\n");
    GRAPH_DEBUG_PRINT("  K2b: kernel_k2b_ff_diag.cpp    - FFV1_0 (D8-D14)\n");
    GRAPH_DEBUG_PRINT("  K3:  kernel_k3_vvv_amp.cpp     - VVV1_0 ONLY\n");
    GRAPH_DEBUG_PRINT("  K4:  kernel_k4_vvvv_color.cpp  - VVVV + color\n");
    GRAPH_DEBUG_PRINT("========================================================\n\n");

    GRAPH_DEBUG_PRINT("Initializing graph...\n");
    ggttg_graph_5k_split.init();

    GRAPH_DEBUG_PRINT("Running graph (1 iteration)...\n");
    ggttg_graph_5k_split.run(1);

    GRAPH_DEBUG_PRINT("Waiting for graph completion...\n");

    GRAPH_DEBUG_PRINT("Graph execution complete.\n");
    ggttg_graph_5k_split.end();

    GRAPH_DEBUG_PRINT("========================================================\n");
    GRAPH_DEBUG_PRINT("Pipeline finished successfully.\n");
    GRAPH_DEBUG_PRINT("========================================================\n");

    return 0;
}
#endif
