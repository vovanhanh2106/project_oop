
#pragma once

#include "tool.h"

#include <filesystem>
#include <string>

class FileTool : public Tool {
private:
    std::filesystem::path workspace;

public:
    explicit FileTool(
        const std::filesystem::path& workspace = "workspace"
    );

    std::string getName() const override;

    std::string getDescription() const override;

    std::string execute(const std::string& args) override;
};
