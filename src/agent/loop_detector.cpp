
#include "loop_detector.h"

#include <iostream>
#include <stdexcept>
#include <utility>

LoopDetector::LoopDetector(
    std::size_t warningThreshold,
    std::size_t criticalThreshold
)
    : warningThreshold(warningThreshold),
      criticalThreshold(criticalThreshold) {

    if (warningThreshold == 0 ||
        criticalThreshold <= warningThreshold) {
        throw std::invalid_argument(
            "criticalThreshold must be greater than "
            "warningThreshold, and warningThreshold must be positive."
        );
    }
}

std::size_t LoopDetector::countRepeatedAction() const {
    if (history.empty()) {
        return 0;
    }

    std::size_t count = 1;
    const std::string& last = history.back();

    for (std::size_t i = history.size() - 1; i > 0; --i) {
        if (history[i - 1] != last) {
            break;
        }

        ++count;
    }

    return count;
}

std::size_t LoopDetector::countPingPongSteps() const {
    // Cần ít nhất 3 hành động để nhận diện A -> B -> A.
    if (history.size() < 3) {
        return 0;
    }

    const std::string& last = history.back();
    const std::string& previous = history[history.size() - 2];

    // Hai hành động giống nhau liên tiếp không phải ping-pong.
    if (last == previous) {
        return 0;
    }

    std::size_t count = 2;

    for (std::size_t i = history.size() - 2; i > 0; --i) {
        if (history[i - 1] != history[i + 1]) {
            break;
        }

        ++count;
    }

    return count;
}

LoopStatus LoopDetector::recordAction(
    const std::string& action
) {
    if (action.empty()) {
        return getStatus();
    }

    history.push_back(action);

    const LoopStatus status = getStatus();

    if (status == LoopStatus::Critical) {
        std::cerr << "[LoopDetector] CRITICAL: loop detected. "
                  << "Stop the agent.\n";
    } else if (status == LoopStatus::Warning) {
        std::cerr << "[LoopDetector] WARNING: repeated actions detected.\n";
    }

    return status;
}

LoopStatus LoopDetector::getStatus() const {
    const std::size_t repeated = countRepeatedAction();
    const std::size_t pingPong = countPingPongSteps();

    if (repeated >= criticalThreshold ||
        pingPong >= criticalThreshold) {
        return LoopStatus::Critical;
    }

    if (repeated >= warningThreshold ||
        pingPong >= warningThreshold) {
        return LoopStatus::Warning;
    }

    return LoopStatus::Normal;
}

void LoopDetector::reset() {
    history.clear();
}

const std::vector<std::string>& LoopDetector::getHistory() const {
    return history;
}
