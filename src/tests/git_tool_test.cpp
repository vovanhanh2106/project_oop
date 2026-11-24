
#include "../tools/git_tool.h"
#include <iostream>
#include <string>

int main() {
    GitTool tool;

    const std::string commands[] = {
        R"({"command":"status"})",
        R"({"command":"branch"})",
        R"({"command":"diff"})"
    };

    for (const auto& args : commands) {
        std::cout << "\n========== TEST ==========\n";
        std::cout << "Arguments: " << args << '\n';
        std::cout << tool.execute(args) << '\n';
    }

    return 0;
}
