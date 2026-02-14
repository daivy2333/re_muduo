cmake_minimum_required(VERSION 3.10)
project(compare_tests)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 官方 muduo 库路径配置
if(NOT DEFINED MUDUO_ROOT)
    set(MUDUO_ROOT "$ENV{HOME}/muduo" CACHE PATH "Muduo root directory")
endif()
if(NOT DEFINED MUDUO_INSTALL_DIR)
    set(MUDUO_INSTALL_DIR "${MUDUO_ROOT}/build/release-cpp11" CACHE PATH "Muduo install directory")
endif()

# 检查 /usr/local 下是否有 muduo 库
if(EXISTS "/usr/local/include/muduo")
    set(MUDUO_ROOT "/usr/local" CACHE PATH "Muduo root directory" FORCE)
    set(MUDUO_INSTALL_DIR "/usr/local" CACHE PATH "Muduo install directory" FORCE)
    message(STATUS "Found muduo in /usr/local")
endif()

# 添加 muduo 头文件路径
include_directories(${MUDUO_ROOT})

# 添加项目头文件路径
include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/../benchmark)

# 查找 muduo 库
find_library(MUDUO_NET_LIB muduo_net PATHS ${MUDUO_INSTALL_DIR}/lib /usr/local/lib)
find_library(MUDUO_BASE_LIB muduo_base PATHS ${MUDUO_INSTALL_DIR}/lib /usr/local/lib)

if(NOT MUDUO_NET_LIB OR NOT MUDUO_BASE_LIB)
    message(FATAL_ERROR "Cannot find muduo libraries. Please install muduo or set MUDUO_INSTALL_DIR correctly.")
endif()

message(STATUS "Found muduo_net: ${MUDUO_NET_LIB}")
message(STATUS "Found muduo_base: ${MUDUO_BASE_LIB}")

# 添加 muduo 测试
add_executable(muduo_latency_test muduo/latency_test.cpp)
target_include_directories(muduo_latency_test PRIVATE
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/../benchmark
)
target_link_libraries(muduo_latency_test
    pthread
    ${MUDUO_NET_LIB}
    ${MUDUO_BASE_LIB}
)

# 添加项目库
set(PROJECT_LIB_DIR "${PROJECT_SOURCE_DIR}/../lib")
if(EXISTS "${PROJECT_LIB_DIR}/libre_muduo.so")
    add_library(re_muduo SHARED IMPORTED)
    set_target_properties(re_muduo PROPERTIES IMPORTED_LOCATION "${PROJECT_LIB_DIR}/libre_muduo.so")
endif()

# 添加 nginx 测试
add_executable(nginx_latency_test nginx/latency_test.cpp)
target_include_directories(nginx_latency_test PRIVATE
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/../benchmark
)
target_link_libraries(nginx_latency_test
    pthread
    re_muduo
)

# 安装目标
install(TARGETS muduo_latency_test nginx_latency_test
    RUNTIME DESTINATION bin
)
