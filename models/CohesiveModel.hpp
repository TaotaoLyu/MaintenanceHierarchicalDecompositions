#pragma once

#include <cstdint>
#include <vector>

#include "Graph.hpp"

class CohesiveModel
{
public:
    virtual ~CohesiveModel() = default;

    virtual void Init() = 0;
    virtual void Insert(const EdgListT &edges) = 0;
    virtual void Remove(const EdgListT &edges) = 0;

    virtual uint32_t GetMaxLevel() = 0;
    virtual std::vector<uint32_t> &GetLevels() = 0;

    int32_t operator[](uint32_t vertex)
    {
        return static_cast<int32_t>(GetLevels()[vertex]);
    }
};
