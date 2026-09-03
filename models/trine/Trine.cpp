#include "Trine.hpp"
#include "Graph.hpp"
#include "defs.h"

Trine::Trine(Graph &g) : g_(g),
                         c_small(std::bind(&Trine::CSmall, this, std::placeholders::_1, std::placeholders::_2)),
                         n_(g.GetVertexCount()),
                         m_(g.GetEdgeCount()),
                         order_obj(g.GetVertexCount(), g.GetEdgeCount()),
                         Cbin(g.GetVertexCount()),
                         max_trine_(0)
{

    cmp_deg_ = std::bind(&Trine::CmpDeg, this, std::placeholders::_1, std::placeholders::_2);
    cmp_triness_ = std::bind(&Trine::CmpTriness, this, std::placeholders::_1, std::placeholders::_2);
    c_.resize(n_, 0);
    triness_.resize(n_);
    rem_.resize(n_, 0);
    ext_.resize(n_, 0);
    ts_.resize(n_, 0);
    bin_.resize(n_, false);

    // new_edg_.resize(n_);
}

void Trine::Init()
{
    TriangleCount(c_, !(COUNT_TS));

    std::vector<uint32_t> order;

    TriDecomp(order);

    TriangleCount(ts_, COUNT_TS);
    std::vector<bool> vis(n_, false);
    std::vector<uint32_t> listorder;

    for (uint32_t v : order)
    {
        vis[v] = true;
        rem_[v] = c_[v];
        // we have a right c_
        ext_[v] = c_[v] = 0;
        // we have c_ and needn't s_
        const uint32_t k = triness_[v];
        while (head_.size() <= k)
        {
            uint32_t newk = (uint32_t)head_.size();
            head_.push_back(n_ + newk);
            listorder.push_back(head_[newk]);
            tail_.push_back(n_ + newk);
        }
        listorder.push_back(v);
        tail_[k] = v;
    }
    order_obj.Load(listorder);
    for (auto e : triness_)
        max_trine_ = std::max(max_trine_, e);
}

const void Trine::TriangleCount(std::vector<uint32_t> &cnt, bool typ)
{
    bin_.resize(n_, false);
    std::vector<int> _(n_);
    for (uint32_t i = 0; i < n_; i++)
        _[i] = i;
    // /*
    // TODO:unfortunately, we can not give sort a member function...

    std::sort(_.begin(), _.end(), cmp_deg_);
    // when -O3 this maybe O(n)...anyway,O(nlogn)<O(m*a(m))
    //  */
    for (uint32_t i = 0; i < n_; i++)
    {
        for (auto j : g_[_[i]])
            if (g_.GetDegrees()[j] < g_.GetDegrees()[_[i]] || (g_.GetDegrees()[j] == g_.GetDegrees()[_[i]] && j < (long long)_[i]))
                bin_[j] = true;
        for (auto j : g_[_[i]])
            if (g_.GetDegrees()[j] < g_.GetDegrees()[_[i]] || (g_.GetDegrees()[j] == g_.GetDegrees()[_[i]] && j < (long long)_[i]))
            {
                for (auto k : g_[j])
                    if (bin_[k])
                    {
                        if (typ || (triness_[_[i]] <= triness_[j] && triness_[_[i]] <= triness_[k]))
                            cnt[_[i]] += 2;
                        if (typ || (triness_[j] <= triness_[_[i]] && triness_[j] <= triness_[k]))
                            cnt[j] += 2;
                        if (typ || (triness_[k] <= triness_[_[i]] && triness_[k] <= triness_[j]))
                            cnt[k] += 2;
                    }
                bin_[j] = false;
            }
    }
}

