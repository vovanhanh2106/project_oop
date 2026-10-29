
#pragma once

#include "tool.h"

#include <string>
#include <vector>

class ExecTool : public Tool {
private:
    std::vector<std::string> allowedCommands;

public:
    ExecTool();

    std::string getName() const override;

    std::string getDescription() const override;

    std::string execute(const std::string& args) override;
};
