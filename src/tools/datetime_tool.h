
#pragma once

#include "tool.h"

class DateTimeTool : public Tool {
public:
    std::string getName() const override;

    std::string getDescription() const override;

    std::string execute(
        const std::string& args
    ) override;
};
