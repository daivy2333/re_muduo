# Benchmark CMakeLists.txt

# 设置benchmark可执行文件输出目录
set(BENCHMARK_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/benchmark)
set(EXECUTABLE_OUTPUT_PATH ${BENCHMARK_OUTPUT_PATH})

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR})

# 编译benchmark_base
add_library(benchmark_base STATIC
    benchmark_base.cpp
)

# Echo服务器基准测试
add_executable(echo_bench
    echo_server_bench.cpp
    throughput_test.cpp
)
target_link_libraries(echo_bench 
    benchmark_base
    re_muduo 
    ${CMAKE_THREAD_LIBS_INIT}
)
target_include_directories(echo_bench PRIVATE ${PROJECT_SOURCE_DIR})

# 延迟测试
add_executable(latency_test
    latency_test.cpp
)
target_link_libraries(latency_test 
    benchmark_base
    re_muduo 
    ${CMAKE_THREAD_LIBS_INIT}
)
target_include_directories(latency_test PRIVATE ${PROJECT_SOURCE_DIR})

# 自包含压力测试
add_executable(self_stress_test
    self_stress_test.cpp
)
target_link_libraries(self_stress_test
    re_muduo
    ${CMAKE_THREAD_LIBS_INIT}
)
target_include_directories(self_stress_test PRIVATE ${PROJECT_SOURCE_DIR})

# 性能测试目标
add_custom_target(benchmark
    COMMAND echo "=== Running Echo Server Benchmark ===" && ./echo_bench
    COMMAND echo "=== Running Latency Test ===" && ./latency_test 127.0.0.1 8888
    DEPENDS echo_bench latency_test
    WORKING_DIRECTORY ${BENCHMARK_OUTPUT_PATH}
)
