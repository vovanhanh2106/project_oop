
#include "../tools/tool_factory.h"
#include "../tools/calculator_tool.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using namespace std;

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

    ToolFactory factory;

    // Test 1: Đăng ký Creator
    factory.registerCreator(
        "calculator",
        []() {
            return make_unique<CalculatorTool>();
        }
    );

    check(
        "Register calculator creator",
        factory.hasCreator("calculator")
    );

    // Test 2: Tạo CalculatorTool
    auto tool = factory.createTool("calculator");

    check(
        "Create calculator successfully",
        tool != nullptr
    );

    // Test 3: Kiểm tra tên Tool
    check(
        "Created tool has correct name",
        tool && tool->getName() == "calculator"
    );

    // Test 4: Tạo hai instance độc lập
    auto tool2 = factory.createTool("calculator");

    check(
        "Create independent tool instances",
        tool2 != nullptr && tool2.get() != tool.get()
    );

    // Test 5: Tên không tồn tại
    bool unknownToolRejected = false;

    try {
        factory.createTool("unknown");
    } catch (const runtime_error&) {
        unknownToolRejected = true;
    }

    check(
        "Reject unknown tool name",
        unknownToolRejected
    );

    // Test 6: Tên đăng ký rỗng
    bool emptyNameRejected = false;

    try {
        factory.registerCreator(
            "",
            []() {
                return make_unique<CalculatorTool>();
            }
        );
    } catch (const invalid_argument&) {
        emptyNameRejected = true;
    }

    check(
        "Reject empty creator name",
        emptyNameRejected
    );

    // Test 7: Creator rỗng
    bool emptyCreatorRejected = false;

    try {
        factory.registerCreator(
            "empty",
            ToolFactory::Creator{}
        );
    } catch (const invalid_argument&) {
        emptyCreatorRejected = true;
    }

    check(
        "Reject empty creator",
        emptyCreatorRejected
    );

    // Test 8: Đăng ký trùng tên
    bool duplicateRejected = false;

    try {
        factory.registerCreator(
            "calculator",
            []() {
                return make_unique<CalculatorTool>();
            }
        );
    } catch (const runtime_error&) {
        duplicateRejected = true;
    }

    check(
        "Reject duplicate creator",
        duplicateRejected
    );

    cout << "\n========== TEST SUMMARY ==========\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
