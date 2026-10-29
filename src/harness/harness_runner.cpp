
#include "harness_runner.h"

#include "keyword_evaluator.h"
#include "functional_evaluator.h"
#include "numeric_evaluator.h"
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

using json = nlohmann::json;


HarnessRunner::HarnessRunner(AgentLoop& agent)
    : agent(agent),
      keywordEvaluator(std::make_unique<KeywordEvaluator>()),
      functionalEvaluator(std::make_unique<FunctionalEvaluator>()),
      numericEvaluator(std::make_unique<NumericEvaluator>()) {
}



HarnessResult HarnessRunner::runTask(
    const HarnessTask& task
) {
    HarnessResult result;

    result.taskName = task.name;
    result.input = task.input;
    result.expected = task.expected;

    // 1. Khởi tạo recorder cho task hiện tại
    trajectoryRecorder.clear();
    trajectoryRecorder.startRun(task.input);

    const auto start = std::chrono::steady_clock::now();

    try {
        // 2. Chạy Agent và truyền StepHook để ghi lại từng bước
        result.actual = agent.run(
            task.input,
            [this](
                const std::string& thought,
                const std::string& action,
                const std::string& toolResult,
                long long latency,
                int tokens
            ) {
                trajectoryRecorder.recordStep(
                    thought,
                    action,
                    toolResult,
                    latency,
                    tokens
                );
            }
        );
    }
    catch (const std::exception& e) {
        result.actual = std::string("Execution error: ") + e.what();
        result.executionError = true;
    }

    // 3. Lưu trajectory kể cả khi Agent gặp exception
    result.trajectoryJson = trajectoryRecorder.toJson();

    const auto end = std::chrono::steady_clock::now();

    result.latency_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        ).count();

    // 4. Đánh giá kết quả
    if (result.executionError) {
        result.evaluation = {
            false,
            0.0,
            "Agent execution failed."
        };
    }
    else {
        const Evaluator* evaluator = nullptr;

        
if (task.evaluatorType == "keyword") {
    evaluator = keywordEvaluator.get();
} else if (task.evaluatorType == "functional") {
    evaluator = functionalEvaluator.get();
} else if (task.evaluatorType == "numeric") {
    evaluator = numericEvaluator.get();
} else {
    throw std::invalid_argument(
        "Unknown evaluator type: " + task.evaluatorType
    );
}


        result.evaluation = evaluator->evaluate(
            task.expected,
            result.actual
        );
    }

    // 5. Lưu kết quả task vào danh sách
    results.push_back(result);

    return result;
}

std::vector<HarnessResult> HarnessRunner::runBatch(
    const std::vector<HarnessTask>& tasks
) {
    results.clear();

    std::vector<HarnessResult> batchResults;
    batchResults.reserve(tasks.size());

    for (const auto& task : tasks) {
        batchResults.push_back(runTask(task));
    }

    return batchResults;
}

const std::vector<HarnessResult>&
HarnessRunner::getResults() const {
    return results;
}

double HarnessRunner::getSuccessRate() const {
    if (results.empty()) {
        return 0.0;
    }

    std::size_t passed = 0;

    for (const auto& result : results) {
        if (result.evaluation.passed) {
            ++passed;
        }
    }

    return static_cast<double>(passed) /
           static_cast<double>(results.size());
}

bool HarnessRunner::saveResultsToJson(
    const std::string& filePath
) const {
    json output = {
        {"total_tasks", results.size()},
        {"success_rate", getSuccessRate()},
        {"results", json::array()}
    };

    
    for (const auto& result : results) {
        // Chuyển chuỗi JSON trajectory thành JSON object
        json trajectory = json::parse(
            result.trajectoryJson,
            nullptr,
            false
        );

        // Nếu trajectory không phải JSON hợp lệ
        if (trajectory.is_discarded()) {
            trajectory = json::object();
        }

        output["results"].push_back({
            {"task_name", result.taskName},
            {"input", result.input},
            {"expected", result.expected},
            {"actual", result.actual},
            {"passed", result.evaluation.passed},
            {"score", result.evaluation.score},
            {"feedback", result.evaluation.feedback},
            {"execution_error", result.executionError},
            {"latency_ms", result.latency_ms},
            {"trajectory", trajectory}
        });
    }


    std::ofstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    const std::string content = output.dump(4);

    file.write(
        content.data(),
        static_cast<std::streamsize>(content.size())
    );

    file.close();

    return file.good();
}