void Trine::SortSwap(
    uint32_t k,
    uint32_t ID, std::vector<uint32_t> &pos,
    std::vector<uint32_t> &vert,
    std::vector<uint32_t> &c_,
    std::vector<uint32_t> &bin)
{
    unsigned oldc_ = c_[ID] < k ? k : c_[ID];
    unsigned oldpos = pos[ID];
    unsigned firpos = bin[oldc_];
    unsigned firvert = vert[firpos];
    if (ID != firvert)
    { // if(ID is not the first c_=c_[ID] one )
        // swap ID with the first one
        std::swap(pos[ID], pos[firvert]);
        vert[pos[ID]] = ID;
        vert[pos[firvert]] = firvert;
    }
    if (c_[ID] > k)
    {
        ++bin[oldc_];
    }
    --c_[ID];
    // we need a right c_ and rem_=c_
}

const void Trine::TriDecomp(std::vector<uint32_t> &order)
{
    uint32_t max_c_ = 0;
    for (auto x : c_)
        if (x > max_c_)
            max_c_ = x;
    std::vector<uint32_t> bin(max_c_ + 1, 0);
    for (auto x : c_)
        ++bin[x];
    uint32_t start = 0;
    for (uint32_t i = 0; i <= max_c_; ++i)
    {
        uint32_t temp = bin[i];
        bin[i] = start;
        start += temp;
    }
    // bin[i] how many vertex has a deg<i
    std::vector<uint32_t> vert(n_);
    std::vector<uint32_t> pos(n_);
    for (uint32_t i = 0; i < n_; ++i)
    {
        pos[i] = bin[c_[i]];
        vert[pos[i]] = i;
        ++bin[c_[i]];
    }
    // bin[i] how many vertex has a deg<=i
    // pos[i] is i's pos
    // vert[pos[i]]=i
    for (uint32_t i = max_c_; i > 0; --i)
    {
        bin[i] = bin[i - 1];
    }
    bin[0] = 0;
    // bin[i] how many vertex has a deg<i
    uint32_t k = 0;
    auto vis = std::vector<bool>(n_, false);
    std::vector<bool> thash(n_, false);
    for (uint32_t i = 0; i < n_; ++i)
    {
        const uint32_t v = vert[i];
        if (c_[v] > k)
            k = c_[v];
        // ASSERT(bin[deg[v]] == i);
        ++bin[k];
        triness_[v] = k;
        vis[v] = true;
        for (uint32_t w : g_[v])
            if (!vis[w])
                thash[w] = true;
        for (uint32_t u : g_[v])
            if (thash[u])
                for (uint32_t w : g_[u])
                    if (thash[w])
                    {
                        SortSwap(k, u, pos, vert, c_, bin);
                        SortSwap(k, w, pos, vert, c_, bin);
                    }
        for (uint32_t w : g_[v])
            thash[w] = false;
        order.push_back(v);
    }
}

