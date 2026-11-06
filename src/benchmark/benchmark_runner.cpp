
#include "benchmark_runner.h"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <utility>

using json = nlohmann::json;

namespace {

// Đọc tổng token từ trajectory JSON của một task.
int extractTokens(const HarnessResult& result) {
    try {
        if (result.trajectoryJson.empty()) {
            return 0;
        }

        const json trajectory =
            json::parse(result.trajectoryJson);

        int total = 0;

        if (!trajectory.contains("steps") ||
            !trajectory["steps"].is_array()) {
            return 0;
        }

        for (const auto& step : trajectory["steps"]) {
            total += step.value("tokens", 0);
        }

        return total;
    }
    catch (const json::exception&) {
        return 0;
    }
}

} // namespace

BenchmarkRunner::BenchmarkRunner(HarnessRunner& harness)
    : harness(harness) {
}

DifficultyStats BenchmarkRunner::calculateStats(
    const std::vector<HarnessResult>& results
) const {
    DifficultyStats stats;

    stats.totalTasks = results.size();

    if (results.empty()) {
        return stats;
    }

    long long totalLatency = 0;
    long long totalTokens = 0;

    for (const auto& result : results) {
        if (result.evaluation.passed && !result.executionError) {
            ++stats.passedTasks;
        } else {
            ++stats.failedTasks;
        }

        if (result.executionError) {
            ++stats.executionErrors;
        }

        totalLatency += result.latency_ms;
        totalTokens += extractTokens(result);
    }

    stats.successRate =
        100.0 * stats.passedTasks / stats.totalTasks;

    stats.averageLatency =
        static_cast<double>(totalLatency) / stats.totalTasks;

    stats.averageTokens =
        static_cast<double>(totalTokens) / stats.totalTasks;

    return stats;
}

BenchmarkSummary BenchmarkRunner::run(
    const std::vector<BenchmarkTask>& tasks
) const {
    BenchmarkSummary summary;

    if (tasks.empty()) {
        return summary;
    }

    // Chuyển BenchmarkTask thành HarnessTask.
    std::vector<HarnessTask> harnessTasks;
    harnessTasks.reserve(tasks.size());

    for (const auto& task : tasks) {
        harnessTasks.push_back(task.task);
    }

    // Harness thực thi toàn bộ task.
    summary.results = harness.runBatch(harnessTasks);

    if (summary.results.size() != tasks.size()) {
        throw std::runtime_error(
            "Benchmark result count does not match task count."
        );
    }

    std::vector<HarnessResult> easyResults;
    std::vector<HarnessResult> mediumResults;
    std::vector<HarnessResult> hardResults;

    // Phân loại kết quả theo độ khó.
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        const auto& result = summary.results[i];

        switch (tasks[i].difficulty) {
            case BenchmarkDifficulty::Easy:
                easyResults.push_back(result);
                break;

            case BenchmarkDifficulty::Medium:
                mediumResults.push_back(result);
                break;

            case BenchmarkDifficulty::Hard:
                hardResults.push_back(result);
                break;
        }
    }

    // Tính thống kê từng nhóm.
    summary.easy = calculateStats(easyResults);
    summary.medium = calculateStats(mediumResults);
    summary.hard = calculateStats(hardResults);

    // Tính thống kê tổng.
    const DifficultyStats overall =
        calculateStats(summary.results);

    summary.totalTasks = overall.totalTasks;
    summary.passedTasks = overall.passedTasks;
    summary.failedTasks = overall.failedTasks;
    summary.executionErrors = overall.executionErrors;

    summary.successRate = overall.successRate;
    summary.averageLatency = overall.averageLatency;
    summary.averageTokens = overall.averageTokens;

    return summary;
}
