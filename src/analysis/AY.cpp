
#include"AY.hpp"
std::vector<CompRecord> __compute_metric_para(ParaCoreTree & pt, score_f f)
{
    std::vector<int> core(pt.g.GetNodeNum());
	for(auto e:pt.t_)
	{
		for(auto v:e->vertex_)
			core[v]=e->k;
	}
    ParaGraph pg(&pt.g,core.data());
	int kmax = pg.max_core;
	int tn = pt.t_.size();
	VLL in(tn, 0), out(tn, 0);
	VI num(tn, 0);
	int n = pg.g->GetNodeNum();

#pragma omp parallel
	{
		VLL in_private(tn, 0), out_private(tn, 0);
		VI num_private(tn, 0);
#pragma omp for nowait
		for (int i = 0; i < n; i++) {
			int tid = pt.tid_[i];
			num_private[tid]++;
			in_private[tid] += 2LL * pg.cgt[i] + pg.ceq[i];
			out_private[tid] += pg.g->GetDeg()[i] - 2LL * pg.cgt[i] - pg.ceq[i];
		}
		for (int i = 0; i < tn; i++) {
			my_fetch_add(&in[i], in_private[i]);
			my_fetch_add(&out[i], out_private[i]);
			my_fetch_add(&num[i], num_private[i]);
		}
	}

	std::vector<CompRecord> ans;
	for (int i = 0; i < tn; i++) {
		int fa_id = pt.t_[i]->pa;
		if (fa_id != -1) {
			num[fa_id] += num[i];
			in[fa_id] += in[i];
			out[fa_id] += out[i];
		}
		CompRecord record;
		record.score = f(pg.g->GetNodeNum(), pg.g->GetEdgeNum(), in[i], out[i], num[i]);
		record.k = pt.t_[i]->k;
		record.v_num=num[i];
		ans.push_back(record);
	}
	return ans;
}


std::vector<CompRecord> __compute_metric_para(TrineTree::ParaTrineTree & pt, score_f f)
{
    std::vector<int> trine(pt.g_.GetNodeNum());
	for(auto e:pt.t_)
	{
		for(auto v:e->vertex_)
			trine[v]=e->k;
	}
    ParaGraph pg(&pt.g_,trine.data());
	int kmax = pg.max_core;
	int tn = pt.t_.size();
	VLL in(tn, 0), out(tn, 0);
	VI num(tn, 0);
	int n = pg.g->GetNodeNum();

#pragma omp parallel
	{
		VLL in_private(tn, 0), out_private(tn, 0);
		VI num_private(tn, 0);
#pragma omp for nowait
		for (int i = 0; i < n; i++) {
			int tid = pt.tid_[i];
			num_private[tid]++;
			// in_private[tid] += pg.cgt[i] + pg.ceq[i]/2;
			in_private[tid] += 2LL * pg.cgt[i] + pg.ceq[i];
			out_private[tid] += pg.g->GetDeg()[i] - 2LL * pg.cgt[i] - pg.ceq[i];
		}
		for (int i = 0; i < tn; i++) {
			my_fetch_add(&in[i], in_private[i]);
			my_fetch_add(&out[i], out_private[i]);
			my_fetch_add(&num[i], num_private[i]);
		}
	}

	std::vector<CompRecord> ans;
	for (int i = 0; i < tn; i++) {
		int fa_id = pt.t_[i]->pa;
		if (fa_id != -1) {
			num[fa_id] += num[i];
			in[fa_id] += in[i];
			out[fa_id] += out[i];
		}
		CompRecord record;
		record.score = f(pg.g->GetNodeNum(), pg.g->GetEdgeNum(), in[i], out[i], num[i]);
		record.k = pt.t_[i]->k;
		record.v_num=num[i];

		ans.push_back(record);
		// printf("tid: %d k: %d in: %lld out: %lld num: %d\n",i,record.k,in[i],out[i],num[i]);
	}

	return ans;
}


