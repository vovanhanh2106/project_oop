
#pragma once

#include "benchmark_dataset.h"
#include "../harness/harness_runner.h"

#include <cstddef>
#include <vector>

struct DifficultyStats {
    std::size_t totalTasks = 0;
    std::size_t passedTasks = 0;
    std::size_t failedTasks = 0;
    std::size_t executionErrors = 0;

    double successRate = 0.0;
    double averageLatency = 0.0;
    double averageTokens = 0.0;
};

struct BenchmarkSummary {
    std::size_t totalTasks = 0;
    std::size_t passedTasks = 0;
    std::size_t failedTasks = 0;
    std::size_t executionErrors = 0;

    double successRate = 0.0;
    double averageLatency = 0.0;
    double averageTokens = 0.0;

    DifficultyStats easy;
    DifficultyStats medium;
    DifficultyStats hard;

    std::vector<HarnessResult> results;
};

class BenchmarkRunner {
private:
    HarnessRunner& harness;

    DifficultyStats calculateStats(
        const std::vector<HarnessResult>& results
    ) const;

public:
    explicit BenchmarkRunner(HarnessRunner& harness);

    BenchmarkSummary run(
        const std::vector<BenchmarkTask>& tasks
    ) const;
};
