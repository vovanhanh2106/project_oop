
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
    
    // "keyword" hoặc "functional"
    std::string evaluatorType = "functional";
};

struct HarnessResult {
    std::string taskName;
    std::string input;
    std::string expected;
    std::string actual;
    std::string trajectoryJson;
    EvaluationResult evaluation;

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
