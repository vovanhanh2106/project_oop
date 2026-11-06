
#pragma once

#include "llm_client.h"

#include <string>

class RemoteLLMClient : public LLMClient {
private:
    std::string base_url;
    std::string model;
    std::string api_token;

    double temperature;
    int max_tokens;

public:
    RemoteLLMClient(
        const std::string& base_url,
        const std::string& model,
        const std::string& api_token,
        double temperature = 0.7,
        int max_tokens = 512
    );

    LLMResponse chat(
        const std::vector<Message>& messages
    ) override;
};
