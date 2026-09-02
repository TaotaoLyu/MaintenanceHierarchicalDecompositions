#include"ParaGraph.hpp"
#include<omp.h>
#include"GraphUtils.h"


ParaGraph::ParaGraph(Graph * g, int * core) : g(g), core(core)
{
	__build_index();
}

void ParaGraph::__build_index()
{
	int n = g->GetNodeNum();
	int mx = 0;
#pragma omp parallel for reduction(max : mx)
	for (int i = 0; i < n; i++) {
		mx = std::max(mx, core[i]);
	}
	max_core = mx;

	seq_.resize(n);
	bin_.resize(max_core + 2);
	rank_.resize(n);
	ceq.resize(n);
	cgt.resize(n);
	int num_threads;
	VVI bin_private;

#pragma omp parallel
	{
#pragma omp single
		{
			num_threads = omp_get_num_threads();
			bin_private = VVI(num_threads, VI(max_core + 1, 0));
		}
		int cur_p = omp_get_thread_num();
		int start = (LL)n * cur_p / num_threads;
		int end = (LL)n * (cur_p + 1) / num_threads;
		end = std::min(end, n);

		for (int v = start; v < end; ++v) {	//O(V) create d
			bin_private[cur_p][core[v]]++;
		}
		for (int k = 0; k <= max_core; k++) {
			my_fetch_add(&bin_[k + 1], bin_private[cur_p][k]);
		}
#pragma omp barrier

#pragma omp single
		{
			for (int k = 1; k <= max_core + 1; ++k) {	//O(MAX_DEG) create b
				bin_[k] += bin_[k - 1];
			}
		}
#pragma omp for
		for (int k = 0; k <= max_core; k++) {
			bin_private[0][k] += bin_[k];
			for (int j = 1; j < num_threads; j++) {
				bin_private[j][k] += bin_private[j - 1][k];
			}
		}
		for (int v = end - 1; v >= start; --v) {	//O(V) create p and D
			int pos = --bin_private[cur_p][core[v]];
			seq_[pos] = v;
			rank_[v] = pos;
		}
#pragma omp for
		for (int v = 0; v < n; v++) {
			int x = 0, y = 0;
			for (int u : (*g)[v]) {
				x += core[u] > core[v];
				y += core[u] == core[v];
			}
			cgt[v] = x, ceq[v] = y;
		}

	}
}

bool ParaGraph::rank_cmp(int x, int y)
{
	return rank_[x] < rank_[y];
}

int ParaGraph::get_coreness(int x)
{
	return core[x];
}