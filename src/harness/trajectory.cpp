
#include "trajectory.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

TrajectoryRecorder::TrajectoryRecorder(
    const std::string& task
) {
    startRun(task);
}

void TrajectoryRecorder::startRun(
    const std::string& task
) {
    this->task = task;
    steps.clear();
}

void TrajectoryRecorder::recordStep(
    const std::string& thought,
    const std::string& action,
    const std::string& result,
    long long latency_ms,
    int tokens
) {
    if (latency_ms < 0) {
        throw std::invalid_argument(
            "latency_ms cannot be negative."
        );
    }

    if (tokens < 0) {
        throw std::invalid_argument(
            "tokens cannot be negative."
        );
    }

    TrajectoryStep step;

    step.step = steps.size() + 1;
    step.thought = thought;
    step.action = action;
    step.result = result;
    step.latency_ms = latency_ms;
    step.tokens = tokens;

    steps.push_back(step);
}

const std::string& TrajectoryRecorder::getTask() const {
    return task;
}

const std::vector<TrajectoryStep>&
TrajectoryRecorder::getSteps() const {
    return steps;
}

std::size_t TrajectoryRecorder::getStepCount() const {
    return steps.size();
}

std::string TrajectoryRecorder::toJson() const {
    json trajectory = {
        {"task", task},
        {"step_count", steps.size()},
        {"steps", json::array()}
    };

    for (const auto& step : steps) {
        trajectory["steps"].push_back({
            {"step", step.step},
            {"thought", step.thought},
            {"action", step.action},
            {"result", step.result},
            {"latency_ms", step.latency_ms},
            {"tokens", step.tokens}
        });
    }

    return trajectory.dump(4);
}

bool TrajectoryRecorder::saveToJson(
    const std::string& filePath
) const {
    std::ofstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    const std::string content = toJson();

    file.write(
        content.data(),
        static_cast<std::streamsize>(content.size())
    );

    file.close();

    return file.good();
}

void TrajectoryRecorder::clear() {
    task.clear();
    steps.clear();
}
