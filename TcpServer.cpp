#include "TcpServer.h"
#include "TcpConnection.h"
#include "Eventloop.h"
#include "Acceptor.h"
#include "Socket.h"
#include <stdio.h>
#include <assert.h>
#include <functional>
using namespace std::placeholders;

using namespace std;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, Option option)
    : loop_(loop),
      ipPort_(listenAddr.toIpPort()),
      name_(ipPort_),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, "TcpServer")),
      connectionCallback_(),
      messageCallback_(),
      writeCompleteCallback_(),
      threadInitCallback_(),
      started_(0),
      nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
        bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    assert(loop_->isInLoopThread());
    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNUm(numThreads);
}

void TcpServer::start()
{
    if (started_++ == 0)
    {
        threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listenning());
        loop_->runInLoop(bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    assert(loop_->isInLoopThread());
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;

    InetAddress localAddr(Socket::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        bind(&TcpServer::removeConnection, this, _1));
    ioLoop->runInLoop(bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    assert(loop_->isInLoopThread());
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        bind(&TcpConnection::connectDestroyed, conn));
}
