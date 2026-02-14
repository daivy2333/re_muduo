#include "Socket.h"
#include "InetAddress.h"
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>

void test_socket_bind() {
    std::cout << "=== Test Socket Bind ===" << std::endl;

    // 创建一个socket
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置地址重用
    sock.setReuseAddr(true);

    // 绑定地址
    InetAddress addr(18080, "127.0.0.1");
    sock.bindAddress(addr);

    std::cout << "Socket bind test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_listen() {
    std::cout << "=== Test Socket Listen ===" << std::endl;

    // 创建一个socket
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置地址重用
    sock.setReuseAddr(true);

    // 绑定地址
    InetAddress addr(18081, "127.0.0.1");
    sock.bindAddress(addr);

    // 开始监听
    sock.listen();

    std::cout << "Socket listen test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_accept() {
    std::cout << "=== Test Socket Accept ===" << std::endl;

    // 创建服务器socket
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(listenfd >= 0);

    Socket listenSock(listenfd);

    // 设置地址重用
    listenSock.setReuseAddr(true);

    // 绑定地址
    InetAddress listenAddr(18082, "127.0.0.1");
    listenSock.bindAddress(listenAddr);
    listenSock.listen();

    // 创建客户端socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);

    // 连接服务器
    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(18082);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // 由于服务器socket是非阻塞的，需要等待一下
    sleep(1);

    connect(clientfd, (sockaddr*)&serverAddr, sizeof serverAddr);

    // 等待连接建立
    sleep(1);

    // 接受连接
    InetAddress peerAddr(0);
    int connfd = listenSock.accept(&peerAddr);

    if (connfd >= 0) {
        std::cout << "Accepted connection from: " << peerAddr.toIpPort() << std::endl;
        close(connfd);
    }

    close(clientfd);
    std::cout << "Socket accept test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_tcp_nodelay() {
    std::cout << "=== Test Socket TCP_NODELAY ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置TCP_NODELAY
    sock.setTcpNoDelay(true);

    // 验证设置是否生效
    int optval;
    socklen_t optlen = sizeof optval;
    getsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
    assert(optval == 1);

    std::cout << "Socket TCP_NODELAY test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_reuse_addr() {
    std::cout << "=== Test Socket SO_REUSEADDR ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置SO_REUSEADDR
    sock.setReuseAddr(true);

    // 验证设置是否生效
    int optval;
    socklen_t optlen = sizeof optval;
    getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
    assert(optval == 1);

    std::cout << "Socket SO_REUSEADDR test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_reuse_port() {
    std::cout << "=== Test Socket SO_REUSEPORT ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置SO_REUSEPORT
    sock.setReusePort(true);

    // 验证设置是否生效
    int optval;
    socklen_t optlen = sizeof optval;
    getsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, &optlen);
    assert(optval == 1);

    std::cout << "Socket SO_REUSEPORT test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_keep_alive() {
    std::cout << "=== Test Socket SO_KEEPALIVE ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置SO_KEEPALIVE
    sock.setKeepAlive(true);

    // 验证设置是否生效
    int optval;
    socklen_t optlen = sizeof optval;
    getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
    assert(optval == 1);

    std::cout << "Socket SO_KEEPALIVE test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_shutdown_write() {
    std::cout << "=== Test Socket Shutdown Write ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置地址重用
    sock.setReuseAddr(true);

    // 绑定地址
    InetAddress addr(18083, "127.0.0.1");
    sock.bindAddress(addr);
    sock.listen();

    // 创建客户端socket并连接
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(18083);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    sleep(1);
    connect(clientfd, (sockaddr*)&serverAddr, sizeof serverAddr);

    // 等待连接建立
    sleep(1);

    InetAddress peerAddr(0);
    int connfd = sock.accept(&peerAddr);

    if (connfd >= 0) {
        // 关闭写端
        sock.shutdownWrite();
        close(connfd);
    }

    close(clientfd);
    std::cout << "Socket shutdown write test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_get_local_addr() {
    std::cout << "=== Test Socket GetLocalAddr ===" << std::endl;

    // 创建一个socket
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置地址重用
    sock.setReuseAddr(true);

    // 绑定地址
    InetAddress bindAddr(18090, "127.0.0.1");
    sock.bindAddress(bindAddr);

    // 获取本地地址
    InetAddress localAddr = Socket::getLocalAddr(sockfd);

    // 验证地址是否正确
    assert(localAddr.toPort() == 18090);
    assert(localAddr.toIp() == "127.0.0.1");

    std::cout << "Socket getLocalAddr test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_noncopyable() {
    std::cout << "=== Test Socket Non-copyable ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 测试Socket不可复制
    // Socket sock2(sock);  // 这行应该编译失败
    // Socket sock3 = sock;  // 这行应该编译失败
    // Socket sock4(std::move(sock));  // Socket也不支持移动

    // 验证Socket对象的基本功能
    assert(sock.fd() == sockfd);
    sock.setReuseAddr(true);
    sock.setReusePort(true);
    sock.setTcpNoDelay(true);
    sock.setKeepAlive(true);

    std::cout << "Socket non-copyable test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_destructor() {
    std::cout << "=== Test Socket Destructor ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    {
        Socket sock(sockfd);
        assert(sock.fd() == sockfd);
    } // sock离开作用域，析构函数应该关闭sockfd

    // 验证sockfd是否被关闭
    int ret = fcntl(sockfd, F_GETFD);
    assert(ret < 0);
    assert(errno == EBADF);

    std::cout << "Socket destructor test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_accept_flags() {
    std::cout << "=== Test Socket Accept Flags ===" << std::endl;

    // 创建服务器socket
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(listenfd >= 0);

    Socket listenSock(listenfd);

    // 设置地址重用
    listenSock.setReuseAddr(true);

    // 绑定地址
    InetAddress listenAddr(18091, "127.0.0.1");
    listenSock.bindAddress(listenAddr);
    listenSock.listen();

    // 创建客户端socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);

    // 连接服务器
    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(18091);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    sleep(1);
    connect(clientfd, (sockaddr*)&serverAddr, sizeof serverAddr);

    // 等待连接建立
    sleep(1);

    // 接受连接
    InetAddress peerAddr(0);
    int connfd = listenSock.accept(&peerAddr);

    if (connfd >= 0) {
        // 验证connfd是否设置了SOCK_NONBLOCK标志
        int flags = fcntl(connfd, F_GETFL, 0);
        assert(flags >= 0);
        assert(flags & O_NONBLOCK);

        // 验证connfd是否设置了FD_CLOEXEC标志
        int flags2 = fcntl(connfd, F_GETFD, 0);
        assert(flags2 >= 0);
        assert(flags2 & FD_CLOEXEC);

        // 验证peerAddr是否正确
        std::cout << "Accepted connection from: " << peerAddr.toIpPort() << std::endl;
        assert(peerAddr.toPort() > 0);
        assert(!peerAddr.toIp().empty());

        close(connfd);
    }

    close(clientfd);
    std::cout << "Socket accept flags test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_tcp_nodelay_off() {
    std::cout << "=== Test Socket TCP_NODELAY Off ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 先设置为true
    sock.setTcpNoDelay(true);

    // 验证设置是否生效
    int optval;
    socklen_t optlen = sizeof optval;
    getsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
    assert(optval == 1);

    // 再设置为false
    sock.setTcpNoDelay(false);

    // 验证设置是否生效
    getsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
    assert(optval == 0);

    std::cout << "Socket TCP_NODELAY off test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_reuse_addr_off() {
    std::cout << "=== Test Socket SO_REUSEADDR Off ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 先设置为true
    sock.setReuseAddr(true);

    // 验证设置是否生效
    int optval;
    socklen_t optlen = sizeof optval;
    getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
    assert(optval == 1);

    // 再设置为false
    sock.setReuseAddr(false);

    // 验证设置是否生效
    getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
    assert(optval == 0);

    std::cout << "Socket SO_REUSEADDR off test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_reuse_port_off() {
    std::cout << "=== Test Socket SO_REUSEPORT Off ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 先设置为true
    sock.setReusePort(true);

    // 验证设置是否生效
    int optval;
    socklen_t optlen = sizeof optval;
    getsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, &optlen);
    assert(optval == 1);

    // 再设置为false
    sock.setReusePort(false);

    // 验证设置是否生效
    getsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, &optlen);
    assert(optval == 0);

    std::cout << "Socket SO_REUSEPORT off test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_keep_alive_off() {
    std::cout << "=== Test Socket SO_KEEPALIVE Off ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 先设置为true
    sock.setKeepAlive(true);

    // 验证设置是否生效
    int optval;
    socklen_t optlen = sizeof optval;
    getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
    assert(optval == 1);

    // 再设置为false
    sock.setKeepAlive(false);

    // 验证设置是否生效
    getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
    assert(optval == 0);

    std::cout << "Socket SO_KEEPALIVE off test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_shutdown_write_effect() {
    std::cout << "=== Test Socket Shutdown Write Effect ===" << std::endl;

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd >= 0);

    Socket sock(sockfd);

    // 设置地址重用
    sock.setReuseAddr(true);

    // 绑定地址
    InetAddress addr(18092, "127.0.0.1");
    sock.bindAddress(addr);
    sock.listen();

    // 创建客户端socket并连接
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(18092);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    sleep(1);
    connect(clientfd, (sockaddr*)&serverAddr, sizeof serverAddr);

    // 等待连接建立
    sleep(1);

    InetAddress peerAddr(0);
    int connfd = sock.accept(&peerAddr);

    if (connfd >= 0) {
        // 创建Socket对象包装connfd
        Socket connSock(connfd);
        
        // 关闭服务器端的写端
        connSock.shutdownWrite();

        // 等待一下，确保对端收到关闭通知
        sleep(1);

        // 从客户端尝试读取，应该读到EOF（返回0）
        char buf[1] = {0};
        ssize_t ret = read(clientfd, buf, 1);
        // 服务器端关闭写端后，客户端read应该返回0（EOF）
        assert(ret == 0);

        // connSock析构时会自动关闭connfd
    }

    close(clientfd);
    std::cout << "Socket shutdown write effect test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_multiple_bind() {
    std::cout << "=== Test Socket Multiple Bind ===" << std::endl;

    // 创建第一个socket
    int sockfd1 = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd1 >= 0);

    Socket sock1(sockfd1);
    sock1.setReuseAddr(true);
    sock1.setReusePort(true);

    InetAddress addr1(18093, "127.0.0.1");
    sock1.bindAddress(addr1);

    // 创建第二个socket，尝试绑定到相同的地址
    int sockfd2 = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(sockfd2 >= 0);

    Socket sock2(sockfd2);
    sock2.setReuseAddr(true);
    sock2.setReusePort(true);

    InetAddress addr2(18093, "127.0.0.1");
    sock2.bindAddress(addr2);  // 应该成功，因为设置了SO_REUSEPORT

    std::cout << "Socket multiple bind test passed" << std::endl;
    std::cout << std::endl;
}

