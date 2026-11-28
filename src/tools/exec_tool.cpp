
#include "exec_tool.h"
#include "../environment/native_environment.h"

#include <nlohmann/json.hpp>

#include <vector>
#include <string>

using json = nlohmann::json;

ExecTool::ExecTool()
    : ownedEnvironment(std::make_unique<NativeEnvironment>()),
      environment(ownedEnvironment.get()) {
}

ExecTool::ExecTool(Environment& environment)
    : environment(&environment) {
}

std::string ExecTool::getName() const {
    return "exec";
}

std::string ExecTool::getDescription() const {
    return
        "Execute an allowed Windows command with arguments. "
        "Input JSON format: "
        "{\"command\":\"ipconfig\",\"args\":[\"/all\"]}. "
        "Allowed commands depend on the current environment. "
        "Arguments are validated separately for each command.";
}

std::string ExecTool::execute(const std::string& args) {
    try {
        // 1. Parse JSON.
        const json input = json::parse(args);

        if (!input.is_object() ||
            !input.contains("command") ||
            !input["command"].is_string()) {
            return "Error: expected JSON with a string command.";
        }

        // 2. Read command.
        const std::string command =
            input["command"].get<std::string>();

        if (command.empty() || command.size() > 100) {
            return "Error: invalid command length.";
        }

        // 3. Read optional arguments.
        std::vector<std::string> commandArgs;

        if (input.contains("args")) {
            if (!input["args"].is_array()) {
                return "Error: args must be an array.";
            }

            const auto& argsArray = input["args"];

            if (argsArray.size() > 10) {
                return "Error: maximum 10 arguments allowed.";
            }

            for (const auto& arg : argsArray) {
                if (!arg.is_string()) {
                    return "Error: every argument must be a string.";
                }

                const std::string value =
                    arg.get<std::string>();

                if (value.size() > 500) {
                    return "Error: argument is too long.";
                }

                commandArgs.push_back(value);
            }
        }

        // 4. Delegate execution to Environment.
        return environment->executeCommand(
            command,
            commandArgs
        );
    }
    catch (const json::exception& e) {
        return std::string("JSON error: ") + e.what();
    }
    catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
