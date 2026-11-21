
#include "agent_loop.h"
#include <iostream>
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


std::string AgentLoop::buildSystemPrompt(const std::string& userInput) const {
    std::string prompt = R"(
You are an AI agent that can use tools to complete tasks.

You MUST respond with exactly one JSON object.

There are only TWO valid response types.

1. Tool call:
{
    "type": "tool_call",
    "name": "EXACT_TOOL_NAME",
    "args": {}
}

2. Final answer:
{
    "type": "final",
    "content": "Your final answer"
}

GENERAL RULES:

- Only use tools listed in the AVAILABLE TOOLS section.
- Never invent or guess tool names.
- Tool names are case-sensitive.
- Follow the exact argument format described by each tool.
- Do not use unsupported action types.
- Do not claim an operation succeeded before receiving its tool result.
- If a tool returns an error, explain the error or try another valid approach.
- For mathematical expressions, preserve all parentheses exactly.
- Do not simplify or modify mathematical expressions before sending them to the calculator.
- Do not wrap JSON in Markdown code fences.
- Do not include text outside the JSON object.

MULTI-TOOL WORKFLOW RULES:

1. Analyze the original user request and identify every required subtask.

2. Select only the tools necessary to complete those subtasks.

3. Execute each required tool and preserve its result.

4. Do not call the same tool repeatedly unless necessary.

5. Do not use web_search when local tools or existing information
   are sufficient.

6. Complete every requested subtask before returning a final answer.

7. Preserve all previous tool results when deciding the next action.

8. Never invent tool results or assume an operation succeeded.

9. Before returning a final response, check whether every
   requested subtask has been completed.

10. The final response MUST include all relevant results
    explicitly requested by the user.

FILE TOOL RULES:

- Carefully distinguish between reading and writing files.
- If the user asks to read, inspect, display, or summarize
  an existing file, use the file reading operation.
- If the user asks to create or write a file, use the
  file writing operation.
- NEVER create, overwrite, append to, or modify a file
  when the user only asks to read it.
- Never substitute a file writing operation for a reading operation.
- Preserve the exact file path provided by the user.
- Use the actual file tool result when answering questions
  about file contents.

CALCULATOR RULES:

- Use the calculator tool for mathematical calculations.
- Pass the original mathematical expression without modification.
- Include the calculator result in the final response.

MEMORY TOOL RULES:

- Memory is a TOOL, not a response type.
- NEVER use "memory" as the value of the top-level "type".
- The top-level "type" MUST always be "tool_call" or "final".
- The memory operation must be specified inside args.action.

To save memory, use:
{
    "type": "tool_call",
    "name": "memory",
    "args": {
        "action": "save",
        "label": "topic",
        "content": "information"
    }
}

To search memory, use:
{
    "type": "tool_call",
    "name": "memory",
    "args": {
        "action": "search",
        "query": "keyword"
    }
}

Additional memory rules:

- When the user asks to remember multiple related facts,
  save all facts together in one memory entry.
- Do not silently omit requested information.
- Use descriptive labels.
- When retrieving multiple facts, search using a keyword
  matching the saved label or content.
- If a search returns no matching result, consider another
  relevant search query before giving up.
- Inspect the returned tool result before generating the final answer.
- Never claim information was saved unless the tool confirms success.
- Only search memory when the user explicitly requests
  memory retrieval.

FINAL RESPONSE RULES:

- Include relevant results from all tools used.
- Do not omit any explicitly requested result.
- Do not claim that a task is complete if some subtasks failed.
- Present the results clearly and separately when the user
  requests multiple outputs.
)";

    prompt += "\n\nAVAILABLE TOOLS:\n";
    prompt += buildToolDescriptions();

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
std::string AgentLoop::buildToolDescriptions() const {
    std::string result;

    const auto tools = toolRegistry.getAllTools();

    for (const Tool* tool : tools) {
        if (tool == nullptr) {
            continue;
        }

        result += "\nTool name: " + tool->getName();
        result += "\nDescription: " + tool->getDescription();
        result += "\n";
    }

    return result;
}
std::string AgentLoop::run(
    const std::string& userInput,
    StepHook stepHook
) {
    int llmCalls = 0;
int toolCalls = 0;
// int toolLatency = 0;
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

// Đo thời gian riêng của LLM
const auto llmStart = std::chrono::steady_clock::now();
++llmCalls;
LLMResponse llmResponse = llm.chat(messages);

const auto llmEnd = std::chrono::steady_clock::now();

const auto llmLatency =
    std::chrono::duration_cast<std::chrono::milliseconds>(
        llmEnd - llmStart
    ).count();

std::string response = llmResponse.content;

int tokens = llmResponse.totalTokens();
std::cout << "[DEBUG] Prompt tokens: "
          << llmResponse.promptTokens << '\n';

std::cout << "[DEBUG] Completion tokens: "
          << llmResponse.completionTokens << '\n';

std::cout << "[DEBUG] Total tokens: "
          << tokens << '\n';
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
                    tokens
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
                    tokens
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
                    tokens
                );
            }
            std::cout << "\n========== AGENT SUMMARY ==========\n";
std::cout << "LLM calls: " << llmCalls << '\n';
std::cout << "Tool calls: " << toolCalls << '\n';
std::cout << "===================================\n";
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
                    tokens
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
                    tokens
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
                    tokens
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
                    tokens
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
                    tokens
                );
            }

            return error;
        }

          const auto toolStart = std::chrono::steady_clock::now();
++toolCalls;
toolResult = tool->execute(toolArgs);

const auto toolEnd = std::chrono::steady_clock::now();

const auto toolLatency =std::chrono::duration_cast<std::chrono::milliseconds>(
        toolEnd - toolStart
    ).count();
        } catch (const std::exception& e) {
            toolResult =
                std::string("Tool execution error: ") + e.what();
        }

        const auto stepEnd = std::chrono::steady_clock::now();

        const auto latency =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                stepEnd - stepStart
            ).count();
        std::cout << "\n========== AGENT PROFILING ==========\n";

std::cout << "Step: " << step + 1 << '\n';

std::cout << "Tool: " << toolName << '\n';

std::cout << "LLM latency: "
          << llmLatency << " ms\n";

// std::cout << "Tool latency: "
//           << toolLatency << " ms\n";

std::cout << "Total step latency: "
          << latency << " ms\n";

std::cout << "Tokens: "
          << tokens << '\n';

std::cout << "=====================================\n";
        // 10. Báo cho hook biết bước này đã hoàn tất.
        if (stepHook) {
            stepHook(
                response,
                toolName,
                toolResult,
                latency,
                tokens
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
