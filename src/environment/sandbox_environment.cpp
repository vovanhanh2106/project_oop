
#include "sandbox_environment.h"

#include <stdexcept>
#include <utility>

SandboxEnvironment::SandboxEnvironment(
    const std::filesystem::path& workspace
)
    : executor(workspace),
      allowedCommands{"whoami", "hostname"} {
}

std::string SandboxEnvironment::executeCommand(
    const std::string& command,
    const std::vector<std::string>& args
) {
    // 1. Check Sandbox whitelist.
    if (!allowedCommands.contains(command)) {
        return "Error: command is not allowed in sandbox.";
    }

    // 2. Delegate process execution to NativeEnvironment.
    // NativeEnvironment will also validate arguments.
    return executor.executeCommand(command, args);
}

const std::filesystem::path&
SandboxEnvironment::getWorkspace() const {
    return executor.getWorkspace();
}
