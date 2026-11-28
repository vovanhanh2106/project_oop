
#include "tools/exec_tool.h"
#include "environment/native_environment.h"
#include "environment/sandbox_environment.h"

#include <iostream>
#include <string>
#include <vector>

struct TestResult {
    std::string name;
    bool passed;
};

int passedCount = 0;
int failedCount = 0;

void runTest(
    const std::string& name,
    ExecTool& tool,
    const std::string& input,
    const std::string& expected,
    bool expectSuccess = false
) {
    const std::string result = tool.execute(input);

    const bool passed = expectSuccess
        ? (!result.empty() && result.rfind("Error:", 0) != 0)
        : (result.find(expected) != std::string::npos);

    if (passed) {
        ++passedCount;
        std::cout << "[PASS] " << name << '\n';
    }
    else {
        ++failedCount;
        std::cout << "[FAIL] " << name << '\n';
        std::cout << "Expected: "
                  << (expectSuccess ? "successful execution" : expected)
                  << '\n';
        std::cout << "Actual: " << result << '\n';
    }
}
int main() {
    NativeEnvironment native;
    SandboxEnvironment sandbox;

    ExecTool nativeTool(native);
    ExecTool sandboxTool(sandbox);

    std::cout << "========== ExecTool Automated Tests ==========\n";

    // Native: allowed commands
    runTest(
    "Native allows whoami",
    nativeTool,
    R"({"command":"whoami"})",
    "",
    true
);

runTest(
    "Sandbox allows hostname",
    sandboxTool,
    R"({"command":"hostname"})",
    "",
    true
);

    runTest(
        "Native allows ipconfig /all",
        nativeTool,
        R"({"command":"ipconfig","args":["/all"]})",
        "Windows IP Configuration"
    );

    // Native: rejected commands and arguments
    runTest(
        "Native rejects cmd.exe",
        nativeTool,
        R"({"command":"cmd.exe"})",
        "Command is not allowed"
    );

    runTest(
        "Native rejects invalid ipconfig argument",
        nativeTool,
        R"({"command":"ipconfig","args":["/release"]})",
        "Allowed ipconfig argument"
    );

    runTest(
        "Native rejects command injection",
        nativeTool,
        R"({"command":"whoami && hostname"})",
        "Command is not allowed"
    );

    runTest(
        "Native rejects shell operators in arguments",
        nativeTool,
        R"({"command":"whoami","args":["&&","hostname"]})",
        "does not accept arguments"
    );

    runTest(
        "Native rejects injected ipconfig argument",
        nativeTool,
        R"({"command":"ipconfig","args":["/all & whoami"]})",
        "Allowed ipconfig argument"
    );

    // Sandbox: allowed and rejected commands
    runTest(
        "Sandbox allows hostname",
        sandboxTool,
        R"({"command":"hostname"})",
        "LAPTOP-" // Placeholder: replace with a generic success check
    );

    runTest(
        "Sandbox rejects ipconfig",
        sandboxTool,
        R"({"command":"ipconfig"})",
        "not allowed in sandbox"
    );

    std::cout << "\n========== Test Summary ==========\n";
    std::cout << "Passed: " << passedCount << '\n';
    std::cout << "Failed: " << failedCount << '\n';

    return failedCount == 0 ? 0 : 1;
}
