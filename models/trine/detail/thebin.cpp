#include "thebin.h"
TB::TB(uint32_t n){
    n_=n;
    pos_.resize(n_,std::make_pair(-1,-1));
    size_=0;
}

void TB::Insert(const uint32_t e,uint32_t k){
    //ASSERT(e<n_);
    ASSERT(IsIn(e)==false);
    // if(sort_bin_.size()<=k)
    //     sort_bin_.resize(k+1);
    // ASSERT(k>=0&&k<=n_);
    while(sort_bin_.size()<=k)
        sort_bin_.push_back({});
    pos_[e].first=k,pos_[e].second=sort_bin_[k].size();
    sort_bin_[k].push_back(e);
    size_++;
    if(k<topKptr||size_==1)   topKptr=k;
}

void TB::Delete(const uint32_t e){
    //ASSERT(e<n_);
    ASSERT(IsIn(e)==true);
    int k=pos_[e].first,ran=pos_[e].second;
    pos_[sort_bin_[k][sort_bin_[k].size()-1]].second=ran;
    std::swap(sort_bin_[k][ran],sort_bin_[k][sort_bin_[k].size()-1]);
    pos_[e].first=pos_[e].second=-1;
    sort_bin_[k].pop_back();
    size_--;
    if(size_)
        while(!sort_bin_[topKptr].size())   topKptr++;
}

uint32_t TB::Top(){
    return sort_bin_[topKptr][0];
}

bool TB::IsIn(const uint32_t e){
    //ASSERT(e<n_);
    return (pos_[e].first!=-1&&pos_[e].second!=-1);
}

bool TB::IsEmpty(){
    return size_==0;
}
bool TB::next(int&i){
    int k=pos_[i].first,id=pos_[i].second;
    if(k==sort_bin_.size()-1&&id==sort_bin_[k].size()-1)
        return false;
    if(id!=sort_bin_[k].size()-1){
        i=sort_bin_[k][id+1];
    }
    else{
        k++;
        while(k<sort_bin_.size()&&!sort_bin_[k].size())
            k++;
        if(k>=sort_bin_.size()) return false;
        i=sort_bin_[k][0];
    }
    return true;
}