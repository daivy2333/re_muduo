#pragma once

/**
 * @brief noncopyable类，禁止拷贝和赋值的基类
 * 
 * 通过私有化拷贝构造函数和赋值运算符，
 * 并使用 = delete 显式删除，使派生类也无法进行拷贝和赋值
 * 
 * 使用方法：让需要禁止拷贝的类继承自noncopyable
 * 
 * 示例：
 * @code
 * class MyClass : public noncopyable {
 *     // ...
 * };
 * @endcode
 */
class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;            ///< 禁止拷贝构造
    noncopyable& operator=(const noncopyable&) = delete; ///< 禁止拷贝赋值

protected:
    noncopyable() = default;   ///< 默认构造函数
    ~noncopyable() = default;  ///< 默认析构函数
};