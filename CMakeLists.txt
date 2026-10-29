cmake_minimum_required(VERSION 3.20)

project(AIAgent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(AIAgent
    src/main.cpp

    src/client/ollama_client.cpp
    src/tools/tool_registry.cpp
    src/tools/calculator_tool.cpp
    src/tools/file_tool.cpp
    src/tools/exec_tool.cpp
    src/tools/web_search_tool.cpp
    src/tools/memory_tool.cpp
    src/agent/skill_loader.cpp
    src/agent/loop_detector.cpp
    src/agent/agent_loop.cpp
    src/harness/keyword_evaluator.cpp
    src/harness/functional_evaluator.cpp
    src/harness/trajectory.cpp
    src/harness/harness_runner.cpp
    src/harness/numeric_evaluator.cpp
)
target_include_directories(AIAgent PRIVATE
    src
    src/client
)

target_link_libraries(AIAgent PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    SQLite3::SQLite3
)
add_executable(TrajectoryTest
    src/tests/trajectory_test.cpp
    src/harness/trajectory.cpp
)

target_include_directories(TrajectoryTest PRIVATE
    src
)

target_link_libraries(TrajectoryTest PRIVATE
    nlohmann_json::nlohmann_json
)
add_executable(HarnessTest
    src/tests/harness_test.cpp
    src/client/ollama_client.cpp
    src/agent/agent_loop.cpp
    src/agent/skill_loader.cpp
    src/agent/loop_detector.cpp
    src/tools/tool_registry.cpp
    src/tools/calculator_tool.cpp
    src/harness/trajectory.cpp
    src/harness/keyword_evaluator.cpp
    src/harness/functional_evaluator.cpp
    src/harness/harness_runner.cpp
    src/harness/numeric_evaluator.cpp
)

target_include_directories(HarnessTest PRIVATE src)

target_link_libraries(HarnessTest PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
)
enable_testing()

add_executable(LoopDetectorTest
    src/tests/loop_detector_test.cpp
    src/agent/loop_detector.cpp
)

target_include_directories(LoopDetectorTest PRIVATE
    src
)

add_test(
    NAME LoopDetectorTest
    COMMAND LoopDetectorTest
)