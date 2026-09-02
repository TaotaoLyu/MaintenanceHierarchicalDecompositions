/*
work about k-order
*/

#include <cstdint>
#include <vector>
#include <functional>
#include <cinttypes>

#include "theheap.h"

class Order{//order and list (and heap)
    public:
        Order(const uint32_t n,const uint32_t m);
        Order(const Order&) = delete;
        ~Order();
        // list maintenance
        void ListInsert(const uint32_t e1, const uint32_t e2);
        void ListRemove(const uint32_t e, uint32_t& head, uint32_t& tail);
        uint32_t ListPrev(const uint32_t e);
        // order maintenace
        void Load(std::vector<uint32_t> order);
        void Insert(const uint32_t e1, const uint32_t e2);
        void Remove(const uint32_t e);
        bool Pred(const uint32_t e1, const uint32_t e2);
        
        std::function<bool (const uint32_t, const uint32_t)> OMPred;
        //the heap
        HP HPobj;
    private:
        uint32_t n_,m_,l_;
        //|v||e|limit

        struct ListNode final {
            uint32_t prev;
            uint32_t next;
        };

        struct OMNode final {
            uint64_t tag;
            uint32_t prev;
            uint32_t next;
        };

        //list
        std::vector<ListNode> node_;

        // data members for order maintenance
        const uint32_t om_grp_ub_ = 30;
        uint32_t om_avail_;
        std::vector<OMNode> om_nodes_;
        std::vector<uint64_t> om_tag_;
        std::vector<uint32_t> om_grp_;
        std::vector<uint32_t> om_cnt_;
        // auxiliary array
        // std::vector<uint32_t>& rank_ = hp_pos_;
};