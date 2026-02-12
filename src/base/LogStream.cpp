#include "LogStream.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <functional>

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

template<typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

template<typename T>
void LogStream::formatInteger(T v) {
    if (avail() >= kMaxNumericSize) {
        size_t len = convert(cur_, v);
        cur_ += len;
    }
}

LogStream& LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (avail() >= kMaxNumericSize) {
        char* buf = cur_;
        *buf++ = '0';
        *buf++ = 'x';

        for (int i = (sizeof(void*) * 2) - 1; i >= 0; --i) {
            int shift = i * 4;
            int nibble = (v >> shift) & 0xf;
            *buf++ = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
        }
        cur_ = buf;
    }
    return *this;
}

LogStream& LogStream::operator<<(double v) {
    if (avail() >= kMaxNumericSize) {
        int len = snprintf(cur_, kMaxNumericSize, "%.12g", v);
        cur_ += len;
    }
    return *this;
}
