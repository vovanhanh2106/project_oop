
#include <iostream>
#include <string>

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

        // 2. Đăng ký các tool
        ToolRegistry toolRegistry;

        toolRegistry.registerTool(
            std::make_unique<CalculatorTool>()
        );

        toolRegistry.registerTool(
            std::make_unique<FileTool>()
        );

        toolRegistry.registerTool(
            std::make_unique<ExecTool>()
        );

        toolRegistry.registerTool(
            std::make_unique<WebSearchTool>()
        );

        toolRegistry.registerTool(
            std::make_unique<MemoryTool>()
        );

        // 3. Khởi tạo skills và loop detector
        SkillLoader skillLoader("skills");
        LoopDetector loopDetector(3, 5);

        // 4. Khởi tạo AgentLoop
        AgentLoop agent(
            llm,
            toolRegistry,
            skillLoader,
            loopDetector,
            8
        );

        // 5. Vòng lặp nhập câu hỏi
        std::string input;

        std::cout << "AI Agent is ready. Type 'exit' to quit.\n";

        while (true) {
            std::cout << "\nYou: ";

            if (!std::getline(std::cin, input)) {
                break;
            }

            if (input == "exit") {
                break;
            }

            if (input.empty()) {
                continue;
            }

            try {
                std::string response = agent.run(input);
                std::cout << "Agent: " << response << '\n';
            }
            catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << '\n';
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Startup error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
