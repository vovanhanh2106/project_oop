
#pragma once

#include <cstddef>
#include <string>
#include <inplace_vector>

struct TrajectoryStep {
    std::size_t step = 0;
    std::string thought;
    std::string action;
    std::string result;
    long long latency_ms = 0;
    int tokens = 0;
};

class TrajectoryRecorder {
private:
    // C++26: Lưu tối đa 64 bước ngay bên trong object.
    std::inplace_vector<TrajectoryStep, 64> steps;

    std::string task;

    // Tổng thời gian thực thi task.
    long long totalTimeMs = 0;

public:
    TrajectoryRecorder() = default;

    explicit TrajectoryRecorder(const std::string& task);

    void startRun(const std::string& task);

    void recordStep(
        const std::string& thought,
        const std::string& action,
        const std::string& result,
        long long latency_ms,
        int tokens = 0
    );

    const std::string& getTask() const;

    const std::inplace_vector<TrajectoryStep, 64>& getSteps() const;

    std::size_t getStepCount() const;
    int getTotalTokens() const;

    void setTotalTimeMs(long long time);
    long long getTotalTimeMs() const;

    std::string toJson() const;

    bool saveToJson(const std::string& filePath) const;

    void clear();
};
