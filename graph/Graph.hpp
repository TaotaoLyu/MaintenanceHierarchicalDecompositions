#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

class Graph
{
public:
    using VertexId = uint32_t;
    using Edge = std::pair<VertexId, VertexId>;
    using EdgeList = std::vector<Edge>;
    using NeighborList = std::vector<VertexId>;

    explicit Graph(const std::string &graph_filename);

    void InsertEdge(VertexId source, VertexId target);
    void RemoveEdge(VertexId source, VertexId target);

    uint32_t GetVertexCount() const noexcept;
    uint32_t GetEdgeCount() const noexcept;
    const std::vector<uint32_t> &GetDegrees() const noexcept;
    const EdgeList &GetRemovalEdges() const noexcept;

    const NeighborList &operator[](VertexId vertex_id) const;

private:
    void CalculateDegrees();
    void SelectRemovalEdges(EdgeList edges);
    void RemoveNeighbor(VertexId source, VertexId target);

    std::vector<NeighborList> adjacency_list_;
    std::vector<uint32_t> degrees_;
    uint32_t vertex_count_ = 0;
    uint32_t edge_count_ = 0;
    EdgeList removal_edges_;
};
