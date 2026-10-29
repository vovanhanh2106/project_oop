
#include "keyword_evaluator.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string trim(const std::string& text) {
    const std::size_t first = text.find_first_not_of(" \t\n\r");

    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = text.find_last_not_of(" \t\n\r");

    return text.substr(first, last - first + 1);
}

std::string toLowerAscii(std::string text) {
    std::transform(
        text.begin(),
        text.end(),
        text.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return text;
}

std::vector<std::string> splitKeywords(
    const std::string& expected
) {
    std::vector<std::string> keywords;
    std::stringstream stream(expected);
    std::string keyword;

    while (std::getline(stream, keyword, ',')) {
        keyword = toLowerAscii(trim(keyword));

        if (!keyword.empty()) {
            keywords.push_back(keyword);
        }
    }

    return keywords;
}

} // namespace

EvaluationResult KeywordEvaluator::evaluate(
    const std::string& expected,
    const std::string& actual
) const {
    const std::vector<std::string> keywords =
        splitKeywords(expected);

    if (keywords.empty()) {
        return {
            false,
            0.0,
            "No expected keywords were provided."
        };
    }

    const std::string normalizedActual = toLowerAscii(actual);

    std::size_t matched = 0;
    std::vector<std::string> missing;

    for (const std::string& keyword : keywords) {
        if (normalizedActual.find(keyword) != std::string::npos) {
            ++matched;
        } else {
            missing.push_back(keyword);
        }
    }

    const double score =
        static_cast<double>(matched) /
        static_cast<double>(keywords.size());

    if (missing.empty()) {
        return {
            true,
            1.0,
            "All expected keywords were found."
        };
    }

    std::string feedback = "Missing keywords: ";

    for (std::size_t i = 0; i < missing.size(); ++i) {
        if (i > 0) {
            feedback += ", ";
        }

        feedback += missing[i];
    }

    return {
        false,
        score,
        feedback
    };
}
