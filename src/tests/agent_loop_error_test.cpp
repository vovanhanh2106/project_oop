
#include "../agent/agent_loop.h"
#include "../tools/tool_registry.h"

#include "mock_llm_client.h"
#include "mock_tool.h"
#include "mock_throw_tool.h"

#include <iostream>
#include <memory>
#include <string>

using namespace std;

int passedTests = 0;
int totalTests = 0;

void check(const string& name, bool condition) {
    ++totalTests;

    cout << name << ": "
         << (condition ? "PASS" : "FAIL") << '\n';

    if (condition) {
        ++passedTests;
    }
}

string makeToolCall(
    const string& toolName,
    const string& value = "A"
) {
    return R"({"type":"tool_call","name":")"
        + toolName
        + R"(","args":{"value":")"
        + value
        + R"("}})";
}

string makeFinal(const string& content) {
    return R"({"type":"final","content":")"
        + content + R"("})";
}

int main() {

    // =========================================
    // TEST 1: TOOL BỊ DENY
    // =========================================
    {
        cout << "\n===== TEST 1: DENIED TOOL =====\n";

        MockLLMClient llm({
            makeToolCall("mock_tool")
        });

        ToolRegistry registry;

        auto tool = make_unique<MockTool>();
        MockTool* toolPtr = tool.get();

        registry.registerTool(move(tool));
        registry.denyTool("mock_tool");

        SkillLoader skillLoader;
        LoopDetector detector;

        AgentLoop agent(
            llm, registry, skillLoader, detector, 8
        );

        int deniedEvents = 0;

        auto hook = [&](const string& thought,
                        const string& action,
                        const string& result,
                        long long latency,
                        int tokens) {
            if (result.find("Tool is not allowed:") == 0) {
                ++deniedEvents;
            }
        };

        string result = agent.run("Test denied tool", hook);

        check(
            "Agent reports permission error",
            result == "Tool is not allowed: mock_tool"
        );

        check(
            "Denied tool was not executed",
            toolPtr->getExecutionCount() == 0
        );

        check(
            "Denial was recorded by StepHook",
            deniedEvents == 1
        );
    }

    // =========================================
    // TEST 2: TOOL NÉM EXCEPTION
    // =========================================
    {
        cout << "\n===== TEST 2: TOOL EXCEPTION =====\n";

        MockLLMClient llm({
            makeToolCall("throwing_tool"),
            makeFinal("Recovered successfully")
        });

        ToolRegistry registry;

        registry.registerTool(
            make_unique<MockThrowingTool>()
        );

        registry.allowTool("throwing_tool");

        SkillLoader skillLoader;
        LoopDetector detector;

        AgentLoop agent(
            llm, registry, skillLoader, detector, 8
        );

        int errorEvents = 0;

        auto hook = [&](const string& thought,
                        const string& action,
                        const string& result,
                        long long latency,
                        int tokens) {
            if (result.find("Tool execution error:") == 0) {
                ++errorEvents;
            }
        };

        string result = agent.run("Test tool exception", hook);

        check(
            "Agent continues after tool exception",
            result == "Recovered successfully"
        );

        check(
            "LLM was called twice",
            llm.getCallCount() == 2
        );

        check(
            "StepHook recorded tool exception",
            errorEvents == 1
        );
    }

    // =========================================
    // TEST 3: LLM TRẢ JSON KHÔNG HỢP LỆ
    // =========================================
    {
        cout << "\n===== TEST 3: INVALID JSON =====\n";

        MockLLMClient llm({
            R"({"type":"tool_call","name":})"
        });

        ToolRegistry registry;

        SkillLoader skillLoader;
        LoopDetector detector;

        AgentLoop agent(
            llm, registry, skillLoader, detector, 8
        );

        int invalidJsonEvents = 0;

        auto hook = [&](const string& thought,
                        const string& action,
                        const string& result,
                        long long latency,
                        int tokens) {
            if (action == "INVALID_JSON") {
                ++invalidJsonEvents;
            }
        };

        string result = agent.run("Test invalid JSON", hook);

        check(
            "Agent reports invalid JSON",
            result.find("Invalid JSON response:") == 0
        );

        check(
            "Invalid JSON event recorded once",
            invalidJsonEvents == 1
        );

        check(
            "LLM called only once",
            llm.getCallCount() == 1
        );
    }
cout << "\n========== FINAL SUMMARY ==========\n";
cout << "Passed: " << passedTests
     << "/" << totalTests << '\n';
cout << "Failed: " << totalTests - passedTests << '\n';
cout << "===================================\n";

return passedTests == totalTests ? 0 : 1;
}
