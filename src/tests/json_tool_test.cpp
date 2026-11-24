
#include "../tools/json_tool.h"

#include <cassert>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    JsonTool tool;

    // Test 1: Pretty print
    {
        std::string args = R"({
            "command": "pretty",
            "data": {"name": "An", "age": 20}
        })";

        std::string result = tool.execute(args);
        json parsed = json::parse(result);

        assert(parsed["name"] == "An");
        assert(parsed["age"] == 20);

        std::cout << "[PASS] Pretty print\n";
    }

    // Test 2: Minify
    {
        std::string args = R"({
            "command": "minify",
            "data": {"name": "An", "age": 20}
        })";

        std::string result = tool.execute(args);
        json parsed = json::parse(result);

        assert(parsed["name"] == "An");
        assert(parsed["age"] == 20);
        assert(result.find('\n') == std::string::npos);

        std::cout << "[PASS] Minify\n";
    }

    // Test 3: Validate JSON hợp lệ
    {
        std::string args = R"({
            "command": "validate",
            "json": "{\"name\":\"An\",\"age\":20}"
        })";

        std::string result = tool.execute(args);

        assert(result == "Valid JSON.");

        std::cout << "[PASS] Validate valid JSON\n";
    }

    // Test 4: Validate JSON không hợp lệ
    {
        std::string args = R"({
            "command": "validate",
            "json": "{\"name\":}"
        })";

        std::string result = tool.execute(args);

        assert(result.find("Invalid JSON:") == 0);

        std::cout << "[PASS] Validate invalid JSON\n";
    }

    // Test 5: Get một field tồn tại
    {
        std::string args = R"({
            "command": "get",
            "data": {
                "user": {
                    "name": "An",
                    "age": 20
                }
            },
            "path": "user.name"
        })";

        std::string result = tool.execute(args);

        assert(result == "An");

        std::cout << "[PASS] Get existing field\n";
    }

    // Test 6: Get một field không tồn tại
    {
        std::string args = R"({
            "command": "get",
            "data": {
                "user": {
                    "name": "An",
                    "age": 20
                }
            },
            "path": "user.email"
        })";

        std::string result = tool.execute(args);

        assert(result == "Error: Path not found: user.email");

        std::cout << "[PASS] Get missing field\n";
    }

    // Test 7: Thiếu data
    {
        std::string args = R"({
            "command": "get",
            "path": "user.name"
        })";

        std::string result = tool.execute(args);

        assert(result == "Error: Missing data field.");

        std::cout << "[PASS] Missing data\n";
    }

    // Test 8: Command không được hỗ trợ
{
    std::string args = R"({
        "command": "unknown",
        "data": {"name": "An"}
    })";

    std::string result = tool.execute(args);

    std::cout << "[DEBUG] Actual result: [" << result << "]\n";

    assert(result.find("Error: Unsupported command.") == 0);

    std::cout << "[PASS] Unsupported command\n";
}

    std::cout << "\nAll JsonTool tests passed!\n";

    return 0;
}
