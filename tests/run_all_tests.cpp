
#include <iostream>
#include <string>
#include <cstdlib>

// 函数声明
int test_timestamp();
int test_inetaddress();
int test_logger();
int test_currentthread();
int test_eventloop();
int test_channel();
int test_poller();
int test_thread();

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running all re_muduo tests..." << std::endl;
    std::cout << "========================================" << std::endl;

    int failedTests = 0;

    // 运行所有测试
    if (test_timestamp() != 0) {
        std::cerr << "Timestamp tests FAILED!" << std::endl;
        failedTests++;
    }

    if (test_inetaddress() != 0) {
        std::cerr << "InetAddress tests FAILED!" << std::endl;
        failedTests++;
    }

    if (test_logger() != 0) {
        std::cerr << "Logger tests FAILED!" << std::endl;
        failedTests++;
    }

    if (test_currentthread() != 0) {
        std::cerr << "CurrentThread tests FAILED!" << std::endl;
        failedTests++;
    }

    if (test_eventloop() != 0) {
        std::cerr << "EventLoop tests FAILED!" << std::endl;
        failedTests++;
    }

    if (test_channel() != 0) {
        std::cerr << "Channel tests FAILED!" << std::endl;
        failedTests++;
    }

    if (test_poller() != 0) {
        std::cerr << "Poller tests FAILED!" << std::endl;
        failedTests++;
    }

    if (test_thread() != 0) {
        std::cerr << "Thread tests FAILED!" << std::endl;
        failedTests++;
    }

    std::cout << "========================================" << std::endl;
    if (failedTests == 0) {
        std::cout << "All tests PASSED!" << std::endl;
    } else {
        std::cout << failedTests << " test suite(s) FAILED!" << std::endl;
    }
    std::cout << "========================================" << std::endl;

    return failedTests;
}
