#include "calculator_tool.h"

#include <cctype>
#include <stdexcept>
#include <string>

namespace {

class Parser {
private:
    const std::string& expression;
    size_t pos = 0;

    void skipSpaces() {
        while (pos < expression.size() &&
               std::isspace(static_cast<unsigned char>(expression[pos]))) {
            ++pos;
        }
    }

    double parseNumber() {
        skipSpaces();

        size_t start = pos;

        while (pos < expression.size() &&
               (std::isdigit(static_cast<unsigned char>(expression[pos])) ||
                expression[pos] == '.')) {
            ++pos;
        }

        if (start == pos) {
            throw std::runtime_error("Expected number");
        }

        return std::stod(expression.substr(start, pos - start));
    }

    double parseFactor() {
        skipSpaces();

        // Xử lý ngoặc
        if (pos < expression.size() && expression[pos] == '(') {
            ++pos; // bỏ '('

           double result = parseExpressionInternal();
            skipSpaces();

            if (pos >= expression.size() || expression[pos] != ')') {
                throw std::runtime_error("Missing ')'");
            }

            ++pos; // bỏ ')'

            return result;
        }

        // Xử lý số âm
        if (pos < expression.size() && expression[pos] == '-') {
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
        }

        return result;
    }

    // Hàm này chỉ parse biểu thức,
    // KHÔNG kiểm tra đã đi hết chuỗi hay chưa.
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
        }

        return result;
    }

public:
    Parser(const std::string& expr)
        : expression(expr) {}

    double parseExpression() {
        double result = parseExpressionInternal();

        skipSpaces();

        // Chỉ kiểm tra hết chuỗi ở đây
        if (pos != expression.size()) {
            throw std::runtime_error("Invalid expression");
        }

        return result;
    }
};

}

std::string CalculatorTool::getName() const {
    return "calculator";
}

std::string CalculatorTool::getDescription() const {
    return "Calculate a mathematical expression.";
}

std::string CalculatorTool::execute(const std::string& args) {
    try {
        Parser parser(args);

        double result = parser.parseExpression();

        return std::to_string(result);
    }
    catch (const std::exception& e) {
        return std::string("Calculator error: ") + e.what();
    }
}