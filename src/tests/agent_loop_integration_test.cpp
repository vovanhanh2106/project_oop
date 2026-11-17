
#include "../agent/loop_detector.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

int passedTests = 0;
int totalTests = 0;

void check(const string& testName, bool condition) {
    cout << testName << ": "
         << (condition ? "PASS" : "FAIL") << '\n';
    totalTests++;
    if (condition) {
        ++passedTests;
    }
}



int main() {
    // =========================================
    // TEST 1: PING-PONG CHƯA ĐỦ CRITICAL
    // A -> B -> A
    // Default threshold: Warning = 3, Critical = 5
    // =========================================
    {
        cout << "\n===== TEST 1: PING-PONG WARNING =====\n";

        LoopDetector detector(3, 5);

        detector.recordAction("A");
        detector.recordAction("B");

        LoopStatus status = detector.recordAction("A");

        check(
            "A-B-A produces Warning",
            status == LoopStatus::Warning
        );
    }

    // =========================================
    // TEST 2: PING-PONG BỊ NGẮT
    // A -> B -> A -> C
    // =========================================
    {
        cout << "\n===== TEST 2: INTERRUPTED PING-PONG =====\n";

        LoopDetector detector(3, 5);

        detector.recordAction("A");
        detector.recordAction("B");
        detector.recordAction("A");

        LoopStatus status = detector.recordAction("C");

        check(
            "A-B-A-C returns Normal",
            status == LoopStatus::Normal
        );
    }

    // =========================================
    // TEST 3: LẶP LIÊN TIẾP
    // A -> A -> A
    // =========================================
    {
        cout << "\n===== TEST 3: REPEATED ACTION =====\n";

        LoopDetector detector(3, 5);

        detector.recordAction("A");
        detector.recordAction("A");

        LoopStatus status = detector.recordAction("A");

        check(
            "Three repeated actions produce Warning",
            status == LoopStatus::Warning
        );
    }

    // =========================================
    // TEST 4: ACTION RỖNG
    // =========================================
    {
        cout << "\n===== TEST 4: EMPTY ACTION =====\n";

        LoopDetector detector;

        detector.recordAction("A");
        detector.recordAction("");

        check(
            "Empty action is not recorded",
            detector.getHistory().size() == 1
        );

        check(
            "History still contains A",
            detector.getHistory()[0] == "A"
        );
    }

    // =========================================
    // TEST 5: RESET
    // =========================================
    {
        cout << "\n===== TEST 5: RESET =====\n";

        LoopDetector detector;

        detector.recordAction("A");
        detector.recordAction("A");

        detector.reset();

        check(
            "History is empty after reset",
            detector.getHistory().empty()
        );

        check(
            "Status is Normal after reset",
            detector.getStatus() == LoopStatus::Normal
        );
    }

    // =========================================
    // TEST 6: WARNING THRESHOLD = 0
    // =========================================
    {
        cout << "\n===== TEST 6: INVALID WARNING =====\n";

        bool exceptionThrown = false;

        try {
            LoopDetector detector(0, 5);
        }
        catch (const invalid_argument&) {
            exceptionThrown = true;
        }

        check(
            "Warning threshold 0 throws exception",
            exceptionThrown
        );
    }

    // =========================================
    // TEST 7: CRITICAL <= WARNING
    // =========================================
    {
        cout << "\n===== TEST 7: INVALID CRITICAL =====\n";

        bool exceptionThrown = false;

        try {
            LoopDetector detector(3, 3);
        }
        catch (const invalid_argument&) {
            exceptionThrown = true;
        }

        check(
            "Critical <= Warning throws exception",
            exceptionThrown
        );
    }

    // =========================================
    // TEST 8: CUSTOM THRESHOLD
    // Warning = 2, Critical = 4
    // =========================================
    {
        cout << "\n===== TEST 8: CUSTOM THRESHOLD =====\n";

        LoopDetector detector(2, 4);

        LoopStatus status1 = detector.recordAction("A");
        LoopStatus status2 = detector.recordAction("A");
        LoopStatus status3 = detector.recordAction("A");
        LoopStatus status4 = detector.recordAction("A");

        check(
            "First action is Normal",
            status1 == LoopStatus::Normal
        );

        check(
            "Second action is Warning",
            status2 == LoopStatus::Warning
        );

        check(
            "Third action remains Warning",
            status3 == LoopStatus::Warning
        );

        check(
            "Fourth action is Critical",
            status4 == LoopStatus::Critical
        );
    }

    // =========================================
    // SUMMARY
    // =========================================
   cout << "\n========== FINAL SUMMARY ==========\n";
cout << "Passed: " << passedTests << "/" << totalTests << '\n';
cout << "Failed: " << totalTests - passedTests << '\n';
cout << "===================================\n";

return passedTests == totalTests ? 0 : 1;
}
