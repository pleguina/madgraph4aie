#include "graph_batch8_grid.h"

#ifndef GGTTG_NUM_ITER
#define GGTTG_NUM_ITER 1
#endif

#ifndef GGTTG_NLANES
#define GGTTG_NLANES 50
#endif

using namespace ggttg;

GraphBatch8Grid<GGTTG_NLANES> grid_graph;

#if defined(__AIESIM__) || defined(__X86SIM__)
int main() {
    grid_graph.init();
    grid_graph.run(GGTTG_NUM_ITER);
    grid_graph.end();
    return 0;
}
#endif