void test_socket_get_local_addr_after_connect() {
    std::cout << "=== Test Socket GetLocalAddr After Connect ===" << std::endl;

    // 创建服务器socket
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    assert(listenfd >= 0);

    Socket listenSock(listenfd);
    listenSock.setReuseAddr(true);

    InetAddress listenAddr(18094, "127.0.0.1");
    listenSock.bindAddress(listenAddr);
    listenSock.listen();

    // 创建客户端socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientfd >= 0);

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof serverAddr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(18094);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    sleep(1);
    connect(clientfd, (sockaddr*)&serverAddr, sizeof serverAddr);

    // 等待连接建立
    sleep(1);

    InetAddress peerAddr(0);
    int connfd = listenSock.accept(&peerAddr);

    if (connfd >= 0) {
        // 获取服务器端的本地地址
        InetAddress serverLocalAddr = Socket::getLocalAddr(connfd);
        assert(serverLocalAddr.toPort() == 18094);
        assert(serverLocalAddr.toIp() == "127.0.0.1");

        // 获取客户端的本地地址
        InetAddress clientLocalAddr = Socket::getLocalAddr(clientfd);
        assert(clientLocalAddr.toPort() > 0);
        assert(!clientLocalAddr.toIp().empty());

        std::cout << "Server local addr: " << serverLocalAddr.toIpPort() << std::endl;
        std::cout << "Client local addr: " << clientLocalAddr.toIpPort() << std::endl;

        close(connfd);
    }

    close(clientfd);
    std::cout << "Socket getLocalAddr after connect test passed" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Socket Tests ===" << std::endl;
    std::cout << std::endl;

    test_socket_bind();
    test_socket_listen();
    test_socket_accept();
    test_socket_tcp_nodelay();
    test_socket_reuse_addr();
    test_socket_reuse_port();
    test_socket_keep_alive();
    test_socket_shutdown_write();
    test_socket_get_local_addr();
    test_socket_noncopyable();
    test_socket_destructor();
    test_socket_accept_flags();
    test_socket_tcp_nodelay_off();
    test_socket_reuse_addr_off();
    test_socket_reuse_port_off();
    test_socket_keep_alive_off();
    test_socket_shutdown_write_effect();
    test_socket_multiple_bind();
    test_socket_get_local_addr_after_connect();

    std::cout << "=== All Socket Tests Passed ===" << std::endl;
    return 0;
}
