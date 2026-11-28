#pragma once

#include "tool.h"
#include "../environment/environment.h"

#include <filesystem>
#include <memory>
#include <string>

class FileTool : public Tool {
private:
    std::unique_ptr<Environment> ownedEnvironment;
    std::filesystem::path workspace;

    bool isPathInsideWorkspace(
        const std::filesystem::path& path
    ) const;

public:
    explicit FileTool();
    explicit FileTool(const Environment& environment);

    std::string getName() const override;

    std::string getDescription() const override;

    std::string execute(const std::string& args) override;
};