cmake_minimum_required(VERSION 3.20)

project(AIAgent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# 1. Dependencies
# ==========================================

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)

# ==========================================
# 2. Core Library
# ==========================================

add_library(AgentCore STATIC

    # Client
    src/client/ollama_client.cpp
    src/client/remote_llm_client.cpp

    # Tools
    src/tools/tool_registry.cpp
    src/tools/calculator_tool.cpp
    src/tools/file_tool.cpp
    src/tools/exec_tool.cpp
    src/tools/web_search_tool.cpp
    src/tools/memory_tool.cpp

    # Agent
    src/agent/skill_loader.cpp
    src/agent/loop_detector.cpp
    src/agent/agent_loop.cpp

    # Harness
    src/harness/keyword_evaluator.cpp
    src/harness/functional_evaluator.cpp
    src/harness/trajectory.cpp
    src/harness/harness_runner.cpp
    src/harness/numeric_evaluator.cpp

    # Benchmark
    src/benchmark/benchmark_dataset.cpp
    src/benchmark/benchmark_runner.cpp
)

target_include_directories(AgentCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(AgentCore PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
)

# ==========================================
# 3. Main Application
# ==========================================

add_executable(AIAgent
    src/main.cpp
)

target_link_libraries(AIAgent PRIVATE
    AgentCore
)

# ==========================================
# 4. Trajectory Test
# ==========================================

add_executable(TrajectoryTest
    src/tests/trajectory_test.cpp
)

target_link_libraries(TrajectoryTest PRIVATE
    AgentCore
)

# ==========================================
# 5. Harness Test
# ==========================================

add_executable(HarnessTest
    src/tests/harness_test.cpp
)

target_link_libraries(HarnessTest PRIVATE
    AgentCore
)

# ==========================================
# 6. Loop Detector Test
# ==========================================

add_executable(LoopDetectorTest
    src/tests/loop_detector_test.cpp
)

target_link_libraries(LoopDetectorTest PRIVATE
    AgentCore
)

# ==========================================
# 7. Benchmark Test
# ==========================================

add_executable(BenchmarkTest
    src/tests/benchmark_test.cpp
)

target_link_libraries(BenchmarkTest PRIVATE
    AgentCore
)

# ==========================================
# 8. CTest
# ==========================================

enable_testing()

add_test(
    NAME TrajectoryTest
    COMMAND TrajectoryTest
)

add_test(
    NAME LoopDetectorTest
    COMMAND LoopDetectorTest
)