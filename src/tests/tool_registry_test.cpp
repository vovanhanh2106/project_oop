#include "../tools/tool_registry.h"
#include <windows.h>

#include <iostream>
#include <memory>
#include <cassert>
#include <string>

using namespace std;

// Tool giả lập với tên tùy chỉnh
class NamedMockTool : public Tool {
private:
    string name;

public:
    explicit NamedMockTool(const string& name) : name(name) {}

    string getName() const override {
        return name;
    }

    string getDescription() const override {
        return "Mock tool";
    }

    string execute(const string& args) override {
        return "Executed";
    }
};

int totalTests = 0;
int passedTests = 0;

void check(bool condition, const string& description) {
    totalTests++;

    if (condition) {
        passedTests++;
        cout << "[PASS] " << description << '\n';
    } else {
        cout << "[FAIL] " << description << '\n';
    }
}

// Test 1: Đăng ký và tìm tool
void testRegisterAndGetTool() {
    cout << "\n=== Test 1: Register and Get Tool ===\n";

    ToolRegistry registry;

    registry.registerTool(make_unique<NamedMockTool>("tool_A"));

    check(registry.hasTool("tool_A"),
          "Tool được đăng ký thành công");

  auto tool = registry.getTool("tool_A");

check(tool.has_value(),
      "getTool trả về optional có giá trị khi tìm thấy");

check(tool->get().getName() == "tool_A",
      "getTool trả về đúng tool");

  auto unknown = registry.getTool("unknown");

check(!unknown.has_value(),
      "getTool trả về nullopt khi không tồn tại");

    check(!registry.hasTool("unknown"),
          "hasTool trả về false khi không tồn tại");
}

// Test 2: Đăng ký nullptr
void testRegisterNullptr() {
    cout << "\n=== Test 2: Register nullptr ===\n";

    ToolRegistry registry;

    registry.registerTool(nullptr);

    check(registry.getAllTools().empty(),
          "Không đăng ký tool khi truyền nullptr");

    check(!registry.hasTool("anything"),
          "Registry vẫn rỗng sau khi đăng ký nullptr");
}

// Test 3: getAllTools
void testGetAllTools() {
    cout << "\n=== Test 3: Get All Tools ===\n";

    ToolRegistry registry;

    registry.registerTool(make_unique<NamedMockTool>("tool_A"));
    registry.registerTool(make_unique<NamedMockTool>("tool_B"));
    registry.registerTool(make_unique<NamedMockTool>("tool_C"));

    vector<const Tool*> allTools = registry.getAllTools();

    check(allTools.size() == 3,
          "getAllTools trả về đủ 3 tool");

    check(allTools.size() == registry.getAllTools().size(),
          "Số lượng tool không thay đổi khi gọi getAllTools");
}

// Test 4: Mặc định cho phép
void testDefaultPermission() {
    cout << "\n=== Test 4: Default Permission ===\n";

    ToolRegistry registry;

    registry.registerTool(make_unique<NamedMockTool>("tool_A"));

    check(registry.isAllowed("tool_A"),
          "Tool được phép khi chưa cấu hình allowlist hoặc denylist");

    check(!registry.isAllowed("unknown"),
          "Tool không tồn tại luôn bị từ chối");
}

// Test 5: Deny tool
void testDenyTool() {
    cout << "\n=== Test 5: Deny Tool ===\n";

    ToolRegistry registry;

    registry.registerTool(make_unique<NamedMockTool>("tool_A"));

    registry.denyTool("tool_A");

    check(!registry.isAllowed("tool_A"),
          "Tool bị deny không được phép");

    check(registry.hasTool("tool_A"),
          "Deny không xóa tool khỏi registry");
}

// Test 6: Allowlist
void testAllowlist() {
    cout << "\n=== Test 6: Allowlist ===\n";

    ToolRegistry registry;

    registry.registerTool(make_unique<NamedMockTool>("tool_A"));
    registry.registerTool(make_unique<NamedMockTool>("tool_B"));
    registry.registerTool(make_unique<NamedMockTool>("tool_C"));

    registry.allowTool("tool_A");

    check(registry.isAllowed("tool_A"),
          "Tool nằm trong allowlist được phép");

    check(!registry.isAllowed("tool_B"),
          "Tool không nằm trong allowlist bị từ chối");

    check(!registry.isAllowed("tool_C"),
          "Các tool khác cũng bị từ chối");
}

// Test 7: Allow sau Deny
void testAllowAfterDeny() {
    cout << "\n=== Test 7: Allow After Deny ===\n";

    ToolRegistry registry;

    registry.registerTool(make_unique<NamedMockTool>("tool_A"));

    registry.denyTool("tool_A");

    check(!registry.isAllowed("tool_A"),
          "Tool bị từ chối sau khi deny");

    registry.allowTool("tool_A");

    check(registry.isAllowed("tool_A"),
          "Allow xóa trạng thái deny trước đó");
}

// Test 8: Deny sau Allow
void testDenyAfterAllow() {
    cout << "\n=== Test 8: Deny After Allow ===\n";

    ToolRegistry registry;

    registry.registerTool(make_unique<NamedMockTool>("tool_A"));

    registry.allowTool("tool_A");

    check(registry.isAllowed("tool_A"),
          "Tool được phép sau khi allow");

    registry.denyTool("tool_A");

    check(!registry.isAllowed("tool_A"),
          "Deny có quyền ưu tiên hơn allow");
}

// Test 9: Đăng ký trùng tên
void testDuplicateRegistration() {
    cout << "\n=== Test 9: Duplicate Registration ===\n";

    ToolRegistry registry;

    registry.registerTool(make_unique<NamedMockTool>("tool_A"));
    registry.registerTool(make_unique<NamedMockTool>("tool_A"));

    check(registry.getAllTools().size() == 1,
          "Đăng ký trùng tên không tạo thêm tool");

  check(registry.getTool("tool_A").has_value(),
      "Tool vẫn tồn tại sau khi đăng ký trùng tên");
}

int main() {
      SetConsoleOutputCP(CP_UTF8);
SetConsoleCP(CP_UTF8);
    testRegisterAndGetTool();
    testRegisterNullptr();
    testGetAllTools();
    testDefaultPermission();
    testDenyTool();
    testAllowlist();
    testAllowAfterDeny();
    testDenyAfterAllow();
    testDuplicateRegistration();

    cout << "\n================================\n";
    cout << "Passed: " << passedTests << "/" << totalTests << '\n';
    cout << "Failed: " << totalTests - passedTests << '\n';
    cout << "================================\n";

    return passedTests == totalTests ? 0 : 1;
}