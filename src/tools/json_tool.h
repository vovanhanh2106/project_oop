
#pragma once

#include "tool.h"
#include <string>

class JsonTool : public Tool {
public:
    std::string getName() const override;
    std::string getDescription() const override;
    std::string execute(const std::string& args) override;
};