void Trine::Insert(const Graph::EdgeList &nedges)
{
    ASSERT(!nedges.empty());
    // do for each nedg:
    // deg;(C)list;
    std::vector<uint32_t> list;
    // sttstcs_.ext_cnt = 0;
    for (auto edg : nedges)
    {
        uint32_t u, v;
        u = edg.first, v = edg.second;
        // ASSERT(u < n_ && v < n_ && u != v);

        /*g_.GetDegrees()[u]++, g_[u].push_back(v);
        g_.GetDegrees()[v]++, g_[v].push_back(u);*/
        g_.InsertEdge(u, v);

        if (g_.GetDegrees()[u] > g_.GetDegrees()[v])
            std::swap(u, v);
        for (auto w : g_[u])
            bin_[w] = true;
        for (auto w : g_[v])
            if (bin_[w])
            {
                // sttstcs_.ext_cnt += 2;
                // ts_
                if (triness_[u] <= triness_[v] && triness_[u] <= triness_[w])
                    ts_[u] += 2;
                if (triness_[v] <= triness_[w] && triness_[v] <= triness_[u])
                    ts_[v] += 2;
                if (triness_[w] <= triness_[u] && triness_[w] <= triness_[v])
                    ts_[w] += 2;
                // rem_ and ext_
                uint32_t p = order_obj.Pred(u, v) ? u : v;
                p = order_obj.Pred(p, w) ? p : w;
                if (rem_[p] < triness_[p])
                    rem_[p] += 2;
                else
                {
                    ext_[p] += 2;
                    if (ext_[p] == 2)
                        list.push_back(p);
                }
            }
        for (auto w : g_[u])
            bin_[w] = false;
    }
    // sort list
    // sort(list.begin(),list.end(),cmp_triness_);
    // do for each node in list;
    // t*,ts,c_,Cbin
    for (auto u : list)
    {
        uint32_t tlas = triness_[u];
        // if(tlas==trinesschange(u,triness_[u])){
        //     if(ext_[u])
        //         order_obj.HPobj.Insert(u);
        // }
        if (ext_[u])
            order_obj.HPobj.Insert(u);
    }

    // for k 0 to ...
    if (Cbin.IsEmpty() && order_obj.HPobj.IsEmpty())
        return;

    // sttstcs_.single_insert_[sttstcs_.ext_cnt].Timestart();

    for (uint32_t k = 0; !Cbin.IsEmpty() || !order_obj.HPobj.IsEmpty(); k += 2)
    {

        if (Cbin.IsEmpty())
            k = std::max(k, triness_[order_obj.HPobj.Top()]);

        while (head_.size() <= k)
        {
            uint32_t newk = (uint32_t)head_.size();
            head_.push_back(n_ + newk);
            order_obj.Insert(head_[newk], tail_[newk - 1]);
            order_obj.ListInsert(head_[newk], tail_[newk - 1]);
            tail_.push_back(n_ + newk);
        }

        // triness<k ==> triness=k
        // updata triness and ext
        if (!Cbin.IsEmpty())
        {
            int u = Cbin.Top();
            do
            {
                for (auto w : g_[u])
                    if (triness_[w] >= k)
                        bin_[w] = true;
                for (auto v : g_[u])
                    if (triness_[v] == k && !Cbin.IsIn(v))
                    {
                        for (auto w : g_[v])
                            if (bin_[w])
                            {
                                ts_[v] += 2;
                                if (Cbin.IsIn(w) || order_obj.Pred(v, w))
                                {
                                    ext_[v] += 2;
                                    if (ext_[v] == 2)
                                        order_obj.HPobj.Insert(v);
                                }
                                // else no process
                            }
                    }
                for (auto w : g_[u])
                    bin_[w] = false;
                triness_[u] = k;
            } while (Cbin.next(u));
        }
        MoveCtoOrder(k, head_[k]);
        while (!order_obj.HPobj.IsEmpty() && triness_[order_obj.HPobj.Top()] == k)
        {
            uint32_t u = order_obj.HPobj.Top();
            order_obj.HPobj.Delete(u);
            // printf("ext+rem %" SCNu32 " k:%" SCNu32 " compare:%s\n",ext_[u]+rem_[u],k,ext_[u]+rem_[u]>k?"true":"false");
            if (ext_[u] + rem_[u] > k)
            {
                uint32_t tlas = k;
                bool flag = true;
                if (rem_[u] == k && false)
                { ////////////////////dont run trinesschange here
                    flag = (tlas == trinesschange(u, k));
                }
                if (flag)
                {
                    c_[u] = rem_[u] + ext_[u];
                    rem_[u] = ext_[u] = 0;
                    for (auto w : g_[u])
                        if (Cbin.IsIn(w) || order_obj.Pred(u, w))
                            bin_[w] = 1;
                    for (auto v : g_[u])
                        if (!Cbin.IsIn(v) && order_obj.Pred(u, v) && triness_[v] == k)
                        {
                            for (auto w : g_[v])
                                if (bin_[w] && (Cbin.IsIn(w) || order_obj.Pred(v, w)))
                                {
                                    ext_[v] += 2;
                                    if (ext_[v] == 2)
                                        order_obj.HPobj.Insert(v);
                                }
                        }
                    for (auto w : g_[u])
                        bin_[w] = 0;
                    order_obj.ListRemove(u, head_[k], tail_[k]);
                    order_obj.Remove(u);
                    // ASSERT(!order_obj.HPobj.IsIn(u));
                    Cbin.Insert(u, c_[u]);
                }
            }
            else
            {
                // printf("ext:%" SCNu32 " c_:%" SCNu32 "\n",ext_[u],c_[u]);
                rem_[u] += ext_[u];
                // printf("rem:%" SCNu32 " K:%" SCNu32 "\n",rem_[u],k);
                // ASSERT(DEBUGrem(u,rem_[u]));
                ext_[u] = c_[u] = 0;
                for (auto w : g_[u])
                    if (Cbin.IsIn(w) || order_obj.Pred(u, w))
                        bin_[w] = 1;
                for (auto v : g_[u])
                    if (Cbin.IsIn(v))
                    {
                        for (auto w : g_[v])
                            if (bin_[w])
                            {
                                Cbin.Delete(v);
                                c_[v] -= 2;
                                Cbin.Insert(v, c_[v]);
                            }
                    }
                for (auto w : g_[u])
                    bin_[w] = 0;
                MoveCtoOrder(k, u);
            }
        }
    }
    // OutNowTriness();

    // sttstcs_.single_insert_[sttstcs_.ext_cnt].Timepause();
}
// 1
void Trine::MoveCtoOrder(uint32_t K, uint32_t las)
{
    while (!Cbin.IsEmpty() && c_[Cbin.Top()] <= K)
    {
        uint32_t u = Cbin.Top();
        // ts_
        ts_[u] = 0;
        for (auto v : g_[u])
            if (K <= triness_[v])
                bin_[v] = true;
        for (auto v : g_[u])
        {
            if (!bin_[v])
                continue;
            for (auto w : g_[v])
                if (bin_[w])
                {
                    if (K <= triness_[v] && K <= triness_[w])
                        ts_[u] += 2;
                }
            bin_[v] = false;
        }
        // triness,rem,c,ext
        triness_[u] = K;
        rem_[u] = c_[u];
        c_[u] = ext_[u] = 0;
        // remove u from Cbin,insert to order
        Cbin.Delete(u);
        order_obj.Insert(u, las);
        order_obj.ListInsert(u, las);
        if (tail_[K] == las)
            tail_[K] = u;
        las = u;

        for (auto w : g_[u])
            if (Cbin.IsIn(w) || order_obj.Pred(u, w))
                bin_[w] = true;
        for (auto v : g_[u])
            if (triness_[v] == K && (Cbin.IsIn(v) || order_obj.Pred(u, v)))
            {
                for (auto w : g_[v])
                    if (bin_[w])
                    {
                        if (Cbin.IsIn(v))
                        {
                            Cbin.Delete(v);
                            c_[v] -= 2;
                            Cbin.Insert(v, c_[v]);
                        }
                        else
                        {
                            if (!Cbin.IsIn(w) && !order_obj.Pred(v, w))
                                continue;
                            // ASSERT(ext_[v]!=0);
                            ext_[v] -= 2;
                            if (!ext_[v])
                                order_obj.HPobj.Delete(v);
                        }
                    }
            }
        for (auto w : g_[u])
            bin_[w] = false;
    }

} // remove one from CvN
// 2
/*
input a node u,K=triness[u]
do:
    if(triness[u] an change to tnew)
        move u to the first one in tnew ,if(rem=ts>tnew) ext=...
    else
        tnew=triness[u]
    return;
*/
uint32_t Trine::trinesschange(uint32_t u, uint32_t K)
{
    if (order_obj.HPobj.IsIn(u))
        order_obj.HPobj.Delete(u);
    if (ext_[u] + rem_[u] <= K)
    {
        rem_[u] += ext_[u], ext_[u] = 0;
        return K;
    }

    int tnew = K;
    int supnow = ts_[u];
    // tnew=d*(d-1)
    int dsupu = 0;
    for (auto v : g_[u])
        if (triness_[v] >= (long long)tnew)
            dsupu++;
    if (dsupu * (dsupu - 1) == supnow)
    {
        tnew = supnow;
    }
    else
    {
        // tnew=max{k}
        if (ts_[u])
        {
            std::vector<int> sum;
            for (auto v : g_[u])
                if (triness_[v] >= K)
                    bin_[v] = true;
            for (auto v : g_[u])
            {
                if (bin_[v])
                    for (auto w : g_[v])
                    {
                        if (!bin_[w])
                            continue;
                        uint32_t min_triness = std::min(triness_[v], triness_[w]);
                        uint32_t delta = min_triness - K;
                        while (sum.size() <= delta)
                            sum.push_back(0);
                        sum[delta] += 2;
                    }
                bin_[v] = false;
            }
            for (uint32_t i = 0;; i += 2)
            {
                if (supnow - sum[i] >= tnew + 2)
                {
                    tnew += 2;
                    supnow -= sum[i];
                }
                else
                    break;
            }
            /*
            std::vector<int> esup;
            for(auto v:g_.Nie[u])
                if(triness_[v]>K)
                    bin_[v]=true;
            for(auto v:g_.Nie[u]){
                if(bin_[v]){
                    int i=esup.size();
                    esup.push_back(0);
                    for(auto w:g_.Nie[v])
                        if(bin_[w]&&order_obj.Pred(v,w)){
                            esup[i]+=2;
                        }
                }
            }
            for(auto v:g_.Nie[u])   bin_[v]=false;

            int l=K+1,r=head_.size()-1;
            int mid=(l+r)>>1;
            while(r-l>=2){
                if(check(mid,u,esup))
                    l=mid;
                else
                    r=mid-1;
                mid=(l+r)>>1;
            }
            for(;r>=l;r--)
                if(check(r,u,esup)){
                    tnew=r;
                    break;
                }
            */
        }

        // tnew=d*(d-1)--do it twice
        int dsupu = 0;
        for (auto v : g_[u])
            if (triness_[v] >= (long long)tnew)
                dsupu++;
        if (dsupu * (dsupu - 1) == supnow)
        {
            tnew = supnow;
        }
    }

    if (tnew == K)
        return tnew;

    // ts_ and c_ and rem_ and ext_
    ts_[u] = c_[u] = ext_[u] = rem_[u] = 0;
    for (auto v : g_[u])
        bin_[v] = true;
    for (auto v : g_[u])
    {
        for (auto w : g_[v])
        {
            if (!bin_[w])
                continue;
            // p=v,w;q=w,v;
            for (auto p : {v, w})
            {
                uint32_t q = v + w - p;
                if (triness_[p] > triness_[q])
                    continue;
                // ts and c for v,w
                if (triness_[p] > (long long)K && triness_[p] <= (long long)tnew)
                {
                    ts_[p] += 2;
                    // if(Cbin.IsIn(p))
                    //     c_[p]+=2;
                }
                // rem and ext for v,w
                if (Cbin.IsIn(p))
                    continue;
                if (!Cbin.IsIn(q) && order_obj.Pred(q, p))
                    continue;
                if (order_obj.Pred(u, p) && triness_[p] < (long long)tnew)
                {
                    if (rem_[p] < triness_[p] && (!Cbin.IsIn(q) || triness_[q] != triness_[p]))
                        rem_[p] += 2;
                    else
                    {
                        ext_[p] += 2;
                        if (ext_[p] == 2)
                            order_obj.HPobj.Insert(p);
                    }
                }
            }
            // u:c_ and ts
            if ((long long)tnew <= triness_[v] && (long long)tnew <= triness_[w])
            {
                ts_[u] += 2;
                if (rem_[u] < (long long)tnew)
                    rem_[u] += 2;
                else
                {
                    ext_[u] += 2;
                    // insert it into heap later
                }
            }
        }
        bin_[v] = false;
    }
    // remove u from K-order
    order_obj.ListRemove(u, head_[K], tail_[K]);
    order_obj.Remove(u);

    triness_[u] = tnew;
    // insert u to tnew-order
    while (head_.size() <= tnew)
    {
        uint32_t newk = (uint32_t)head_.size();
        head_.push_back(n_ + newk);
        order_obj.Insert(head_[newk], tail_[newk - 1]);
        order_obj.ListInsert(head_[newk], tail_[newk - 1]);
        tail_.push_back(n_ + newk);
    }
    order_obj.Insert(u, head_[tnew]);
    order_obj.ListInsert(u, head_[tnew]);
    if (tail_[tnew] == head_[tnew])
        tail_[tnew] = u;
    if (ext_[u])
        order_obj.HPobj.Insert(u);
    return tnew;
} // change triness and order

