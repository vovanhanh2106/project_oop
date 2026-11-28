
#pragma once

#include "environment.h"
#include "native_environment.h"

#include <string>
#include <unordered_set>
#include <vector>

class SandboxEnvironment : public Environment {
private:
    NativeEnvironment executor;
    std::unordered_set<std::string> allowedCommands;

public:
    explicit SandboxEnvironment(
        const std::filesystem::path& workspace = "sandbox_workspace"
    );

    std::string executeCommand(
        const std::string& command,
        const std::vector<std::string>& args
    ) override;

    const std::filesystem::path& getWorkspace() const override;
};
