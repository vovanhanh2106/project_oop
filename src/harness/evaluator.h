#pragma once

#include <string>

struct EvaluationResult {
    bool passed = false;
    double score = 0.0;
    std::string feedback;
};

class Evaluator {
public:
    virtual ~Evaluator() = default;

    virtual EvaluationResult evaluate(
        const std::string& expected,
        const std::string& actual
    ) const = 0;
};