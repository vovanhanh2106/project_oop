
#include <iostream>
#include <string>
#include <memory>
#include <cstdlib>
#include <stdexcept>

#include <vector>

#include <limits>
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

void configureToolPermissions(ToolRegistry& registry) {
    const std::vector<std::string> toolNames = {
        "calculator",
        "file",
        "web_search",
        "memory",
        "exec"
    };

    const std::vector<std::string> displayNames = {
        "Calculator",
        "File",
        "Web Search",
        "Memory",
        "Exec"
    };

    // Ban đầu cho phép tất cả tool.
    for (const auto& name : toolNames) {
        registry.allowTool(name);
    }

    while (true) {
        std::cout << "\n========================================\n";
        std::cout << "       AI AGENT - TOOL PERMISSIONS\n";
        std::cout << "========================================\n\n";

        for (size_t i = 0; i < toolNames.size(); ++i) {
            std::cout << "[" << i + 1 << "] "
                      << displayNames[i] << "\t["
                      << (registry.isAllowed(toolNames[i])
                              ? "ENABLED"
                              : "DISABLED")
                      << "]\n";
        }

        std::cout << "\nEnter tool number to toggle.\n";
        std::cout << "Enter 0 to finish configuration.\n";
        std::cout << "\nYour choice: ";

        int choice;

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n'
            );

            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (choice == 0) {
            break;
        }

        if (choice < 1 ||
            choice > static_cast<int>(toolNames.size())) {
            std::cout << "Invalid tool number.\n";
            continue;
        }

        const std::string& selected =
            toolNames[choice - 1];

        if (registry.isAllowed(selected)) {
            registry.denyTool(selected);
            std::cout << displayNames[choice - 1]
                      << " is now DISABLED.\n";
        } else {
            registry.allowTool(selected);
            std::cout << displayNames[choice - 1]
                      << " is now ENABLED.\n";
        }
    }

    std::cout << "\nPermission configuration completed.\n";
}

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
                "gemma3:4b",
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
configureToolPermissions(toolRegistry);
        // 3. Khởi tạo skills và loop detector
        SkillLoader skillLoader("skills");
skillLoader.loadSkills();

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
