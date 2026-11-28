
#include "web_search_tool.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

namespace {

size_t writeCallback(
    char* data,
    size_t size,
    size_t count,
    void* userData
) {
    const size_t bytes = size * count;

    auto* response = static_cast<std::string*>(userData);
    response->append(data, bytes);

    return bytes;
}

std::string urlEncode(CURL* curl, const std::string& value) {
    char* encoded = curl_easy_escape(
        curl,
        value.c_str(),
        static_cast<int>(value.size())
    );

    if (encoded == nullptr) {
        throw std::runtime_error("Failed to encode search query.");
    }

    std::string result(encoded);
    curl_free(encoded);

    return result;
}

}

std::string WebSearchTool::getName() const {
    return "web_search";
}

std::string WebSearchTool::getDescription() const {
    return
        "Search the web using SearXNG. "
        "Input must be a JSON object: "
        "{\"query\":\"C++ programming\"}. "
        "Returns web result titles, summaries, URLs, and search engines. "
        "Use this tool when the user requests information "
        "that requires searching the web.";
}

std::string WebSearchTool::execute(const std::string& args) {
    try {
        // 1. Parse input JSON
        const json input = json::parse(args);

        if (!input.is_object() ||
            !input.contains("query") ||
            !input["query"].is_string()) {
            return "Error: expected JSON with a string query.";
        }

        const std::string query =
            input["query"].get<std::string>();

        if (query.empty() || query.size() > 500) {
            return "Error: query must contain 1-500 characters.";
        }

        // 2. Initialize CURL
        CURL* curl = curl_easy_init();

        if (curl == nullptr) {
            return "Error: could not initialize HTTP client.";
        }

        std::string response;

        // 3. Build SearXNG API URL
        const std::string url =
            "http://127.0.0.1:8080/search?q=" +
            urlEncode(curl, query) +
            "&format=json";

        // 4. Configure HTTP request
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(
            curl,
            CURLOPT_USERAGENT,
            "AIAgentStudentProject/1.0"
        );

        // 5. Send request
        const CURLcode result = curl_easy_perform(curl);

        long statusCode = 0;

        curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &statusCode
        );

        curl_easy_cleanup(curl);

        // 6. Handle HTTP errors
        if (result != CURLE_OK) {
            return std::string("HTTP request failed: ") +
                   curl_easy_strerror(result);
        }

        if (statusCode != 200) {
            return "SearXNG returned HTTP " +
                   std::to_string(statusCode);
        }

        // 7. Parse SearXNG JSON response
        const json data = json::parse(response);

        if (!data.contains("results") ||
            !data["results"].is_array()) {
            return "Error: unexpected SearXNG response.";
        }

        const auto& results = data["results"];

        if (results.empty()) {
            return "No web results found for: " + query;
        }

        // 8. Format search results
        std::ostringstream output;

        output << "Web search results for: "
               << query << "\n\n";

        std::size_t count = 0;

        for (const auto& item : results) {
            if (count >= 5) {
                break;
            }

            const std::string title =
                item.value("title", std::string("Untitled"));

            const std::string content =
                item.value("content", std::string(""));

            const std::string url =
                item.value("url", std::string(""));

            const std::string engine =
                item.value("engine", std::string("Unknown"));

            output << ++count << ". " << title << "\n"
                   << "   Summary: " << content << "\n"
                   << "   URL: " << url << "\n"
                   << "   Source: " << engine << "\n\n";
        }

        return output.str();
    }
    catch (const json::exception& e) {
        return std::string("JSON error: ") + e.what();
    }
    catch (const std::exception& e) {
        return std::string("Search error: ") + e.what();
    }
}
