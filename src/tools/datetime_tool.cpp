
#include "datetime_tool.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

std::string DateTimeTool::getName() const {
    return "datetime";
}

std::string DateTimeTool::getDescription() const {
    return
        "Get the current local date and time. "
        "The args MUST be a JSON object containing a command field. "
        "Supported commands: date, time, datetime, weekday. "
        "Example args: {\"command\":\"date\"}. "
        "Use date for current date, time for current time, "
        "datetime for both, and weekday for day of the week.";
}


std::string DateTimeTool::execute(
    const std::string& args
) {
    std::string command;

    // Hỗ trợ cả chuỗi thông thường và JSON object.
    try {
        auto parsed = nlohmann::json::parse(args);

        if (parsed.is_object()) {
            command = parsed.value(
                "command",
                std::string{}
            );
        }
        else if (parsed.is_string()) {
            command = parsed.get<std::string>();
        }
    }
    catch (const nlohmann::json::exception&) {
        // Nếu không phải JSON thì xem args là chuỗi lệnh.
        command = args;
    }

    const auto now = std::chrono::system_clock::now();

    const std::time_t currentTime =
        std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    if (localtime_s(&localTime, &currentTime) != 0) {
        throw std::runtime_error("Cannot get local time");
    }
#else
    if (localtime_r(&currentTime, &localTime) == nullptr) {
        throw std::runtime_error("Cannot get local time");
    }
#endif

    std::ostringstream output;

    if (command == "date") {
        output << std::put_time(&localTime, "%Y-%m-%d");
    }
    else if (command == "time") {
        output << std::put_time(&localTime, "%H:%M:%S");
    }
    else if (command == "datetime") {
        output << std::put_time(
            &localTime,
            "%Y-%m-%d %H:%M:%S"
        );
    }
    else if (command == "weekday") {
        output << std::put_time(&localTime, "%A");
    }
    else {
        return "Invalid command. Use: date, time, datetime, weekday";
    }

    return output.str();
}
