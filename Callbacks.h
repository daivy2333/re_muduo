#pragma once

#include<memory>
#include<functional>
using namespace std;
class Buffer;
class TcpConnection;
class Timestamp;
using TcpConnectionPtr = shared_ptr<TcpConnection>;
using ConnectionCallback = function<void(const TcpConnectionPtr&)>;
using CloseCallback = function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = function<void(const TcpConnectionPtr&, size_t)>; 

using MessageCallback = function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;
using TimerCallback = function<void()>;