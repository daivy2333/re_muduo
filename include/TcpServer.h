#pragma once

#include "Eventloop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"
using namespace std;
#include <functional>
#include <string>
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include<atomic>
#include <unordered_map>
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop *loop, const InetAddress &listenAddr, Option option = kNoReusePort);

    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) {threadInitCallback_ = cb;}
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) {writeCompleteCallback_ = cb;}
    void setThreadNum(int numThreads);
    void start();
private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = unordered_map<string, TcpConnectionPtr>;
    EventLoop *loop_;
    const string ipPort_;
    const string name_;
    unique_ptr<Acceptor> acceptor_;
    shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;
    atomic_int  started_;

    int nextConnId_;
    ConnectionMap connections_;
};