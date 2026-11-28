#pragma once

#include "tool_factory.h"
#include "../environment/environment.h"

#include <string>

void registerDefaultTools(
    ToolFactory& factory,
    Environment& environment,
    const std::string& databasePath = "agent_memory.db"
);