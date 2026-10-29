
#include "../harness/trajectory.h"

#include <iostream>

int main() {
    TrajectoryRecorder recorder("Tính 17 nhân 13");

    // Giả lập bước 1: Agent gọi Calculator.
    recorder.recordStep(
        R"({"type":"tool_call","name":"calculator"})",
        "calculator",
        "221",
        120,
        0
    );

    // Giả lập bước 2: Agent trả lời cuối cùng.
    recorder.recordStep(
        R"({"type":"final","content":"221"})",
        "final",
        "221",
        80,
        0
    );

    // In JSON ra terminal.
    std::cout << recorder.toJson() << '\n';

    // Lưu kết quả vào file.
    if (recorder.saveToJson("trajectory_test.json")) {
        std::cout << "Trajectory saved successfully.\n";
    } else {
        std::cerr << "Failed to save trajectory.\n";
        return 1;
    }

    return 0;
}
