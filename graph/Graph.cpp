#include "Graph.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <random>
#include <set>
#include <stdexcept>

namespace
{
    constexpr std::size_t kRemovalEdgeCount = 10;
}

Graph::Graph(const std::string &graph_filename)
{
    std::ifstream input(graph_filename);
    if (!input)
        throw std::runtime_error("Failed to open graph file: " + graph_filename);

    std::printf("[Graph] Loading %s.\n", graph_filename.c_str());

    std::set<Edge> unique_edges;
    VertexId source = 0;
    VertexId target = 0;
    while (input >> source >> target)
    {
        ++edge_count_;
        unique_edges.emplace(std::min(source, target),
                             std::max(source, target));
        vertex_count_ = std::max(vertex_count_, std::max(source, target));
    }

    SelectRemovalEdges(EdgeList(unique_edges.begin(), unique_edges.end()));

    ++vertex_count_;
    adjacency_list_.resize(vertex_count_);
    degrees_.resize(vertex_count_);
    for (const Edge &edge : unique_edges)
    {
        adjacency_list_[edge.first].push_back(edge.second);
        adjacency_list_[edge.second].push_back(edge.first);
    }

    CalculateDegrees();
    std::printf("[Graph] Loaded %u vertices and %u edges.\n",
                vertex_count_, edge_count_);
    std::fflush(stdout);
}

void Graph::InsertEdge(VertexId source, VertexId target)
{
    ++degrees_[source];
    adjacency_list_[source].push_back(target);
    ++degrees_[target];
    adjacency_list_[target].push_back(source);
    ++edge_count_;
}

void Graph::RemoveEdge(VertexId source, VertexId target)
{
    --degrees_[source];
    RemoveNeighbor(source, target);
    --degrees_[target];
    RemoveNeighbor(target, source);
    --edge_count_;
}

uint32_t Graph::GetVertexCount() const noexcept
{
    return vertex_count_;
}

uint32_t Graph::GetEdgeCount() const noexcept
{
    return edge_count_;
}

const std::vector<uint32_t> &Graph::GetDegrees() const noexcept
{
    return degrees_;
}

const Graph::EdgeList &Graph::GetRemovalEdges() const noexcept
{
    return removal_edges_;
}

const Graph::NeighborList &Graph::operator[](VertexId vertex_id) const
{
    return adjacency_list_[vertex_id];
}

void Graph::CalculateDegrees()
{
    for (VertexId vertex_id = 0; vertex_id < vertex_count_; ++vertex_id)
    {
        degrees_[vertex_id] =
            static_cast<uint32_t>(adjacency_list_[vertex_id].size());
    }
}

void Graph::SelectRemovalEdges(EdgeList edges)
{
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::shuffle(edges.begin(), edges.end(), generator);

    const std::size_t selected_count =
        std::min(kRemovalEdgeCount, edges.size());
    removal_edges_.assign(edges.begin(),
                          edges.begin() + selected_count);
}

void Graph::RemoveNeighbor(VertexId source, VertexId target)
{
    NeighborList &neighbors = adjacency_list_[source];
    const auto position = std::find(neighbors.begin(), neighbors.end(), target);
    if (position == neighbors.end())
        return;

    *position = neighbors.back();
    neighbors.pop_back();
}
