#include <iostream>
#include <iomanip>
#include <memory>
#include <windows.h>

#include "client/ollama_client.h"
#include "agent/agent_loop.h"
#include "agent/skill_loader.h"
#include "agent/loop_detector.h"

#include "tools/tool_registry.h"
#include "tools/calculator_tool.h"
#include "tools/file_tool.h"
#include "tools/exec_tool.h"
#include "tools/web_search_tool.h"
#include "tools/memory_tool.h"

#include "harness/harness_runner.h"
#include "benchmark/benchmark_dataset.h"
#include "benchmark/benchmark_runner.h"

using namespace std;

void printStats(
    const string& title,
    const DifficultyStats& stats
) {
    cout << "\n========== " << title << " ==========\n";

    cout << "Total tasks:       " << stats.totalTasks << '\n';
    cout << "Passed:            " << stats.passedTasks << '\n';
    cout << "Failed:            " << stats.failedTasks << '\n';
    cout << "Execution errors:  " << stats.executionErrors << '\n';

    cout << fixed << setprecision(2);

    cout << "Success rate:      "
         << stats.successRate << "%\n";

    cout << "Average latency:   "
         << stats.averageLatency << " ms\n";

    cout << "Average tokens:    "
         << stats.averageTokens << '\n';
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    try {
        // 1. Khởi tạo LLM
        OllamaClient llm(
            "http://localhost:11434",
            "gemma3",
            0.2,
            512
        );

        // 2. Đăng ký tools
        ToolRegistry toolRegistry;

        toolRegistry.registerTool(
            make_unique<CalculatorTool>()
        );

        toolRegistry.registerTool(
            make_unique<FileTool>()
        );

        toolRegistry.registerTool(
            make_unique<ExecTool>()
        );

        toolRegistry.registerTool(
            make_unique<WebSearchTool>()
        );

        toolRegistry.registerTool(
            make_unique<MemoryTool>()
        );

        // 3. Khởi tạo Agent
        SkillLoader skillLoader("skills");
        LoopDetector loopDetector(3, 5);

        AgentLoop agent(
            llm,
            toolRegistry,
            skillLoader,
            loopDetector,
            8
        );

        // 4. Khởi tạo Harness
        HarnessRunner harness(agent);

        // 5. Khởi tạo Benchmark
        BenchmarkRunner benchmarkRunner(harness);

        // 6. Tạo bộ dữ liệu benchmark
        vector<BenchmarkTask> tasks =
            BenchmarkDataset::createTasks();

        cout << "====================================\n";
        cout << "       AI AGENT BENCHMARK\n";
        cout << "====================================\n";

        cout << "Total benchmark tasks: "
             << tasks.size() << '\n';

        cout << "\nStarting benchmark...\n";

        // 7. Chạy benchmark
        BenchmarkSummary summary =
            benchmarkRunner.run(tasks);

        // 8. In báo cáo
        cout << "\n\n";
        cout << "====================================\n";
        cout << "          BENCHMARK REPORT\n";
        cout << "====================================\n";

        printStats("OVERALL", {
            summary.totalTasks,
            summary.passedTasks,
            summary.failedTasks,
            summary.executionErrors,
            summary.successRate,
            summary.averageLatency,
            summary.averageTokens
        });

        printStats("EASY", summary.easy);
        printStats("MEDIUM", summary.medium);
        printStats("HARD", summary.hard);

        // 9. In kết quả từng task
        cout << "\n========== TASK DETAILS ==========\n";

        for (const auto& result : summary.results) {
            cout << "\nTask: " << result.taskName << '\n';

            cout << "Status: "
                 << (result.evaluation.passed &&
                     !result.executionError
                     ? "PASSED"
                     : "FAILED")
                 << '\n';

            cout << "Score: "
                 << result.evaluation.score << '\n';

            cout << "Latency: "
                 << result.latency_ms << " ms\n";

            if (result.executionError) {
                cout << "Execution error: "
                     << result.actual << '\n';
            }
        }

        // 10. Lưu kết quả
        if (harness.saveResultsToJson("benchmark_results.json")) {
            cout << "\nSaved: benchmark_results.json\n";
        }
        else {
            cout << "\nFailed to save benchmark results.\n";
        }

        cout << "\nBenchmark completed.\n";
    }
    catch (const exception& e) {
        cerr << "\nBenchmark error: "
             << e.what() << '\n';

        return 1;
    }

    return 0;
}