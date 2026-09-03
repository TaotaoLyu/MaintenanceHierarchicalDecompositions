#pragma once
#include<vector>
#include<functional>
#include"Graph.hpp"
#include "CohesiveModel.hpp"


#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <map>
#include <unordered_set>
#include <queue>
#include <functional>
#include <chrono>
#include <cinttypes>
#include <cstdio>

#include "defs.h"
#include "Graph.hpp"
#include "order.h"
#include "thebin.h"



#define COUNT_TS false

class Trine : public CohesiveModel {
public:
    Trine(Graph &g);
    Trine(const Trine&) = delete;
    ~Trine() override = default;
    void Init() override;
    //do triange count and trine decomposition for static graph
    void Insert(const Graph::EdgeList& nedges) override;
    void Remove(const Graph::EdgeList& redges) override;

    //void YikaiRemove(const Graph::EdgeList& redges);

    bool CSmall(const uint32_t x, const uint32_t y) {
        return c_[x] < c_[y] || (c_[x] == c_[y] && x < y);
    }
    std::function<bool(const uint32_t, const uint32_t)> c_small;
    uint32_t GetMaxTrine() { return max_trine_; };
    uint32_t operator[](uint32_t x) { return triness_[x]; }
    std::vector<uint32_t>& GetTriness() { return triness_; }
    uint32_t GetMaxLevel() override { return GetMaxTrine(); }
    std::vector<uint32_t> &GetLevels() override { return GetTriness(); }
private:

    const uint32_t n_, m_;
    //|V|

    Graph &g_;
    // graph.h class Graph

    bool CmpDeg(int x, int y) {
        return g_.GetDegrees()[x] > g_.GetDegrees()[y] || (g_.GetDegrees()[x] == g_.GetDegrees()[y] && x > y);
    }
    bool CmpTriness(int x, int y) {
        return triness_[x] > triness_[y];
    }

    std::function<bool(const uint32_t, const uint32_t)> cmp_deg_;
    //used by TriangleCount;

    std::function<bool(const uint32_t, const uint32_t)> cmp_triness_;

    // class Order;
    Order order_obj;
    //an obj for K-order
    TB Cbin;
    //some ele has c_!=0 when insert


    std::vector<uint32_t> c_;
    //it is c (in Batch Insert) and sup (in Tri Decomp) in paper
    std::vector<uint32_t> triness_;
    uint32_t max_trine_;
    //triness

    //std::vector<uint32_t> triness_plus_;

    std::vector<uint32_t> rem_;
    //rem
    std::vector<uint32_t> ext_;
    //ext
    std::vector<uint32_t> ts_;
    //ts

    struct TriangleT {
        std::pair<uint32_t, uint32_t> vertex[3];
        TriangleT() {
            vertex[0] = std::make_pair(0, 0);
            vertex[1] = std::make_pair(0, 0);
            vertex[2] = std::make_pair(0, 0);
        }
        TriangleT(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f) {
            vertex[0] = std::make_pair(a, b);
            vertex[1] = std::make_pair(c, d);
            vertex[2] = std::make_pair(e, f);
        }
        //ext_tri_id_[triangle_[i].vertex[x][0]][triangle_[i].vertex[x][1]]=i;
    };

    std::vector<bool> bin_;

    std::vector<uint32_t> head_;
    std::vector<uint32_t> tail_;

    const void TriangleCount(std::vector<uint32_t>& cnt, bool typ);
    void SortSwap(
        uint32_t k,
        uint32_t ID, std::vector<uint32_t>& pos,
        std::vector<uint32_t>& vert,
        std::vector<uint32_t>& c_,
        std::vector<uint32_t>& bin
    );
    const void TriDecomp(std::vector<uint32_t>& order);
    //used in insert;
    void MoveCtoOrder(uint32_t k, uint32_t las);
    uint32_t trinesschange(uint32_t u, uint32_t K);
};