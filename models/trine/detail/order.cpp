/*
order.h
*/

#include "order.h"
#include "defs.h"

Order::Order(const uint32_t n,const uint32_t m):
    OMPred(std::bind(&Order::Pred,this,std::placeholders::_1,std::placeholders::_2)),
    HPobj(OMPred),
    n_(n),
    m_(m)
{
}

Order::~Order(){
    //we have no ptr
}

void Order::ListInsert(const uint32_t e1, const uint32_t e2){
    if(e1>=l_)  printf("e1 is %" PRId32 "\n",e1);
    ASSERT(e1<l_);
    node_[e1].next = node_[e2].next;
    node_[e1].prev = e2;
    node_[e2].next = e1;
    if (UINT32_MAX != node_[e1].next) node_[node_[e1].next].prev = e1;
}

void Order::ListRemove(const uint32_t e, uint32_t& head, uint32_t& tail){
    if (head == tail) {
        ASSERT(false);
        // head = tail = UINT32_MAX;
    } else if (e == head) {
        ASSERT(false);
        // head = node_[e].next;
    } else if (e == tail) {
        tail = node_[e].prev;
    }
    if (UINT32_MAX != node_[e].prev) {
        node_[node_[e].prev].next = node_[e].next;
    }
    else
        ASSERT(false);
    if (UINT32_MAX != node_[e].next) {
        node_[node_[e].next].prev = node_[e].prev;
    }
    node_[e].prev = node_[e].next = UINT32_MAX;
}

uint32_t Order::ListPrev(const uint32_t e){
    if(node_[e].prev==UINT32_MAX)
        printf("???%" PRId32 "\n",e);
    ASSERT(node_[e].prev!=UINT32_MAX);
    return node_[e].prev;
}

void Order::Load(std::vector<uint32_t> order){
    // list
    l_=4*(n_+m_);
    uint32_t prev_v = l_;
    node_.resize(prev_v+1,{UINT32_MAX,UINT32_MAX});
    node_[prev_v].next=node_[prev_v].prev=UINT32_MAX;
    for(uint32_t v:order){
        node_[prev_v].next=v;
        node_[v].prev=prev_v;
        node_[v].next=UINT32_MAX;
        prev_v=v;
    }
    
    //OM
    //ASSERT(/*l_ >= 64 && */l_ < (static_cast<uint32_t>(1) << 29));
    om_nodes_ = std::vector<OMNode>(l_ + 2);
    om_tag_ = std::vector<uint64_t>(l_ + 1, 0);
    om_grp_ = std::vector<uint32_t>(l_ + 1, 0);
    om_cnt_ = std::vector<uint32_t>(l_ + 2, 0);
    // available group ids
    om_avail_ = 0;
    om_nodes_[0].prev = om_nodes_[l_].next = UINT32_MAX;
    for (uint32_t i = 1; i <= l_; ++i) {
        om_nodes_[i].prev = i - 1;
        om_nodes_[i - 1].next = i;
    }
    const uint64_t step = (static_cast<uint64_t>(1) << 60) / (l_ + 1);
    uint32_t cnt = 0;
    uint32_t tgid = UINT32_MAX;
    for (uint32_t p = l_; UINT32_MAX != p; p = node_[p].next) {
        if (unlikely(0 == cnt)) {
            // initialize the group
            tgid = om_avail_;
            om_avail_ = om_nodes_[om_avail_].next;
            om_nodes_[om_avail_].prev = UINT32_MAX;
            om_nodes_[tgid].next = l_ + 1;
            if (0 == tgid) {
                om_nodes_[tgid].prev = UINT32_MAX;
                om_nodes_[tgid].tag = 0;
            } else {
                om_nodes_[tgid].prev = tgid - 1;
                om_nodes_[tgid - 1].next = tgid;
                om_nodes_[tgid].tag = om_nodes_[tgid - 1].tag + step;
            }
            om_cnt_[tgid] = 1;
            // initialize the edge
            om_tag_[p] = 0;
            om_grp_[p] = tgid;
        } else {
            om_tag_[p] = om_tag_[node_[p].prev] + (static_cast<uint64_t>(1) << 30);
            om_grp_[p] = tgid;
            ++om_cnt_[tgid];
        }
        if (++cnt == om_grp_ub_ / 2) cnt = 0;
    }
    // the end mark of the order
    om_cnt_[l_ + 1] = 0;
    om_nodes_[l_ + 1].tag = (static_cast<uint64_t>(1) << 60) - 1;
    om_nodes_[l_ + 1].next = UINT32_MAX;
    om_nodes_[l_ + 1].prev = tgid;
    HPobj.Load(n_);
}

