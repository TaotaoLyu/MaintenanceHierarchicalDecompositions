#pragma once
#include "Graph.hpp"
#include <vector>

using VI=std::vector<int>;
using VVI=std::vector<std::vector<int>>;


class ParaGraph
{
public:
    ParaGraph(Graph *g, int *core);

    bool rank_cmp(int x, int y);
    int get_coreness(int x);

    std::vector<int> cgt, ceq;
    std::vector<int> seq_, bin_, rank_;
    Graph *g;
    int *core;
    int max_core;

private:
    ParaGraph();
    void __build_index();
};