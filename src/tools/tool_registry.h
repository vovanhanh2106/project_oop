#pragma once

#include "tool.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class ToolRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<Tool>> tools;

    std::unordered_set<std::string> allowedTools;
    std::unordered_set<std::string> deniedTools;

public:
    // Đăng ký tool
    void registerTool(std::unique_ptr<Tool> tool);

    // Tìm tool theo tên
    Tool* getTool(const std::string& name);

    // Kiểm tra tool có tồn tại
    bool hasTool(const std::string& name) const;

    // Policy
    void allowTool(const std::string& name);
    void denyTool(const std::string& name);

    // Kiểm tra tool có được phép sử dụng không
    bool isAllowed(const std::string& name) const;
};