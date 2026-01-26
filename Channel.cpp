#include "Channel.h"
#include "Eventloop.h"
#include <sys/epoll.h>
#include <iostream>

// 静态常量定义
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

/**
 * Channel类的构造函数，用于初始化Channel对象
 * @param loop 指向EventLoop的指针，表示Channel所属的事件循环
 * @param fd 文件描述符，表示Channel所关注的I/O事件
 */
Channel::Channel(EventLoop* loop, int fd)
    : Loop(loop),          // 初始化事件循环指针
      Fd(fd),              // 初始化文件描述符
      Events(kNoneEvent),  // 初始化事件类型为无事件
      Revents(kNoneEvent), // 初始化活跃事件类型为无事件
      Index(-1),
      Tied(false) {        // 初始化是否绑定的标志为false
}

/**
 * Channel类的析构函数
 * 用于在Channel对象被销毁时进行资源清理和释放工作
 */
Channel::~Channel() {
    // 析构函数体为空，可能是因为：
    // 1. 该类没有需要手动释放的资源
    // 2. 资源清理工作已经在其他成员函数中完成
    // 3. 使用了智能指针等现代C++特性自动管理资源
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    TieObj = obj;
    Tied = true;
}

/**
 * 处理通道事件
 * @param receiveTime 接收到事件的时间戳
 */
void Channel::handleEvent(Timestamp receiveTime) {
    // 创建一个空的共享指针用于保护对象生命周期
    std::shared_ptr<void> guard;
    // 检查是否有对象被绑定到该通道
    if (Tied) {
        // 尝试获取绑定的弱引用对象
        guard = TieObj.lock();
        // 如果获取成功（即对象仍然存在），则处理事件
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
    } else {
        // 如果没有绑定对象，直接处理事件
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    // 处理挂起事件（对端关闭连接）
    if ((Revents & EPOLLHUP) && !(Revents & EPOLLIN)) {
        if (CloseCallback) {
            CloseCallback();
        }
    }

    // 处理错误事件
    if (Revents & EPOLLERR) {
        if (ErrorCallback) {
            ErrorCallback();
        }
    }

    // 处理读事件（包括对端关闭连接的EPOLLRDHUP）
    if (Revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (ReadCallback) {
            ReadCallback(receiveTime);
        }
    }

    // 处理写事件
    if (Revents & EPOLLOUT) {
        if (WriteCallback) {
            WriteCallback();
        }
    }
}

void Channel::update() {
    Loop->updateChannel(this);
}

/**
 * 从事件循环中移除当前通道
 * 该函数用于将当前通道从其所属的事件循环中移除，
 * 这通常在通道不再需要监控事件时调用，以释放资源
 */
void Channel::remove() {
    Loop->removeChannel(this);
}
