#include "Timestamp.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

void test_timestamp_default_constructor() {
    std::cout << "Test Timestamp default constructor" << std::endl;
    Timestamp ts;
    assert(!ts.valid()); // 默认构造的时间戳应该是无效的
    assert(ts.microSecondsSinceEpoch() == 0);
    std::cout << "Default constructor test passed" << std::endl;
}

void test_timestamp_constructor_with_microseconds() {
    std::cout << "Test Timestamp constructor with microseconds" << std::endl;
    int64_t microseconds = 123456789;
    Timestamp ts(microseconds);
    assert(ts.valid());
    assert(ts.microSecondsSinceEpoch() == microseconds);
    std::string ts_str = ts.to_string();
    std::cout << "Timestamp with " << microseconds << " microseconds: " << ts_str << std::endl;
    std::cout << "Constructor with microseconds test passed" << std::endl;
}

void test_timestamp_now() {
    std::cout << "Test Timestamp::now()" << std::endl;
    Timestamp ts1 = Timestamp::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Timestamp ts2 = Timestamp::now();

    assert(ts1.valid());
    assert(ts2.valid());
    assert(ts2 > ts1); // ts2应该晚于ts1

    std::string ts_str = ts1.to_string();
    assert(!ts_str.empty());

    std::cout << "Current timestamp 1: " << ts_str << std::endl;
    std::cout << "Current timestamp 2: " << ts2.to_string() << std::endl;
    std::cout << "Timestamp::now() test passed" << std::endl;
}

void test_timestamp_to_string() {
    std::cout << "Test Timestamp::to_string()" << std::endl;
    Timestamp ts = Timestamp::now();
    std::string ts_str = ts.to_string();
    std::cout << "Timestamp to string: " << ts_str << std::endl;
    assert(!ts_str.empty());
    assert(ts_str.length() >= 17); // 格式: YYYYMMDD HH:MM:SS.mmmmmm
    std::cout << "Timestamp::to_string() test passed" << std::endl;
}

void test_timestamp_comparison_operators() {
    std::cout << "Test Timestamp comparison operators" << std::endl;

    Timestamp ts1(1000000); // 1秒
    Timestamp ts2(2000000); // 2秒
    Timestamp ts3(2000000); // 2秒（与ts2相同）
    Timestamp ts4(3000000); // 3秒

    // 测试 <
    assert(ts1 < ts2);
    assert(!(ts2 < ts1));
    assert(!(ts2 < ts3));

    // 测试 >
    assert(ts2 > ts1);
    assert(!(ts1 > ts2));
    assert(!(ts2 > ts3));

    // 测试 ==
    assert(ts2 == ts3);
    assert(!(ts1 == ts2));

    // 测试 !=
    assert(ts1 != ts2);
    assert(!(ts2 != ts3));

    // 测试 <=
    assert(ts1 <= ts2);
    assert(ts2 <= ts3);
    assert(!(ts2 <= ts1));

    // 测试 >=
    assert(ts2 >= ts1);
    assert(ts2 >= ts3);
    assert(!(ts1 >= ts2));

    std::cout << "Comparison operators test passed" << std::endl;
}

void test_timestamp_valid() {
    std::cout << "Test Timestamp::valid()" << std::endl;

    Timestamp ts1; // 默认构造，应该是无效的
    assert(!ts1.valid());

    Timestamp ts2(0); // 0微秒，应该是无效的
    assert(!ts2.valid());

    Timestamp ts3(1); // 1微秒，应该是有效的
    assert(ts3.valid());

    Timestamp ts4(-1); // 负数，应该是无效的
    assert(!ts4.valid());

    std::cout << "Timestamp::valid() test passed" << std::endl;
}

void test_timestamp_invalid() {
    std::cout << "Test Timestamp::invalid()" << std::endl;

    Timestamp ts = Timestamp::invalid();
    assert(!ts.valid());
    assert(ts.microSecondsSinceEpoch() == 0);

    std::cout << "Timestamp::invalid() test passed" << std::endl;
}

