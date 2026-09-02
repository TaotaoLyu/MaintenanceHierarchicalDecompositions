#pragma once

#include <cstdint>
#include <vector>

#include "Graph.hpp"

class ParaTrineGraph
{
public:
    ParaTrineGraph(Graph &g, uint32_t *trine);

    bool rank_cmp(int x, int y);
    int get_coreness(int x);

    std::vector<int> seq_, bin_, rank_;
    Graph &g;
    uint32_t *trine;
    uint32_t max_trine;

private:
    ParaTrineGraph();
    void __build_index();
};
