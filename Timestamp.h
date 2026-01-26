#pragma once

#include <iostream>
#include <string>

/**
 * @brief Timestamp类，用于表示时间戳
 * 
 * 封装了微秒级的时间戳，提供了获取当前时间和格式化输出的功能
 */
class Timestamp {
public:
    /**
     * @brief 默认构造函数，构造一个无效的时间戳
     */
    Timestamp();

    /**
     * @brief 构造函数，使用微秒数构造时间戳
     * @param microSecondsSinceEpoch 从纪元开始的微秒数
     */
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    /**
     * @brief 获取当前时间
     * @return 当前时间的Timestamp对象
     */
    static Timestamp now();

    /**
     * @brief 转换为字符串
     * @return 时间戳的字符串表示
     */
    std::string to_string() const;

    /**
     * @brief 判断时间戳是否有效
     * @return true表示有效，false表示无效
     */
    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    /**
     * @brief 获取微秒数
     * @return 从纪元开始的微秒数
     */
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    /**
     * @brief 获取无效的时间戳
     * @return 无效的Timestamp对象
     */
    static Timestamp invalid();

    static const int kMicroSecondsPerSecond = 1000 * 1000;

    // 比较运算符
    bool operator<(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ < rhs.microSecondsSinceEpoch_;
    }
    bool operator>(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ > rhs.microSecondsSinceEpoch_;
    }
    bool operator==(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ == rhs.microSecondsSinceEpoch_;
    }
    bool operator!=(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ != rhs.microSecondsSinceEpoch_;
    }
    bool operator<=(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ <= rhs.microSecondsSinceEpoch_;
    }
    bool operator>=(const Timestamp& rhs) const {
        return microSecondsSinceEpoch_ >= rhs.microSecondsSinceEpoch_;
    }

private:
    int64_t microSecondsSinceEpoch_;  ///< 从纪元开始的微秒数
};

// 在时间戳上增加指定秒数
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

// 计算两个时间戳之间的时间差（秒）
inline double timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}