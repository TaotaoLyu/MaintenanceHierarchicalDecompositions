#pragma once

#include <string>

namespace App
{
    void RunBatchSizeBenchmark(const std::string &graph_filename);
    void RunOptimizationBenchmark(const std::string &graph_filename);
    void RunThreadScalingBenchmark(const std::string &graph_filename);
}
