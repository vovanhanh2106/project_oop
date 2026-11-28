
#pragma once

#include "../agent/agent_loop.h"
#include "evaluator.h"
#include "trajectory.h"

#include <memory>
#include <string>
#include <vector>

struct HarnessTask {
    std::string name;
    std::string input;
    std::string expected;

    // "keyword", "functional" hoặc "numeric"
    std::string evaluatorType = "functional";

    // Các tool bắt buộc phải sử dụng trong task
    std::vector<std::string> requiredTools;
    std::string taskId;
};

struct HarnessResult {
    std::string taskName;
    std::string input;
    std::string expected;
    std::string actual;
    std::string trajectoryJson;

    EvaluationResult evaluation;

    // Thông tin đánh giá tool usage
    std::vector<std::string> usedTools;
    std::vector<std::string> missingTools;

    bool executionError = false;
    long long latency_ms = 0;
};

class HarnessRunner {
private:
    AgentLoop& agent;

    std::unique_ptr<Evaluator> keywordEvaluator;
    std::unique_ptr<Evaluator> functionalEvaluator;
    std::unique_ptr<Evaluator> numericEvaluator;

    std::vector<HarnessResult> results;

public:
    explicit HarnessRunner(AgentLoop& agent);

    TrajectoryRecorder trajectoryRecorder;

    HarnessResult runTask(const HarnessTask& task);

    std::vector<HarnessResult> runBatch(
        const std::vector<HarnessTask>& tasks
    );

    const std::vector<HarnessResult>& getResults() const;

    double getSuccessRate() const;

    bool saveResultsToJson(
        const std::string& filePath
    ) const;
};
