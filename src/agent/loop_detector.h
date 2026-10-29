
#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum class LoopStatus {
    Normal,
    Warning,
    Critical
};

class LoopDetector {
private:
    std::vector<std::string> history;

    std::size_t warningThreshold;
    std::size_t criticalThreshold;

    std::size_t countRepeatedAction() const;
    std::size_t countPingPongSteps() const;

public:
    explicit LoopDetector(
        std::size_t warningThreshold = 3,
        std::size_t criticalThreshold = 5
    );

    LoopStatus recordAction(const std::string& action);

    LoopStatus getStatus() const;

    void reset();

    const std::vector<std::string>& getHistory() const;
};
