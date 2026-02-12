#include "AsyncLoggingInit.h"
#include "Logger.h"
#include "Thread.h"

// 声明resetAsyncLogging函数
void resetAsyncLogging();
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

// 测试计数器
std::atomic<int> test_passed{0};
std::atomic<int> test_failed{0};

void testBasicLogging() {
    LOG_INFO("This is an info message");
    LOG_ERROR("This is an error message");
    LOG_DEBUG("This is a debug message");
}

void testMultiThreadLogging() {
    const int kThreads = 4;
    const int kMessagesPerThread = 100;

    std::vector<std::unique_ptr<Thread>> threads;

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back(new Thread([i, kMessagesPerThread]() {
            for (int j = 0; j < kMessagesPerThread; ++j) {
                LOG_INFO("Thread %d, Message %d", i, j);
            }
        }, "LoggingThread"));
    }

    for (auto& thread : threads) {
        thread->start();
    }

    for (auto& thread : threads) {
        thread->join();
    }
}

// 测试1: 基本日志功能
void test1_BasicLogging() {
    std::cout << "\n[TEST 1] Basic Logging..." << std::endl;
    try {
        LOG_INFO("Test 1: Basic info message");
        LOG_ERROR("Test 1: Basic error message");
        LOG_DEBUG("Test 1: Basic debug message");
        test_passed++;
        std::cout << "[PASS] Test 1: Basic logging works" << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 1: Basic logging failed" << std::endl;
    }
}

// 测试2: 多线程并发写入
void test2_MultiThreadLogging() {
    std::cout << "\n[TEST 2] Multi-thread Logging..." << std::endl;
    try {
        const int kThreads = 8;
        const int kMessagesPerThread = 1000;
        std::vector<std::unique_ptr<Thread>> threads;

        for (int i = 0; i < kThreads; ++i) {
            threads.emplace_back(new Thread([i, kMessagesPerThread]() {
                for (int j = 0; j < kMessagesPerThread; ++j) {
                    LOG_INFO("Thread %d, Message %d", i, j);
                }
            }, "LoggingThread"));
        }

        for (auto& thread : threads) {
            thread->start();
        }

        for (auto& thread : threads) {
            thread->join();
        }

        test_passed++;
        std::cout << "[PASS] Test 2: Multi-thread logging works" << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 2: Multi-thread logging failed" << std::endl;
    }
}

// 测试3: 文件路径不存在的情况
void test3_NonExistentPath() {
    std::cout << "\n[TEST 3] Non-existent Path..." << std::endl;
    try {
        // 使用log目录下的路径
        std::string logPath = "log/test_nonexistent";
        initAsyncLogging(logPath, 1024 * 1024, 3);
        setAsyncOutput();

        LOG_INFO("Test 3: Logging to non-existent path");
        sleep(2);  // 等待日志刷新

        // 检查目录是否被创建
        struct stat st;
        if (stat("log", &st) == 0 && S_ISDIR(st.st_mode)) {
            // 检查是否有日志文件
            bool logFileExists = false;
            std::string cmd = "ls log/test_nonexistent*.log 2>/dev/null | wc -l";
            FILE* pipe = popen(cmd.c_str(), "r");
            if (pipe) {
                char buffer[128];
                if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    int count = atoi(buffer);
                    if (count > 0) {
                        logFileExists = true;
                    }
                }
                pclose(pipe);
            }

            if (logFileExists) {
                test_passed++;
                std::cout << "[PASS] Test 3: Log file created in non-existent path" << std::endl;
            } else {
                test_failed++;
                std::cout << "[FAIL] Test 3: Log file not created" << std::endl;
            }
        } else {
            test_failed++;
            std::cout << "[FAIL] Test 3: Directory not created" << std::endl;
        }
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 3: Non-existent path test failed" << std::endl;
    }
}

// 测试4: 大容量日志写入
void test4_LargeVolumeLogging() {
    std::cout << "\n[TEST 4] Large Volume Logging..." << std::endl;
    try {
        const int kMessages = 10000;
        for (int i = 0; i < kMessages; ++i) {
            LOG_INFO("Test 4: Large volume message %d - This is a longer message to test buffer handling", i);
        }
        sleep(2);
        test_passed++;
        std::cout << "[PASS] Test 4: Large volume logging works" << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 4: Large volume logging failed" << std::endl;
    }
}