void Order::Insert(const uint32_t e1, const uint32_t e2){
    // the group is full; that is, a new group needs to be created
    if (om_cnt_[om_grp_[e2]] == om_grp_ub_) {
        // the new group id
        const uint32_t ngid = om_avail_;
        om_avail_ = om_nodes_[om_avail_].next;
        om_nodes_[om_avail_].prev = UINT32_MAX;
        // the group to which e2 belongs and the next group
        const uint32_t gid1 = om_grp_[e2];
        const uint32_t gid2 = om_nodes_[gid1].next;
        // there is no vacancy; relabeling is needed
        if (om_nodes_[gid1].tag + 1 == om_nodes_[gid2].tag) {
            const uint64_t tag = om_nodes_[gid1].tag;
            uint32_t cnt = 1;
            uint32_t threshold = 1;
            uint64_t mask = UINT64_MAX;
            uint32_t ph = gid1, pe = gid1;
            // find the "smallest" enclosing tag range with low enough density
            while (cnt >= threshold) {
                threshold = threshold << 1;
                mask = mask << 2;
                while (UINT32_MAX != om_nodes_[ph].prev &&
                (om_nodes_[om_nodes_[ph].prev].tag & mask) == (tag & mask)) {
                    ph = om_nodes_[ph].prev;
                    ++cnt;
                }
                while (
                    UINT32_MAX != om_nodes_[pe].next && 
                    (om_nodes_[om_nodes_[pe].next].tag&mask) == (tag&mask)
                ){
                    pe = om_nodes_[pe].next;
                    ++cnt;
                }
            }
            const uint64_t step = ((~mask) + 1) / cnt;
            om_nodes_[ph].tag = (tag & mask);
            uint32_t p = om_nodes_[ph].next;
            while (--cnt > 0) {
                om_nodes_[p].tag = om_nodes_[om_nodes_[p].prev].tag + step;
                p = om_nodes_[p].next;
            }
        }
        ASSERT(om_nodes_[gid1].tag + 1 < om_nodes_[gid2].tag);
        // insert the new group to the list
        om_nodes_[ngid].prev = gid1;
        om_nodes_[ngid].next = gid2;
        om_nodes_[gid1].next = ngid;
        om_nodes_[gid2].prev = ngid;
        // assign the next tag instead of the middle of gid1 and gid2
        om_nodes_[ngid].tag = om_nodes_[gid1].tag + 1;
        // distribute the edges
        // step 1. find the first edge in the group
        uint32_t p = e2;
        while (UINT32_MAX != node_[p].prev && gid1 == om_grp_[node_[p].prev]) {
            p = node_[p].prev;
        }
        uint32_t cnt = 0;
        // step 2. assign new tags to the edges remaining in the original group
        om_tag_[p] = 0;
        p = node_[p].next;
        while (++cnt < om_grp_ub_ / 2) {
            om_tag_[p] = om_tag_[node_[p].prev] + (static_cast<uint64_t>(1) << 30);
            p = node_[p].next;
        }
        om_cnt_[gid1] = om_grp_ub_ / 2;
        // step 3. distribute the edges
        om_tag_[p] = 0;
        om_grp_[p] = ngid;
        p = node_[p].next;
        while (++cnt < om_grp_ub_) {
            om_tag_[p] = om_tag_[node_[p].prev] + (static_cast<uint64_t>(1) << 30);
            om_grp_[p] = ngid;
            p = node_[p].next;
        }
        om_cnt_[ngid] = om_grp_ub_ - om_grp_ub_ / 2;
        // in case @om_grp_ub_ is set as an odd number
        ASSERT(om_cnt_[ngid] == om_cnt_[gid1]);
    }
    // whether to relabel the edges in group @om_grp_[e2]
    bool relabel = false;
    const uint32_t e3 = node_[e2].next;
    if (UINT32_MAX == e3 || om_grp_[e3] != om_grp_[e2]) {
    relabel = (om_tag_[e2] >= (static_cast<uint64_t>(1) << 60));
    } else {
    relabel = (om_tag_[e2] + 1 == om_tag_[e3]);
    }
    if (relabel) {
    const uint32_t gid1 = om_grp_[e2];
    uint32_t p = e2;
    while (UINT32_MAX != node_[p].prev && gid1 == om_grp_[node_[p].prev]) {
    p = node_[p].prev;
    }
    om_tag_[p] = 0;
    p = node_[p].next;
    while (UINT32_MAX != p && gid1 == om_grp_[p]) {
    om_tag_[p] = om_tag_[node_[p].prev] + (static_cast<uint64_t>(1) << 30);
    p = node_[p].next;
    }
    }
    // set the new edge
    ++om_cnt_[om_grp_[e2]];
    om_grp_[e1] = om_grp_[e2];
    if (UINT32_MAX == e3 || om_grp_[e3] != om_grp_[e2]) {
    om_tag_[e1] = om_tag_[e2] + (static_cast<uint64_t>(1) << 30);
    } else {
    om_tag_[e1] = (om_tag_[e2] + om_tag_[e3]) / 2;
    }
}

void Order::Remove(const uint32_t e){
    const uint32_t gid = om_grp_[e];
    ASSERT(om_cnt_[gid] >= 1);
    if (1 == om_cnt_[gid]) {
        om_nodes_[om_nodes_[gid].prev].next = om_nodes_[gid].next;
        om_nodes_[om_nodes_[gid].next].prev = om_nodes_[gid].prev;
        // update the available groups
        om_nodes_[gid].prev = UINT32_MAX;
        om_nodes_[gid].next = om_avail_;
        om_nodes_[om_avail_].prev = gid;
        om_avail_ = gid;
    }
    --om_cnt_[gid];
}

bool Order::Pred(const uint32_t e1, const uint32_t e2){
    if (om_grp_[e1] == om_grp_[e2]) {
        return om_tag_[e1] < om_tag_[e2];
    } else {
        return om_nodes_[om_grp_[e1]].tag < om_nodes_[om_grp_[e2]].tag;
    }
}