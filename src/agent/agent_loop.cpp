
#include "agent_loop.h"

#include <iostream>
#include <chrono>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

using json = nlohmann::json;

namespace {

// Tách nhiều JSON object độc lập trong cùng một response.
// Có xử lý dấu ngoặc nhọn nằm bên trong JSON string.
std::vector<std::string> extractJsonObjects(
    const std::string& response
) {
    std::vector<std::string> objects;

    std::size_t start = std::string::npos;
    int depth = 0;

    bool inString = false;
    bool escaped = false;

    for (std::size_t i = 0; i < response.size(); ++i) {
        const char c = response[i];

        if (inString) {
            if (escaped) {
                escaped = false;
            }
            else if (c == '\\') {
                escaped = true;
            }
            else if (c == '"') {
                inString = false;
            }

            continue;
        }

        if (c == '"') {
            inString = true;
            continue;
        }

        if (c == '{') {
            if (depth == 0) {
                start = i;
            }

            ++depth;
        }
        else if (c == '}') {
            --depth;

            if (depth < 0) {
                return {};
            }

            if (depth == 0 && start != std::string::npos) {
                objects.push_back(
                    response.substr(start, i - start + 1)
                );

                start = std::string::npos;
            }
        }
    }

    // JSON object chưa đóng hoàn chỉnh.
    if (depth != 0 || inString) {
        return {};
    }

    return objects;
}

} // namespace


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
You are an AI agent that can use tools to complete tasks.

You may respond with ONE OR MORE independent JSON objects.
Each JSON object must be valid and complete.

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

MULTIPLE TOOL CALLS:

- You may return multiple independent tool-call JSON objects
  in one response.
- Separate each JSON object with a newline.
- Every object must be a complete valid JSON object.
- Do not wrap JSON objects in Markdown code fences.
- Do not put explanatory text between JSON objects.
- Do not combine multiple objects into one malformed JSON object.
- Each tool call will be executed sequentially.
- Do not return a final answer until all required tool calls
  have been completed.

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
- Do not include text outside JSON objects.

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

JSON TOOL RULES:

When the user requests JSON validation, formatting,
minification, or data extraction:

1. You MUST call the json tool.
2. Never perform these operations yourself.
3. The response MUST be a valid JSON object.
4. Always include "type": "tool_call".
5. Always include "name": "json".
6. Always include "args" as a JSON object.

For pretty, minify and get:
- Use "data" as a nested JSON value.
- Do NOT encode data as a JSON string.

For validate:
- Use "json" as a string containing the JSON text.

Examples:

{"type":"tool_call","name":"json","args":{"command":"pretty","data":{"name":"An","age":20}}}

{"type":"tool_call","name":"json","args":{"command":"minify","data":{"name":"An","age":20}}}

{"type":"tool_call","name":"json","args":{"command":"get","data":{"user":{"name":"An"}},"path":"user.name"}}

{"type":"tool_call","name":"json","args":{"command":"validate","json":"{\"name\":\"An\"}"}}

Never omit the type, name, or args fields.

GIT TOOL RULES:
- Use the git tool when the user asks about repository status,
  commit history, branches, or uncommitted changes.
- Tool name must be exactly "git".
- Arguments must be a JSON object.
- Always provide the command field.
- Valid commands: status, log, branch, diff.
- Do not invent Git output.
- This tool only supports read-only inspection.

Examples:

{"type":"tool_call","name":"git","args":{"command":"status"}}

{"type":"tool_call","name":"git","args":{"command":"log"}}

{"type":"tool_call","name":"git","args":{"command":"branch"}}

{"type":"tool_call","name":"git","args":{"command":"diff"}}

DATETIME TOOL RULES:

- Use datetime when the user asks about the current date or time.
- The datetime tool name is "datetime".
- The args MUST be a JSON object.
- Always provide a "command" field.
- Valid commands: date, time, datetime, weekday.

Examples:

{"type":"tool_call","name":"datetime","args":{"command":"date"}}

{"type":"tool_call","name":"datetime","args":{"command":"time"}}

MEMORY TOOL RULES:

