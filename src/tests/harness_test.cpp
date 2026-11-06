
#include "../client/ollama_client.h"
#include "../agent/agent_loop.h"
#include "../agent/skill_loader.h"
#include "../agent/loop_detector.h"
#include "../tools/tool_registry.h"
#include "../tools/calculator_tool.h"
#include "../harness/harness_runner.h"

#include <iostream>
#include <memory>
#include <vector>

int main() {
    OllamaClient llm(
        "http://localhost:11434",
        "gemma3",
        0.2,
        512
    );

    ToolRegistry registry;
    registry.registerTool(
        std::make_unique<CalculatorTool>()
    );

    SkillLoader skills("skills");
    LoopDetector loopDetector(3, 5);

    AgentLoop agent(
        llm,
        registry,
        skills,
        loopDetector,
        8
    );

    HarnessRunner harness(agent);

    std::vector<HarnessTask> tasks = {
        {
            "Multiplication",
            "Tính 17 nhân 13",
            "221",
            "numeric"
        },
        {
            "Calculator keyword",
            "Tính 17 nhân 13",
            "221",
            "keyword"
        },{
            "Calculator functional",
            "Tính 17 nhân 13",
            "221",
            "functional"
        }, {
            "Calculator functional with extra text",
            "Tính 17 nhân 13 và cộng thêm 5",
            "226",
            "numeric"
        }, {
            "Calculator functional with complex expression",
            "Tính (17 nhân 13) cộng (5 chia 2)",
            "223.5",
            "numeric"
        }, {
            "Calculator functional with parentheses",
            "Tính (17 nhân (13 cộng 5)) chia 2",
            "153",
            "functional"
        }, {
            "Calculator functional with negative numbers",
            "Tính -17 nhân -13 cộng -5",
            "216",
            "functional"
        }, {
            "Calculator functional with decimals",
            "Tính 17.5 nhân 13.2 cộng 5.1",
            "234.6",
            "functional"
        }, {
            "Calculator functional with large numbers",
            "Tính 123456789 nhân 987654321 cộng 123456789",
            "1219326311126352690",
            "functional"
        }, {
            "Calculator functional with division by zero",
            "Tính 17 chia 0",
            "",
            "functional"
        }
    };

    const auto results = harness.runBatch(tasks);

    for (const auto& result : results) {
        std::cout
            << "\nTask: " << result.taskName
            << "\nExpected: " << result.expected
            << "\nActual: " << result.actual
            << "\nPassed: "
            << (result.evaluation.passed ? "YES" : "NO")
            << "\nScore: " << result.evaluation.score
            << "\nLatency: " << result.latency_ms << " ms\n";
    }

    std::cout
        << "\nSuccess rate: "
        << harness.getSuccessRate() * 100.0
        << "%\n";

    if (harness.saveResultsToJson("harness_results.json")) {
        std::cout << "Results saved to harness_results.json\n";
    } else {
        std::cerr << "Failed to save results.\n";
        return 1;
    }

    return 0;
}
