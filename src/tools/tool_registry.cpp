#include "tool_registry.h"

void ToolRegistry::registerTool(std::unique_ptr<Tool> tool) {
    if (!tool) {
        return;
    }

    std::string name = tool->getName();

    tools[name] = std::move(tool);
}

Tool* ToolRegistry::getTool(const std::string& name) {
    auto it = tools.find(name);

    if (it == tools.end()) {
        return nullptr;
    }

    return it->second.get();
}

bool ToolRegistry::hasTool(const std::string& name) const {
    return tools.find(name) != tools.end();
}

void ToolRegistry::allowTool(const std::string& name) {
    allowedTools.insert(name);

    // Nếu trước đó bị deny thì bỏ deny
    deniedTools.erase(name);
}

void ToolRegistry::denyTool(const std::string& name) {
    deniedTools.insert(name);
}

bool ToolRegistry::isAllowed(const std::string& name) const {

    // Không tồn tại tool
    if (!hasTool(name)) {
        return false;
    }

    // Nếu nằm trong danh sách deny
    if (deniedTools.find(name) != deniedTools.end()) {
        return false;
    }

    // Nếu không cấu hình allowlist
    // thì mặc định cho phép
    if (allowedTools.empty()) {
        return true;
    }

    // Có allowlist -> phải nằm trong allowlist
    return allowedTools.find(name) != allowedTools.end();
}