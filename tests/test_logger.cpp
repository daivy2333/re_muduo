
#include "Logger.h"
#include <iostream>
#include <cassert>

void test_logger_instance() {
    Logger& logger1 = Logger::instance();
    Logger& logger2 = Logger::instance();

    // 验证单例模式
    assert(&logger1 == &logger2);
    std::cout << "Logger instance test passed" << std::endl;
}

void test_logger_log_levels() {
    // 测试不同级别的日志输出
    LOG_INFO("This is an info message");
    LOG_ERROR("This is an error message");

    // 注意：LOG_FATAL 会调用 exit(-1)，所以不能在测试中直接调用
    // LOG_FATAL("This is a fatal message");

    std::cout << "Logger log levels test passed" << std::endl;
}

void test_logger_set_log_level() {
    Logger& logger = Logger::instance();

    // 设置不同的日志级别
    logger.setLogLevel(INFO);
    logger.log("Test message at INFO level");

    logger.setLogLevel(ERROR);
    logger.log("Test message at ERROR level");

    std::cout << "Logger set log level test passed" << std::endl;
}

int main() {
    std::cout << "=== Logger Tests ===" << std::endl;
    test_logger_instance();
    test_logger_log_levels();
    test_logger_set_log_level();
    std::cout << "=== All Logger Tests Passed ===" << std::endl;
    return 0;
}
