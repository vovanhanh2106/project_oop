#include "tools/web_search_tool.h"

#include <iostream>
#include <string>

int main() {
    WebSearchTool tool;

    std::cout << "========== WebSearchTool Manual Test ==========\n";

    const std::string args = R"({
        "query": "C++ programming"
    })";

    std::cout << "\n[1] Executing web search...\n\n";

    const std::string result = tool.execute(args);

    std::cout << result << "\n";

    // Check whether the tool returned an error
    if (result.find("Error:") != std::string::npos ||
        result.find("HTTP request failed:") != std::string::npos ||
        result.find("SearXNG returned HTTP") != std::string::npos ||
        result.find("Search error:") != std::string::npos ||
        result.find("JSON error:") != std::string::npos ||
        result.find("No web results found") != std::string::npos) {

        std::cout << "\n[FAILED] Web search test failed.\n";
        return 1;
    }

    // Check whether search results were returned
    if (result.find("Web search results for:") == std::string::npos) {
        std::cout << "\n[FAILED] Unexpected response.\n";
        return 1;
    }

    std::cout << "\n[PASSED] Web search test passed.\n";

    return 0;
}