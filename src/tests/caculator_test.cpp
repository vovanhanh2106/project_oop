
#include <iostream>
#include "../tools/calculator_tool.h"

int main() {
    CalculatorTool calculator;

   const std::string tests[] = {
    "1abc + 2",
    "1 2",
    "()",
    "2++3",
    "2**3",
    "5 +",
    "* 5",
    "10 / / 2",
    "((2 + 3)",
    "2 + 3)",
    "",
    "   ",
    "1e",
    "1e+",
    "1.2.3"
};

    for (const auto& expression : tests) {
        std::cout << "Expression: " << expression << '\n';

        std::cout << "Result: "
                
                  << calculator.execute(expression)
                  << "\n\n";
    }

    return 0;
}
