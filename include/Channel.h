#pragma once

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop;

/**
 * @brief Channel类，用于管理文件描述符的事件和回调
 * 
 * Channel类不拥有文件描述符，它只是对文件描述符的封装
 * Channel的生命周期由其拥有者控制（通常是TcpConnection）
 */
class Channel : noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    /**
     * @brief 构造函数
     * @param loop 所属的EventLoop
     * @param fd 文件描述符
     */
    Channel(EventLoop* loop, int fd);
    ~Channel();

    /**
     * @brief 处理事件
     * @param receiveTime 事件到达时间
     */
    void handleEvent(Timestamp receiveTime);

    /**
     * @brief 设置读事件回调
     */
    void setReadCallback(ReadEventCallback cb) { ReadCallback = std::move(cb); }

    /**
     * @brief 设置写事件回调
     */
    void setWriteCallback(EventCallback cb) { WriteCallback = std::move(cb); }

    /**
     * @brief 设置关闭事件回调
     */
    void setCloseCallback(EventCallback cb) { CloseCallback = std::move(cb); }

    /**
     * @brief 设置错误事件回调
     */
    void setErrorCallback(EventCallback cb) { ErrorCallback = std::move(cb); }

    /**
     * @brief 绑定对象，防止Channel在回调期间被意外销毁
     * @param obj 要绑定的对象
     */
    void tie(const std::shared_ptr<void>& obj);

    /**
     * @brief 获取文件描述符
     */
    int fd() const { return Fd; }

    /**
     * @brief 获取关注的事件
     */
    int events() const { return Events; }

    /**
     * @brief 设置活跃事件（由Poller设置）
     */
    void setRevents(int revt) { Revents = revt; }

    /**
     * @brief 判断是否没有关注任何事件
     */
    bool isNoneEvent() const { return Events == kNoneEvent; }

    /**
     * @brief 判断是否关注写事件
     */
    bool isWriting() const { return Events & kWriteEvent; }

    /**
     * @brief 判断是否关注读事件
     */
    bool isReading() const { return Events & kReadEvent; }

    /**
     * @brief 获取在Poller中的索引
     */
    int index() { return Index; }

    /**
     * @brief 设置在Poller中的索引
     */
    void setIndex(int idx) { Index = idx; }

    /**
     * @brief 启用读事件
     */
    void enableReading() { Events |= kReadEvent; update(); }

    /**
     * @brief 禁用读事件
     */
    void disableReading() { Events &= ~kReadEvent; update(); }

    /**
     * @brief 启用写事件
     */
    void enableWriting() { Events |= kWriteEvent; update(); }

    /**
     * @brief 禁用写事件
     */
    void disableWriting() { Events &= ~kWriteEvent; update(); }

    /**
     * @brief 禁用所有事件
     */
    void disableAll() { Events = kNoneEvent; update(); }

    /**
     * @brief 获取所属的EventLoop
     */
    EventLoop* ownerLoop() { return Loop; }

    /**
     * @brief 从EventLoop中移除自己
     */
    void remove();

private:
    /**
     * @brief 更新Channel到Poller
     */
    void update();

    /**
     * @brief 实际处理事件（在tie保护下）
     */
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;  ///< 无事件
    static const int kReadEvent;   ///< 读事件
    static const int kWriteEvent;  ///< 写事件

    EventLoop* Loop;               ///< 所属的EventLoop
    const int Fd;                  ///< 文件描述符
    int Events;                    ///< 关注的事件
    int Revents;                   ///< 活跃的事件（由Poller设置）
    int Index;                     ///< 在Poller中的索引

    std::weak_ptr<void> TieObj;    ///< 绑定的对象（用于延长生命周期）
    bool Tied;                     ///< 是否已绑定

    ReadEventCallback ReadCallback;    ///< 读事件回调
    EventCallback WriteCallback;       ///< 写事件回调
    EventCallback CloseCallback;       ///< 关闭事件回调
    EventCallback ErrorCallback;       ///< 错误事件回调
};