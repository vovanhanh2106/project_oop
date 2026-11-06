
#include <iostream>
#include <string>
#include <memory>
#include <cstdlib>
#include <stdexcept>

#include <windows.h>

#include "client/llm_client.h"
#include "client/ollama_client.h"
#include "client/remote_llm_client.h"

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
        // 1. Chọn LLM Client
        std::unique_ptr<LLMClient> llm;

        const char* modeEnv = std::getenv("AGENT_MODE");
        std::string mode = modeEnv ? modeEnv : "local";

        if (mode == "remote") {
            const char* url = std::getenv("AGENT_REMOTE_URL");
            const char* token = std::getenv("AGENT_API_TOKEN");

            if (!url || !token) {
                throw std::runtime_error(
                    "Missing AGENT_REMOTE_URL or AGENT_API_TOKEN"
                );
            }

            llm = std::make_unique<RemoteLLMClient>(
                url,
                "gemma3:4b",
                token,
                0.2,
                512
            );

            std::cout << "Using Remote LLM (Google Colab)\n";
        }
        else if (mode == "local") {
            llm = std::make_unique<OllamaClient>(
                "http://localhost:11434",
                "gemma3",
                0.2,
                512
            );

            std::cout << "Using Local LLM (Ollama)\n";
        }
        else {
            throw std::runtime_error(
                "Invalid AGENT_MODE. Use local or remote."
            );
        }

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
            *llm,
            toolRegistry,
            skillLoader,
            loopDetector,
            8
        );

        // 5. Vòng lặp nhập câu hỏi
        std::string input;

        std::cout
            << "AI Agent is ready. Type 'exit' to quit.\n";

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
