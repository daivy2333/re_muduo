#include "TcpConnection.h"
#include "Eventloop.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include "Logger.h"
#include "Timer.h"
#include <unistd.h>
#include <errno.h>
#include <functional>

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(loop),
      name_(name),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, socket_->fd())),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      state_(kConnecting),
      highWaterMark_(64*1024*1024),
      idleTimeout_(0),
      keepAliveInterval_(30),
      keepAliveEnabled_(false),
      lastActivityTime_(Timestamp::now())
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG("TcpConnection created, name=%s, fd=%d, state=%d", 
              name_.c_str(), socket_->fd(), state_);
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    LOG_INFO("Connection established: %s", name_.c_str());
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
    LOG_INFO("Connection destroyed: %s", name_.c_str());
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        
        // 重置空闲定时器
        if (idleTimeout_ > 0)
        {
            resetIdleTimer();
        }
        
        lastActivityTime_ = receiveTime;
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting())
    {
        ssize_t n = ::write(channel_->fd(),
                           outputBuffer_.peek(),
                           outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::handleWrite write error, name=%s, errno=%d", name_.c_str(), errno);
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    handleClose();
                }
            }
        }
    }
    else
    {
        LOG_INFO("handleWrite called but channel is not writing, name=%s", name_.c_str());
    }
}

void TcpConnection::handleClose()
{
    setState(kDisconnected);
    channel_->disableAll();
    
    // 取消所有定时器
    if (connectionTimeoutTimerId_.isValid())
    {
        loop_->cancel(connectionTimeoutTimerId_);
    }
    if (idleTimerId_.isValid())
    {
        loop_->cancel(idleTimerId_);
    }
    if (keepAliveTimerId_.isValid())
    {
        loop_->cancel(keepAliveTimerId_);
    }
    
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
    
}

void TcpConnection::handleError()
{
    int err = 0;
    socklen_t optlen = sizeof err;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &err, &optlen) < 0)
    {
        err = errno;
    }
    LOG_ERROR("TcpConnection::handleError, name=%s, errno=%d", name_.c_str(), err);
}

void TcpConnection::send(const std::string& message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            void (TcpConnection::*fp)(const std::string&) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp,
                         this,
                         message));
        }
    }
}

void TcpConnection::send(Buffer* message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message->peek(), message->readableBytes());
            message->retrieveAll();
        }
        else
        {
            std::string msg(message->retrieveAllAsString());
            void (TcpConnection::*fp)(const std::string&) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp,
                         this,
                         msg));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == kDisconnected)
    {
        LOG_INFO("TcpConnection::sendInLoop, connection disconnected, name=%s", name_.c_str());
        return;
    }

    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop write error, name=%s, errno=%d", name_.c_str(), errno);
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        LOG_INFO("TcpConnection::sendInLoop, name=%s, oldLen=%zu, remaining=%zu, highWaterMark_=%zu",
                  name_.c_str(), oldLen, remaining, highWaterMark_);
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            LOG_INFO("TcpConnection::sendInLoop, high water mark reached, name=%s, size=%zu",
                     name_.c_str(), oldLen + remaining);
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->runInLoop(
            std::bind(&TcpConnection::forceCloseInLoop, this));
    }
}

void TcpConnection::forceCloseInLoop()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        handleClose();
    }
}

// 定时器相关方法实现
void TcpConnection::setConnectionTimeout(double seconds)
{
    if (loop_->isInLoopThread())
    {
        setConnectionTimeoutInLoop(seconds);
    }
    else
    {
        loop_->runInLoop(std::bind(&TcpConnection::setConnectionTimeoutInLoop,
                                  shared_from_this(), seconds));
    }
}

void TcpConnection::setConnectionTimeoutInLoop(double seconds)
{
    // 取消现有超时定时器
    if (connectionTimeoutTimerId_.isValid())
    {
        loop_->cancel(connectionTimeoutTimerId_);
    }

    if (seconds > 0)
    {
        // 设置新的超时定时器
        connectionTimeoutTimerId_ = loop_->runAfter(
            seconds,
            std::bind(&TcpConnection::onConnectionTimeout, shared_from_this()));
    }
}

void TcpConnection::setIdleTimeout(double seconds)
{
    idleTimeout_ = seconds;
    if (seconds > 0)
    {
        resetIdleTimer();
    }
    else if (idleTimerId_.isValid())
    {
        loop_->cancel(idleTimerId_);
        idleTimerId_ = TimerId();
    }
}

void TcpConnection::resetIdleTimer()
{
    if (idleTimeout_ <= 0) return;

    if (loop_->isInLoopThread())
    {
        resetIdleTimerInLoop();
    }
    else
    {
        loop_->runInLoop(std::bind(&TcpConnection::resetIdleTimerInLoop,
                                  shared_from_this()));
    }
}

void TcpConnection::resetIdleTimerInLoop()
{
    if (idleTimerId_.isValid())
    {
        loop_->cancel(idleTimerId_);
    }

    idleTimerId_ = loop_->runAfter(
        idleTimeout_,
        std::bind(&TcpConnection::onIdleTimeout, shared_from_this()));

    lastActivityTime_ = Timestamp::now();
}

void TcpConnection::enableKeepAlive(bool enable, int interval)
{
    keepAliveEnabled_ = enable;
    keepAliveInterval_ = interval;

    if (enable)
    {
        if (loop_->isInLoopThread())
        {
            setupKeepAliveTimer();
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::setupKeepAliveTimer,
                                      shared_from_this()));
        }
    }
    else if (keepAliveTimerId_.isValid())
    {
        loop_->cancel(keepAliveTimerId_);
        keepAliveTimerId_ = TimerId();
    }
}

void TcpConnection::setupKeepAliveTimer()
{
    if (!keepAliveEnabled_) return;

    keepAliveTimerId_ = loop_->runEvery(
        keepAliveInterval_,
        std::bind(&TcpConnection::onKeepAliveTimeout, shared_from_this()));
}

void TcpConnection::onConnectionTimeout()
{
    LOG_INFO("Connection %s timeout, closing", name_.c_str());
    forceClose();
}

void TcpConnection::onIdleTimeout()
{
    LOG_INFO("Connection %s idle timeout, closing", name_.c_str());
    forceClose();
}

void TcpConnection::onKeepAliveTimeout()
{
    if (!connected())
    {
        if (keepAliveTimerId_.isValid())
        {
            loop_->cancel(keepAliveTimerId_);
        }
        return;
    }

    // 发送心跳包
    if (state_ == kConnected)
    {
        // 发送心跳包
        std::string heartbeatMsg = "HEARTBEAT";
        sendInLoop(heartbeatMsg);
        LOG_DEBUG("Send keepalive to connection %s", name_.c_str());
    }
}
