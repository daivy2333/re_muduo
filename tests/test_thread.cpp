#include "Thread.h"
#include "CurrentThread.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>
#include <atomic>
#include <set>
#include <memory>


// 测试线程的基本创建和启动
void test_thread_creation() {
    std::cout << "=== Test Thread Creation ===" << std::endl;

    bool flag = false;
    Thread thread([&flag]() {
        flag = true;
    }, "TestThread");

    assert(!thread.started());
    assert(thread.name() == "TestThread");

    thread.start();
    assert(thread.started());

    thread.join();
    assert(flag);

    std::cout << "Thread creation test passed" << std::endl;
}

// 测试线程ID获取
void test_thread_id() {
    std::cout << "=== Test Thread ID ===" << std::endl;

    Thread thread([]() {
        std::cout << "Thread running with TID: " << CurrentThread::tid() << std::endl;
    }, "TIDTest");

    thread.start();

    // 等待线程启动并获取tid
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pid_t tid = thread.tid();
    assert(tid > 0);
    std::cout << "Thread TID from Thread object: " << tid << std::endl;

    thread.join();
    std::cout << "Thread ID test passed" << std::endl;
}

// 测试默认命名
void test_default_naming() {
    std::cout << "=== Test Default Naming ===" << std::endl;

    std::vector<std::unique_ptr<Thread>> threads;
    const int numThreads = 3;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(new Thread([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }));
    }

    for (int i = 0; i < numThreads; ++i) {
        threads[i]->start();
        std::cout << "Thread " << i << " name: " << threads[i]->name() << std::endl;
    }

    for (auto& t : threads) {
        t->join();
    }

    std::cout << "Default naming test passed" << std::endl;
}

// 测试多线程并发执行
void test_concurrent_threads() {
    std::cout << "=== Test Concurrent Threads ===" << std::endl;

    const int numThreads = 5;
    std::atomic<int> counter(0);
    std::vector<std::unique_ptr<Thread>> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(new Thread([&counter]() {
            counter.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }));
    }

    for (auto& t : threads) {
        t->start();
    }

    for (auto& t : threads) {
        t->join();
    }

    assert(counter == numThreads);
    std::cout << "Concurrent threads test passed" << std::endl;
}

// 测试线程计数
void test_thread_count() {
    std::cout << "=== Test Thread Count ===" << std::endl;

    int initialCount = Thread::numCreated();
    std::cout << "Initial thread count: " << initialCount << std::endl;

    {
        Thread t1([]() {});
        Thread t2([]() {});

        t1.start();
        t2.start();

        assert(Thread::numCreated() == initialCount + 2);

        t1.join();
        t2.join();
    }

    std::cout << "Final thread count: " << Thread::numCreated() << std::endl;
    std::cout << "Thread count test passed" << std::endl;
}

// 测试线程多次join
void test_multiple_join() {
    std::cout << "=== Test Multiple Join ===" << std::endl;

    Thread thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    thread.start();
    thread.join();

    // 第二次join不应该有副作用
    thread.join();

    std::cout << "Multiple join test passed" << std::endl;
}

// 测试重复启动线程
void test_duplicate_start() {
    std::cout << "=== Test Duplicate Start ===" << std::endl;
    Thread thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    thread.start();
    // 重复启动应该抛出异常
    bool exceptionThrown = false;
    try {
        thread.start();
    } catch (const std::runtime_error& e) {
        exceptionThrown = true;
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    
    assert(exceptionThrown);
    thread.join();
    std::cout << "Duplicate start test passed" << std::endl;
}

// 测试析构函数的detach行为
void test_destructor_detach() {
    std::cout << "=== Test Destructor Detach ===" << std::endl;
    std::atomic<bool> running(false);
    std::atomic<bool> finished(false);
    
    {
        Thread thread([&running, &finished]() {
            running = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            finished = true;
        });
        thread.start();
        // 等待线程真正启动
        while (!running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        // 线程在这里离开作用域，析构函数应该detach线程
    }
    
    // 等待线程完成
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    assert(finished);
    std::cout << "Destructor detach test passed" << std::endl;
}

// 测试线程函数中的异常
void test_thread_exception() {
    std::cout << "=== Test Thread Exception ===" << std::endl;
    std::atomic<bool> exceptionCaught(false);
    
    Thread thread([&exceptionCaught]() {
        try {
            throw std::runtime_error("Test exception");
        } catch (const std::exception& e) {
            exceptionCaught = true;
        }
    });
    
    thread.start();
    thread.join();
    assert(exceptionCaught);
    std::cout << "Thread exception test passed" << std::endl;
}

// 测试CurrentThread::tid()的线程隔离性
void test_currentthread_tid_isolation() {
    std::cout << "=== Test CurrentThread TID Isolation ===" << std::endl;
    
    std::vector<pid_t> tids;
    std::vector<std::unique_ptr<Thread>> threads;
    const int numThreads = 5;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(new Thread([&tids]() {
            tids.push_back(CurrentThread::tid());
        }));
    }
    
    for (auto& t : threads) {
        t->start();
    }
    
    for (auto& t : threads) {
        t->join();
    }
    
    // 验证所有线程的TID都不相同
    std::set<pid_t> uniqueTids(tids.begin(), tids.end());
    assert(uniqueTids.size() == numThreads);
    std::cout << "CurrentThread TID isolation test passed" << std::endl;
}

// 测试未启动线程的tid()
void test_unstarted_thread_tid() {
    std::cout << "=== Test Unstarted Thread TID ===" << std::endl;
    
    Thread thread([]() {});
    assert(thread.tid() == 0);
    
    thread.start();
    thread.join();
    assert(thread.tid() > 0);
    
    std::cout << "Unstarted thread TID test passed" << std::endl;
}

// 测试高并发下的线程计数
void test_high_concurrent_thread_count() {
    std::cout << "=== Test High Concurrent Thread Count ===" << std::endl;
    
    const int numThreads = 100;
    std::vector<std::unique_ptr<Thread>> threads;
    
    int initialCount = Thread::numCreated();
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(new Thread([]() {}));
    }
    
    // 验证线程计数是否正确
    assert(Thread::numCreated() == initialCount + numThreads);
    
    for (auto& t : threads) {
        t->start();
    }
    
    for (auto& t : threads) {
        t->join();
    }
    
    std::cout << "High concurrent thread count test passed" << std::endl;
}

