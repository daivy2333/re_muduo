#pragma once

#include "Eventloop.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "noncopyable.h"
#include "Buffer.h"
#include "Timestamp.h"
#include "Timer.h"
#include <string>
#include <memory>

class Channel;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                 const std::string& name,
                 int sockfd,
                 const InetAddress& localAddr,
                 const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    void send(const std::string& message);
    void send(Buffer* message);
    void shutdown();
    void forceClose();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    // 定时器相关接口
    void setConnectionTimeout(double seconds);
    void setIdleTimeout(double seconds);
    void resetIdleTimer();
    void enableKeepAlive(bool enable, int interval = 30);

    void connectEstablished();
    void connectDestroyed();

    Buffer* inputBuffer() { return &inputBuffer_; }
    Buffer* outputBuffer() { return &outputBuffer_; }

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

    void setState(StateE s) { state_ = s; }
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const std::string& message);
    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();

    // 定时器相关私有方法
    void setConnectionTimeoutInLoop(double seconds);
    void resetIdleTimerInLoop();
    void setupKeepAliveTimer();
    void onConnectionTimeout();
    void onIdleTimeout();
    void onKeepAliveTimeout();

    EventLoop* loop_;
    std::string name_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    StateE state_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;

    // 定时器相关成员变量
    TimerId connectionTimeoutTimerId_;
    TimerId idleTimerId_;
    TimerId keepAliveTimerId_;
    double idleTimeout_;
    double keepAliveInterval_;
    bool keepAliveEnabled_;
    Timestamp lastActivityTime_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};
