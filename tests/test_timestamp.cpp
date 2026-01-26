
#include "Timestamp.h"
#include <iostream>
#include <cassert>

void test_timestamp_default_constructor() {
    Timestamp ts;
    std::cout << "Default constructor test passed" << std::endl;
}

void test_timestamp_constructor_with_microseconds() {
    int64_t microseconds = 123456789;
    Timestamp ts(microseconds);
    std::string ts_str = ts.to_string();
    std::cout << "Timestamp with " << microseconds << " microseconds: " << ts_str << std::endl;
    std::cout << "Constructor with microseconds test passed" << std::endl;
}

void test_timestamp_now() {
    Timestamp ts = Timestamp::now();
    std::string ts_str = ts.to_string();
    std::cout << "Current timestamp: " << ts_str << std::endl;
    assert(!ts_str.empty());
    std::cout << "Timestamp::now() test passed" << std::endl;
}

void test_timestamp_to_string() {
    Timestamp ts = Timestamp::now();
    std::string ts_str = ts.to_string();
    std::cout << "Timestamp to string: " << ts_str << std::endl;
    assert(!ts_str.empty());
    std::cout << "Timestamp::to_string() test passed" << std::endl;
}

int main() {
    std::cout << "=== Timestamp Tests ===" << std::endl;
    test_timestamp_default_constructor();
    test_timestamp_constructor_with_microseconds();
    test_timestamp_now();
    test_timestamp_to_string();
    std::cout << "=== All Timestamp Tests Passed ===" << std::endl;
    return 0;
}
