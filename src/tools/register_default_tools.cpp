
#include "register_default_tools.h"

#include "calculator_tool.h"
#include "file_tool.h"
#include "exec_tool.h"
#include "web_search_tool.h"
#include "memory_tool.h"
#include "datetime_tool.h"
#include "git_tool.h"
#include "json_tool.h"

#include <memory>

void registerDefaultTools(
    ToolFactory& factory,
    Environment& environment,
    const std::string& databasePath
) {
    // Không cần dependency
    factory.registerCreator(
        "calculator",
        []() {
            return std::make_unique<CalculatorTool>();
        }
    );

    factory.registerCreator(
        "web_search",
        []() {
            return std::make_unique<WebSearchTool>();
        }
    );

    factory.registerCreator(
        "datetime",
        []() {
            return std::make_unique<DateTimeTool>();
        }
    );

    factory.registerCreator(
        "git",
        []() {
            return std::make_unique<GitTool>();
        }
    );

    factory.registerCreator(
        "json",
        []() {
            return std::make_unique<JsonTool>();
        }
    );

    // Cần Environment
    factory.registerCreator(
        "file",
        [&environment]() {
            return std::make_unique<FileTool>(environment);
        }
    );

    factory.registerCreator(
        "exec",
        [&environment]() {
            return std::make_unique<ExecTool>(environment);
        }
    );

    // Cần databasePath
    factory.registerCreator(
        "memory",
        [databasePath]() {
            return std::make_unique<MemoryTool>(databasePath);
        }
    );
}
