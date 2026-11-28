
#pragma once

#include "environment.h"

#include <filesystem>
#include <string>
#include <vector>

class NativeEnvironment : public Environment {
private:
    std::filesystem::path workspace;

public:
    explicit NativeEnvironment(
        const std::filesystem::path& workspace = "workspace"
    );

    std::string executeCommand(
        const std::string& command,
        const std::vector<std::string>& args
    ) override;

    const std::filesystem::path& getWorkspace() const override;
};
