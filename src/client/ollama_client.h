#pragma once

#include "llm_client.h"

#include <string>

class OllamaClient : public LLMClient {
private:
    std::string base_url;
    std::string model;

    double temperature;
    int max_tokens;

public:
    OllamaClient(
        const std::string& base_url,
        const std::string& model,
        double temperature = 0.7,
        int max_tokens = 512
    );

    std::string chat(
        const std::vector<Message>& messages
    ) override;
};