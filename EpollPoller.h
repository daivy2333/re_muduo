#pragma once

#include <sys/epoll.h>
#include <vector>

#include "Poller.h"

/**
 * @brief EpollPoller类，基于epoll的IO复用Poller实现
 * 
 * EpollPoller是Poller的子类，使用Linux的epoll系统调用实现IO复用
 * 这是muduo在Linux平台下的默认Poller实现
 */
class EpollPoller : public Poller {
public:
    /**
     * @brief 构造函数
     * @param loop 所属的EventLoop
     */
    explicit EpollPoller(EventLoop* loop);

    /**
     * @brief 析构函数
     */
    ~EpollPoller() override;

    /**
     * @brief 等待IO事件
     * @param timeoutMs 超时时间（毫秒）
     * @param activeChannels 活跃的Channel列表
     * @return 事件发生的时间戳
     */
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;

    /**
     * @brief 更新Channel
     * @param channel 要更新的Channel
     */
    void updateChannel(Channel* channel) override;

    /**
     * @brief 移除Channel
     * @param channel 要移除的Channel
     */
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;  ///< 初始事件列表大小

    /**
     * @brief 填充活跃的Channel列表
     * @param numEvents 事件数量
     * @param activeChannels 活跃的Channel列表
     */
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    /**
     * @brief 更新Channel到epoll
     * @param operation 操作类型（EPOLL_CTL_ADD/MOD/DEL）
     * @param channel 要更新的Channel
     */
    void update(int operation, Channel* channel);

    using EventList = std::vector<epoll_event>;  ///< 事件列表类型

    int epollFd_;      ///< epoll文件描述符
    EventList events_; ///< 活跃事件列表
};