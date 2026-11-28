
#pragma once

#include "../client/llm_client.h"
#include "../tools/tool_registry.h"
#include "base_agent.h"
#include "loop_detector.h"
#include "skill_loader.h"

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

class AgentLoop : public BaseAgent {
private:
    LLMClient& llm;
    ToolRegistry& toolRegistry;
    SkillLoader& skillLoader;
    LoopDetector& loopDetector;

    // Trạng thái của một lần chạy Agent
    std::vector<Message> messages;
    LLMResponse lastLLMResponse;

    std::chrono::steady_clock::time_point stepStart;

    long long llmLatencyMs = 0;

    int llmCalls = 0;
    int toolCalls = 0;

    std::size_t currentStep = 0;

    std::string buildSystemPrompt(
        const std::string& userInput
    ) const;

    std::string buildToolDescriptions() const;

    std::string extractJson(
        const std::string& response
    ) const;

protected:
    // Các bước triển khai từ BaseAgent
    void initialize(
        const std::string& userInput
    ) override;

    std::string observe() override;

    AgentDecision think(
        const std::string& observation
    ) override;

    AgentStepResult act(
        const AgentDecision& decision,
        StepHook& stepHook
    ) override;

    AgentStepResult onMaxStepsReached(
        StepHook& stepHook
    ) override;

public:
    // Giữ tương thích với AgentLoop::StepHook cũ
    using StepHook = BaseAgent::StepHook;

    AgentLoop(
        LLMClient& llm,
        ToolRegistry& toolRegistry,
        SkillLoader& skillLoader,
        LoopDetector& loopDetector,
        std::size_t maxSteps = 8
    );

    const std::string& getModelName() const;
};
