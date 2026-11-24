
#include "json_tool.h"

#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

using json = nlohmann::json;

std::string JsonTool::getName() const {
    return "json";
}

std::string JsonTool::getDescription() const {
    return R"(
Process JSON data.

IMPORTANT: Always copy the complete JSON provided by the user
into the correct argument field.

Arguments:
- command: validate, pretty, minify, or get
- data: required for pretty, minify, and get.
  It must be a nested JSON value, not a string.
- json: required for validate. It must be a string.
- path: required for get.

Examples:

Pretty:
{"command":"pretty","data":{"name":"An","age":20}}

Minify:
{"command":"minify","data":{"name":"An","age":20}}

Get:
{"command":"get","data":{"user":{"name":"An","age":20}},"path":"user.email"}

Validate:
{"command":"validate","json":"{\"name\":\"An\"}"}
)";
}


std::string JsonTool::execute(const std::string& args) {
    try {
        json input = json::parse(args);

        if (!input.is_object()) {
            return "Error: Arguments must be a JSON object.";
        }

        const std::string command = input.value("command", "");

        // Validate vẫn nhận JSON dưới dạng string,
        // vì cần kiểm tra cả trường hợp JSON sai cú pháp.
        if (command == "validate") {
            if (!input.contains("json") || !input["json"].is_string()) {
                return "Error: validate requires a JSON string.";
            }

            try {
                json::parse(input["json"].get<std::string>());
                return "Valid JSON.";
            }
            catch (const json::parse_error& e) {
                return std::string("Invalid JSON: ") + e.what();
            }
        }

        // Các command còn lại nhận JSON object trực tiếp.
        if (!input.contains("data")) {
            return "Error: Missing data field.";
        }

        const json& data = input["data"];

        if (command == "pretty") {
            return data.dump(4);
        }

        if (command == "minify") {
            return data.dump();
        }

        if (command == "get") {
            const std::string path = input.value("path", "");

            if (path.empty()) {
                return "Error: Missing path.";
            }

            const json* current = &data;
            std::stringstream ss(path);
            std::string key;

            while (std::getline(ss, key, '.')) {
                if (!current->is_object() ||
                    !current->contains(key)) {
                    return "Error: Path not found: " + path;
                }

                current = &current->at(key);
            }

            if (current->is_string()) {
                return current->get<std::string>();
            }

            return current->dump(4);
        }

        return "Error: Unsupported command. "
               "Use validate, pretty, minify, or get.";
    }
    catch (const json::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
