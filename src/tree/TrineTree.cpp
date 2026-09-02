#include "TrineTree.hpp"
#include "AlgoProfile.hpp"
#include "TreeBuildIndex.hpp"
#include <queue>

namespace TrineTree
{
    int thread_num = 32;

    double get_time_s(hclock::time_point s, hclock::time_point t)
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(t - s).count();
    }

    void ParaTrineTree::SetThreadNum(int num)
    {
        if (num <= 0)
            return;
        thread_num = num;
    }

    void ParaTrineTree::Build()
    {
        omp_set_num_threads(thread_num);

        TreeBuildIndex pg(g_, trine_.GetTriness().data());

        int n = pg.g.GetNodeNum(); // modify
        tid_ = std::vector<uint32_t>(n, -1);
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
                    for (int v : pg.g[u]) // modify
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
                    TrineTreeNode *newv = new TrineTreeNode;
                    newv->k = k;
                    newv->pa = -1;
#pragma omp critical
                    {
                        tid_[v] = (uint32_t)t_.size();
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

        int tn = (int)t_.size();
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
    void ParaTrineTree::PrintTree()
    {
        printf("=============================================\n");
        for (int i = 0; i < t_.size(); ++i)
        {
            printf("id: %d\n", i);
            printf("k: %d\n", t_[i]->k);
            printf("parent: %d\n", t_[i]->pa);
            printf("vertex:\n");
            for (auto e : t_[i]->vertex_)
                printf("%d ", e);
            printf("\n");

            printf("child:\n");
            for (auto e : t_[i]->child_)
                printf("%d ", e);
            printf("\n\n");
        }
        printf("=============================================\n");
    }
    // void ParaTrineTree::dfs(std::vector<int> &Jump, int t, int v)
    // {
    //     if (t < Jump.size())
    //     {
    //         printf("dfs error\n");
    //     }
    //     Jump[t] = v;
    //     for (auto e : t_[t]->child_)
    //         dfs(Jump, e, v);
    // }

    void ParaTrineTree::MoveToNewTid(std::vector<int> &changever, std::vector<int> &old_tid, std::vector<int> &old_trine)
    {
        std::vector<std::vector<bool>> need_erase_vertex(thread_num, std::vector<bool>(t_.size()));

        // printf("p1\n");
        std::vector<TrineTreeNode *> copy_t_;
        copy_t_.resize(t_.size());
#pragma omp parallel for
        for (int i = 0; i < t_.size(); ++i)
            copy_t_[i] = t_[i];

#pragma omp parallel for
        for (int i = 0; i < tid_.size(); ++i)
        {
            // printf("for start\n");
            if (trine_[i] != copy_t_[tid_[i]]->k)
            {
                int cur_p = omp_get_thread_num();

                int old_t = tid_[i];
                int old_k = 0;
                old_k = copy_t_[tid_[i]]->k;
                need_erase_vertex[cur_p][tid_[i]] = true;

                // printf("hello\n");
                auto p = new TrineTreeNode;
                p->vertex_.push_back(i);
                p->k = trine_[i];
                p->pa = -1;
#pragma omp critical(add_t)
                {
                    tid_[i] = t_.size();
                    t_.push_back(p);
                    changever.push_back(i);
                    old_tid.push_back(old_t);
                    old_trine.push_back(old_k);
                }
            }
        }

        std::vector<int> emptyNode;
#pragma omp parallel for
        for (int i = 0; i < need_erase_vertex[0].size(); ++i)
        {
            for (int j = 0; j < thread_num; ++j)
            {
                if (need_erase_vertex[j][i])
                {
                    std::vector<int> new_vt;
                    for (auto e : t_[i]->vertex_)
                    {
                        if (trine_[e] == t_[i]->k)
                            new_vt.push_back(e);
                    }
                    new_vt.swap(t_[i]->vertex_);
                    if (t_[i]->vertex_.size() == 0)
                    {
#pragma omp critical
                        {
                            emptyNode.push_back(i);
                        }
                    }
                    break;
                }
            }
        }

        // process emptynode
        for (int i = 0; i < emptyNode.size(); ++i)
        {
            int tid = emptyNode[i];
            int ptid = t_[tid]->pa;
            if (ptid != -1)
            {
                t_[ptid]->child_.insert(t_[ptid]->child_.begin(), t_[tid]->child_.begin(), t_[tid]->child_.end());
            }
            for (auto e : t_[tid]->child_)
                t_[e]->pa = ptid;

            t_[tid]->child_.clear();
        }
    }

    void ParaTrineTree::AddChildren()
    {
        int tn = (int)t_.size();
        VVI bin_private = VVI(thread_num, VI(tn, 0));
#pragma omp parallel
        {
            int cur_p = omp_get_thread_num();

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
                for (int j = 1; j < thread_num; j++)
                {
                    bin_private[j][k] += bin_private[j - 1][k];
                }
                t_[k]->child_.resize(bin_private[thread_num - 1][k]);
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

    void ParaTrineTree::Merge(TrineTreeNode *dst, TrineTreeNode *src)
    {
        // for (auto e : src->child_)
        // 	dst->child_.push_back(e);

        dst->vertex_.resize(dst->vertex_.size() + src->vertex_.size());
        int vsize = dst->vertex_.size();
#pragma omp parallel for
        for (int i = 0; i < src->vertex_.size(); ++i)
        {
            int v = src->vertex_[i];
            int pos = my_fetch_add(&vsize, -1) - 1;
            dst->vertex_[pos] = v;
        }
#pragma omp parallel for
        for (int i = 0; i < src->vertex_.size(); ++i)
        {
            int v = src->vertex_[i];
            tid_[v] = dst->tid;
        }

        src->child_.clear();
        src->vertex_.clear();
    }
    void ParaTrineTree::InsertEdge(const EdgListT &edg, bool flag,bool flag1)
    {
        omp_set_num_threads(thread_num);

        AlgoProfile::GetInstance()->SetStatus(AlgoProfile::INSERT);
        AlgoProfile::GetInstance()->SetInsertEdgeNum((int)edg.size());
        AlgoProfile::GetInstance()->SetOldMaxTriness(trine_.GetMaxTrine());
        auto s1 = AlgoProfile::CaptureClock();
        trine_.Insert(edg);
        auto s2 = AlgoProfile::CaptureClock();
        AlgoProfile::GetInstance()->SetUpdateTrinessTime(s1, s2);
        AlgoProfile::GetInstance()->SetNewMaxTriness(trine_.GetMaxTrine());

        s1 = AlgoProfile::CaptureClock();

        // int thread_num = 32;
        std::vector<int> changever;

        std::vector<int> old_tid;
        std::vector<int> old_trine;
        MoveToNewTid(changever, old_tid, old_trine);

#pragma omp parallel for
        for (int i = 0; i < t_.size(); ++i)
        {
            if (t_[i]->pa != -1)
            {
                t_[i]->pa = t_[t_[i]->pa]->vertex_.front();
            }
        }
        std::vector<std::vector<std::pair<int, int>>> merged_bucket(trine_.GetMaxTrine() + 1);

        std::vector<std::vector<std::vector<std::pair<int, int>>>> bucket(thread_num, std::vector<std::vector<std::pair<int, int>>>(trine_.GetMaxTrine() + 1));

#pragma omp parallel for
        for (int i = 0; i < changever.size(); ++i)
        {
            int cur_p = omp_get_thread_num();
            int u = changever[i];
            int u_tid = tid_[u];
            for (auto v : g_[u])
            {
                int v_tid = tid_[v];
                if(flag) // modify
                {
                    if (t_[v_tid]->k > old_trine[i])
                        bucket[cur_p][std::max(t_[u_tid]->k, t_[v_tid]->k)].push_back(std::make_pair(u_tid, v_tid));
                }
                else
                {
                    bucket[cur_p][std::max(t_[u_tid]->k, t_[v_tid]->k)].push_back(std::make_pair(u_tid, v_tid));
                }
            }
            int old_t = old_tid[i];
            if (t_[old_t]->vertex_.size() == 0)
            {
                old_t = t_[old_t]->pa;
                if (old_t != -1)
                    old_t = tid_[old_t];
            }

            if (old_t != -1)
                bucket[cur_p][std::max(t_[u_tid]->k, t_[old_t]->k)].push_back(std::make_pair(u_tid, old_t));
        }

#pragma omp parallel for
        for (int i = 0; i < edg.size(); ++i)
        {
            int cur_p = omp_get_thread_num();
            int u = edg[i].first;
            int v = edg[i].second;
            int u_tid = tid_[u];
            int v_tid = tid_[v];
            bucket[cur_p][std::max(t_[u_tid]->k, t_[v_tid]->k)].push_back(std::make_pair(u_tid, v_tid));
        }

#pragma omp parallel for
        for (int i = 0; i <= trine_.GetMaxTrine(); ++i)
        {
            std::vector<std::pair<int, int>> k_vedge;
            std::unordered_set<uint64_t> global_seen;
            int total_size = 0;
            for (int j = 0; j < thread_num; ++j)
            {
                total_size += bucket[j][i].size();
            }
            if (total_size == 0)
                continue;
            k_vedge.reserve(total_size);
            for (int j = 0; j < thread_num; ++j)
            {
                for (const auto &e : bucket[j][i])
                {
                    int u = std::min(e.first, e.second);
                    int v = std::max(e.first, e.second);
                    uint64_t key = (uint64_t)u << 32 | v;

                    if (global_seen.count(key) == 0)
                    {
                        k_vedge.push_back(e);
                        global_seen.insert(key);
                    }
                }
            }
            merged_bucket[i].swap(k_vedge);
        }

        TreeBuildIndex pg(g_, trine_.GetTriness().data());

        WaitFreeRankDSU dsu_(g_.GetNodeNum(), pg.rank_);
        std::vector<int> visted(t_.size());
        std::vector<std::vector<int>> merge_node(t_.size());
        std::vector<std::vector<int>> add_edge(t_.size());

        std::vector<std::mutex> mt_node(t_.size());
        // std::vector<std::mutex> mt_k(trine_.GetMaxTrine() + 1);
        int k = trine_.GetMaxTrine();
        // printf("start k\n");

        // {
        //     int cc=0;
        //     for(auto e:merged_bucket)
        //     {
        //         for(auto pq:e)
        //         {
        //             int n1=pq.first;
        //             int n2=pq.second;
        //             if(t_[n1]->k<t_[n2]->k)
        //                 std::swap(n1,n2);

        //             // assert(n1 >= 0 && n1 < t_.size());
        //             // assert(n2 >= 0 && n2 < t_.size());
        //             while (t_[n1]->k>t_[n2]->k)
        //             {
        //                 cc++;
        //                 n1=tid_[t_[n1]->pa];
        //                 if(n1==-1)
        //                     break;
        //             }
        //         }
        //     }

        //     printf("=====================\n");
        //     printf("cc= %d\n",cc);
        //     printf("=====================\n");
        // }

        // printf("=====================\n");





        while (k >= 0)
        {
            std::vector<int> need_work;
            std::vector<std::pair<int, int>> &k_vedge = merged_bucket[k];
            if (k_vedge.size() == 0)
            {
                k--;
                continue;
            }
            // printf("p1\n");

            // printf("p1\n");
// #pragma omp parallel for
            for (int i = 0; i < k_vedge.size(); ++i)
            {
                int u_tid = k_vedge[i].first;
                int v_tid = k_vedge[i].second;
                int u_k = t_[u_tid]->k;
                int v_k = t_[v_tid]->k;
                if (u_k == v_k) // merge
                {
                    dsu_.joint(t_[u_tid]->vertex_.front(), t_[v_tid]->vertex_.front());
                }
                if (u_k != v_k)
                {
                    if (u_k < v_k)
                        std::swap(u_tid, v_tid);
                    // std::lock_guard<std::mutex> lock(mt_node[u_tid]);
                    add_edge[u_tid].push_back(v_tid);
                }
            }
            // printf("p2\n");

// #pragma omp parallel for
            for (int i = 0; i < k_vedge.size(); ++i)
            {
                int u_tid = k_vedge[i].first;
                int v_tid = k_vedge[i].second;
                if (t_[u_tid]->k < t_[v_tid]->k)
                    std::swap(u_tid, v_tid);
                // uid
                if (dsu_.get_pivot(t_[u_tid]->vertex_.front()) == t_[u_tid]->vertex_.front() && my_fetch_add(&visted[u_tid], 1) == 0)
                {
// #pragma omp critical(need_work)
                    {
                        need_work.push_back(u_tid);
                    }
                }
                if (dsu_.get_pivot(t_[u_tid]->vertex_.front()) != t_[u_tid]->vertex_.front() && my_fetch_add(&visted[u_tid], 1) == 0)
                {
                    int to_tid = tid_[dsu_.get_pivot(t_[u_tid]->vertex_.front())];
                    // std::lock_guard<std::mutex> lock(mt_node[to_tid]);
                    merge_node[to_tid].push_back(u_tid);
                }

                // vid
                if (t_[v_tid]->k != t_[u_tid]->k)
                    continue;
                if (dsu_.get_pivot(t_[v_tid]->vertex_.front()) == t_[v_tid]->vertex_.front() && my_fetch_add(&visted[v_tid], 1) == 0)
                {
// #pragma omp critical(need_work)
                    {
                        need_work.push_back(v_tid);
                    }
                }
                if (dsu_.get_pivot(t_[v_tid]->vertex_.front()) != t_[v_tid]->vertex_.front() && my_fetch_add(&visted[v_tid], 1) == 0)
                {
                    int to_tid = tid_[dsu_.get_pivot(t_[v_tid]->vertex_.front())];
                    // std::lock_guard<std::mutex> lock(mt_node[to_tid]);
                    merge_node[to_tid].push_back(v_tid);
                }
            }

            for (int i = 0; i < need_work.size(); ++i)
            {
                int an_tid = need_work[i];
                std::vector<std::vector<int> *> add_edge_ptr;
                add_edge_ptr.push_back(&add_edge[an_tid]);
                if (t_[an_tid]->vertex_.size() == 0)
                    continue;
                // printf("for p1\n");
                if (merge_node[an_tid].size() != 0)
                {
                    // need merge
                    t_[an_tid]->tid = an_tid;

                    for (auto e : merge_node[an_tid])
                    {
                        Merge(t_[an_tid], t_[e]);
                        add_edge_ptr.push_back(&add_edge[e]);
                        if (t_[e]->pa != -1)
                            add_edge[e].push_back(tid_[t_[e]->pa]);
                    }
                }
                // printf("for p2\n");

                if (t_[an_tid]->pa != -1)
                    add_edge[an_tid].push_back(tid_[t_[an_tid]->pa]);
                if (add_edge_ptr.size() != 0)
                {
                    int maxk = -1;
                    int maxid = -1;
                    for (int j = 0; j < add_edge_ptr.size(); ++j)
                    {
                        std::vector<int> &add_ = *(add_edge_ptr[j]);
                        for (auto e : add_)
                        {
                            if (t_[e]->k < k && t_[e]->k > maxk)
                            {
                                maxk = t_[e]->k;
                                maxid = e;
                            }
                        }
                    }

                    if (maxid == -1)
                        t_[an_tid]->pa = -1;
                    else
                        t_[an_tid]->pa = t_[maxid]->vertex_.front();

                    if (maxid == -1)
                        continue;

                    std::unordered_set<int> ust;
                    for (int j = 0; j < add_edge_ptr.size(); ++j)
                    {
                        std::vector<int> &add_ = *(add_edge_ptr[j]);
                        for (auto tid : add_)
                        {
                            {
                                ust.insert(tid);
                            }
                        }
                    }
                    // std::lock_guard<std::mutex> lock(mt_k[maxk]);
                    if (flag1)
                    {



                        for (auto &tid : ust)
                        {
                            merged_bucket[maxk].push_back(std::make_pair(tid, maxid));
                        }



                    }
                    else
                    {
                        std::vector<int> tmp(ust.begin(),ust.end());
                        for(int b=0;b<tmp.size();++b)
                        {
                            for(int d=b+1;d<tmp.size();++d)
                            {
                                int uid=tmp[b];
                                int vid=tmp[d];
                                int mk=std::max(t_[uid]->k,t_[vid]->k);
                                merged_bucket[mk].push_back(std::make_pair(uid, vid));
                            }
                        }

                    }
                }
            }
            // printf("p4\n");
            k--;
        }

#pragma omp parallel for
        for (int i = 0; i < t_.size(); ++i)
        {
            if (t_[i]->pa != -1)
                t_[i]->pa = tid_[t_[i]->pa];
            t_[i]->child_.clear();
        }

        AddChildren();
        CleanEmptyNode();
        s2 = AlgoProfile::CaptureClock();
        AlgoProfile::GetInstance()->SetInsertTime(s1, s2);
    }

    std::pair<int, int> ParaTrineTree::GenrateTask(int v)
    {
        int new_v_trine = trine_[v];
        int tid = tid_[v];
        int old_v_trine = t_[tid]->k;
        if (new_v_trine == old_v_trine)
            return {-1, -1};
        int cur = tid;
        int parent = t_[cur]->pa;
        while (parent != -1 && t_[parent]->k > new_v_trine)
        {
            cur = parent;
            parent = t_[parent]->pa;
        }
        if (parent != -1 && t_[parent]->k == new_v_trine)
            return {-1, -1};
        else
            return {cur, new_v_trine};
    }
    int ParaTrineTree::FindCorrectTid(int v)
    {
        int k = trine_[v];
        int cur = tid_[v];
        while (t_[cur]->k != k)
        {
            // printf("cur: %d k: %d find K: %d\n",cur, t_[cur]->k,k);

            cur = t_[cur]->pa;
        }
        return cur;
    }

    void ParaTrineTree::CreateNewNode(const std::vector<uint32_t> &change_ver)
    {
        // printf("old node num: %d\n",t_.size());
        std::vector<std::set<int>> t_task(t_.size()); // modify
        int create_new_node_size = 0;
#pragma omp parallel for
        for (int i = 0; i < change_ver.size(); ++i)
        {
            int v = change_ver[i];
            auto pr = GenrateTask(v);
            if (pr.first == -1)
                continue;
#pragma omp critical
            {
                if (!t_task[pr.first].count(pr.second))
                {
                    t_task[pr.first].insert(pr.second);
                    create_new_node_size++;
                }
            }
        }
        int node_num = (int)t_.size();
        t_.resize(node_num + create_new_node_size);
        std::vector<std::vector<std::pair<int, int>>> change_child_task(node_num);

        int pnid = (int)t_.size() - 1;
#pragma omp parallel for
        for (int i = node_num; i < t_.size(); ++i)
            t_[i] = new TrineTreeNode;
#pragma omp parallel for
        for (int i = 0; i < t_task.size(); ++i)
        {
            if (t_task[i].size() == 0)
                continue;
            std::vector<int> new_node;
            int parent = -1;
            // std::sort(t_task[i].begin(),t_task[i].end());
            for (auto k : t_task[i])
            {
                int p = my_fetch_add(&pnid, -1);
                t_[p]->k = k;
                t_[p]->pa = parent;
                if (parent != -1)
                    t_[new_node.back()]->child_.push_back(p);
                parent = p;
                new_node.push_back(p);
            }
            int lasttid = new_node.back();
            int firsttid = new_node.front();

            if (t_[i]->pa != -1)
            {
#pragma omp critical
                {
                    change_child_task[t_[i]->pa].emplace_back(i, firsttid);
                }
            }
            t_[firsttid]->pa = t_[i]->pa;
            t_[lasttid]->child_.push_back(i);
            t_[i]->pa = lasttid;
        }

#pragma omp parallel for
        for (int i = 0; i < node_num; ++i)
        {
            if (change_child_task[i].size() == 0)
                continue;
            std::sort(change_child_task[i].begin(), change_child_task[i].end());
            std::sort(t_[i]->child_.begin(), t_[i]->child_.end());
            int p1 = 0;
            for (auto &e : t_[i]->child_)
            {
                if (e == change_child_task[i][p1].first)
                {
                    e = change_child_task[i][p1].second;
                    p1++;
                }
                if (p1 == change_child_task[i].size())
                    break;
            }
        }
    }

    void ParaTrineTree::CreateMoveTask(const std::vector<uint32_t> &change_ver, std::vector<std::vector<std::vector<int>>> &need_add, std::vector<std::vector<bool>> &need_erase)
    {
        // uint32_t thread_task_num = change_ver.size() / thread_num;

#pragma omp parallel for
        for (int i = 0; i < change_ver.size(); ++i)
        {
            int v = change_ver[i];
            need_erase[omp_get_thread_num()][tid_[v]] = true;
            int newtid = FindCorrectTid(v);
            need_add[omp_get_thread_num()][newtid].push_back(v);
        }
    }

    void ParaTrineTree::EraseVervex(const std::vector<std::vector<bool>> &need_erase)
    {
        // erase
#pragma omp parallel for
        for (int i = 0; i < need_erase[0].size(); ++i)
        {
            for (int j = 0; j < thread_num; ++j)
            {
                if (need_erase[j][i])
                {
                    auto &vt = t_[i]->vertex_;
                    std::vector<int> newvt;
                    newvt.reserve(vt.size());
                    for (int h = 0; h < vt.size(); ++h)
                    {
                        if (trine_[vt[h]] == t_[i]->k)
                        {
                            newvt.push_back(vt[h]);
                        }
                    }
                    vt.swap(newvt);
                    break;
                }
            }
        }
    }

    void ParaTrineTree::AddVertex(const std::vector<std::vector<std::vector<int>>> &need_add)
    {
// add
#pragma omp parallel for
        for (int i = 0; i < t_.size(); ++i)
        {

            for (int j = 0; j < thread_num; ++j)
            {
                for (auto newv : need_add[j][i])
                {

                    t_[i]->vertex_.push_back(newv);
                    tid_[newv] = i;
                }
            }
        }
    }

    void ParaTrineTree::RemoveEdge(const EdgListT &edg, bool flag)
    {
        omp_set_num_threads(thread_num);

        AlgoProfile::GetInstance()->SetStatus(AlgoProfile::REMOVE);

        AlgoProfile::GetInstance()->SetRemoveEdgeNum((int)edg.size());
        std::vector<uint32_t> old_trine_ = trine_.GetTriness();

        AlgoProfile::GetInstance()->SetOldMaxTriness(trine_.GetMaxTrine());
        auto s1 = AlgoProfile::CaptureClock();
        trine_.Remove(edg);
        auto s2 = AlgoProfile::CaptureClock();
        AlgoProfile::GetInstance()->SetUpdateTrinessTime(s1, s2);
        AlgoProfile::GetInstance()->SetNewMaxTriness(trine_.GetMaxTrine());

        s1 = AlgoProfile::CaptureClock();

        TreeBuildIndex pg(g_, trine_.GetTriness().data());

        std::vector<uint32_t> change_ver;
        for (int i = 0; i < g_.GetNodeNum(); ++i)
            if (old_trine_[i] != trine_[i])
                change_ver.emplace_back(i);

        uint32_t thread_task_num = change_ver.size() / thread_num;
        std::vector<std::vector<std::vector<int>>> need_split(thread_num, std::vector<std::vector<int>>(trine_.GetMaxTrine() + 1));

        int node_num = (int)t_.size();
        // printf("old num: %d\n", node_num);
        // printf("phase1\n");
        // PrintTree();

        CreateNewNode(change_ver);
        // PrintTree();
        // move changed vertices to correct node
        // printf("phase2\n");

        std::vector<std::vector<bool>> need_erase(thread_num, std::vector<bool>(node_num));
        std::vector<std::vector<std::vector<int>>> need_add(thread_num, std::vector<std::vector<int>>(t_.size()));

        CreateMoveTask(change_ver, need_add, need_erase);

        // printf("phase3\n");

#pragma omp parallel for // modify
        for (int i = 0; i < change_ver.size(); ++i)
        {

            // printf("i: %d\n", i);
            int u = change_ver[i];
            int newk = trine_[u];
            int curid = tid_[u];
            // printf("crr id: %d\n", curid);
            // printf("new_k %d\n", newk);
            while (t_[curid]->k != newk)
            {
                need_split[omp_get_thread_num()][t_[curid]->k].push_back(curid);
                // printf("curid: %d\n", curid);
                curid = t_[curid]->pa;
                // printf("pa: %d\n", curid);
                // printf("curid_k: %d\n", t_[curid]->k);
            }
        }

        // printf("phase4\n");

        EraseVervex(need_erase);

        // printf("phase5\n");

        AddVertex(need_add);

        // printf("phase6\n");

        std::vector<bool> need_transmit(t_.size());
        for (auto e : edg)
        {
            int u = e.first;
            int v = e.second;
            if (trine_[u] > trine_[v])
                std::swap(u, v);
            need_split[omp_get_thread_num()][trine_[u]].push_back(tid_[u]);
            need_transmit[tid_[u]] = true;
        }
        // printf("phase7\n");

        // split node down to up
        int cur_k = trine_.GetMaxTrine();
        std::vector<std::vector<TrineTreeNode *>> push_to_chil(t_.size());
        WaitFreeRankDSU dsu_(g_.GetNodeNum(), pg.rank_);
        VI tag(g_.GetNodeNum(), -1);
        for (int cur_k = trine_.GetMaxTrine(); cur_k >= 0; cur_k--)
        {
            // printf("%d\n", cur_k);
            std::vector<bool> vised_(t_.size());
            std::vector<int> split_tid;
            split_tid.reserve(10);
            for (int i = 0; i < thread_num; ++i)
            {
                for (int v : need_split[i][cur_k])
                {
                    if (vised_[v] == false)
                    {
                        split_tid.push_back(v);
                        vised_[v] = true;
                    }
                }
            }
            if (split_tid.size() == 0)
                continue;
            // printf("%d\n", cur_k);

            std::vector<std::vector<TrineTreeNode *>> push_to_t(thread_num);
            std::vector<int> Jump(t_.size(), -1);
            // split one node

            // printf("for start\n");
            for (int i = 0; i < split_tid.size(); ++i)
            {
                // printf("i++");
                {
                    int local_max = -1;
                    int local_min = INT_MAX;
                    int n = g_.GetNodeNum(); // modify

                    int old_t_num = (int)t_.size();

                    if (t_[split_tid[i]]->vertex_.size() == 0)
                    {

                        // printf("for phase 0\n");
                        if (t_[split_tid[i]]->pa != -1)
                        {
#pragma omp parallel for
                            for (int j = 0; j < t_[split_tid[i]]->child_.size(); ++j)
                            {
                                int ch = t_[split_tid[i]]->child_[j];
                                t_[ch]->pa = t_[split_tid[i]]->pa;
#pragma omp critical
                                {
                                    t_[t_[split_tid[i]]->pa]->child_.push_back(ch);
                                }
                            }
                        }
                        // printf("for phase 0.1\n");
                        t_[split_tid[i]]->child_.clear();
                        int parent = t_[split_tid[i]]->pa;
                        if (parent != -1)
                        {
                            if (need_transmit[split_tid[i]] || !flag)  // modify
                            {

                                need_split[omp_get_thread_num()][t_[parent]->k].push_back(parent);
#pragma omp critical
                                {
                                    need_transmit[parent] = true;
                                }
                            }
                        }
                    }
                    else
                    {
                        // printf("for phase1\n");
                        // init jump

#pragma omp parallel for
                        for (int j = 0; j < t_[split_tid[i]]->child_.size(); ++j)
                        {
                            int chilid = t_[split_tid[i]]->child_[j];
                            if (t_[chilid]->vertex_.size() == 0)
                                continue;
                            int tg = t_[chilid]->vertex_.front();
                            std::queue<int> qe;
                            qe.push(chilid);
                            while (!qe.empty())
                            {
                                int cid = qe.front();
                                qe.pop();
                                Jump[cid] = tg;
                                for (auto e : t_[cid]->child_)
                                    qe.push(e);
                            }
                        }

                        // create new node

#pragma omp parallel
                        {
                            std::vector<int> nbr_max_rk_private;

#pragma omp for
                            for (int j = 0; j < t_[split_tid[i]]->vertex_.size(); ++j)
                            {
                                int u = t_[split_tid[i]]->vertex_[j];
                                for (auto v : g_[u])
                                {

                                    if (trine_[v] > cur_k)
                                    {
                                        int pivot = Jump[tid_[v]];

                                        int t = tag[pivot];
                                        if (t != cur_k && my_compare_swap(&tag[pivot], t, cur_k) == t)
                                        {
                                            nbr_max_rk_private.push_back(pivot);
                                        }
                                        dsu_.joint(u, dsu_.get_pivot(pivot));
                                    }
                                    else if (trine_[v] == cur_k)
                                    {

                                        int pivot = dsu_.get_pivot(v);
                                        dsu_.joint(u, pivot);
                                    }
                                }
                            }

#pragma omp for
                            for (int j = 0; j < t_[split_tid[i]]->vertex_.size(); ++j)
                            {
                                int v = t_[split_tid[i]]->vertex_[j];
                                // printf("v: %d\n", v);
                                if (dsu_.get_pivot(v) != v)
                                    continue;
                                TrineTreeNode *newv = new TrineTreeNode;
                                // printf("new node\n");
                                newv->k = cur_k;
                                newv->vertex_.push_back(v); // warning
                                newv->pa = t_[split_tid[i]]->pa;
                                // printf("hello\n");
#pragma omp critical
                                {
                                    tid_[v] = t_.size();
                                    t_.push_back(newv);
                                }
                            }
                            // printf("for phase4\n");

#pragma omp for
                            for (int j = 0; j < t_[split_tid[i]]->vertex_.size(); ++j)
                            {
                                int v = t_[split_tid[i]]->vertex_[j];
                                int pivot = dsu_.get_pivot(v);
                                tid_[v] = tid_[pivot];
                            }

                            for (int j = 0; j < nbr_max_rk_private.size(); ++j)
                            {
                                int v = nbr_max_rk_private[j];
                                int tid_son = tid_[v];
                                int pivot = dsu_.get_pivot(v);
                                t_[tid_son]->pa = tid_[pivot];
#pragma omp critical
                                {
                                    // if (tid_[pivot] != -1) // BUG
                                    t_[tid_[pivot]]->child_.push_back(tid_son);
                                }
                            }
                        }

                        // hclock::time_point s100 = hclock::now();

                        if (t_[split_tid[i]]->pa != -1)
                        {
                            for (int j = old_t_num; j < t_.size(); ++j)
                            {
#pragma omp critical
                                {
                                    t_[t_[split_tid[i]]->pa]->child_.push_back(j);
                                }
                            }
                        }
                        // printf("for phase6\n");

                        // move vertex to node
                        std::vector<std::vector<int>> bin_private(thread_num, std::vector<int>(t_.size() - old_t_num));
                        // printf("%ld %d\n", t_.size(), old_t_num);
#pragma omp parallel
                        {
                            int cur_p = omp_get_thread_num();
#pragma omp for
                            for (int j = 0; j < t_[split_tid[i]]->vertex_.size(); ++j)
                            {

                                int v = t_[split_tid[i]]->vertex_[j];
                                // printf("v: %d\n", v);
                                bin_private[cur_p][tid_[v] - old_t_num]++;
                            }
                            // printf("for phase7\n");

#pragma omp for
                            for (int k = 0; k < t_.size() - old_t_num; ++k)
                            {
                                for (int j = 1; j < thread_num; ++j)
                                {
                                    bin_private[j][k] += bin_private[j - 1][k];
                                }
                                t_[k + old_t_num]->vertex_.resize(bin_private[thread_num - 1][k]);
                            }
                            // printf("for phase8\n");

#pragma omp for
                            for (int j = 0; j < t_[split_tid[i]]->vertex_.size(); ++j)
                            {
                                int v = t_[split_tid[i]]->vertex_[j];
                                int tid = tid_[v];
                                int pos = my_fetch_add(&bin_private[cur_p][tid - old_t_num], -1) - 1;
                                t_[tid]->vertex_[pos] = v;
                            }
                        }
                        // printf("for phase9\n");

                        // process parent's children

#pragma omp parallel for
                        for (int j = 0; j < t_[split_tid[i]]->child_.size(); ++j)
                        {
                            int ch = t_[split_tid[i]]->child_[j];
                            if (t_[ch]->pa == split_tid[i])
                            {
                                t_[ch]->pa = t_[split_tid[i]]->pa;
                                if (t_[split_tid[i]]->pa != -1)
                                {
#pragma omp critical
                                    {
                                        t_[t_[split_tid[i]]->pa]->child_.push_back(ch);
                                    }
                                }
                            }
                        }
                        // printf("for phase10\n");

                        t_[split_tid[i]]->child_.clear();

                        // check
                        if (need_transmit[split_tid[i]] || !flag)  // modify
                        {
#if defined(__linux__)
#pragma omp parallel for reduction(max : local_max) reduction(min : local_min)
#endif
                            for (int j = 0; j < t_[split_tid[i]]->vertex_.size(); ++j)
                            {
                                int v = t_[split_tid[i]]->vertex_[j];
                                int tid_val = tid_[v];
                                local_max = std::max(local_max, tid_val);
                                local_min = std::min(local_min, tid_val);
                            }
                            if (local_max != local_min || !flag) //modify
                            {
                                int parent = t_[split_tid[i]]->pa;
                                if (parent != -1)
                                {
                                    need_split[omp_get_thread_num()][t_[parent]->k].push_back(parent);
#pragma omp critical
                                    {
                                        need_transmit[parent] = true;
                                    }
                                }
                            }
                            local_min = INT_MAX;
                            local_max = -1;
                        }
                        // printf("for phase11\n");
                        t_[split_tid[i]]->vertex_.clear();
                        // printf("for phase 12\n");
                    }
                }
            }
        }
        // printf("phase8\n");

        // clear empty node
        CleanEmptyNode();
        // printf("phase9\n");
        s2 = AlgoProfile::CaptureClock();

        AlgoProfile::GetInstance()->SetRemoveTime(s1, s2);
    }

    void ParaTrineTree::CleanEmptyNode()
    {
        std::vector<std::vector<bool>> need_clear_empty_child(thread_num, std::vector<bool>(t_.size()));
        int unemptynum = (int)t_.size();
#pragma omp parallel for
        for (int i = 0; i < t_.size(); ++i)
        {
            if (t_[i]->vertex_.size() == 0)
            {
                if (t_[i]->pa != -1)
                    need_clear_empty_child[omp_get_thread_num()][t_[i]->pa] = true;
                else
                {
                    delete t_[i];
                    t_[i] = nullptr;
                }
#pragma omp atomic
                unemptynum--;
            }
        }

#pragma omp parallel for
        for (int i = 0; i < t_.size(); ++i)
        {
            for (int j = 0; j < thread_num; ++j)
            {
                if (need_clear_empty_child[j][i])
                {
                    std::vector<int> newvt;
                    newvt.reserve(t_[i]->child_.size());
                    for (auto ch : t_[i]->child_)
                    {

                        if (t_[ch]->vertex_.size() == 0)
                        {
                            delete t_[ch];
                            t_[ch] = nullptr;
                        }
                        else
                            newvt.push_back(ch);
                    }
                    newvt.swap(t_[i]->child_);
                    break;
                }
            }
        }

        std::vector<int> old_to_new(t_.size());

        std::vector<TrineTreeNode *> newt_(unemptynum);
#pragma omp parallel for
        for (int i = 0; i < t_.size(); ++i)
        {
            if (t_[i] != nullptr)
            {

                old_to_new[i] = my_fetch_add(&unemptynum, -1) - 1;
                newt_[old_to_new[i]] = t_[i];
            }
        }
        newt_.swap(t_);

#pragma omp parallel for
        for (int i = 0; i < tid_.size(); ++i)
        {
            tid_[i] = old_to_new[tid_[i]];
        }

#pragma omp parallel for
        for (int i = 0; i < t_.size(); ++i)
        {
            if (t_[i]->pa != -1)
                t_[i]->pa = old_to_new[t_[i]->pa];
            for (auto &e : t_[i]->child_)
                e = old_to_new[e];
        }
    }

    bool ParaTrineTree::check()
    {

        AlgoProfile::GetInstance()->SetCorrect(false);
        int sum = 0;
        for (int i = 0; i < t_.size(); ++i)
        {
            for (auto e : t_[i]->vertex_)
            {
                if (tid_[e] != i)
                    return false;
                sum++;
            }
        }
        for (int i = 0; i < t_.size(); ++i)
        {
            for (auto e : t_[i]->child_)
            {
                if (t_[e]->pa != i)
                    return false;
            }
        }


        if (sum != tid_.size())
            return false;
        std::vector<TrineTreeNode *> cy_t_;
        std::vector<uint32_t> cy_tid_;
        cy_t_.swap(t_);
        cy_tid_.swap(tid_);
        auto s1 = AlgoProfile::CaptureClock();
        Build();
        auto s2 = AlgoProfile::CaptureClock();
        AlgoProfile::GetInstance()->SetRebuildTime(s1, s2);
        std::unordered_map<int, int> ump;
        for (int i = 0; i < tid_.size(); ++i)
        {
            int tid = tid_[i];
            if (ump.count(tid) == 0)
            {
                ump[tid] = cy_tid_[i];
            }
            else
            {
                if (ump[tid] != cy_tid_[i])
                    return false;
            }
        }
        ump[-1] = -1;
        for (int i = 0; i < tid_.size(); ++i)
        {
            int pt1 = t_[tid_[i]]->pa;
            int pt2 = cy_t_[cy_tid_[i]]->pa;
            if (ump[pt1] != pt2)
                return false;
        }
        AlgoProfile::GetInstance()->SetCorrect(true);
        return true;
    }
}
