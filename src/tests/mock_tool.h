
#pragma once

#include "../tools/tool.h"

#include <string>

class MockTool : public Tool {
private:
    int executionCount = 0;

public:
    std::string getName() const override {
        return "mock_tool";
    }

    std::string getDescription() const override {
        return "Mock tool for integration testing";
    }

    std::string execute(
        const std::string& args
    ) override {
        ++executionCount;
        return "Mock tool executed successfully";
    }

    int getExecutionCount() const {
        return executionCount;
    }
};