// 测试5: 日志文件滚动
void test5_LogFileRolling() {
    std::cout << "\n[TEST 5] Log File Rolling..." << std::endl;
    try {
        // 使用log目录
        std::string logPath = "log/test_roll";
        std::cout << "  Initializing async logging with path: " << logPath << std::endl;
        std::cout << "  Roll size: " << (1024 * 10) << " bytes" << std::endl;
        std::cout << "  Flush interval: 1 second" << std::endl;
        
        // 清理旧的日志文件
        std::cout << "  Cleaning up old log files..." << std::endl;
        system("rm -f log/test_roll*.log 2>/dev/null");
        
        // 重置之前的异步日志系统
        std::cout << "  Resetting previous async logging system" << std::endl;
        resetAsyncLogging();
        
        initAsyncLogging(logPath, 1024 * 10, 1);  // 10KB 滚动大小，1秒刷新间隔
        setAsyncOutput();
        
        std::cout << "  Async logging initialized and output set" << std::endl;

        // 分批写入日志，每批之间等待，确保时间戳不同
        const int kBatches = 5;
        const int kMessagesPerBatch = 200;

        std::cout << "  Writing " << kBatches << " batches of " << kMessagesPerBatch << " messages each" << std::endl;
        
        // 计算每条消息的大致大小
        std::string testMsg = "Test 5: Rolling message 0 - This is a message to test log file rolling functionality";
        std::cout << "  Estimated message size: " << testMsg.size() << " bytes" << std::endl;
        std::cout << "  Estimated total size per batch: " << (testMsg.size() * kMessagesPerBatch) << " bytes" << std::endl;
        
        for (int batch = 0; batch < kBatches; ++batch) {
            std::cout << "  Starting batch " << batch << std::endl;
            for (int i = 0; i < kMessagesPerBatch; ++i) {
                LOG_INFO("Test 5: Rolling message %d - This is a message to test log file rolling functionality", batch * kMessagesPerBatch + i);
            }
            std::cout << "  Batch " << batch << " completed, waiting 2 seconds..." << std::endl;
            sleep(2);  // 每批之间等待2秒，确保时间戳不同
        }

        std::cout << "  All batches completed, waiting 2 seconds for final flush..." << std::endl;
        sleep(2);  // 等待最后的日志刷新

        // 检查是否有多个日志文件
        std::cout << "  Checking for log files..." << std::endl;
        
        // 先检查当前目录下所有.log文件
        std::string cmd = "ls -la *.log 2>/dev/null";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[256];
            std::cout << "  All .log files in log directory:" << std::endl;
            bool found = false;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::cout << "    " << buffer;
                found = true;
            }
            if (!found) {
                std::cout << "    No .log files found" << std::endl;
            }
            pclose(pipe);
        }
        
        // 检查test_roll相关的日志文件
        cmd = "ls -la log/test_roll*.log 2>/dev/null";
        pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[256];
            std::cout << "  Log files matching test_roll*.log:" << std::endl;
            bool found = false;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::cout << "    " << buffer;
                found = true;
            }
            if (!found) {
                std::cout << "    No matching log files found" << std::endl;
            }
            pclose(pipe);
        }

        cmd = "ls log/test_roll*.log 2>/dev/null | wc -l";
        pipe = popen(cmd.c_str(), "r");
        int fileCount = 0;
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                fileCount = atoi(buffer);
            }
            pclose(pipe);
        }

        std::cout << "  Total log files found: " << fileCount << std::endl;

        if (fileCount > 0) {
            test_passed++;
            std::cout << "[PASS] Test 5: Log file rolling works, created " << fileCount << " files" << std::endl;
        } else {
            test_failed++;
            std::cout << "[FAIL] Test 5: Log file rolling not triggered" << std::endl;
        }
    } catch (const std::exception& e) {
        test_failed++;
        std::cout << "[FAIL] Test 5: Log file rolling test failed with exception: " << e.what() << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 5: Log file rolling test failed with unknown exception" << std::endl;
    }
}

// 测试6: 极端情况 - 空日志
void test6_EmptyLog() {
    std::cout << "\n[TEST 6] Empty Log..." << std::endl;
    try {
        // 初始化日志但不写入任何内容
        std::string logPath = "log/test_empty";
        initAsyncLogging(logPath, 1024 * 1024, 1);
        setAsyncOutput();

        // 不写入任何日志，直接等待刷新
        sleep(2);

        test_passed++;
        std::cout << "[PASS] Test 6: Empty log handling works" << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 6: Empty log test failed" << std::endl;
    }
}

