/*
    there is a heap
    we use it store type-2 and type-3 node
    if you ask me why we donot use priority_queue in stl
    I have to say "Idontknow"
    Maybe it makes our time complexity more strict,as we have delete option.
*/

#include<vector>
#include<cstdint>
#include<cinttypes>
#include<functional>

class HP{
    public:
        HP(std::function<bool (const uint32_t, const uint32_t)> CMP);
        bool IsIn(const uint32_t e);
        void Load(uint32_t n);
        void Insert(const uint32_t e);
        void Delete(const uint32_t e);
        uint32_t Top();
        bool IsEmpty();
    private:
        
        void Up(const uint32_t h, const uint32_t e);
        void Down(const uint32_t h, const uint32_t e);

        std::function<bool (const uint32_t, const uint32_t)> HPcmp;
        
        // data members for heap maintenance
        uint32_t n_;
        std::vector<uint32_t> hp_tbl_;
        std::vector<uint32_t> hp_pos_;   
};