
#include "benchmark_dataset.h"

std::vector<BenchmarkTask> BenchmarkDataset::createTasks() {
    std::vector<BenchmarkTask> tasks = {

        // =========================
        // EASY: 4 TASKS
        // =========================

        {
            {
                "B01 - Multiplication",
                "Calculate 17 multiplied by 13. "
                "Return the numerical result.",
                "221",
                "numeric"
            },
            BenchmarkDifficulty::Easy,
            {"calculator"}
        },

        {
            {
                "B02 - Parenthesized expression",
                "Calculate (25 + 15) * 3 - 12. "
                "Return the numerical result.",
                "108",
                "numeric"
            },
            BenchmarkDifficulty::Easy,
            {"calculator"}
        },

        {
            {
                "B03 - Read file",
                "Read the file benchmark_info.txt "
                "inside your workspace and tell me "
                "the exact code written in it.",
                "AGENT-BENCHMARK-2026",
                "keyword"
            },
            BenchmarkDifficulty::Easy,
            {"file"}
        },

        {
            {
                "B04 - Save memory",
                "Save this information into your memory: "
                "The benchmark project code is AI-25120054. "
                "Confirm when it has been saved.",
                "saved",
                "keyword"
            },
            BenchmarkDifficulty::Easy,
            {"memory"}
        },

        // =========================
        // MEDIUM: 4 TASKS
        // =========================

        {
            {
                "B05 - Complex calculation",
                "Calculate ((125 * 4) + (250 / 2)) - 75. "
                "Return the numerical result.",
                "550",
                "numeric"
            },
            BenchmarkDifficulty::Medium,
            {"calculator"}
        },

        {
            {
                "B06 - Read and calculate",
                "Read benchmark_numbers.txt from your "
                "workspace. Calculate the sum of all "
                "numbers in the file.",
                "60",
                "numeric"
            },
            BenchmarkDifficulty::Medium,
            {"file", "calculator"}
        },

        {
            {
                "B07 - Memory retrieval",
                "Save the following information in memory: "
                "The project uses C++20 and CMake. "
                "Then search your memory for C++20 and "
                "report the associated build system.",
                "CMake",
                "keyword"
            },
            BenchmarkDifficulty::Medium,
            {"memory"}
        },

        {
            {
                "B08 - Wikipedia search",
                "Search Wikipedia for the C++ programming "
                "language and summarize what it is. "
                "Include the phrase programming language "
                "in your answer.",
                "programming language",
                "keyword"
            },
            BenchmarkDifficulty::Medium,
            {"web_search"}
        },

        // =========================
        // HARD: 2 TASKS
        // =========================

        {
            {
                "B09 - File calculation and memory",
                "Read benchmark_numbers.txt from your "
                "workspace. Calculate the sum of all "
                "numbers. Save the result in memory "
                "with label benchmark_sum. Finally, "
                "confirm the saved result.",
                "60",
                "keyword"
            },
            BenchmarkDifficulty::Hard,
            {"file", "calculator", "memory"}
        },

        {
            {
                "B10 - Web search and memory",
                "Search Wikipedia for SQLite. Find out "
                "what SQLite is, save a short description "
                "in memory with label sqlite_info, "
                "then search your memory for sqlite_info "
                "and summarize the saved information.",
                "SQLite",
                "keyword"
            },
            BenchmarkDifficulty::Hard,
            {"web_search", "memory"}
        }
    };

    return tasks;
}

std::string BenchmarkDataset::difficultyToString(
    BenchmarkDifficulty difficulty
) {
    switch (difficulty) {
        case BenchmarkDifficulty::Easy:
            return "Easy";

        case BenchmarkDifficulty::Medium:
            return "Medium";

        case BenchmarkDifficulty::Hard:
            return "Hard";

        default:
            return "Unknown";
    }
}
