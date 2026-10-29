
#include "functional_evaluator.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace {

std::string trim(const std::string& text) {
    const std::size_t first = text.find_first_not_of(" \t\n\r");

    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = text.find_last_not_of(" \t\n\r");

    return text.substr(first, last - first + 1);
}

std::string normalize(const std::string& text) {
    std::string result = trim(text);

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return result;
}

} // namespace

EvaluationResult FunctionalEvaluator::evaluate(
    const std::string& expected,
    const std::string& actual
) const {
    const std::string normalizedExpected = normalize(expected);
    const std::string normalizedActual = normalize(actual);

    if (normalizedExpected.empty()) {
        return {
            false,
            0.0,
            "Expected result cannot be empty."
        };
    }

    if (normalizedExpected == normalizedActual) {
        return {
            true,
            1.0,
            "Actual result matches the expected result."
        };
    }

    return {
        false,
        0.0,
        "Actual result does not match the expected result."
    };
}
