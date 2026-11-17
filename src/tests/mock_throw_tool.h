
#pragma once

#include "../tools/tool.h"

#include <stdexcept>
#include <string>

class MockThrowingTool : public Tool {
public:
    std::string getName() const override {
        return "throwing_tool";
    }

    std::string getDescription() const override {
        return "A tool that throws an exception";
    }

    std::string execute(const std::string& args) override {
        throw std::runtime_error("Mock tool failure");
    }
};