std::vector<CompRecord> get_cc_para(ParaCoreTree & pt)
{
    std::vector<int> core(pt.g.GetNodeNum());
	for(auto e:pt.t_)
	{
		for(auto v:e->vertex_)
			core[v]=e->k;
	}
    ParaGraph pg(&pt.g,core.data());
	int n = pg.g->GetNodeNum();
	int tn = pt.t_.size();
	int kmax = pg.max_core;
	VLL triangle(tn, 0), triplet(tn, 0);

#pragma omp parallel
	{
		bool *tab = new bool[n];
		std::memset(tab, 0, n);
		VI cnt(kmax + 1, 0);
		VI last(kmax + 1, 0);
		VLL triangle_private(tn, 0), triplet_private(tn, 0);

#pragma omp for schedule(dynamic) nowait
		for (int v = 0; v < n; v++) {
			int tid = pt.tid_[v];
			int k = pg.get_coreness(v);
			int dv = pg.g->GetDeg()[v];

			ll valid_nbr = pg.ceq[v] + pg.cgt[v];
			triplet_private[tid] += ll(valid_nbr - 1) * valid_nbr / 2;

			std::vector<uint32_t> &nbr = pg.g->g_[v];
			std::fill(cnt.begin(), cnt.begin() + k, 0);
			for (int u : nbr) {
				int cu = pg.core[u];
				cnt[cu]++;
				last[cu] = pt.tid_[u];
			}

			ll num_gt_k = valid_nbr;
			for (int kk = k - 1; kk >= 0; kk--) {
				ll num_eq_k = cnt[kk];
				ll add = (ll)num_eq_k * num_gt_k + (ll)num_eq_k * (num_eq_k - 1) / 2;
				if (add > 0) {
					triplet_private[last[kk]] += add;
				}
				num_gt_k += num_eq_k;
			}

			for (int u : nbr) tab[u] = true;
			for (int u : nbr) {
				int du = pg.g->GetDeg()[u];
				if (!(du < dv || (du == dv && u < v))) continue;
				for (int w : (*pg.g)[u]) {
					if (tab[w] && pg.rank_cmp(w, u) && pg.rank_cmp(w, v)) {
						triangle_private[pt.tid_[w]]++;
					}
				}
			}
			for (int u : nbr) tab[u] = false;
		}
		for (int i = 0; i < tn; i++) {
			my_fetch_add(&triangle[i], triangle_private[i]);
			my_fetch_add(&triplet[i], triplet_private[i]);
		}
		delete[] tab;
	}

	std::vector<CompRecord> ans;
	for (int i = 0; i < tn; i++) {
		int fa_id = pt.t_[i]->pa;
		if (fa_id != -1) {
			triangle[fa_id] += triangle[i];
			triplet[fa_id] += triplet[i];
		}
		CompRecord record;
		record.score = triangle[i] * 3. / triplet[i];
		record.k = pt.t_[i]->k;
		ans.push_back(record);
	}
	return ans;
}


std::vector<CompRecord> get_cc_para(TrineTree::ParaTrineTree & pt)
{
    std::vector<int> trine(pt.g_.GetNodeNum());
	for(auto e:pt.t_)
	{
		for(auto v:e->vertex_)
			trine[v]=e->k;
	}
    ParaGraph pg(&pt.g_,trine.data());
	int n = pg.g->GetNodeNum();
	int tn = pt.t_.size();
	int kmax = pg.max_core;
	VLL triangle(tn, 0), triplet(tn, 0);

#pragma omp parallel
	{
		bool *tab = new bool[n];
		std::memset(tab, 0, n);
		VI cnt(kmax + 1, 0);
		VI last(kmax + 1, 0);
		VLL triangle_private(tn, 0), triplet_private(tn, 0);

#pragma omp for schedule(dynamic) nowait
		for (int v = 0; v < n; v++) {
			int tid = pt.tid_[v];
			int k = pg.get_coreness(v);
			int dv = pg.g->GetDeg()[v];

			ll valid_nbr = pg.ceq[v] + pg.cgt[v];
			triplet_private[tid] += ll(valid_nbr - 1) * valid_nbr / 2;

			std::vector<uint32_t> &nbr = pg.g->g_[v];
			std::fill(cnt.begin(), cnt.begin() + k, 0);
			for (int u : nbr) {
				int cu = pg.core[u];
				cnt[cu]++;
				last[cu] = pt.tid_[u];
			}

			ll num_gt_k = valid_nbr;
			for (int kk = k - 1; kk >= 0; kk--) {
				ll num_eq_k = cnt[kk];
				ll add = (ll)num_eq_k * num_gt_k + (ll)num_eq_k * (num_eq_k - 1) / 2;
				if (add > 0) {
					triplet_private[last[kk]] += add;
				}
				num_gt_k += num_eq_k;
			}

			for (int u : nbr) tab[u] = true;
			for (int u : nbr) {
				int du = pg.g->GetDeg()[u];
				if (!(du < dv || (du == dv && u < v))) continue;
				for (int w : (*pg.g)[u]) {
					if (tab[w] && pg.rank_cmp(w, u) && pg.rank_cmp(w, v)) {
						triangle_private[pt.tid_[w]]++;
					}
				}
			}
			for (int u : nbr) tab[u] = false;
		}
		for (int i = 0; i < tn; i++) {
			my_fetch_add(&triangle[i], triangle_private[i]);
			my_fetch_add(&triplet[i], triplet_private[i]);
		}
		delete[] tab;
	}

	std::vector<CompRecord> ans;
	for (int i = 0; i < tn; i++) {
		int fa_id = pt.t_[i]->pa;
		if (fa_id != -1) {
			triangle[fa_id] += triangle[i];
			triplet[fa_id] += triplet[i];
		}
		CompRecord record;
		record.score = triangle[i] * 3. / triplet[i];
		record.k = pt.t_[i]->k;
		ans.push_back(record);
	}
	return ans;
}