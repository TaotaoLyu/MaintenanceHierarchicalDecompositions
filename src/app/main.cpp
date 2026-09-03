#include <cstdio>
#include <cstdlib>
#include <string>

#include "AlgoProfile.hpp"
#include "Daemon.hpp"
#include "Tree.hpp"

namespace
{
    constexpr int kDefaultThreadCount = 32;

    void PrintUsage(const char *executable_name)
    {
        std::printf("Usage: %s <graph_file>\n", executable_name);
        std::printf("Example: %s Graph/Google.txt\n", executable_name);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }

    const std::string graph_filename = argv[1];
    // App::StartDaemon("exp13.txt");

    Tree::Tree tree(graph_filename, kDefaultThreadCount);

    std::printf("[Build] Starting tree construction.\n");
    std::fflush(stdout);
    const hclock::time_point build_start = hclock::now();
    tree.Build();
    const hclock::time_point build_end = hclock::now();
    std::printf("[Build] Completed in %.3f s.\n",
                Tree::get_time_s(build_start, build_end));

    const Graph::EdgeList &update_edges = tree.g_.GetRemovalEdges();
    std::printf("[Remove] Starting update for %zu edges.\n",
                update_edges.size());
    std::fflush(stdout);
    tree.RemoveEdge(update_edges);
    const bool removal_is_valid = tree.check();
    std::printf("[Remove] Completed. Validation: %s.\n",
                removal_is_valid ? "passed" : "failed");

    std::printf("[Insert] Starting update for %zu edges.\n",
                update_edges.size());
    std::fflush(stdout);
    tree.InsertEdge(update_edges);
    const bool insertion_is_valid = tree.check();
    std::printf("[Insert] Completed. Validation: %s.\n",
                insertion_is_valid ? "passed" : "failed");

    AlgoProfile::GetInstance()->Print();

    if (!removal_is_valid || !insertion_is_valid)
    {
        std::fprintf(stderr, "[Result] Validation failed.\n");
        return EXIT_FAILURE;
    }

    std::printf("[Result] Completed successfully.\n");
    return EXIT_SUCCESS;
}
