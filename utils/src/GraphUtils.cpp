#include "GraphUtils.h"

int WaitFreeRankDSU::find(int x)
{
	int v = pa[x], w = pa[v];
	while (v != w)
	{
#if defined(__linux__)
#pragma omp atomic write
#else
#pragma omp critical
		{
#endif
			pa[x] = w;
			v = pa[x], w = pa[v];
#ifndef __linux__
		}
#endif
#if defined(__linux__)
#pragma omp atomic write
#else
#pragma omp critical
		{
#endif
			pa[x] = w;
			x = v, v = pa[x], w = pa[v];
#ifndef __linux__
		}
#endif
	}
	return v;
}

void WaitFreeRankDSU::init(int n)
{
	n_ = n;
	pa.resize(n);
#pragma omp parallel for
	for (int i = 0; i < n; i++)
	{
		pa[i] = i;
	}
}

void WaitFreeRankDSU::joint(int u, int v)
{
	int t;
	while (true)
	{
		u = find(u), v = find(v);
		if (u == v)
			return;
		if (rank[u] > rank[v])
			std::swap(u, v);
		// if (my_compare_swap(&pa[v], v, u) == v) return;
#if defined(__linux__)
#pragma omp atomic capture
#else
#pragma omp critical
#endif
		{
			t = pa[v];
			pa[v] = u;
		}
		if (t != v)
		{
			v = t;
		}
		else
		{
			return;
		}
	}
}




