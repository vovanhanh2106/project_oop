
#include "numeric_evaluator.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <regex>
#include <string>

namespace {


bool extractNumber(
    const std::string& text,
    long double& number
) {
    static const std::regex numberPattern(
        R"([-+]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][-+]?[0-9]+)?)"
    );

    std::vector<long double> numbers;

    for (
        std::sregex_iterator it(
            text.begin(),
            text.end(),
            numberPattern
        );
        it != std::sregex_iterator();
        ++it
    ) {
        try {
            numbers.push_back(std::stold(it->str()));
        }
        catch (...) {
            return false;
        }
    }

    if (numbers.empty()) {
        return false;
    }

    // Chỉ có một số trong câu trả lời.
    if (numbers.size() == 1) {
        number = numbers.front();
        return std::isfinite(number);
    }

    // Tìm kết quả sau từ "is" ở cuối câu.
    static const std::regex finalAnswerPattern(
        R"(\bis\s+([-+]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][-+]?[0-9]+)?)\s*[.!]?\s*$)",
        std::regex_constants::icase
    );

    std::smatch match;

    if (std::regex_search(text, match, finalAnswerPattern)) {
        try {
            number = std::stold(match[1].str());
            return std::isfinite(number);
        }
        catch (...) {
            return false;
        }
    }

    return false;
}

} // namespace

EvaluationResult NumericEvaluator::evaluate(
    const std::string& expected,
    const std::string& actual
) const {
    long double expectedNumber = 0.0L;
    long double actualNumber = 0.0L;

    if (!extractNumber(expected, expectedNumber)) {
        return {
            false,
            0.0,
            "Expected result must contain a valid number."
        };
    }

    if (!extractNumber(actual, actualNumber)) {
        return {
            false,
            0.0,
            "Actual result must contain a clear numerical answer."
        };
    }

    constexpr long double tolerance = 1e-9L;

    const long double difference =
        std::fabs(expectedNumber - actualNumber);

    const long double scale = std::max({
        1.0L,
        std::fabs(expectedNumber),
        std::fabs(actualNumber)
    });

    if (difference <= tolerance * scale) {
        return {
            true,
            1.0,
            "Numeric result matches the expected value."
        };
    }

    return {
        false,
        0.0,
        "Numeric result does not match the expected value."
    };
}
