
#include "web_search_tool.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <cctype>
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

std::string htmlToText(const std::string& html) {
    std::string result;
    bool insideTag = false;

    for (char c : html) {
        if (c == '<') {
            insideTag = true;
        } else if (c == '>') {
            insideTag = false;
        } else if (!insideTag) {
            result += c;
        }
    }

    return result;
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
        "Search Wikipedia articles. "
        "Input JSON: {\"query\":\"C++ programming\"}. "
        "Returns article titles, snippets, and URLs.";
}

std::string WebSearchTool::execute(const std::string& args) {
    try {
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

        CURL* curl = curl_easy_init();

        if (curl == nullptr) {
            return "Error: could not initialize HTTP client.";
        }

        std::string response;

        const std::string url =
            "https://en.wikipedia.org/w/api.php"
            "?action=query"
            "&list=search"
            "&format=json"
            "&utf8=1"
            "&srlimit=5"
            "&srsearch=" + urlEncode(curl, query);

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

        const CURLcode result = curl_easy_perform(curl);

        long statusCode = 0;
        curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &statusCode
        );

        curl_easy_cleanup(curl);

        if (result != CURLE_OK) {
            return std::string("HTTP request failed: ") +
                   curl_easy_strerror(result);
        }

        if (statusCode != 200) {
            return "Search API returned HTTP " +
                   std::to_string(statusCode);
        }

        const json data = json::parse(response);

        if (!data.contains("query") ||
            !data["query"].contains("search") ||
            !data["query"]["search"].is_array()) {
            return "Error: unexpected search API response.";
        }

        const auto& results = data["query"]["search"];

        if (results.empty()) {
            return "No Wikipedia articles found for: " + query;
        }

        std::ostringstream output;
        output << "Wikipedia search results for: "
               << query << "\n\n";

        int index = 1;

        for (const auto& item : results) {
            const std::string title =
                item.value("title", std::string("Untitled"));

            const std::string snippet =
                htmlToText(
                    item.value("snippet", std::string(""))
                );

            const auto pageId = item.value("pageid", 0);

            output << index++ << ". " << title << "\n"
                   << "   Summary: " << snippet << "\n"
                   << "   URL: https://en.wikipedia.org/?curid="
                   << pageId << "\n\n";
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
