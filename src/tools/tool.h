#pragma once

#include <string>
#include <functional>

class Tool {
public:
    virtual ~Tool() = default;

    virtual std::string getName() const = 0;

    virtual std::string getDescription() const = 0;

    virtual std::string execute(
        const std::string& args
    ) = 0;
};