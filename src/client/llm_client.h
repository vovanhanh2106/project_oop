#pragma once

#include <string>
#include <vector>
#include <span>

struct Message {
    std::string role;
    std::string content;
};

struct LLMResponse {
    std::string content;

    int promptTokens = 0;
    int completionTokens = 0;

    int totalTokens() const {
        return promptTokens + completionTokens;
    }
};

class LLMClient {
public:
    virtual ~LLMClient() = default;

    virtual LLMResponse chat(
        std::span<const Message> messages
    ) = 0;
    virtual const std::string& getModelName() const {
    static const std::string unknown = "unknown";
    return unknown;
    }
};