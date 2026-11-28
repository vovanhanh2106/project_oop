
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
                "numeric",
                {},
                "B01"
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
                "numeric",
                {},
                "B02"
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
                "keyword",
                {},
                "B03"
            },
            BenchmarkDifficulty::Easy,
            {"file"}
        },

        {
            {
                "B04 - Save and retrieve memory",
        "Save this information into your memory: "
        "The benchmark project code is AI-25120054. "
        "Then search your memory for AI-25120054 "
        "and report the associated project code.",
        "AI-25120054",
        "keyword",
        {},
        "B04"
            },
            BenchmarkDifficulty::Easy,
            {"memory"}
        },

        // =========================
        // MEDIUM: 4 TASKS
        // =========================

        {
            {
        "B05 - Calculate and remember",
        "Calculate ((125 * 4) + (250 / 2)) - 75. "
        "Save the result in memory with label medium_calculation. "
        "Then retrieve that information and return the numerical result.",
        "550",
        "numeric",
        {},
        "B05"
    },
    BenchmarkDifficulty::Medium,
    {"calculator", "memory"}
        },

        {
             {
        "B06 - Read and calculate",
        "Read benchmark_numbers.txt from your workspace. "
        "The file contains integers separated by whitespace. "
        "Calculate the sum of all numbers and return the result.",
        "60",
        "numeric",
        {},
        "B06"
    },
    BenchmarkDifficulty::Medium,
    {"file", "calculator"}
        },

        {
             {
        "B07 - Memory retrieval",
        "Save the following information in memory: "
        "The project uses C++20 and CMake. "
        "Search your memory for C++20. "
        "Report the build system associated with it.",
        "CMake",
        "keyword",
        {},
        "B07"
    },
    BenchmarkDifficulty::Medium,
    {"memory"}
        },

        {
            {
        "B08 - Search and remember C++",
        "Search Wikipedia for the C++ programming language. "
        "Summarize its purpose in one or two sentences. "
        "Save the summary in memory with label cpp_info. "
        "Retrieve cpp_info and include the phrase "
        "programming language in your final answer.",
        "programming language",
        "keyword",
        {},
        "B08"
    },
    BenchmarkDifficulty::Medium,
    {"web_search", "memory"}
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
                "keyword",
                {},
                "B09"
            },
            BenchmarkDifficulty::Hard,
            {"file", "calculator", "memory"}
        },

        {
            
    {
        "B10 - Research, remember and save SQLite",
        "Search for SQLite and find out what it is. "
        "Write a short description into a file named "
        "sqlite_summary.txt inside your workspace. "
        "Save the same description in memory with "
        "label sqlite_info. Then read the file and "
        "retrieve sqlite_info from memory. "
        "Summarize the information in your final answer.",
        "SQLite",
        "keyword",
        {},
        "B10"
    },
    BenchmarkDifficulty::Hard,
    {"web_search", "file", "memory"}
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
