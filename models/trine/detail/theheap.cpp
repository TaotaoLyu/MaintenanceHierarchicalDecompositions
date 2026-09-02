
#include "theheap.h"
#include "defs.h"

HP::HP(std::function<bool (const uint32_t, const uint32_t)> CMP){
    HPcmp=CMP;
}
bool HP::IsIn(const uint32_t e){
    return hp_pos_.at(e)!= UINT32_MAX;
}

void HP::Load(uint32_t n){
    n_=n;
    ASSERT(0 < n_ && n_ < (static_cast<uint32_t>(1) << 29));
    
    hp_pos_.resize(n_ + 1, UINT32_MAX);
    hp_tbl_.push_back(UINT32_MAX);
}

void HP::Insert(const uint32_t e){
    ASSERT(UINT32_MAX == hp_pos_.at(e));
    const uint32_t size = hp_tbl_.size();
    hp_tbl_.push_back(e);
    Up(size, e);
    // printf("insert %" PRId32 "top is %" PRId32 "\n",e,Top());
}

void HP::Delete(const uint32_t e){
    ASSERT(UINT32_MAX != hp_pos_.at(e));
    const uint32_t size = hp_tbl_.size() - 1;
    const uint32_t e2 = hp_tbl_[size];
    hp_tbl_.pop_back();
    // shift down or up
    if (size != hp_pos_[e]) {
        const uint32_t h = hp_pos_[e];
        if (h / 2 != 0 && HPcmp(e2, hp_tbl_[h / 2])) {
            Up(h, e2);
        } else {
            Down(h, e2);
        }
    }
    hp_pos_[e] = UINT32_MAX;
    // if(!IsEmpty())  printf("delete %" PRId32 ": top is %" PRId32 "\n",e,Top());
    // else    printf("delete %" PRId32 ": top is empty!\n",e);
}

uint32_t HP::Top(){
    ASSERT(hp_tbl_.size() > 1);
    return hp_tbl_[1];
}

bool HP::IsEmpty(){
    return hp_tbl_.size()<=1;
}

void HP::Up(const uint32_t h, const uint32_t e){
    uint32_t c = h;
    uint32_t p = c / 2;
    while (0 != p && HPcmp(e, hp_tbl_[p])) {
        hp_tbl_[c] = hp_tbl_[p];
        hp_pos_[hp_tbl_[p]] = c;
        c = p;
        p = c / 2;
    }
    hp_tbl_[c] = e;
    hp_pos_[e] = c;
}

void HP::Down(const uint32_t h, const uint32_t e){
    const uint32_t size = hp_tbl_.size();
    uint32_t p = h;
    uint32_t c = p * 2;
    while (c < size) {
        if (c + 1 < size && HPcmp(hp_tbl_[c + 1], hp_tbl_[c])) ++c;
        if (HPcmp(e, hp_tbl_[c])) break;
        hp_tbl_[p] = hp_tbl_[c];
        hp_pos_[hp_tbl_[c]] = p;
        p = c;
        c = p * 2;
    }
    hp_tbl_[p] = e;
    hp_pos_[e] = p;
}