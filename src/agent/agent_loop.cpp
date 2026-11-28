
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
    : BaseAgent(maxSteps),
      llm(llm),
      toolRegistry(toolRegistry),
      skillLoader(skillLoader),
      loopDetector(loopDetector) {
}
void AgentLoop::initialize(
    const std::string& userInput
) {
    // Reset trạng thái của lần chạy trước
    loopDetector.reset();

    messages.clear();

    llmCalls = 0;
    toolCalls = 0;
    currentStep = 0;

    llmLatencyMs = 0;

    // Thêm system prompt
    messages.push_back({
        "system",
        buildSystemPrompt(userInput)
    });

    // Thêm yêu cầu của người dùng
    messages.push_back({
        "user",
        userInput
    });
}
std::string AgentLoop::observe() {
    // Bắt đầu tính thời gian của vòng lặp
    stepStart = std::chrono::steady_clock::now();

    // Bắt đầu gọi LLM
    const auto llmStart =
        std::chrono::steady_clock::now();

    ++llmCalls;
    ++currentStep;

    // Gửi lịch sử hội thoại cho LLM
    lastLLMResponse = llm.chat(messages);

    // Kết thúc gọi LLM
    const auto llmEnd =
        std::chrono::steady_clock::now();

    llmLatencyMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            llmEnd - llmStart
        ).count();

    // Lấy số token
    const int tokens = lastLLMResponse.totalTokens();

    std::cout << "[DEBUG] Prompt tokens: "
              << lastLLMResponse.promptTokens << '\n';

    std::cout << "[DEBUG] Completion tokens: "
              << lastLLMResponse.completionTokens << '\n';

    std::cout << "[DEBUG] Total tokens: "
              << tokens << '\n';

    // Trả về nội dung phản hồi của LLM
    return lastLLMResponse.content;
}

AgentDecision AgentLoop::think(
    const std::string& observation
) {
    // Tạo kết quả quyết định
    AgentDecision decision;

    // Lưu phản hồi gốc từ LLM
    decision.rawResponse = observation;

    // Tách các JSON object từ phản hồi
    decision.jsonObjects = extractJsonObjects(observation);

    // Không tìm thấy JSON object nào
    if (decision.jsonObjects.empty()) {
        decision.valid = false;
        decision.error = "INVALID_RESPONSE";
    }

    return decision;
}

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

