/*
    there is a bin
    we use it store CorN node
*/
#include<vector>
#include<cstdint>
#include<cinttypes>
#include<functional>

#include "defs.h"

class TB{
    public:
        TB(uint32_t n);
        // void Load(uint32_t n);
        void Insert(const uint32_t e,uint32_t k);
        void Delete(const uint32_t e);
        uint32_t Top();
        bool IsIn(const uint32_t e);
        bool IsEmpty();
        bool next(int& i);
    private:
        uint32_t size_;
        uint32_t topKptr;
        uint32_t n_;
        std::vector<std::vector<uint32_t > >sort_bin_;
        std::vector<std::pair<uint32_t ,uint32_t > > pos_;
};