# 性能对照测试

本目录包含了对 re_muduo、原版 muduo 和 nginx 的性能对照测试代码。

## 目录结构

```
compare/
├── muduo/
│   └── latency_test.cpp    # 原版 muduo 的延迟测试
├── nginx/
│   ├── latency_test.cpp    # nginx 的延迟测试
│   └── echo_server.conf    # nginx echo 服务器配置
├── CMakeLists.txt          # 构建配置
└── README.md              # 本文件
```

## 测试说明

### 1. 原版 muduo 测试 (muduo/latency_test.cpp)

测试官方 muduo 库的性能表现，使用与 re_muduo 相同的测试参数：
- 并发客户端数：1000
- 测试持续时间：60秒
- 数据包大小：64, 256, 1024, 4096, 16384, 65536 字节
- 服务器线程数：8

**注意**：此测试使用官方 muduo 库，需要先安装官方 muduo。安装方法见下方。

### 2. Nginx 测试 (nginx/latency_test.cpp)

测试 nginx 的性能表现，使用相同的测试参数。

## 使用方法

### 配置官方 muduo 库路径

在构建之前，需要确保官方 muduo 库已安装。默认情况下，CMake 会在 `~/muduo` 目录下查找 muduo 库。

如果 muduo 安装在其他位置，可以在 CMake 配置时指定路径：

```bash
cd compare
mkdir build && cd build
cmake -DMUDUO_ROOT=/path/to/muduo ..
make
```

### 构建测试程序

在 compare 目录下单独构建：

```bash
cd compare
mkdir build && cd build
cmake ..
make
```

如果官方 muduo 库未找到，CMake 会显示错误信息，请先安装官方 muduo 库。

### 运行测试

#### 测试 re_muduo

```bash
# 在 re_muduo/build 目录下
./latency_test [server_ip] [port]
```
默认参数：127.0.0.1 8888

#### 测试官方 muduo

```bash
# 在 compare/build 目录下
./muduo_latency_test [server_ip] [port]
```
默认参数：127.0.0.1 8888

#### 测试 nginx

1. 启动 nginx echo 服务器：
```bash
# 使用提供的配置文件启动 nginx
sudo nginx -c /path/to/compare/nginx/echo_server.conf

# 或者使用 nginx 默认配置，需要确保 nginx 已安装 echo 模块
```

2. 运行测试：
```bash
# 在 compare/build 目录下
./nginx_latency_test [server_ip] [port]
```
默认参数：127.0.0.1 8888

**注意**：所有测试程序都会在项目根目录生成对应的测试报告文件。

## 测试结果

测试结果会保存在项目根目录的以下文件中：
- `latency_report.txt` - re_muduo 的测试报告
- `muduo_latency_report.txt` - 官方 muduo 的测试报告
- `nginx_latency_report.txt` - nginx 的测试报告

每个报告文件包含以下信息：
- 测试时间
- 并发客户端数
- 测试持续时间
- 不同数据包大小下的延迟指标（平均延迟、P50/P95/P99 延迟）

## 对比分析

测试完成后，可以对比以下指标：
- 平均延迟
- P50/P95/P99 延迟
- 吞吐量
- QPS

## 注意事项

1. **环境一致性**：确保测试环境一致（CPU、内存、网络等）
2. **系统负载**：测试时关闭其他不必要的程序，避免干扰
3. **多次测试**：多次运行测试取平均值，以获得更稳定的结果
4. **nginx 模块**：对于 nginx，需要安装 echo 模块（nginx-echo-module）
5. **端口占用**：确保测试端口（默认 8888）未被其他程序占用
6. **日志目录**：确保有权限在项目根目录创建 log 目录和报告文件

## 官方 muduo 库安装

如果系统中没有安装官方 muduo 库，可以按照以下步骤安装：

```bash
# 克隆官方 muduo 仓库
git clone https://github.com/chenshuo/muduo.git
cd muduo

# 构建
./build.sh

# 安装（可选，如果需要安装到系统目录）
sudo make install
```

构建完成后，muduo 库文件会在 `build/release-cpp11/lib/` 目录下，头文件在 `muduo/` 目录下。

## Nginx Echo 模块安装

如果系统中没有安装 nginx echo 模块，可以按照以下步骤安装：

```bash
# 下载 nginx echo 模块
git clone https://github.com/openresty/echo-nginx-module.git

# 下载 nginx 源码
wget http://nginx.org/download/nginx-1.21.6.tar.gz
tar -xzf nginx-1.21.6.tar.gz
cd nginx-1.21.6

# 编译安装
./configure --add-module=/path/to/echo-nginx-module
make
sudo make install
```

## 故障排除

### 问题：官方 muduo 库未找到

解决：
1. 确认已安装官方 muduo 库
2. 检查 muduo 库路径是否正确
3. 使用 `-DMUDUO_ROOT=/path/to/muduo` 指定正确的路径

### 问题：nginx 测试连接失败

解决：
1. 确保 nginx 已正确启动
2. 检查 nginx 配置文件路径是否正确
3. 确认 nginx 监听端口与测试端口一致
4. 检查防火墙设置

### 问题：编译错误

解决：
1. 确保已安装必要的依赖：
   - pthread
   - 官方 muduo 库（对于 muduo 测试）
   - nginx 和 echo 模块（对于 nginx 测试）
2. 检查 CMake 版本是否满足要求（>= 3.10）
3. 清理构建目录后重新构建：`rm -rf build && mkdir build && cd build && cmake ..`

### 问题：测试报告未生成

解决：
1. 检查是否有权限在项目根目录创建文件
2. 确保磁盘空间充足
3. 查看日志输出，确认测试是否正常运行
