#pragma once

#include <unordered_map>
#include <vector>

#include "noncopyable.h"
#include "Timestamp.h"

// 前向声明，避免循环依赖
// Eventloop.h 和 Poller.h 相互包含，需要使用前向声明
class EventLoop;
class Channel;

/**
 * @brief Poller类，IO复用的基类
 * 
 * Poller是IO复用的抽象基类，提供了统一的接口
 * 具体实现由EpollPoller等子类完成
 */
class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    /**
     * @brief 构造函数
     * @param loop 所属的EventLoop
     */
    explicit Poller(EventLoop* loop);

    /**
     * @brief 虚析构函数
     */
    virtual ~Poller();

    /**
     * @brief 等待IO事件
     * @param timeoutMs 超时时间（毫秒）
     * @param activeChannels 活跃的Channel列表
     * @return 事件发生的时间戳
     */
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    /**
     * @brief 更新Channel
     * @param channel 要更新的Channel
     */
    virtual void updateChannel(Channel* channel) = 0;

    /**
     * @brief 移除Channel
     * @param channel 要移除的Channel
     */
    virtual void removeChannel(Channel* channel) = 0;

    /**
     * @brief 判断是否包含某个Channel
     * @param channel 要判断的Channel
     * @return 是否包含
     */
    bool hasChannel(Channel* channel) const;

    /**
     * @brief 创建默认的Poller
     * @param loop 所属的EventLoop
     * @return Poller指针
     */
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;  ///< Channel映射表类型
    ChannelMap channels_;  ///< Channel映射表

private:
    EventLoop* ownerLoop_;  ///< 所属的EventLoop
};