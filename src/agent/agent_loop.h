
#pragma once

#include "../client/llm_client.h"
#include "../tools/tool_registry.h"
#include "loop_detector.h"
#include "skill_loader.h"
// #include "../harness/trajectory.h"
#include <cstddef>
#include <string>
#include <vector>
#include <functional>

class AgentLoop {
private:
    LLMClient& llm;
    ToolRegistry& toolRegistry;
    SkillLoader& skillLoader;
    LoopDetector& loopDetector;

    std::size_t maxSteps;

    std::string buildSystemPrompt(
        const std::string& userInput
    ) const;

    std::string extractJson(const std::string& response) const;

public:
    AgentLoop(
        LLMClient& llm,
        ToolRegistry& toolRegistry,
        SkillLoader& skillLoader,
        LoopDetector& loopDetector,
        std::size_t maxSteps = 8
    );
    using StepHook = std::function<void(
    const std::string& thought,
    const std::string& action,
    const std::string& result,
    long long latency_ms,
    int tokens
)>;

    std::string run(
    const std::string& userInput,
    StepHook stepHook = {}
);
};
