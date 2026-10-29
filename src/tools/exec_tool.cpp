
#include "exec_tool.h"

#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

ExecTool::ExecTool()
    : allowedCommands{"whoami", "hostname"} {}

std::string ExecTool::getName() const {
    return "exec";
}

std::string ExecTool::getDescription() const {
    return
        "Run an allowlisted system information command. "
        "Input JSON: {\"command\":\"whoami\"} "
        "or {\"command\":\"hostname\"}.";
}

std::string ExecTool::execute(const std::string& args) {
    try {
        const json input = json::parse(args);

        if (!input.is_object() ||
            !input.contains("command") ||
            !input["command"].is_string()) {
            return "Error: expected JSON with a string command.";
        }

        const std::string command =
            input["command"].get<std::string>();

        // Chỉ cho phép các lệnh trong allowlist.
        bool allowed = false;

        for (const auto& item : allowedCommands) {
            if (item == command) {
                allowed = true;
                break;
            }
        }

        if (!allowed) {
            return "Error: command is not allowed.";
        }

        // Chỉ ánh xạ sang executable cố định, không ghép
        // dữ liệu do người dùng cung cấp vào shell command.
        std::string executable;

        if (command == "whoami") {
            executable = "whoami";
        } else if (command == "hostname") {
            executable = "hostname";
        } else {
            return "Error: unsupported command.";
        }

        // Chuyển output về file tạm để đọc kết quả.
        // Tên file tạm cố định chỉ phù hợp cho bản demo đơn luồng.
        const std::filesystem::path outputFile =
            std::filesystem::temp_directory_path()
            / "ai_agent_exec_output.txt";

        const std::string fullCommand =
            "\"" + executable + "\" > \"" +
            outputFile.string() + "\" 2>&1";

        const int exitCode = std::system(fullCommand.c_str());

        std::ifstream file(outputFile, std::ios::binary);
        std::ostringstream buffer;

        if (file) {
            buffer << file.rdbuf();
        }

        std::error_code ec;
        std::filesystem::remove(outputFile, ec);

        const std::string output = buffer.str();

        if (exitCode != 0) {
            return "Command failed. Output: " + output;
        }

        return output.empty() ? "(no output)" : output;
    }
    catch (const json::exception& e) {
        return std::string("JSON error: ") + e.what();
    }
    catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
