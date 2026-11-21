
#include "../agent/loop_detector.h"
#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <string>

int passed = 0;
int failed = 0;

void check(
    const std::string& testName,
    bool condition
) {
    if (condition) {
        std::cout << "[PASS] " << testName << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << testName << '\n';
        ++failed;
    }
}

int main() {
     SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // TEST 1: Initial state
    {
        LoopDetector detector;

        check(
            "Initial status is Normal",
            detector.getStatus() == LoopStatus::Normal
        );

        check(
            "Initial history is empty",
            detector.getHistory().empty()
        );
    }

    // TEST 2: Different actions
    {
        LoopDetector detector;

        detector.recordAction("calculator:1+2");
        LoopStatus status =
            detector.recordAction("search:weather");

        check(
            "Different actions remain Normal",
            status == LoopStatus::Normal
        );

        check(
            "History contains two actions",
            detector.getHistory().size() == 2
        );
    }

    // TEST 3: Repeated action below threshold
    {
        LoopDetector detector;

        detector.recordAction("A");
        LoopStatus status = detector.recordAction("A");

        check(
            "Two repeated actions remain Normal",
            status == LoopStatus::Normal
        );
    }

    // TEST 4: Repeated action reaches Warning
    {
        LoopDetector detector;

        detector.recordAction("A");
        detector.recordAction("A");

        LoopStatus status = detector.recordAction("A");

        check(
            "Three repeated actions trigger Warning",
            status == LoopStatus::Warning
        );
    }

    // TEST 5: Repeated action reaches Critical
    {
        LoopDetector detector;

        LoopStatus status = LoopStatus::Normal;

        for (int i = 0; i < 5; ++i) {
            status = detector.recordAction("A");
        }

        check(
            "Five repeated actions trigger Critical",
            status == LoopStatus::Critical
        );
    }

    // TEST 6: Ping-pong Warning
    {
        LoopDetector detector;

        detector.recordAction("A");
        detector.recordAction("B");

        LoopStatus status = detector.recordAction("A");

        check(
            "A-B-A triggers Warning",
            status == LoopStatus::Warning
        );
    }

    // TEST 7: Ping-pong Critical
    {
        LoopDetector detector;

        detector.recordAction("A");
        detector.recordAction("B");
        detector.recordAction("A");
        detector.recordAction("B");

        LoopStatus status = detector.recordAction("A");

        check(
            "A-B-A-B-A triggers Critical",
            status == LoopStatus::Critical
        );
    }

    // TEST 8: Empty action
    {
        LoopDetector detector;

        detector.recordAction("A");

        std::size_t before =
            detector.getHistory().size();

        LoopStatus status = detector.recordAction("");

        check(
            "Empty action does not change status",
            status == LoopStatus::Normal
        );

        check(
            "Empty action is not recorded",
            detector.getHistory().size() == before
        );
    }

    // TEST 9: Reset
    {
        LoopDetector detector;

        detector.recordAction("A");
        detector.recordAction("A");
        detector.recordAction("A");

        detector.reset();

        check(
            "Reset clears history",
            detector.getHistory().empty()
        );

        check(
            "Reset restores Normal status",
            detector.getStatus() == LoopStatus::Normal
        );
    }

    // TEST 10: Custom thresholds
    {
        LoopDetector detector(2, 4);

        detector.recordAction("A");

        LoopStatus status = detector.recordAction("A");

        check(
            "Custom warning threshold works",
            status == LoopStatus::Warning
        );

        detector.recordAction("A");
        status = detector.recordAction("A");

        check(
            "Custom critical threshold works",
            status == LoopStatus::Critical
        );
    }

    // TEST 11: Custom threshold with ping-pong
    {
        LoopDetector detector(2, 4);

        detector.recordAction("A");
        LoopStatus status = detector.recordAction("B");

        check(
            "Two ping-pong steps remain Normal",
            status == LoopStatus::Normal
        );

        status = detector.recordAction("A");

        check(
            "Three ping-pong steps trigger Warning",
            status == LoopStatus::Warning
        );
    }

    // TEST 12: Different actions interrupt repetition
{
    LoopDetector detector;

    detector.recordAction("A");
    detector.recordAction("A");
    detector.recordAction("B");

    LoopStatus status = detector.recordAction("C");

    check(
        "Different actions interrupt repetition",
        status == LoopStatus::Normal
    );
}

    // TEST 13: Non-alternating sequence
    {
        LoopDetector detector;

        detector.recordAction("A");
        detector.recordAction("B");
        detector.recordAction("C");
        detector.recordAction("A");

        check(
            "A-B-C-A does not trigger ping-pong",
            detector.getStatus() == LoopStatus::Normal
        );
    }

    // TEST 14: Invalid constructor thresholds
    {
        bool caughtZeroWarning = false;
        bool caughtInvalidCritical = false;

        try {
            LoopDetector detector(0, 5);
        }
        catch (const std::invalid_argument&) {
            caughtZeroWarning = true;
        }

        try {
            LoopDetector detector(3, 3);
        }
        catch (const std::invalid_argument&) {
            caughtInvalidCritical = true;
        }

        check(
            "Zero warning threshold is rejected",
            caughtZeroWarning
        );

        check(
            "Critical threshold <= warning is rejected",
            caughtInvalidCritical
        );
    }

    // Summary
    std::cout << "\n========== TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total : " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