void Trine::Remove(const Graph::EdgeList &redges)
{
    ASSERT(!redges.empty());

    std::queue<uint32_t> Q;

    // do for each redg
    for (auto edg : redges)
    {
        uint32_t u, v;
        u = edg.first, v = edg.second;
        ASSERT(u < n_ && v < n_ && u != v);
        if (g_.GetDegrees()[u] > g_.GetDegrees()[v])
            std::swap(u, v);

        for (auto w : g_[u])
            bin_[w] = true;
        for (auto w : g_[v])
            if (bin_[w])
            {
                uint32_t x;
                x = order_obj.Pred(u, v) ? u : v;
                x = order_obj.Pred(x, w) ? x : w;
                rem_[x] -= 2;
                // p=u,v,w:
                for (auto p : {u, v, w})
                {
                    if (triness_[p] == triness_[x])
                    {
                        if (ts_[p] == triness_[p])
                            Q.push(p);
                        ts_[p] -= 2;
                    }
                }
            }
        for (auto w : g_[u])
            bin_[w] = false;

        // p=u,v;q=v,u;
        /*for (auto p : { u,v }) {
            uint32_t q = u + v - p;
            int i;
            g_.GetDegrees()[p]--;
            for (int i = 0; i < g_[p].size(); i++)
                if (g_[p][i] == q) {
                    std::swap(g_[p][i], g_[p][g_[p].size() - 1]);
                    g_[p].pop_back();
                    break;
                }
        }*/
        g_.RemoveEdge(u, v);
    }

    // Q---

    while (!Q.empty())
    {
        uint32_t u = Q.front();

        Q.pop();

        // sttstcs_.change_cnt++;

        std::vector<std::pair<int, int>> infl_trangle;
        std::vector<uint32_t> sum(triness_[u] + 1, 0);
        for (auto v : g_[u])
            bin_[v] = true;
        for (auto v : g_[u])
        {
            for (auto w : g_[v])
            {
                if (!bin_[w])
                    continue;
                uint32_t min_triness;
                min_triness = triness_[v] <= triness_[w] ? triness_[v] : triness_[w];
                if (min_triness > triness_[u])
                    min_triness = triness_[u];
                else
                    infl_trangle.push_back(std::make_pair(v, w));
                sum[min_triness] += 2;
            }
            bin_[v] = false;
        }

        uint32_t tnew;
        for (tnew = triness_[u] - 1; tnew > 0; tnew--)
        {
            sum[tnew] += sum[tnew + 1];
            if (sum[tnew] >= tnew)
                break;
        }
        ts_[u] = sum[tnew];

        if (tnew == triness_[u])
            continue;

        for (auto triange_ : infl_trangle)
        {
            int v = triange_.first, w = triange_.second;
            for (auto p : {v, w})
            {
                uint32_t q = v + w - p;
                if (triness_[p] > triness_[q])
                    continue;
                // ts for v,w
                if (ts_[p] >= triness_[p])
                {
                    if (triness_[p] > tnew)
                        ts_[p] -= 2;
                    if (ts_[p] == triness_[p] - 2)
                        Q.push(p);
                }
                // rem for u and v,w
                if (order_obj.Pred(q, p))
                    continue;
                if (order_obj.Pred(p, u) && triness_[p] > tnew)
                    rem_[p] -= 2, rem_[u] += 2;
            }
        }
        order_obj.Remove(u);
        order_obj.ListRemove(u, head_[triness_[u]], tail_[triness_[u]]);

        order_obj.Insert(u, tail_[tnew]);
        order_obj.ListInsert(u, tail_[tnew]);
        tail_[tnew] = u;
        // sttstcs_.tnew_dalta+=triness_[u]-tnew;
        triness_[u] = tnew;
    }
}
