
#include "tool_factory.h"

#include <stdexcept>
#include <utility>

void ToolFactory::registerCreator(
    const std::string& name,
    Creator creator
) {
    if (name.empty()) {
        throw std::invalid_argument(
            "Tool name cannot be empty."
        );
    }

    if (!creator) {
        throw std::invalid_argument(
            "Tool creator cannot be empty."
        );
    }

    auto [it, inserted] = creators.emplace(
        name,
        std::move(creator)
    );

    if (!inserted) {
        throw std::runtime_error(
            "Tool creator already registered: " + name
        );
    }
}

std::unique_ptr<Tool> ToolFactory::createTool(
    const std::string& name
) const {
    auto it = creators.find(name);

    if (it == creators.end()) {
        throw std::runtime_error(
            "Unknown tool: " + name
        );
    }

    return it->second();
}

bool ToolFactory::hasCreator(
    const std::string& name
) const {
    return creators.find(name) != creators.end();
}