AgentStepResult AgentLoop::onMaxStepsReached(
    StepHook& stepHook
) {
    std::string error =
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

    return {
        true,
        error
    };
}
AgentStepResult AgentLoop::act(
    const AgentDecision& decision,
    StepHook& stepHook
) {
    using json = nlohmann::json;

    // 1. Xử lý khi LLM không trả về JSON
    if (!decision.valid) {
        if (stepHook) {
            stepHook(
                decision.rawResponse,
                "INVALID_RESPONSE",
                decision.rawResponse,
                llmLatencyMs,
                lastLLMResponse.totalTokens()
            );
        }

        return {
            true,
            decision.rawResponse
        };
    }

    std::string allToolResults;
    bool firstToolInStep = true;
    // 2. Xử lý từng JSON object
    for (const std::string& jsonObject : decision.jsonObjects) {

        json action;

        // 3. Parse JSON
        try {
            action = json::parse(jsonObject);
        }
        catch (const std::exception& e) {
            std::string error =
                "Invalid JSON response: " + std::string(e.what());

            if (stepHook) {
                stepHook(
                    decision.rawResponse,
                    "INVALID_JSON",
                    error,
                    llmLatencyMs,
                    lastLLMResponse.totalTokens()
                );
            }

            return {
                true,
                error
            };
        }

        // 4. JSON phải là object
        if (!action.is_object()) {
            return {
                true,
                "Invalid action: JSON must be an object."
            };
        }

        // 5. Xác định loại action
        std::string type = action.value("type", "");

        // Tự suy luận type nếu LLM quên khai báo
        if (type.empty() &&
            action.contains("name") &&
            action["name"].is_string() &&
            action.contains("args")) {

            type = "tool_call";
        }

        // 6. Nếu là final thì kết thúc Agent
        if (type == "final") {
            std::string answer =
                action.value("content", "");

            if (stepHook) {
                stepHook(
                    decision.rawResponse,
                    "FINAL",
                    answer,
                    llmLatencyMs,
                    lastLLMResponse.totalTokens()
                );
            }

            std::cout
                << "\n[SUMMARY] LLM calls: "
                << llmCalls
                << ", Tool calls: "
                << toolCalls
                << '\n';

            return {
                true,
                answer
            };
        }

        // 7. Chỉ chấp nhận tool_call
        if (type != "tool_call") {
            std::string error =
                "Invalid action type: " + type;

            if (stepHook) {
                stepHook(
                    decision.rawResponse,
                    "INVALID_ACTION",
                    error,
                    llmLatencyMs,
                    lastLLMResponse.totalTokens()
                );
            }

            return {
                true,
                error
            };
        }

        // 8. Lấy tên Tool và arguments
        std::string toolName =
            action.value("name", "");

        json args = action.value("args", json::object());

        if (toolName.empty() || !args.is_object()) {
            std::string error =
                "Invalid tool call: missing name or invalid args.";

            if (stepHook) {
                stepHook(
                    decision.rawResponse,
                    "INVALID_TOOL_CALL",
                    error,
                    llmLatencyMs,
                    lastLLMResponse.totalTokens()
                );
            }

            return {
                true,
                error
            };
        }

        // 9. Kiểm tra Tool có được phép sử dụng không
        if (!toolRegistry.isAllowed(toolName)) {
            std::string error =
                "Tool is not allowed: " + toolName;

            if (stepHook) {
                stepHook(
                    decision.rawResponse,
                    toolName,
                    error,
                    llmLatencyMs,
                    lastLLMResponse.totalTokens()
                );
            }

            return {
                true,
                error
            };
        }

        // 10. Tìm Tool trong registry
        auto tool = toolRegistry.getTool(toolName);

        if (!tool) {
            std::string error =
                "Tool not found: " + toolName;

            if (stepHook) {
                stepHook(
                    decision.rawResponse,
                    toolName,
                    error,
                    llmLatencyMs,
                    lastLLMResponse.totalTokens()
                );
            }

            return {
                true,
                error
            };
        }

        // 11. Chuẩn bị arguments cho Tool
        std::string toolArgs;

        if (toolName == "calculator") {
            toolArgs = args.value("expression", "");
        }
        else {
            toolArgs = args.dump();
        }

        // 12. Phát hiện Agent gọi lặp cùng một hành động
        std::string actionSignature =
            toolName + ":" + args.dump();

        auto loopStatus =
            loopDetector.recordAction(actionSignature);

        if (loopStatus == LoopStatus::Critical) {
            std::string error =
                "Agent stopped: repeated tool calls detected.";

            if (stepHook) {
                stepHook(
                    decision.rawResponse,
                    "LOOP_DETECTED",
                    error,
                    llmLatencyMs,
                    lastLLMResponse.totalTokens()
                );
            }

            return {
                true,
                error
            };
        }

        // 13. Thực thi Tool và đo thời gian
        auto toolStart =
            std::chrono::steady_clock::now();

        std::string toolResult;

        try {
            toolResult = tool->get().execute(toolArgs);
        }
        catch (const std::exception& e) {
            toolResult =
                "Tool execution error: " + std::string(e.what());
        }

        auto toolEnd =
            std::chrono::steady_clock::now();

        long long toolLatencyMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                toolEnd - toolStart
            ).count();

        ++toolCalls;

        // 14. Tính latency và token cho StepHook
        long long totalLatencyMs = toolLatencyMs;

        int tokens = 0;

        // Tool đầu tiên bao gồm cả thời gian gọi LLM
     


if (firstToolInStep) {
    totalLatencyMs += llmLatencyMs;
    tokens = lastLLMResponse.totalTokens();

    firstToolInStep = false;
}

        // 15. Ghi log profiling
        std::cout
            << "[PROFILE] Step: " << currentStep
            << ", Tool: " << toolName
            << ", Tool latency: " << toolLatencyMs << " ms"
            << ", Total latency: " << totalLatencyMs << " ms"
            << ", Tokens: " << tokens
            << '\n';

        // 16. Gọi callback cho Harness
        if (stepHook) {
            stepHook(
                decision.rawResponse,
                toolName,
                toolResult,
                totalLatencyMs,
                tokens
            );
        }

        // 17. Lưu kết quả Tool
        allToolResults +=
            "\nTool: " + toolName +
            "\nResult: " + toolResult + "\n";
    }

    // 18. Đưa phản hồi và kết quả Tool vào lịch sử hội thoại
    messages.push_back({
        "assistant",
        decision.rawResponse
    });

    messages.push_back({
        "user",
        "Tool execution results:\n" +
        allToolResults +
        "\nContinue your task. If finished, return a final action."
    });

    // Chưa kết thúc, cần gọi LLM tiếp
    return {
        false,
        ""
    };
}
const std::string& AgentLoop::getModelName() const {
    return llm.getModelName();
}