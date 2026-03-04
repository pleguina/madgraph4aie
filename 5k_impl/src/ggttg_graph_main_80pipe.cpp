// ============================================================================
// ggttg_graph_main_80pipe.cpp
//
// Main file for full-scale: 80-pipeline packet-switched architecture
//
// ARCHITECTURE:
//   10 column-groups × 8 rows = 80 total pipelines
//   Packet-switched: pktsplit<8> → pipelines → pktmerge<8>
//   Each pipeline: K1 (pkt) → K2a → K2b → K3 → K4 (pkt)
//   Each group spans 5 consecutive cols; PLIO pinned to center col.
// ============================================================================

#include "ggttg_graph_80pipe_pktsplit.h"
#include "ggttg_config.h"
#include "ggttg_params.h"

#if defined(__AIESIM__) || defined(__X86SIM__)
  #include <cstdio>
  #define GRAPH_DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
  #define GRAPH_DEBUG_PRINT(...)
#endif

static constexpr int BATCH  = GGTTG_BATCH;
static constexpr int NUM_GROUPS      = 10;  // column groups (one PLIO each)
static constexpr int ROWS_PER_GROUP  = 8;   // pipelines per group (rows 0-7)
static constexpr int TOTAL_PIPES     = 80;

using namespace ggttg;

// Instantiate the 80-Pipeline Packet-Switched Graph
Graph_80Pipeline_PktSplit ggttg_graph_80pipe;

#if defined(__AIESIM__) || defined(__X86SIM__)
int main(int argc, char** argv) {
    GRAPH_DEBUG_PRINT("\n========================================================\n");
    GRAPH_DEBUG_PRINT("GGTTG 80-Pipeline Packet-Switched Architecture\n");
    GRAPH_DEBUG_PRINT("========================================================\n");
    GRAPH_DEBUG_PRINT("Configuration:\n");
    GRAPH_DEBUG_PRINT("  BATCH         = %d\n", BATCH);
    GRAPH_DEBUG_PRINT("  NUM_GROUPS    = %d\n", NUM_GROUPS);
    GRAPH_DEBUG_PRINT("  ROWS/GROUP    = %d\n", ROWS_PER_GROUP);
    GRAPH_DEBUG_PRINT("  TOTAL_PIPES   = %d\n", TOTAL_PIPES);
    GRAPH_DEBUG_PRINT("========================================================\n");
    GRAPH_DEBUG_PRINT("Architecture:\n");
    GRAPH_DEBUG_PRINT("  Input:  10 PLIOs (32-bit) → pktsplit<8>\n");
    GRAPH_DEBUG_PRINT("  Pipeline: K1(pkt) → K2a → K2b → K3 → K4(pkt)\n");
    GRAPH_DEBUG_PRINT("  Output: pktmerge<8> → 10 PLIOs (32-bit)\n");
    GRAPH_DEBUG_PRINT("========================================================\n\n");

    GRAPH_DEBUG_PRINT("Initializing graph...\n");
    ggttg_graph_80pipe.init();

    GRAPH_DEBUG_PRINT("Running graph (1 iteration = 80 PSPs)...\n");
    ggttg_graph_80pipe.run(1);

    GRAPH_DEBUG_PRINT("Waiting for graph completion...\n");
    ggttg_graph_80pipe.wait();

    GRAPH_DEBUG_PRINT("Graph execution complete.\n");
    ggttg_graph_80pipe.end();

    GRAPH_DEBUG_PRINT("========================================================\n");
    GRAPH_DEBUG_PRINT("Pipeline finished successfully.\n");
    GRAPH_DEBUG_PRINT("========================================================\n");

    return 0;
}
#endif
