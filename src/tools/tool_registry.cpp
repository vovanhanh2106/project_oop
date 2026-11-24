#include "tool_registry.h"
#include <ranges> //c++23
void ToolRegistry::registerTool(std::unique_ptr<Tool> tool) {
    if (!tool) {
        return;
    }

    std::string name = tool->getName();

    tools[name] = std::move(tool);
}
//c++17
std::optional<std::reference_wrapper<Tool>>
ToolRegistry::getTool(std::string_view name) {
    auto it = tools.find(std::string(name));

    if (it == tools.end()) {
        return std::nullopt;
    }

    return std::ref(*it->second);
}

bool ToolRegistry::hasTool(const std::string& name) const {
    return tools.find(name) != tools.end();
}
std::vector<const Tool*> ToolRegistry::getAllTools() const {
    auto toolPointers =
        tools
        | std::views::values
        | std::views::transform([](const auto& tool) -> const Tool* {
              return tool.get();
          });

    return std::ranges::to<std::vector<const Tool*>>(toolPointers);
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