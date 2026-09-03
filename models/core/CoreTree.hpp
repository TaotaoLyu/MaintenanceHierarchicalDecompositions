#pragma once
#include "Graph.hpp"
#include <stdlib.h>
#include <omp.h>
#include <assert.h>
#include "GraphUtils.h"
#include "TreeBuildIndex.hpp"
#define frac 0.98

typedef int vid_t;
typedef int eid_t;
struct PKCGraph
{
    long n;
    long m;

    vid_t *adj;
    eid_t *num_edges;
};
int free_graph(PKCGraph *g);
void PKC(Graph *g, int *deg);
struct CoreTreeNode
{
public:
    int id_max_rk;
    int k;
    int pa;
    int sz;

    VI vertex_;
    VI child_;
};
class ParaCoreTree
{
public:
    ParaCoreTree(Graph &g_)
    :g(g_)
    {
    }
    void build()
    {
        std::vector<int> deg(g.GetVertexCount());
        
        PKC(&g, deg.data());
        TreeBuildIndex pg(g, (uint32_t *)deg.data());
        int n = pg.g.GetVertexCount();
        tid_ = VI(n, -1);
        VI tag(n, -1);
        WaitFreeRankDSU dsu_(n, pg.rank_);

        int k = pg.max_level;
        int start = pg.bin_[k], end = pg.bin_[k + 1];
#pragma omp parallel
        {
            while (k >= 0)
            {

                VI nbr_max_rk_private;
#pragma omp for
                for (int i = start; i < end; i++)
                {
                    int u = pg.seq_[i];
                    for (int v : pg.g[u])
                    {
                        if (pg.get_level(v) >= k)
                        {
                            int pivot = dsu_.get_pivot(v);
                            if (pg.get_level(pivot) != k)
                            {
                                int t = tag[pivot];
                                if (t != k && my_compare_swap(&tag[pivot], t, k) == t)
                                {
                                    nbr_max_rk_private.push_back(pivot);
                                }
                            }
                            dsu_.joint(u, pivot);
                        }
                    }
                }

#pragma omp for
                for (int i = start; i < end; i++)
                {
                    int v = pg.seq_[i];
                    if (dsu_.get_pivot(v) != v)
                        continue;
                    CoreTreeNode *newv = new CoreTreeNode;
                    newv->k = k;
                    newv->pa = -1;
#pragma omp critical
                    {
                        tid_[v] = t_.size();
                        t_.push_back(newv);
                    }
                }

#pragma omp for
                for (int i = start; i < end; i++)
                {
                    int v = pg.seq_[i];
                    int pivot = dsu_.get_pivot(v);
                    tid_[v] = tid_[pivot];
                }

                for (int i = 0; i < nbr_max_rk_private.size(); i++)
                {
                    int v = nbr_max_rk_private[i];
                    int tid_son = tid_[v];
                    int pivot = dsu_.get_pivot(v);
                    t_[tid_son]->pa = tid_[pivot];
                }
#pragma omp single
                {
                    k--;
                    while (k >= 0 && pg.bin_[k] == pg.bin_[k + 1])
                        k--;
                    if (k >= 0)
                        start = pg.bin_[k], end = pg.bin_[k + 1];
                }
            }
        }

        int tn = t_.size();
        int num_threads;
        VVI bin_private;
#pragma omp parallel
        {
#pragma omp single
            {
                num_threads = omp_get_num_threads();
                bin_private = VVI(num_threads, VI(tn, 0));
            }

            int cur_p = omp_get_thread_num();
#pragma omp for
            for (int v = 0; v < n; ++v)
            { // O(V) create d
                bin_private[cur_p][tid_[v]]++;
            }

#pragma omp for
            for (int k = 0; k < tn; k++)
            {
                for (int j = 1; j < num_threads; j++)
                {
                    bin_private[j][k] += bin_private[j - 1][k];
                }
                t_[k]->vertex_.resize(bin_private[num_threads - 1][k]);
            }
#pragma omp for
            for (int v = 0; v < n; ++v)
            { // O(V) create p and D
                int tid = tid_[v];
                int pos = my_fetch_add(&bin_private[cur_p][tid], -1) - 1;
                t_[tid]->vertex_[pos] = v;
            }

            std::fill(bin_private[cur_p].begin(), bin_private[cur_p].end(), 0);
#pragma omp for
            for (int k = 0; k < tn; k++)
            {
                int pa = t_[k]->pa;
                if (pa != -1)
                    bin_private[cur_p][pa]++;
            }
#pragma omp for
            for (int k = 0; k < tn; k++)
            {
                for (int j = 1; j < num_threads; j++)
                {
                    bin_private[j][k] += bin_private[j - 1][k];
                }
                t_[k]->child_.resize(bin_private[num_threads - 1][k]);
            }
#pragma omp for
            for (int k = 0; k < tn; ++k)
            { // O(V) create p and D
                int pa = t_[k]->pa;
                if (pa != -1)
                {
                    int pos = my_fetch_add(&bin_private[cur_p][pa], -1) - 1;
                    t_[pa]->child_[pos] = k;
                }
            }
        }
    }

    std::vector<CoreTreeNode *> t_;
    Graph &g;
    VI tid_;
};

