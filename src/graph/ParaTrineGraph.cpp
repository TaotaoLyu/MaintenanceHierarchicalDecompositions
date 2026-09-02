#include "ParaTrineGraph.hpp"

#include <algorithm>

#include "GraphUtils.h"

ParaTrineGraph::ParaTrineGraph(Graph &g, uint32_t *trine) : g(g), trine(trine)
{
    __build_index();
}

void ParaTrineGraph::__build_index()
{
    int n = g.GetNodeNum();
    uint32_t mx = 0;
#if defined(__linux__)
#pragma omp parallel for reduction(max : mx)
#endif
    for (int i = 0; i < n; i++)
    {
        mx = std::max(mx, trine[i]);
    }
    max_trine = mx;

    seq_.resize(n);
    bin_.resize(max_trine + 2);
    rank_.resize(n);
    int num_threads;
    VVI bin_private;

#pragma omp parallel
    {
#pragma omp single
        {
            num_threads = omp_get_num_threads();
            bin_private = VVI(num_threads, VI(max_trine + 1, 0));
        }
        int cur_p = omp_get_thread_num();
        int start = (LL)n * cur_p / num_threads;
        int end = (LL)n * (cur_p + 1) / num_threads;
        end = std::min(end, n);

        for (int v = start; v < end; ++v)
        {
            bin_private[cur_p][trine[v]]++;
        }
        for (LL k = 0; k <= max_trine; k++)
        {
            my_fetch_add(&bin_[k + 1], bin_private[cur_p][k]);
        }
#pragma omp barrier

#pragma omp single
        {
            for (LL k = 1; k <= max_trine + 1; ++k)
            {
                bin_[k] += bin_[k - 1];
            }
        }
#pragma omp for
        for (LL k = 0; k <= max_trine; k++)
        {
            bin_private[0][k] += bin_[k];
            for (int j = 1; j < num_threads; j++)
            {
                bin_private[j][k] += bin_private[j - 1][k];
            }
        }
        for (int v = end - 1; v >= start; --v)
        {
            int pos = --bin_private[cur_p][trine[v]];
            seq_[pos] = v;
            rank_[v] = pos;
        }
    }
}

bool ParaTrineGraph::rank_cmp(int x, int y)
{
    return rank_[x] < rank_[y];
}

int ParaTrineGraph::get_coreness(int x)
{
    return trine[x];
}
