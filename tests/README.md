
# re_muduo 测试指南

本目录包含 re_muduo 项目的测试代码，用于验证各个组件的功能。

## 测试文件

- `test_timestamp.cpp` - 测试 Timestamp 类
- `test_inetaddress.cpp` - 测试 InetAddress 类
- `test_logger.cpp` - 测试 Logger 类
- `test_currentthread.cpp` - 测试 CurrentThread 命名空间
- `test_eventloop.cpp` - 测试 EventLoop 类
- `test_channel.cpp` - 测试 Channel 类
- `test_poller.cpp` - 测试 Poller 和 EpollPoller 类
- `test_tcpserver.cpp` - 测试 TcpServer 类

## 构建和运行测试

### 构建项目

```bash
# 在项目根目录下创建 build 目录
mkdir build
cd build

# 配置 CMake
cmake ..

# 构建项目
make
```

### 运行所有测试

```bash
# 在 build 目录下运行所有测试
ctest
```

### 运行单个测试

```bash
# 运行特定的测试
./bin/test_timestamp
./bin/test_inetaddress
./bin/test_logger
./bin/test_currentthread
./bin/test_eventloop
./bin/test_channel
./bin/test_poller
./bin/test_tcpserver
```

## 测试覆盖

测试覆盖了以下功能：

1. **Timestamp**
   - 默认构造函数
   - 带微秒参数的构造函数
   - 当前时间获取
   - 时间戳转字符串

2. **InetAddress**
   - 仅端口号构造
   - IP地址和端口号构造
   - 获取 IP 地址
   - 获取 IP 地址和端口
   - 获取端口号
   - 获取 sockaddr_in 结构

3. **Logger**
   - 单例模式
   - 不同级别的日志输出
   - 设置日志级别

4. **CurrentThread**
   - 获取当前线程 ID
   - 多线程环境下的线程 ID 获取

5. **EventLoop**
   - 基本功能
   - Channel 管理
   - 跨线程任务调度

6. **Channel**
   - 创建和初始化
   - 事件管理（读/写事件）
   - 回调函数设置

7. **Poller/EpollPoller**
   - 创建和初始化
   - Channel 操作（添加/更新/移除）
   - 事件轮询

8. **TcpServer**
   - 服务器创建和启动
   - 多线程服务器
   - 连接管理
   - 线程初始化回调

## 注意事项

- 某些测试需要多线程支持
- EventLoop 和 Channel 测试需要 Linux 的 eventfd 支持
- EpollPoller 测试需要 Linux 的 epoll 支持
- 运行测试前请确保系统环境满足要求
