#pragma once

#include <string>
#include <cstring>
#include <algorithm>

class LogStream {
public:
    typedef LogStream self;

    enum { kMaxNumericSize = 48 };

    LogStream() : cur_(data_) {}

    ~LogStream() = default;

    self& operator<<(bool v) {
        append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);

    self& operator<<(const void*);

    self& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }

    self& operator<<(double);
    self& operator<<(char v) {
        append(&v, 1);
        return *this;
    }

    self& operator<<(const char* str) {
        if (str) {
            append(str, strlen(str));
        } else {
            append("(null)", 6);
        }
        return *this;
    }

    self& operator<<(const unsigned char* str) {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    self& operator<<(const std::string& v) {
        append(v.c_str(), v.size());
        return *this;
    }

    void append(const char* data, size_t len) {
        if (static_cast<size_t>(avail()) > len) {
            memcpy(cur_, data, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }

    void reset() { cur_ = data_; }
    void bzero() { memset(data_, 0, sizeof(data_)); }

private:
    char data_[4000];
    char* cur_;

    static const int kSmallBuffer = 4000;
    static const int kLargeBuffer = 4000 * 1000;

    int avail() const { return static_cast<int>(end() - cur_); }
    const char* end() const { return data_ + sizeof(data_); }

    template<typename T>
    void formatInteger(T);
};

class FixedBuffer {
public:
    FixedBuffer()
        : cur_(data_) {}

    ~FixedBuffer() = default;

    void append(const char* buf, size_t len) {
        if (static_cast<size_t>(avail()) > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }

    char* current() { return cur_; }
    void add(size_t len) { cur_ += len; }
    void reset() { cur_ = data_; }
    void bzero() { memset(data_, 0, sizeof(data_)); }
    int avail() const { return static_cast<int>(end() - cur_); }
    const char* end() const { return data_ + sizeof(data_); }

    static const int kSize = 4000;

private:
    char data_[kSize];
    char* cur_;
};
