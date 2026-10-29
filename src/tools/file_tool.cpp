
#include "file_tool.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

FileTool::FileTool(const std::filesystem::path& workspace)
    : workspace(std::filesystem::absolute(workspace).lexically_normal()) {
    std::filesystem::create_directories(this->workspace);
}

std::string FileTool::getName() const {
    return "file";
}

std::string FileTool::getDescription() const {
    return "Read or write text files inside the workspace. "
           "Arguments are JSON with action, path, and optionally content.";
}

std::string FileTool::execute(const std::string& args) {
    try {
        const json input = json::parse(args);

        if (!input.is_object() ||
            !input.contains("action") ||
            !input.contains("path") ||
            !input["action"].is_string() ||
            !input["path"].is_string()) {
            return "Error: expected JSON with string action and path.";
        }

        const std::string action = input["action"].get<std::string>();
        const std::filesystem::path relativePath =
            input["path"].get<std::string>();

        // Chỉ cho phép đường dẫn tương đối.
        if (relativePath.empty() || relativePath.is_absolute() ||
            relativePath.has_root_name()) {
            return "Error: path must be relative to the workspace.";
        }

        // Chặn đường dẫn đi ra ngoài workspace bằng ..
        for (const auto& part : relativePath) {
            if (part == "..") {
                return "Error: path traversal is not allowed.";
            }
        }

        const auto target =
            (workspace / relativePath).lexically_normal();

        // Kiểm tra đường dẫn vẫn nằm bên trong workspace.
        const auto relativeTarget =
            target.lexically_relative(workspace);

        if (relativeTarget.empty() ||
            relativeTarget.is_absolute() ||
            *relativeTarget.begin() == "..") {
            return "Error: path is outside the workspace.";
        }

        if (action == "read") {
            if (!std::filesystem::exists(target) ||
                !std::filesystem::is_regular_file(target)) {
                return "Error: file does not exist.";
            }

            std::ifstream file(target, std::ios::binary);

            if (!file) {
                return "Error: cannot open file for reading.";
            }

            std::ostringstream buffer;
            buffer << file.rdbuf();

            if (file.bad()) {
                return "Error: failed while reading file.";
            }

            return buffer.str();
        }

        if (action == "write") {
            if (!input.contains("content") ||
                !input["content"].is_string()) {
                return "Error: write action requires string content.";
            }

            // Tạo thư mục cha nếu chưa có.
            std::filesystem::create_directories(target.parent_path());

            std::ofstream file(
                target,
                std::ios::binary | std::ios::trunc
            );

            if (!file) {
                return "Error: cannot open file for writing.";
            }

            const std::string content =
                input["content"].get<std::string>();

            file << content;

            if (!file) {
                return "Error: failed while writing file.";
            }

            return "File written successfully: " +
                   relativePath.generic_string();
        }

        return "Error: action must be 'read' or 'write'.";
    }
    catch (const json::exception& e) {
        return std::string("JSON error: ") + e.what();
    }
    catch (const std::filesystem::filesystem_error& e) {
        return std::string("Filesystem error: ") + e.what();
    }
    catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
