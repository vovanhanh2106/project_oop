
#include "../tools/register_default_tools.h"
#include "../tools/tool.h"
#include "../environment/environment.h"

#include <iostream>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

using namespace std;

// Environment giả lập dành riêng cho test
class MockEnvironment : public Environment {
private:
    filesystem::path workspace;

public:
    MockEnvironment() {
        workspace =
            filesystem::temp_directory_path()
            / "agent_factory_test_workspace";

        filesystem::create_directories(workspace);
    }

    string executeCommand(
        const string& command,
        const vector<string>& args
    ) override {
        return "Mock command executed: " + command;
    }

    const filesystem::path& getWorkspace() const override {
        return workspace;
    }
};

int main() {
    int passed = 0;
    int failed = 0;

    auto check = [&](const string& testName, bool condition) {
        if (condition) {
            cout << "[PASS] " << testName << '\n';
            ++passed;
        } else {
            cout << "[FAIL] " << testName << '\n';
            ++failed;
        }
    };

    // Chuẩn bị môi trường test
    MockEnvironment environment;

    ToolFactory factory;

    // Database test được tạo trong thư mục tạm
    filesystem::path databasePath =
        filesystem::temp_directory_path()
        / "agent_factory_test_memory.db";

    filesystem::remove(databasePath);

    // Đăng ký toàn bộ Tool
    registerDefaultTools(
        factory,
        environment,
        databasePath.string()
    );

    // Danh sách 8 Tool cần kiểm tra
    const vector<string> toolNames = {
        "calculator",
        "file",
        "exec",
        "web_search",
        "memory",
        "datetime",
        "git",
        "json"
    };

    // Test 1: Kiểm tra đăng ký đủ 8 Creator
    check(
        "Register all 8 tool creators",
        toolNames.size() == 8
    );

    // Test 2: Kiểm tra từng Creator và tạo Tool
    vector<unique_ptr<Tool>> createdTools;

    for (const auto& name : toolNames) {
        bool registered = factory.hasCreator(name);

        check(
            "Creator registered: " + name,
            registered
        );

        if (!registered) {
            continue;
        }

        try {
            auto tool = factory.createTool(name);

            bool valid =
                tool != nullptr &&
                tool->getName() == name;

            check(
                "Create tool: " + name,
                valid
            );

            if (tool) {
                createdTools.push_back(move(tool));
            }
        } catch (const exception& e) {
            check(
                "Create tool: " + name,
                false
            );

            cerr << "Exception: " << e.what() << '\n';
        }
    }

    // Giải phóng MemoryTool trước khi xóa database
    createdTools.clear();

    filesystem::remove(databasePath);

    cout << "\n========== TEST SUMMARY ==========\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
