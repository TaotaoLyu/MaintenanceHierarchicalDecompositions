#include "Benchmarks.hpp"

#include <cstddef>
#include <cstdio>

#include "Tree.hpp"

namespace
{
    constexpr int kDefaultThreadCount = 32;
    constexpr std::size_t kUpdateEdgeCount = 10;

    void BuildAndReport(Tree::Tree &tree)
    {
        std::printf("[Build] Starting tree construction.\n");
        std::fflush(stdout);

        const hclock::time_point start_time = hclock::now();
        tree.Build();
        const hclock::time_point end_time = hclock::now();

        std::printf("[Build] Completed in %.3f s.\n",
                    Tree::get_time_s(start_time, end_time));
        std::fflush(stdout);
    }

    bool SelectEdges(Tree::Tree &tree, std::size_t edge_count,
                     Graph::EdgeList &selected_edges, const char *benchmark_name)
    {
        const Graph::EdgeList &candidate_edges = tree.g_.GetRemovalEdges();
        if (candidate_edges.size() < edge_count)
        {
            std::printf("[%s] Skipped %zu edges: only %zu available.\n",
                        benchmark_name, edge_count, candidate_edges.size());
            return false;
        }

        selected_edges.assign(candidate_edges.begin(),
                              candidate_edges.begin() + edge_count);
        return true;
    }
}

namespace App
{
    void RunBatchSizeBenchmark(const std::string &graph_filename)
    {
        Tree::Tree tree(graph_filename, kDefaultThreadCount);
        BuildAndReport(tree);

        const std::size_t batch_sizes[] = {1, 4, 16, 64, 256, 1024, 10};
        for (const std::size_t edge_count : batch_sizes)
        {
            Graph::EdgeList edges;
            if (!SelectEdges(tree, edge_count, edges, "Batch Size"))
                continue;

            const hclock::time_point remove_start = hclock::now();
            tree.RemoveEdge(edges);
            const hclock::time_point remove_end = hclock::now();
            tree.InsertEdge(edges);
            const hclock::time_point insert_end = hclock::now();

            std::printf("[Batch Size] Edges: %zu | Remove: %.3f s | Insert: %.3f s\n",
                        edge_count,
                        Tree::get_time_s(remove_start, remove_end),
                        Tree::get_time_s(remove_end, insert_end));
            std::fflush(stdout);
        }
    }

    void RunOptimizationBenchmark(const std::string &graph_filename)
    {
        Tree::Tree tree(graph_filename, kDefaultThreadCount);
        BuildAndReport(tree);

        Graph::EdgeList edges;
        if (!SelectEdges(tree, kUpdateEdgeCount, edges, "Optimization"))
            return;

        tree.RemoveEdge(edges);
        tree.InsertEdge(edges);

        const hclock::time_point baseline_remove_start = hclock::now();
        tree.RemoveEdge(edges);
        const hclock::time_point baseline_remove_end = hclock::now();
        tree.InsertEdge(edges);
        const hclock::time_point baseline_insert_end = hclock::now();

        tree.RemoveEdge(edges);
        const hclock::time_point no_compression_start = hclock::now();
        tree.InsertEdge(edges, false, true);
        const hclock::time_point no_compression_end = hclock::now();

        tree.RemoveEdge(edges);
        const hclock::time_point no_deferred_propagation_start = hclock::now();
        tree.InsertEdge(edges, true, false);
        const hclock::time_point no_deferred_propagation_end = hclock::now();

        const hclock::time_point unoptimized_remove_start = hclock::now();
        tree.RemoveEdge(edges, false);
        const hclock::time_point unoptimized_remove_end = hclock::now();
        tree.InsertEdge(edges);

        const hclock::time_point second_remove_start = hclock::now();
        tree.RemoveEdge(edges);
        const hclock::time_point second_remove_end = hclock::now();
        tree.InsertEdge(edges);
        const hclock::time_point second_insert_end = hclock::now();

        std::printf("[Optimization] Insert baseline: %.3f s | No virtual-edge compression: %.3f s | No deferred propagation: %.3f s\n",
                    Tree::get_time_s(baseline_remove_end, baseline_insert_end),
                    Tree::get_time_s(no_compression_start, no_compression_end),
                    Tree::get_time_s(no_deferred_propagation_start,
                                     no_deferred_propagation_end));
        std::printf("[Optimization] Remove baseline: %.3f s | Optimizations disabled: %.3f s\n",
                    Tree::get_time_s(baseline_remove_start,
                                     baseline_remove_end),
                    Tree::get_time_s(unoptimized_remove_start,
                                     unoptimized_remove_end));
        std::printf("[Optimization] Verification run | Remove: %.3f s | Insert: %.3f s\n",
                    Tree::get_time_s(second_remove_start, second_remove_end),
                    Tree::get_time_s(second_remove_end, second_insert_end));
        std::fflush(stdout);
    }

    void RunThreadScalingBenchmark(const std::string &graph_filename)
    {
        Tree::Tree tree(graph_filename, kDefaultThreadCount);
        BuildAndReport(tree);

        Graph::EdgeList edges;
        if (!SelectEdges(tree, kUpdateEdgeCount, edges, "Thread Scaling"))
            return;

        for (int thread_count = 1; thread_count <= kDefaultThreadCount;
             thread_count *= 2)
        {
            tree.SetThreadNum(thread_count);

            const hclock::time_point remove_start = hclock::now();
            tree.RemoveEdge(edges);
            const hclock::time_point remove_end = hclock::now();
            tree.InsertEdge(edges);
            const hclock::time_point insert_end = hclock::now();

            std::printf("[Thread Scaling] Threads: %d | Remove: %.3f s | Insert: %.3f s\n",
                        thread_count,
                        Tree::get_time_s(remove_start, remove_end),
                        Tree::get_time_s(remove_end, insert_end));
            std::fflush(stdout);
        }
    }
}
