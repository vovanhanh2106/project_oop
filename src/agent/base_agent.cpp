
#include "base_agent.h"

BaseAgent::BaseAgent(std::size_t maxSteps)
    : maxSteps(maxSteps) {
}

std::string BaseAgent::run(
    const std::string& userInput,
    StepHook stepHook
) {
    // Bước 1: Khởi tạo Agent
    initialize(userInput);

    // Bước 2: Chạy vòng lặp tối đa maxSteps lần
    for (std::size_t step = 0; step < maxSteps; ++step) {

        // Bước 3: Quan sát môi trường / nhận phản hồi từ LLM
        std::string observation = observe();

        // Bước 4: Phân tích phản hồi và đưa ra quyết định
        AgentDecision decision = think(observation);

        // Bước 5: Thực thi quyết định
        AgentStepResult result = act(decision, stepHook);

        // Bước 6: Kiểm tra Agent đã hoàn thành chưa
        if (result.finished) {
            return result.output;
        }
    }

    // Bước 7: Xử lý khi vượt quá số bước cho phép
    AgentStepResult result = onMaxStepsReached(stepHook);

    return result.output;
}
