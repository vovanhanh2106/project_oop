
#pragma once

#include "tool.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class ToolFactory {
public:
    using Creator = std::function<std::unique_ptr<Tool>()>;

private:
    std::unordered_map<std::string, Creator> creators;

public:
    // Đăng ký một hàm tạo Tool
    void registerCreator(
        const std::string& name,
        Creator creator
    );

    // Tạo Tool theo tên
    std::unique_ptr<Tool> createTool(
        const std::string& name
    ) const;

    // Kiểm tra Factory có hỗ trợ Tool không
    bool hasCreator(const std::string& name) const;
};
