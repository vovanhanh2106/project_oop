#include "git_tool.h"

#include <nlohmann/json.hpp>
#include <cstdio>
#include <array>
#include <memory>
#include <string>

using json = nlohmann::json;

std::string GitTool::getName() const {
    return "git";
}

std::string GitTool::getDescription() const {
    return R"(
Inspect a Git repository.

Arguments must be a JSON object:
- command: status, log, branch, or diff
- path: optional repository directory

Examples:
{"command":"status"}
{"command":"log"}
{"command":"branch"}
{"command":"diff"}
)";
}

std::string GitTool::execute(const std::string& args) {
    try {
        json input = json::parse(args);

        if (!input.is_object()) {
            return "Error: Arguments must be a JSON object.";
        }

        const std::string command = input.value("command", "");
        const std::string path = input.value("path", ".");

        std::string gitCommand;

        if (command == "status") {
            gitCommand = "git status --short --branch";
        }
        else if (command == "log") {
            gitCommand = "git log -5 --oneline";
        }
        else if (command == "branch") {
            gitCommand = "git branch --list";
        }
        else if (command == "diff") {
            gitCommand = "git diff --no-ext-diff --";
        }
        else {
            return "Error: Unsupported command. "
                   "Use status, log, branch, or diff.";
        }

        // Restrict the repository path to the project's workspace.
        // The tool only accepts the default current directory.
        if (path != ".") {
            return "Error: Custom repository paths are disabled. "
                   "Run the agent from the target repository.";
        }

        std::array<char, 256> buffer{};
        std::string output;

#ifdef _WIN32
        std::string fullCommand =
            "git -C . " + gitCommand.substr(4) + " 2>&1";
        FILE* pipe = _popen(fullCommand.c_str(), "r");
#else
        std::string fullCommand =
            "git -C . " + gitCommand.substr(4) + " 2>&1";
        FILE* pipe = popen(fullCommand.c_str(), "r");
#endif

        if (!pipe) {
            return "Error: Failed to execute Git command.";
        }

        while (fgets(buffer.data(),
                     static_cast<int>(buffer.size()),
                     pipe) != nullptr) {
            output += buffer.data();
        }

#ifdef _WIN32
        int exitCode = _pclose(pipe);
#else
        int exitCode = pclose(pipe);
#endif

        if (exitCode != 0) {
            return "Git command failed:\n" + output;
        }

        if (output.empty()) {
            return "Git command completed successfully. No output.";
        }

        return output;
    }
    catch (const json::exception& e) {
        return std::string("Error: ") + e.what();
    }
}