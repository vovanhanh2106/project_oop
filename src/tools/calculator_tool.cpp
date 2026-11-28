
#include "calculator_tool.h"
#include <iostream>
#include <charconv>  // C++17: std::from_chars
#include <cctype>
#include <expected>
#include <stdexcept>
#include <string>
#include <cmath>
namespace {

class Parser {
private:
    const std::string& expression;
    size_t pos = 0;

    void skipSpaces() {
        while (
            pos < expression.size() &&
            std::isspace(
                static_cast<unsigned char>(expression[pos])
            )
        ) {
            ++pos;
        }
    }
    void checkResult(double value) {
    if (!std::isfinite(value)) {
        throw std::runtime_error(
            "Calculation result is out of range"
        );
    }
}
    
double parseNumber() {
    skipSpaces();

    size_t start = pos;

    bool hasDigit = false;

    // Đọc phần nguyên.
    while (
        pos < expression.size() &&
        std::isdigit(
            static_cast<unsigned char>(expression[pos])
        )
    ) {
        hasDigit = true;
        ++pos;
    }

    // Đọc phần thập phân.
    if (
        pos < expression.size() &&
        expression[pos] == '.'
    ) {
        ++pos;

        while (
            pos < expression.size() &&
            std::isdigit(
                static_cast<unsigned char>(expression[pos])
            )
        ) {
            hasDigit = true;
            ++pos;
        }
    }

    if (!hasDigit) {
        throw std::runtime_error("Invalid number");
    }

    // Đọc phần số mũ: e hoặc E.
    if (
        pos < expression.size() &&
        (expression[pos] == 'e' || expression[pos] == 'E')
    ) {
        ++pos;

        // Số mũ có thể có dấu + hoặc -.
        if (
            pos < expression.size() &&
            (expression[pos] == '+' || expression[pos] == '-')
        ) {
            ++pos;
        }

        size_t exponentStart = pos;

        while (
            pos < expression.size() &&
            std::isdigit(
                static_cast<unsigned char>(expression[pos])
            )
        ) {
            ++pos;
        }

        if (exponentStart == pos) {
            throw std::runtime_error("Invalid exponent");
        }
    }

    double value;

    const char* first = expression.data() + start;
    const char* last = expression.data() + pos;

    auto [ptr, error] = std::from_chars(
        first,
        last,
        value,
        std::chars_format::general
    );

    if (
        error != std::errc{} ||
        ptr != last ||
        !std::isfinite(value)
    ) {
        throw std::runtime_error("Invalid or out-of-range number");
    }

    return value;
}

    double parseFactor() {
        skipSpaces();

        if (
            pos < expression.size() &&
            expression[pos] == '('
        ) {
            ++pos;

            double result = parseExpressionInternal();

            skipSpaces();

            if (
                pos >= expression.size() ||
                expression[pos] != ')'
            ) {
                throw std::runtime_error("Missing ')'");
            }

            ++pos;

            return result;
        }

        if (
            pos < expression.size() &&
            expression[pos] == '-'
        ) {
            ++pos;
            return -parseFactor();
        }

        return parseNumber();
    }

    double parseTerm() {
    double result = parseFactor();

    while (true) {
        skipSpaces();

        if (pos >= expression.size()) {
            break;
        }

        char op = expression[pos];

        if (op != '*' && op != '/') {
            break;
        }

        ++pos;

        double value = parseFactor();

        if (op == '*') {
            result *= value;
        }
        else {
            if (value == 0) {
                throw std::runtime_error("Division by zero");
            }

            result /= value;
        }

        checkResult(result);
    }

    return result;
}
     

   double parseExpressionInternal() {
    double result = parseTerm();

    while (true) {
        skipSpaces();

        if (pos >= expression.size()) {
            break;
        }

        char op = expression[pos];

        if (op != '+' && op != '-') {
            break;
        }

        ++pos;

        double value = parseTerm();

        if (op == '+') {
            result += value;
        }
        else {
            result -= value;
        }

        checkResult(result);
    }

    return result;
}
public:
    explicit Parser(const std::string& expr)
        : expression(expr) {}

    double parseExpression() {
        double result = parseExpressionInternal();

        skipSpaces();

        if (pos != expression.size()) {
            throw std::runtime_error(
                "Invalid expression"
            );
        }

        return result;
    }
};

// C++23: std::expected
std::expected<double, std::string> calculateExpression(
    const std::string& expression
) {
    try {
        Parser parser(expression);

        return parser.parseExpression();
    }
    catch (const std::exception& e) {
        return std::unexpected(
            std::string(e.what())
        );
    }
}

} // namespace

std::string CalculatorTool::getName() const {
    return "calculator";
}

std::string CalculatorTool::getDescription() const {
    return
        "Calculate mathematical expressions. "
        "Input must be a raw mathematical expression string. "
        "Supported operators: +, -, *, /. "
        "Parentheses are supported. "
        "Preserve all parentheses and operator order exactly. "
        "Example: (25 + 15) * 3 - 12";
}

std::string CalculatorTool::execute(
    const std::string& args
) {
   
    auto result = calculateExpression(args);

    if (!result) {
        return "Calculator error: " + result.error();
    }

    return std::to_string(result.value());
}
