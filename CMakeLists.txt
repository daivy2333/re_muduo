cmake_minimum_required(VERSION 3.15)
project(re_muduo)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

# 添加源文件路径
aux_source_directory(${PROJECT_SOURCE_DIR}/src/base SRC_LIST)

# 排除旧的Logger.cpp，使用新的Logger_new.cpp
list(REMOVE_ITEM SRC_LIST ${PROJECT_SOURCE_DIR}/src/base/Logger.cpp)

add_library(re_muduo SHARED ${SRC_LIST})


# 测试配置
enable_testing()

# 测试可执行文件
add_executable(test_timestamp tests/test_timestamp.cpp)
target_link_libraries(test_timestamp re_muduo pthread)
add_test(NAME test_timestamp COMMAND test_timestamp)

add_executable(test_inetaddress tests/test_inetaddress.cpp)
target_link_libraries(test_inetaddress re_muduo pthread)
add_test(NAME test_inetaddress COMMAND test_inetaddress)

add_executable(test_logger tests/test_logger.cpp)
target_link_libraries(test_logger re_muduo pthread)
add_test(NAME test_logger COMMAND test_logger)

add_executable(test_currentthread tests/test_currentthread.cpp)
target_link_libraries(test_currentthread re_muduo pthread)
add_test(NAME test_currentthread COMMAND test_currentthread)

add_executable(test_eventloop tests/test_eventloop.cpp)
target_link_libraries(test_eventloop re_muduo pthread)
add_test(NAME test_eventloop COMMAND test_eventloop)

add_executable(test_channel tests/test_channel.cpp)
target_link_libraries(test_channel re_muduo pthread)
add_test(NAME test_channel COMMAND test_channel)

add_executable(test_poller tests/test_poller.cpp)
target_link_libraries(test_poller re_muduo pthread)
add_test(NAME test_poller COMMAND test_poller)

add_executable(test_thread tests/test_thread.cpp)
target_link_libraries(test_thread re_muduo pthread)
add_test(NAME test_thread COMMAND test_thread)

add_executable(test_eventloopthread tests/test_eventloopthread.cpp)
target_link_libraries(test_eventloopthread re_muduo pthread)
add_test(NAME test_eventloopthread COMMAND test_eventloopthread)

add_executable(test_eventloopthreadpool tests/test_eventloopthreadpool.cpp)
target_link_libraries(test_eventloopthreadpool re_muduo pthread)
add_test(NAME test_eventloopthreadpool COMMAND test_eventloopthreadpool)

add_executable(test_socket tests/test_socket.cpp)
target_link_libraries(test_socket re_muduo pthread)
add_test(NAME test_socket COMMAND test_socket)

add_executable(test_acceptor tests/test_acceptor.cpp)
target_link_libraries(test_acceptor re_muduo pthread)
add_test(NAME test_acceptor COMMAND test_acceptor)

add_executable(test_tcpserver tests/test_tcpserver.cpp)
target_link_libraries(test_tcpserver re_muduo pthread)
add_test(NAME test_tcpserver COMMAND test_tcpserver)

add_executable(test_buffer tests/test_buffer.cpp)
target_link_libraries(test_buffer re_muduo pthread)
add_test(NAME test_buffer COMMAND test_buffer)

add_executable(test_tcpconnection tests/test_tcpconnection.cpp)
target_link_libraries(test_tcpconnection re_muduo pthread)
add_test(NAME test_tcpconnection COMMAND test_tcpconnection)

add_executable(test_timer tests/test_timer.cpp)
target_link_libraries(test_timer re_muduo pthread)
add_test(NAME test_timer COMMAND test_timer)

add_executable(test_timer_queue tests/test_timer_queue.cpp)
target_link_libraries(test_timer_queue re_muduo pthread)
add_test(NAME test_timer_queue COMMAND test_timer_queue)

add_executable(test_timer_class tests/test_timer_class.cpp)
target_link_libraries(test_timer_class re_muduo pthread)
add_test(NAME test_timer_class COMMAND test_timer_class)

add_executable(test_timer_integration tests/test_timer_integration.cpp)
target_link_libraries(test_timer_integration re_muduo pthread)
add_test(NAME test_timer_integration COMMAND test_timer_integration)

add_executable(test_async_logging tests/test_async_logging.cpp)
target_link_libraries(test_async_logging re_muduo pthread)
add_test(NAME test_async_logging COMMAND test_async_logging)

# 添加benchmark子目录
add_subdirectory(benchmark)

# 添加benchmark测试到CTest
add_test(NAME echo_bench COMMAND echo_bench)
add_test(NAME latency_test COMMAND latency_test)
add_test(NAME self_stress_test COMMAND self_stress_test)

# 创建自定义目标：一键运行所有benchmark测试
add_custom_target(run_benchmarks
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L benchmark
    DEPENDS echo_bench latency_test self_stress_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all benchmark tests..."
)