// 测试线程名称的边界情况
void test_thread_name_edge_cases() {
    std::cout << "=== Test Thread Name Edge Cases ===" << std::endl;
    
    // 空名称（使用默认命名）
    Thread t1([]() {});
    assert(!t1.name().empty());
    
    // 特殊字符名称
    Thread t2([]() {}, "Test@Thread#123");
    assert(t2.name() == "Test@Thread#123");
    
    // 长名称
    std::string longName(1000, 'A');
    Thread t3([]() {}, longName);
    assert(t3.name() == longName);
    
    std::cout << "Thread name edge cases test passed" << std::endl;
}

// 测试join()的线程安全性
void test_join_thread_safety() {
    std::cout << "=== Test Join Thread Safety ===" << std::endl;
    
    Thread thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    thread.start();
    
    // 多次join应该是安全的
    thread.join();
    thread.join();
    thread.join();
    
    std::cout << "Join thread safety test passed" << std::endl;
}

// 测试线程函数中的资源管理
void test_thread_resource_management() {
    std::cout << "=== Test Thread Resource Management ===" << std::endl;
    
    std::shared_ptr<std::atomic<int>> counter = 
        std::make_shared<std::atomic<int>>(0);
    
    {
        std::vector<std::unique_ptr<Thread>> threads;
        for (int i = 0; i < 5; ++i) {
            threads.emplace_back(new Thread([counter]() {
                (*counter)++;
            }));
            threads.back()->start();
        }
        
        for (auto& t : threads) {
            t->join();
        }
    }
    
    assert(*counter == 5);
    std::cout << "Thread resource management test passed" << std::endl;
}

// 测试未启动线程的join
void test_join_without_start() {
    std::cout << "=== Test Join Without Start ===" << std::endl;
    
    Thread thread([]() {});
    // 未启动就join，验证行为
    thread.join();
    
    std::cout << "Join without start test passed" << std::endl;
}

// 测试线程函数中访问Thread对象
void test_thread_self_access() {
    std::cout << "=== Test Thread Self Access ===" << std::endl;
    
    std::atomic<pid_t> threadTid(0);
    Thread thread([&threadTid]() {
        threadTid = CurrentThread::tid();
    }, "SelfAccess");
    
    thread.start();
    thread.join();
    
    assert(thread.tid() == threadTid);
    assert(thread.name() == "SelfAccess");
    
    std::cout << "Thread self access test passed" << std::endl;
}

// 测试大量线程创建和销毁
void test_massive_thread_creation() {
    std::cout << "=== Test Massive Thread Creation ===" << std::endl;
    
    const int numIterations = 10;
    const int threadsPerIteration = 50;
    
    for (int iter = 0; iter < numIterations; ++iter) {
        std::vector<std::unique_ptr<Thread>> threads;
        std::atomic<int> counter(0);
        
        for (int i = 0; i < threadsPerIteration; ++i) {
            threads.emplace_back(new Thread([&counter]() {
                counter.fetch_add(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }));
        }
        
        for (auto& t : threads) {
            t->start();
        }
        
        for (auto& t : threads) {
            t->join();
        }
        
        assert(counter == threadsPerIteration);
    }
    
    std::cout << "Massive thread creation test passed" << std::endl;
}

int main() {
    std::cout << "=== Thread Tests ===" << std::endl;

    test_thread_creation();
    test_thread_id();
    test_default_naming();
    test_concurrent_threads();
    test_thread_count();
    test_multiple_join();
    test_duplicate_start();
    test_destructor_detach();
    test_thread_exception();
    test_currentthread_tid_isolation();
    test_unstarted_thread_tid();
    test_high_concurrent_thread_count();
    test_thread_name_edge_cases();
    test_join_thread_safety();
    test_thread_resource_management();
    test_join_without_start();
    test_thread_self_access();
    test_massive_thread_creation();

    std::cout << "=== All Thread Tests Passed ===" << std::endl;
    return 0;
}