void test_addTime() {
    std::cout << "Test addTime()" << std::endl;

    Timestamp ts(1000000); // 1秒

    // 测试增加正数秒
    Timestamp ts1 = addTime(ts, 1.0); // 增加1秒
    assert(ts1.microSecondsSinceEpoch() == 2000000);

    // 测试增加负数秒
    Timestamp ts2 = addTime(ts, -0.5); // 减少0.5秒
    assert(ts2.microSecondsSinceEpoch() == 500000);

    // 测试增加小数秒
    Timestamp ts3 = addTime(ts, 0.5); // 增加0.5秒
    assert(ts3.microSecondsSinceEpoch() == 1500000);

    // 测试增加0秒
    Timestamp ts4 = addTime(ts, 0.0); // 不增加
    assert(ts4.microSecondsSinceEpoch() == 1000000);

    std::cout << "addTime() test passed" << std::endl;
}

void test_timeDifference() {
    std::cout << "Test timeDifference()" << std::endl;

    Timestamp ts1(1000000); // 1秒
    Timestamp ts2(2000000); // 2秒
    Timestamp ts3(3000000); // 3秒

    // 测试正常时间差
    double diff1 = timeDifference(ts2, ts1);
    assert(diff1 >= 0.99 && diff1 <= 1.01); // 约1秒

    // 测试相等时间戳
    double diff2 = timeDifference(ts1, ts1);
    assert(diff2 >= -0.01 && diff2 <= 0.01); // 约0秒

    // 测试时间差累加
    double diff3 = timeDifference(ts3, ts1);
    assert(diff3 >= 1.99 && diff3 <= 2.01); // 约2秒

    std::cout << "timeDifference() test passed" << std::endl;
}

void test_timestamp_boundary_values() {
    std::cout << "Test Timestamp boundary values" << std::endl;

    // 测试最大值
    int64_t maxMicroseconds = INT64_MAX;
    Timestamp maxTs(maxMicroseconds);
    assert(maxTs.valid());

    // 测试最小正值
    Timestamp minTs(1);
    assert(minTs.valid());

    // 测试0
    Timestamp zeroTs(0);
    assert(!zeroTs.valid());

    // 测试-1
    Timestamp negTs(-1);
    assert(!negTs.valid());

    std::cout << "Boundary values test passed" << std::endl;
}

void test_timestamp_precision() {
    std::cout << "Test Timestamp precision" << std::endl;

    // 测试微秒级精度
    Timestamp ts1(1234567890);
    Timestamp ts2(1234567891); // 相差1微秒

    assert(ts2 > ts1);
    double diff = timeDifference(ts2, ts1);
    assert(diff >= 0.0000009 && diff <= 0.0000011); // 约1微秒

    std::cout << "Timestamp precision test passed" << std::endl;
}

void test_timestamp_format() {
    std::cout << "Test Timestamp format" << std::endl;

    // 创建一个已知时间的时间戳
    Timestamp ts(1234567890000000LL); // 2009-02-13 23:31:30.000000

    std::string ts_str = ts.to_string();
    std::cout << "Formatted timestamp: " << ts_str << std::endl;

    // 验证格式：YYYYMMDD HH:MM:SS.mmmmmm
    assert(ts_str.length() == 17 + 7); // 17位日期时间 + 7位微秒

    std::cout << "Timestamp format test passed" << std::endl;
}

int main() {
    std::cout << "=== Timestamp Tests ===" << std::endl;

    test_timestamp_default_constructor();
    test_timestamp_constructor_with_microseconds();
    test_timestamp_now();
    test_timestamp_to_string();
    test_timestamp_comparison_operators();
    test_timestamp_valid();
    test_timestamp_invalid();
    test_addTime();
    test_timeDifference();
    test_timestamp_boundary_values();
    test_timestamp_precision();
    test_timestamp_format();

    std::cout << "=== All Timestamp Tests Passed ===" << std::endl;
    return 0;
}
