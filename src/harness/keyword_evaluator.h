#pragma once

#include "evaluator.h"

#include <string>

class KeywordEvaluator : public Evaluator {
public:
    EvaluationResult evaluate(
        const std::string& expected,
        const std::string& actual
    ) const override;
};