#include "TcpConnection.h"
#include "Eventloop.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include "Logger.h"
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
      highWaterMark_(64*1024*1024)
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
            // TODO: handle error
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
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
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
