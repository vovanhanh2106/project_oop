#include "ollama_client.h"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <stdexcept>

using json = nlohmann::json;

static size_t WriteCallback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userp
) {
    size_t total_size = size * nmemb;

    std::string* response =
        static_cast<std::string*>(userp);

    response->append(
        static_cast<char*>(contents),
        total_size
    );

    return total_size;
}

OllamaClient::OllamaClient(
    const std::string& base_url,
    const std::string& model,
    double temperature,
    int max_tokens
)
    : base_url(base_url),
      model(model),
      temperature(temperature),
      max_tokens(max_tokens)
{
}


std::string OllamaClient::chat(
    const std::vector<Message>& messages
) {
    CURL* curl = curl_easy_init();

    if (!curl) {
        throw std::runtime_error("Cannot initialize CURL");
    }

    json json_messages = json::array();

    for (const auto& message : messages) {
        json_messages.push_back({
            {"role", message.role},
            {"content", message.content}
        });
    }

    json body = {
        {"model", model},
        {"messages", json_messages},
        {"stream", false},
        {
            "options",
            {
                {"temperature", temperature},
                {"num_predict", max_tokens}
            }
        }
    };

    std::string body_string = body.dump();
    std::string response;
    std::string url = base_url + "/api/chat";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(
        headers,
        "Content-Type: application/json"
    );

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(
        curl,
        CURLOPT_POSTFIELDS,
        body_string.c_str()
    );
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        WriteCallback
    );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode result = curl_easy_perform(curl);

    long http_status = 0;
    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &http_status
    );

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        throw std::runtime_error(
            std::string("Ollama request failed: ") +
            curl_easy_strerror(result)
        );
    }

    if (http_status < 200 || http_status >= 300) {
        throw std::runtime_error(
            "Ollama HTTP error " +
            std::to_string(http_status) +
            ": " + response
        );
    }

    try {
        json response_json = json::parse(response);

        if (!response_json.contains("message") ||
            !response_json["message"].is_object() ||
            !response_json["message"].contains("content") ||
            !response_json["message"]["content"].is_string()) {
            throw std::runtime_error(
                "Ollama response is missing message.content"
            );
        }

        return response_json["message"]["content"]
            .get<std::string>();
    }
    catch (const json::exception& e) {
        throw std::runtime_error(
            std::string("Invalid Ollama response: ") + e.what()
        );
    }
}
