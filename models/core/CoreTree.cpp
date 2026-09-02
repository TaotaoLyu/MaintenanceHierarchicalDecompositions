#include"CoreTree.hpp"


int free_graph(PKCGraph *g)
{
    if (g->num_edges != NULL)
        free(g->num_edges), g->num_edges = NULL;

    if (g->adj != NULL)
        free(g->adj), g->adj = NULL;

    return 0;
}


void PKC(Graph *g, int *deg)
{
    int NUM_THREADS = 1;
#pragma omp parallel
    {
#pragma omp master
        NUM_THREADS = omp_get_num_threads();
    }

    long n = g->GetNodeNum();
    long reduceN = 0;
    long Size = 0;
    long visited = 0;

    int *newDeg = NULL;
    unsigned int *mapIndexToVtx = NULL;
    vid_t *vertexToIndex = (vid_t *)malloc(n * sizeof(vid_t));
    unsigned int *cumNumEdges = new unsigned int[NUM_THREADS];
    unsigned int part = 0;
    PKCGraph g_small;
    g_small.num_edges = g_small.adj = NULL;

#pragma omp parallel
    {
        int level = 0;
        int tid = omp_get_thread_num();

        vid_t *buff = (vid_t *)malloc((n * sizeof(vid_t)) / NUM_THREADS * 1.5);
        assert(buff != NULL);

        long start = 0, end = 0;
        int useSmallQ = 0;

#pragma omp for schedule(static)
        for (long i = 0; i < n; i++)
        {
            deg[i] = g->GetDeg()[i];
        }

        while (visited < n)
        {

            if ((useSmallQ == 0) && (visited >= (long)(n * frac)))
            {

                useSmallQ = 1;
                if (tid == 0)
                {
                    reduceN = n - visited;
                    newDeg = (int *)malloc(reduceN * sizeof(int));
                    mapIndexToVtx = (unsigned int *)malloc(reduceN * sizeof(unsigned int));
                    g_small.n = reduceN;
                    g_small.num_edges = (vid_t *)malloc((reduceN + 1) * sizeof(vid_t));
                    g_small.num_edges[0] = 0;

                    part = reduceN / NUM_THREADS;
                }
#pragma omp barrier

#pragma omp for schedule(static)
                for (long i = 0; i < n; i++)
                {
                    if (deg[i] >= level)
                    {
                        buff[end] = i;
                        end++;
                    }
                }

                // Now add them atomically
                long begin = my_fetch_add(&Size, end);

                for (long i = 0; i < end; i++)
                {
                    newDeg[begin + i] = deg[buff[i]];
                    mapIndexToVtx[begin + i] = buff[i];
                    vertexToIndex[buff[i]] = begin + i;
                }

                end = 0;
#pragma omp barrier

                // Make a graph with reduceN vertices
                unsigned int edgeCount = 0;

                if (tid == NUM_THREADS - 1)
                {
                    for (long i = tid * part; i < Size; i++)
                    {
                        unsigned int v = mapIndexToVtx[i];
                        unsigned int prevEdgeCount = edgeCount;
                        for (int u : (*g)[v])
                        {
                            if (deg[u] >= level)
                            {
                                edgeCount++;
                            }
                        }
                        g_small.num_edges[i] = prevEdgeCount;
                    }
                    cumNumEdges[tid] = edgeCount;
                }
                else
                {

                    for (long i = tid * part; i < (tid + 1) * part; i++)
                    {
                        unsigned int v = mapIndexToVtx[i];
                        unsigned int prevEdgeCount = edgeCount;
                        for (int u : (*g)[v])
                        {
                            if (deg[u] >= level)
                            {
                                edgeCount++;
                            }
                        }
                        g_small.num_edges[i] = prevEdgeCount;
                    }
                    cumNumEdges[tid] = edgeCount;
                }

#pragma omp barrier
                if (tid == 0)
                {
                    long start = cumNumEdges[0];
                    for (int i = 1; i < NUM_THREADS; i++)
                    {
                        unsigned int prevEdgeCount = start;
                        start = start + cumNumEdges[i];
                        cumNumEdges[i] = prevEdgeCount;
                    }
                    g_small.m = start;
                    g_small.num_edges[Size] = start;

                    g_small.adj = (eid_t *)malloc(g_small.m * sizeof(eid_t));
                    cumNumEdges[0] = 0;
                }
#pragma omp barrier
                if (tid == NUM_THREADS - 1)
                {

                    for (long i = tid * part; i < Size; i++)
                    {
                        g_small.num_edges[i] = g_small.num_edges[i] + cumNumEdges[tid];

                        unsigned int v = mapIndexToVtx[i];
                        for (int u : (*g)[v])
                        {
                            if (deg[u] >= level)
                            {
                                g_small.adj[g_small.num_edges[i]] = vertexToIndex[u];
                                g_small.num_edges[i]++;
                            }
                        }
                    }
                }
                else
                {
                    for (long i = tid * part; i < (tid + 1) * part; i++)
                    {
                        g_small.num_edges[i] = g_small.num_edges[i] + cumNumEdges[tid];

                        unsigned int v = mapIndexToVtx[i];
                        for (int u : (*g)[v])
                        {
                            if (deg[u] >= level)
                            {
                                g_small.adj[g_small.num_edges[i]] = vertexToIndex[u];
                                g_small.num_edges[i]++;
                            }
                        }
                    }
                }
#pragma omp barrier
                // Now fix num_edges array
                if (tid == NUM_THREADS - 1)
                {

                    for (long i = Size - 1; i >= tid * part + 1; i--)
                    {
                        g_small.num_edges[i] = g_small.num_edges[i - 1];
                    }
                    g_small.num_edges[tid * part] = cumNumEdges[tid];
                }
                else
                {

                    for (long i = (tid + 1) * part - 1; i >= tid * part + 1; i--)
                    {
                        g_small.num_edges[i] = g_small.num_edges[i - 1];
                    }
                    g_small.num_edges[tid * part] = cumNumEdges[tid];
                }
#pragma omp barrier
            }

            if (useSmallQ == 0)
            {

#pragma omp for schedule(static)
                for (long i = 0; i < n; i++)
                {
                    if (deg[i] == level)
                    {
                        buff[end] = i;
                        end++;
                    }
                }

                // Get work from curr queue and also add work after the current size
                while (start < end)
                {

                    vid_t v = buff[start];
                    start++;

                    for (int u : (*g)[v])
                    {
                        int deg_u = deg[u];

                        if (deg_u > level)
                        {
                            int du = my_fetch_add(&deg[u], -1);

                            if (du == (level + 1))
                            {
                                buff[end] = u;
                                end++;
                            }

                            if (du <= level)
                            {
                                my_fetch_add(&deg[u], 1);
                            }
                        } // deg_u > level

                    } // visit adjacencies
                } // end of while loop
            }
            else
            {

#pragma omp for schedule(static)
                for (long i = 0; i < Size; i++)
                {
                    if (newDeg[i] == level)
                    {
                        buff[end] = i;
                        end++;
                    }
                }

                // Get work from curr queue and also add work after the current size
                while (start < end)
                {

                    vid_t v = buff[start];
                    start++;

                    for (eid_t j = g_small.num_edges[v]; j < g_small.num_edges[v + 1]; j++)
                    {
                        vid_t u = g_small.adj[j];

                        int deg_u = newDeg[u];

                        if (deg_u > level)
                        {
                            int du = my_fetch_add(&newDeg[u], -1);

                            if (du == (level + 1))
                            {
                                buff[end] = u;
                                end++;
                            }

                            if (du <= level)
                            {
                                my_fetch_add(&newDeg[u], 1);
                            }
                        } // deg_u > level
                    } // visit adjacencies
                } // end of while loop
            }

            my_fetch_add(&visited, end);

#pragma omp barrier
            start = 0;
            end = 0;
            level = level + 1;

        } // end of #visited < n

        // copy core values from newDeg to deg
#pragma omp for schedule(static)
        for (long i = 0; i < Size; i++)
        {
            deg[mapIndexToVtx[i]] = newDeg[i];
        }
        free(buff);
    } // #end of parallel region

    free(newDeg);
    free(mapIndexToVtx);
    free(vertexToIndex);
    free_graph(&g_small);
    delete[] cumNumEdges;
}