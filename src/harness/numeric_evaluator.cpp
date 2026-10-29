
#include "numeric_evaluator.h"
#include <algorithm>
#include <cmath>
#include <regex>
#include <string>

namespace {

bool extractSingleNumber(
    const std::string& text,
    long double& number
) {
    // Nhận số nguyên, số âm và số thập phân.
    // Ví dụ: 221, -5, 3.14, .5
    static const std::regex numberPattern(
        R"([-+]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][-+]?[0-9]+)?)"
    );

    std::sregex_iterator it(
        text.begin(),
        text.end(),
        numberPattern
    );

    const std::sregex_iterator end;

    // Không tìm thấy số
    if (it == end) {
        return false;
    }

    const std::string firstNumber = it->str();
    ++it;

    // Có nhiều số thì không đoán số nào là đáp án
    if (it != end) {
        return false;
    }

    try {
        std::size_t parsedLength = 0;
        number = std::stold(firstNumber, &parsedLength);

        return parsedLength == firstNumber.size()
            && std::isfinite(number);
    }
    catch (...) {
        return false;
    }
}

} // namespace

EvaluationResult NumericEvaluator::evaluate(
    const std::string& expected,
    const std::string& actual
) const {
    long double expectedNumber = 0.0L;
    long double actualNumber = 0.0L;

    if (!extractSingleNumber(expected, expectedNumber)) {
        return {
            false,
            0.0,
            "Expected result must contain exactly one valid number."
        };
    }

    if (!extractSingleNumber(actual, actualNumber)) {
        return {
            false,
            0.0,
            "Actual result must contain exactly one valid number."
        };
    }

    // Dung sai tương đối và tuyệt đối cho số thập phân
    constexpr long double tolerance = 1e-9L;

    const long double difference =
        std::fabs(expectedNumber - actualNumber);

    const long double scale =
        std::max(
            1.0L,
            std::max(
                std::fabs(expectedNumber),
                std::fabs(actualNumber)
            )
        );

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
