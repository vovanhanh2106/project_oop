
#include "../agent/loop_detector.h"

#include <iostream>
#include <string>
#include <vector>

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
    // TEST 1: Lịch sử ban đầu phải rỗng
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

    // TEST 2: Hai hành động khác nhau không tạo cảnh báo
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

    // TEST 3: Hai hành động giống nhau chưa đạt ngưỡng cảnh báo
    {
        LoopDetector detector;

        detector.recordAction("calculator:1+2");
        LoopStatus status =
            detector.recordAction("calculator:1+2");

        check(
            "Two repeated actions remain Normal",
            status == LoopStatus::Normal
        );
    }

    // TEST 4: Lặp liên tiếp 3 lần phải Warning
    {
        LoopDetector detector;

        detector.recordAction("calculator:1+2");
        detector.recordAction("calculator:1+2");
        LoopStatus status =
            detector.recordAction("calculator:1+2");

        check(
            "Three repeated actions trigger Warning",
            status == LoopStatus::Warning
        );
    }

    // TEST 5: Lặp liên tiếp 5 lần phải Critical
    {
        LoopDetector detector;

        LoopStatus status = LoopStatus::Normal;

        for (int i = 0; i < 5; ++i) {
            status = detector.recordAction("calculator:1+2");
        }

        check(
            "Five repeated actions trigger Critical",
            status == LoopStatus::Critical
        );
    }

    // TEST 6: Chuỗi A -> B -> A phải Warning
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

    // TEST 7: Chuỗi A -> B -> A -> B -> A phải Critical
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

    // TEST 8: Hành động rỗng không được thêm vào lịch sử
    {
        LoopDetector detector;

        detector.recordAction("A");
        const std::size_t sizeBefore =
            detector.getHistory().size();

        LoopStatus status = detector.recordAction("");

        check(
            "Empty action does not change status",
            status == LoopStatus::Normal
        );

        check(
            "Empty action is not added to history",
            detector.getHistory().size() == sizeBefore
        );
    }

    // TEST 9: Reset phải xóa lịch sử và đưa trạng thái về Normal
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

    // TEST 10: Kiểm tra ngưỡng tùy chỉnh
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

    // Tổng kết
    std::cout << "\n========== TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total : " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
