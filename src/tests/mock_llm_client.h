#pragma once

#include "../client/llm_client.h"

#include <stdexcept>
#include <string>
#include <vector>

class MockLLMClient : public LLMClient {
private:
    std::vector<std::string> responses;
    std::size_t currentIndex = 0;
    int callCount = 0;

public:
    explicit MockLLMClient(
        const std::vector<std::string>& responses
    ) : responses(responses) {}

    LLMResponse chat(
        std::span<const Message> messages
    ) override {
        ++callCount;

        if (currentIndex >= responses.size()) {
            throw std::runtime_error(
                "MockLLMClient: no more responses"
            );
        }

        return {
            responses[currentIndex++],
            10,
            20
        };
    }

    int getCallCount() const {
        return callCount;
    }
};