- Memory is a TOOL, not a response type.
- NEVER use "memory" as the value of the top-level "type".
- The top-level "type" MUST always be "tool_call" or "final".
- The memory operation must be specified inside args.action.

To save memory, use:
{"type":"tool_call","name":"memory","args":{"action":"save","label":"topic","content":"information"}}

To search memory, use:
{"type":"tool_call","name":"memory","args":{"action":"search","query":"topic"}}

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


// Giữ lại hàm này để tương thích với khai báo trong header.
std::string AgentLoop::extractJson(
    const std::string& response
) const {
    const auto objects = extractJsonObjects(response);

    if (objects.empty()) {
        return "";
    }

    return objects.front();
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

        // 1. Gọi LLM.
        const auto llmStart = std::chrono::steady_clock::now();

        ++llmCalls;

        LLMResponse llmResponse = llm.chat(messages);

        const auto llmEnd = std::chrono::steady_clock::now();

        const auto llmLatency =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                llmEnd - llmStart
            ).count();

        const std::string response = llmResponse.content;
        const int tokens = llmResponse.totalTokens();

        std::cout << "[DEBUG] Prompt tokens: "
                  << llmResponse.promptTokens << '\n';

        std::cout << "[DEBUG] Completion tokens: "
                  << llmResponse.completionTokens << '\n';

        std::cout << "[DEBUG] Total tokens: "
                  << tokens << '\n';


        // 2. Tách tất cả JSON object trong response.
        const std::vector<std::string> jsonObjects =
            extractJsonObjects(response);

        if (jsonObjects.empty()) {

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


        // 3. Parse và thực thi từng JSON object.
        std::string allToolResults;
        bool executedAnyTool = false;

        for (const std::string& jsonText : jsonObjects) {

            json action;

            try {
                action = json::parse(jsonText);
            }
            catch (const json::exception& e) {

                const std::string error =
                    std::string("Invalid JSON response: ") + e.what();

                const auto stepEnd =
                    std::chrono::steady_clock::now();

                const auto latency =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        stepEnd - stepStart
                    ).count();

                if (stepHook) {
                    stepHook(
                        response,
                        "INVALID_JSON",
                        error,
                        latency,
                        executedAnyTool ? 0 : tokens
                    );
                }

                return error;
            }


            // Kiểm tra JSON phải là object.
            if (!action.is_object()) {
                return "Invalid action: JSON must be an object.";
            }


            // 4. Xác định loại action.
            std::string type =
                action.value("type", std::string{});

            // Tương thích với model quên trường type.
            if (type.empty() &&
                action.contains("name") &&
                action["name"].is_string() &&
                action.contains("args")) {

                type = "tool_call";

                std::cout
                    << "[DEBUG] Missing type field. "
                    << "Inferred type = tool_call\n";
            }


            // 5. Final answer.
            if (type == "final") {

                const std::string answer =
                    action.value("content", std::string{});

                const auto stepEnd =
                    std::chrono::steady_clock::now();

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
                        executedAnyTool ? 0 : tokens
                    );
                }

                std::cout
                    << "\n========== AGENT SUMMARY ==========\n";

                std::cout << "LLM calls: " << llmCalls << '\n';
                std::cout << "Tool calls: " << toolCalls << '\n';

                std::cout
                    << "===================================\n";

                return answer;
            }


            // 6. Kiểm tra action type.
            if (type != "tool_call") {

                const std::string error =
                    "Unknown action type: " + type;

                const auto stepEnd =
                    std::chrono::steady_clock::now();

                const auto latency =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        stepEnd - stepStart
                    ).count();

                if (stepHook) {
                    stepHook(
                        response,
                        "INVALID_ACTION",
                        error,
                        latency,
                        executedAnyTool ? 0 : tokens
                    );
                }

                return error;
            }


            // 7. Lấy tên tool và arguments.
            const std::string toolName =
                action.value("name", std::string{});

            const json args =
                action.value("args", json::object());


            if (toolName.empty() || !args.is_object()) {

                const std::string error =
                    "Tool call error: missing tool name or invalid args.";

                if (stepHook) {
                    stepHook(
                        response,
                        "INVALID_TOOL_CALL",
                        error,
                        0,
                        executedAnyTool ? 0 : tokens
                    );
                }

                return error;
            }


            // 8. Kiểm tra quyền sử dụng tool.
            if (!toolRegistry.isAllowed(toolName)) {

                const std::string error =
                    "Tool is not allowed: " + toolName;

                if (stepHook) {
                    stepHook(
                        response,
                        toolName,
                        error,
                        0,
                        executedAnyTool ? 0 : tokens
                    );
                }

                return error;
            }


            // 9. Lấy tool từ registry.
            auto toolOptional =
                toolRegistry.getTool(toolName);

            if (!toolOptional.has_value()) {

                const std::string error =
                    "Tool not found: " + toolName;

                if (stepHook) {
                    stepHook(
                        response,
                        toolName,
                        error,
                        0,
                        executedAnyTool ? 0 : tokens
                    );
                }

                return error;
            }

            Tool& tool = toolOptional->get();


            // 10. Chuyển arguments thành chuỗi.
            std::string toolArgs;

            if (toolName == "calculator") {
                toolArgs =
                    args.value("expression", std::string{});
            }
            else {
                toolArgs = args.dump();
            }


            // 11. Kiểm tra loop.
            const std::string actionSignature =
                toolName + ":" + args.dump();

            const LoopStatus loopStatus =
                loopDetector.recordAction(actionSignature);

            if (loopStatus == LoopStatus::Critical) {

                const std::string error =
                    "Agent stopped: critical loop detected.";

                if (stepHook) {
                    stepHook(
                        response,
                        "LOOP_DETECTED",
                        error,
                        0,
                        executedAnyTool ? 0 : tokens
                    );
                }

                return error;
            }


            // 12. Thực thi tool.
            const auto toolStart =
                std::chrono::steady_clock::now();

            std::string toolResult;

            ++toolCalls;

            try {
                toolResult = tool.execute(toolArgs);
            }
            catch (const std::exception& e) {
                toolResult =
                    std::string("Tool execution error: ") + e.what();
            }

            const auto toolEnd =
                std::chrono::steady_clock::now();

            const auto toolLatency =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    toolEnd - toolStart
                ).count();


            // 13. Tổng latency:
            // Tool đầu tiên tính cả thời gian gọi LLM.
            // Các tool tiếp theo tính thời gian thực thi riêng.
            const auto stepEnd =
                std::chrono::steady_clock::now();

            const auto totalStepLatency =
                executedAnyTool
                    ? toolLatency
                    : std::chrono::duration_cast<
                        std::chrono::milliseconds
                      >(stepEnd - stepStart).count();


            std::cout
                << "\n========== AGENT PROFILING ==========\n";

            std::cout << "Step: " << step + 1 << '\n';
            std::cout << "Tool: " << toolName << '\n';
            std::cout << "LLM latency: "
                      << llmLatency << " ms\n";

            std::cout << "Tool latency: "
                      << toolLatency << " ms\n";

            std::cout << "Total step latency: "
                      << totalStepLatency << " ms\n";

            std::cout << "Tokens: "
                      << (executedAnyTool ? 0 : tokens) << '\n';

            std::cout
                << "=====================================\n";


            // 14. Ghi trajectory.
            if (stepHook) {
                stepHook(
                    response,
                    toolName,
                    toolResult,
                    totalStepLatency,
                    executedAnyTool ? 0 : tokens
                );
            }


            // 15. Lưu kết quả tool.
            executedAnyTool = true;

            allToolResults +=
                "\nTool result for " + toolName + ":\n";

            allToolResults += toolResult;
            allToolResults += "\n";
        }


        // 16. Đưa response và toàn bộ kết quả tool vào history.
        messages.push_back({
            "assistant",
            response
        });

        messages.push_back({
            "user",
            allToolResults +
            "\nContinue the task. Use the tool results above. "
            "Respond with one or more valid JSON objects."
        });
    }


    // 17. Agent đạt giới hạn số bước.
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
