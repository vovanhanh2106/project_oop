
#pragma once

#include "tool.h"

#include <concepts>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

class ToolRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<Tool>> tools;

    std::unordered_set<std::string> allowedTools;
    std::unordered_set<std::string> deniedTools;

public:
    // Đăng ký Tool thông thường
    void registerTool(std::unique_ptr<Tool> tool);

    // C++20 Concepts:
    // Chỉ chấp nhận class kế thừa từ Tool
    template <typename T>
        requires std::derived_from<T, Tool>
    void registerTool(std::unique_ptr<T> tool) {
        registerTool(std::unique_ptr<Tool>(std::move(tool)));
    }

    std::vector<const Tool*> getAllTools() const;

    Tool* getTool(const std::string& name);

    bool hasTool(const std::string& name) const;

    void allowTool(const std::string& name);
    void denyTool(const std::string& name);

    bool isAllowed(const std::string& name) const;
};
