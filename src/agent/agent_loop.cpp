
#include "agent_loop.h"

#include <chrono>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

using json = nlohmann::json;

AgentLoop::AgentLoop(
    LLMClient& llm,
    ToolRegistry& toolRegistry,
    SkillLoader& skillLoader,
    LoopDetector& loopDetector,
    std::size_t maxSteps
)
    : llm(llm),
      toolRegistry(toolRegistry),
      skillLoader(skillLoader),
      loopDetector(loopDetector),
      maxSteps(maxSteps) {}

std::string AgentLoop::buildSystemPrompt(
    const std::string& userInput
) const {
    std::string prompt = R"(
You are an AI agent.
You must respond with exactly one JSON object.

To call a tool:
{
  "type": "tool_call",
  "name": "calculator",
  "args": {
    "expression": "17 * 13"
  }
}

To finish:
{
  "type": "final",
  "content": "The answer is 221"
}

Use tools when appropriate.
Do not include Markdown fences around JSON.
)";

    prompt += "\n\n";
    prompt += skillLoader.buildSkillPrompt(userInput);

    return prompt;
}

std::string AgentLoop::extractJson(
    const std::string& response
) const {
    const std::size_t start = response.find('{');
    const std::size_t end = response.rfind('}');

    if (start == std::string::npos ||
        end == std::string::npos ||
        end < start) {
        return "";
    }

    return response.substr(start, end - start + 1);
}

std::string AgentLoop::run(
    const std::string& userInput,
    StepHook stepHook
) {
    loopDetector.reset();

    std::vector<Message> messages;

    messages.push_back({
        "system",
        buildSystemPrompt(userInput)
    });

    messages.push_back({
        "user",
        userInput
    });

    for (std::size_t step = 0; step < maxSteps; ++step) {
        const auto stepStart = std::chrono::steady_clock::now();

        std::string response = llm.chat(messages);
        std::string jsonText = extractJson(response);

        // 1. LLM trả về nội dung không có JSON hợp lệ.
        if (jsonText.empty()) {
            const auto stepEnd = std::chrono::steady_clock::now();

            const auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stepEnd - stepStart
                ).count();

            if (stepHook) {
                stepHook(
                    response,
                    "INVALID_RESPONSE",
                    response,
                    latency,
                    0
                );
            }

            return response;
        }

        json action;

        // 2. Parse JSON.
        try {
            action = json::parse(jsonText);
        } catch (const json::exception& e) {
            const auto stepEnd = std::chrono::steady_clock::now();

            const auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stepEnd - stepStart
                ).count();

            const std::string error =
                std::string("Invalid JSON response: ") + e.what();

            if (stepHook) {
                stepHook(
                    response,
                    "INVALID_JSON",
                    error,
                    latency,
                    0
                );
            }

            return error;
        }

        const std::string type =
            action.value("type", std::string{});

        // 3. Agent hoàn thành nhiệm vụ.
        if (type == "final") {
            const std::string answer =
                action.value("content", std::string{});

            const auto stepEnd = std::chrono::steady_clock::now();

            const auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stepEnd - stepStart
                ).count();

            if (stepHook) {
                stepHook(
                    response,
                    "FINAL",
                    answer,
                    latency,
                    0
                );
            }

            return answer;
        }

        // 4. Action không hợp lệ.
        if (type != "tool_call") {
            const auto stepEnd = std::chrono::steady_clock::now();

            const auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stepEnd - stepStart
                ).count();

            const std::string error =
                "Unknown action type: " + type;

            if (stepHook) {
                stepHook(
                    response,
                    "INVALID_ACTION",
                    error,
                    latency,
                    0
                );
            }

            return error;
        }

        const std::string toolName =
            action.value("name", std::string{});

        const json args =
            action.value("args", json::object());

        // 5. Tool call thiếu tên.
        if (toolName.empty()) {
            const auto stepEnd = std::chrono::steady_clock::now();

            const auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stepEnd - stepStart
                ).count();

            const std::string error =
                "Tool call error: missing tool name.";

            if (stepHook) {
                stepHook(
                    response,
                    "INVALID_TOOL_CALL",
                    error,
                    latency,
                    0
                );
            }

            return error;
        }

        // 6. Kiểm tra tool có được phép sử dụng không.
        if (!toolRegistry.isAllowed(toolName)) {
            const auto stepEnd = std::chrono::steady_clock::now();

            const auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stepEnd - stepStart
                ).count();

            const std::string error =
                "Tool is not allowed: " + toolName;

            if (stepHook) {
                stepHook(
                    response,
                    toolName,
                    error,
                    latency,
                    0
                );
            }

            return error;
        }

        // 7. Lấy tool từ registry.
        auto tool = toolRegistry.getTool(toolName);

        if (tool == nullptr) {
            const auto stepEnd = std::chrono::steady_clock::now();

            const auto latency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stepEnd - stepStart
                ).count();

            const std::string error =
                "Tool not found: " + toolName;

            if (stepHook) {
                stepHook(
                    response,
                    toolName,
                    error,
                    latency,
                    0
                );
            }

            return error;
        }

        // 8. Chuyển args JSON thành chuỗi cho tool.execute().
        std::string toolArgs;

        if (toolName == "calculator" && args.is_object()) {
            toolArgs = args.value("expression", std::string{});
        } else if (args.is_string()) {
            toolArgs = args.get<std::string>();
        } else {
            toolArgs = args.dump();
        }

        // 9. Thực thi tool.
        std::string toolResult;

        try {
            
        // Phát hiện vòng lặp trước khi thực thi tool
        const std::string actionSignature =
            toolName + ":" + args.dump();

        const LoopStatus loopStatus =
            loopDetector.recordAction(actionSignature);

        if (loopStatus == LoopStatus::Critical) {
            const std::string error =
                "Agent stopped: critical loop detected.";

            const auto stepEnd =
                std::chrono::steady_clock::now();

            const auto latency =
                std::chrono::duration_cast<
                    std::chrono::milliseconds
                >(stepEnd - stepStart).count();

            if (stepHook) {
                stepHook(
                    response,
                    "LOOP_DETECTED",
                    error,
                    latency,
                    0
                );
            }

            return error;
        }

            toolResult = tool->execute(toolArgs);
        } catch (const std::exception& e) {
            toolResult =
                std::string("Tool execution error: ") + e.what();
        }

        const auto stepEnd = std::chrono::steady_clock::now();

        const auto latency =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                stepEnd - stepStart
            ).count();

        // 10. Báo cho hook biết bước này đã hoàn tất.
        if (stepHook) {
            stepHook(
                response,
                toolName,
                toolResult,
                latency,
                0
            );
        }

        // 11. Đưa kết quả tool vào lịch sử hội thoại để LLM xử lý tiếp.
        messages.push_back({
            "assistant",
            response
        });

        messages.push_back({
            "user",
            "Tool result for " + toolName + ": " + toolResult +
            "\nContinue the task. Respond with one JSON object."
        });
    }

    // 12. Agent đã đạt giới hạn số bước.
    const std::string error =
        "Agent stopped: maximum number of steps reached.";

    if (stepHook) {
        stepHook(
            "",
            "MAX_STEPS",
            error,
            0,
            0
        );
    }

    return error;
}
