#pragma once

#include "tool.h"

class CalculatorTool : public Tool {
public:
    std::string getName() const override;

    std::string getDescription() const override;

    std::string execute(
        const std::string& args
    ) override;
};