
#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

// Kết quả sau bước Think
struct AgentDecision {
    std::vector<std::string> jsonObjects;
    std::string rawResponse;

    bool valid = true;
    std::string error;
};

// Kết quả sau bước Act
struct AgentStepResult {
    bool finished = false;
    std::string output;
};

class BaseAgent {
public:
    using StepHook = std::function<void(
        const std::string& thought,
        const std::string& action,
        const std::string& result,
        long long latency_ms,
        int tokens
    )>;

    virtual ~BaseAgent() = default;

    // Template Method: điều khiển toàn bộ vòng đời Agent
    std::string run(
        const std::string& userInput,
        StepHook stepHook = {}
    );

protected:
    explicit BaseAgent(std::size_t maxSteps);

    // Các bước do lớp con triển khai
    virtual void initialize(
        const std::string& userInput
    ) = 0;

    virtual std::string observe() = 0;

    virtual AgentDecision think(
        const std::string& observation
    ) = 0;

    virtual AgentStepResult act(
        const AgentDecision& decision,
        StepHook& stepHook
    ) = 0;

    virtual AgentStepResult onMaxStepsReached(
        StepHook& stepHook
    ) = 0;

    std::size_t maxSteps;
};
