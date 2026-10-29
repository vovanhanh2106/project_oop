
#pragma once

#include "evaluator.h"

#include <string>

class NumericEvaluator : public Evaluator {
public:
    EvaluationResult evaluate(
        const std::string& expected,
        const std::string& actual
    ) const override;
};
