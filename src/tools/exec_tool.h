#pragma once

#include "tool.h"
#include "../environment/environment.h"

#include <memory>
#include <string>

class ExecTool : public Tool {
private:
    std::unique_ptr<Environment> ownedEnvironment;
    Environment* environment;

public:
    explicit ExecTool();
    explicit ExecTool(Environment& environment);

    std::string getName() const override;

    std::string getDescription() const override;

    std::string execute(const std::string& args) override;
};