
#pragma once

#include "../harness/harness_runner.h"

#include <string>
#include <vector>

enum class BenchmarkDifficulty {
    Easy,
    Medium,
    Hard
};

struct BenchmarkTask {
    HarnessTask task;

    BenchmarkDifficulty difficulty;

    // Các tool mà task dự kiến cần sử dụng.
    std::vector<std::string> requiredTools;
};

class BenchmarkDataset {
public:
    static std::vector<BenchmarkTask> createTasks();

    static std::string difficultyToString(
        BenchmarkDifficulty difficulty
    );
};
