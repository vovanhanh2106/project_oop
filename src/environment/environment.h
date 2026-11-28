
#pragma once

#include <filesystem>
#include <string>
#include <vector>

class Environment {
public:
    virtual ~Environment() = default;

    // Execute an allowed command with arguments.
    virtual std::string executeCommand(
        const std::string& command,
        const std::vector<std::string>& args
    ) = 0;

    // Get the workspace of the current environment.
    virtual const std::filesystem::path& getWorkspace() const = 0;
};