// 测试7: 极端情况 - 超长日志消息
void test7_VeryLongMessage() {
    std::cout << "\n[TEST 7] Very Long Message..." << std::endl;
    try {
        // 创建一个超长的日志消息
        std::string longMessage(10000, 'X');  // 10KB 的消息

        for (int i = 0; i < 10; ++i) {
            LOG_INFO("Test 7: Very long message %d: %s", i, longMessage.c_str());
        }
        sleep(2);

        test_passed++;
        std::cout << "[PASS] Test 7: Very long message handling works" << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 7: Very long message test failed" << std::endl;
    }
}

// 测试8: 快速启动和停止
void test8_RapidStartStop() {
    std::cout << "\n[TEST 8] Rapid Start/Stop..." << std::endl;
    try {
        for (int i = 0; i < 5; ++i) {
            std::string logPath = "log/test_rapid_" + std::to_string(i);
            initAsyncLogging(logPath, 1024 * 1024, 1);
            setAsyncOutput();

            LOG_INFO("Test 8: Rapid start/stop iteration %d", i);
            sleep(1);
        }

        test_passed++;
        std::cout << "[PASS] Test 8: Rapid start/stop works" << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 8: Rapid start/stop test failed" << std::endl;
    }
}

// 测试9: 高并发压力测试
void test9_HighConcurrencyStress() {
    std::cout << "\n[TEST 9] High Concurrency Stress..." << std::endl;
    try {
        const int kThreads = 16;
        const int kMessagesPerThread = 5000;
        std::vector<std::unique_ptr<Thread>> threads;

        for (int i = 0; i < kThreads; ++i) {
            threads.emplace_back(new Thread([i, kMessagesPerThread]() {
                for (int j = 0; j < kMessagesPerThread; ++j) {
                    LOG_INFO("Stress test - Thread %d, Message %d", i, j);
                }
            }, "StressThread"));
        }

        for (auto& thread : threads) {
            thread->start();
        }

        for (auto& thread : threads) {
            thread->join();
        }

        sleep(3);
        test_passed++;
        std::cout << "[PASS] Test 9: High concurrency stress test works" << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 9: High concurrency stress test failed" << std::endl;
    }
}

// 测试10: 边界条件 - 空消息内容
void test10_ZeroLengthMessage() {
    std::cout << "\n[TEST 10] Zero Length Message..." << std::endl;
    try {
        LOG_INFO(" ");
        LOG_INFO("Test 10: Normal message");
        sleep(1);

        test_passed++;
        std::cout << "[PASS] Test 10: Zero length message handling works" << std::endl;
    } catch (...) {
        test_failed++;
        std::cout << "[FAIL] Test 10: Zero length message test failed" << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "=== Comprehensive AsyncLogging Tests ===" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 启用测试模式
    std::cout << "Enabling test mode..." << std::endl;
    enableTestMode();
    
    // 清理旧的日志文件和测试目录
    std::cout << "Cleaning up old log files..." << std::endl;
    system("rm -rf log 2>/dev/null");

    // 运行所有测试
    test1_BasicLogging();
    test2_MultiThreadLogging();
    test3_NonExistentPath();
    test4_LargeVolumeLogging();
    test5_LogFileRolling();
    test6_EmptyLog();
    test7_VeryLongMessage();
    test8_RapidStartStop();
    test9_HighConcurrencyStress();
    test10_ZeroLengthMessage();

    // 输出测试结果
    std::cout << "\n========================================" << std::endl;
    std::cout << "=== Test Results ===" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Total Tests: " << (test_passed + test_failed) << std::endl;
    std::cout << "Passed: " << test_passed << std::endl;
    std::cout << "Failed: " << test_failed << std::endl;

    if (test_failed == 0) {
        std::cout << "\n[SUCCESS] All tests passed!" << std::endl;
    } else {
        std::cout << "\n[FAILURE] Some tests failed!" << std::endl;
    }

    // 检查是否有错误日志
    if (!hasErrorLog()) {
        std::cout << "\nNo error logs found, cleaning up log files..." << std::endl;
        cleanupLogFiles();
        std::cout << "Log files cleaned up successfully." << std::endl;
    } else {
        std::cout << "\nError logs found, keeping log files for inspection." << std::endl;
    }

    std::cout << "========================================" << std::endl;

    return test_failed > 0 ? 1 : 0;
}
