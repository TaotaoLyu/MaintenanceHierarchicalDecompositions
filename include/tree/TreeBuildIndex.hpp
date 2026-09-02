#pragma once

#include <cstdint>
#include <vector>

#include "Graph.hpp"

class TreeBuildIndex
{
public:
    TreeBuildIndex(Graph &g, uint32_t *level);

    bool rank_cmp(int x, int y);
    int get_level(int x);

    std::vector<int> seq_, bin_, rank_;
    Graph &g;
    uint32_t *level;
    uint32_t max_level;

private:
    TreeBuildIndex();
    void __build_index();
};
