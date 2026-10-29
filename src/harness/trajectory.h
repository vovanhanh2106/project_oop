
#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Đại diện cho một bước thực thi của Agent.
struct TrajectoryStep {
    std::size_t step = 0;

    // Phản hồi của LLM ở bước này.
    std::string thought;

    // Tên tool hoặc hành động được thực hiện.
    std::string action;

    // Kết quả thực thi hành động.
    std::string result;

    // Thời gian thực hiện bước, tính bằng milliseconds.
    long long latency_ms = 0;

    // Số token sử dụng ở bước này.
    // Bằng 0 nếu chưa có dữ liệu.
    int tokens = 0;
};

// Ghi lại toàn bộ quá trình chạy của Agent.
class TrajectoryRecorder {
private:
    std::string task;
    std::vector<TrajectoryStep> steps;

public:
    TrajectoryRecorder() = default;

    explicit TrajectoryRecorder(const std::string& task);

    // Bắt đầu một lượt chạy mới.
    void startRun(const std::string& task);

    // Ghi nhận một bước thực thi.
    void recordStep(
        const std::string& thought,
        const std::string& action,
        const std::string& result,
        long long latency_ms,
        int tokens = 0
    );

    // Truy cập dữ liệu trajectory.
    const std::string& getTask() const;
    const std::vector<TrajectoryStep>& getSteps() const;

    // Lấy tổng số bước.
    std::size_t getStepCount() const;

    // Chuyển dữ liệu sang JSON.
    std::string toJson() const;

    // Xuất dữ liệu ra file JSON.
    bool saveToJson(const std::string& filePath) const;

    // Xóa dữ liệu của lượt chạy hiện tại.
    void clear();
};
