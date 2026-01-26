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

    std::cout << "=== All Socket Tests Passed ===" << std::endl;
    return 0;
}
