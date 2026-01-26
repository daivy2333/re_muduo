#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <strings.h>

/**
 * @brief InetAddress类，用于封装IPv4的socket地址
 * 
 * 提供了IP地址和端口的封装，方便进行网络编程
 */
class InetAddress {
public:
    /**
     * @brief 构造函数，通过端口号和IP地址创建InetAddress
     * @param port 端口号
     * @param ip IP地址，默认为"127.0.0.1"
     */
    explicit InetAddress(uint16_t port, std::string ip = "127.0.0.1");

    /**
     * @brief 构造函数，通过sockaddr_in结构体创建InetAddress
     * @param addr sockaddr_in结构体
     */
    explicit InetAddress(const sockaddr_in& addr)
        : addr(addr) {}

    /**
     * @brief 获取IP地址字符串
     * @return IP地址字符串
     */
    std::string toIp() const;

    /**
     * @brief 获取IP地址和端口号字符串
     * @return IP地址:端口号格式的字符串
     */
    std::string toIpPort() const;

    /**
     * @brief 获取端口号
     * @return 端口号（主机字节序）
     */
    uint16_t toPort() const;

    /**
     * @brief 获取sockaddr_in结构体指针
     * @return sockaddr_in结构体指针
     */
    const sockaddr_in* getSockAddr() const { return &addr; }

    /**
     * @brief 设置sockaddr_in结构体
     * @param addr sockaddr_in结构体
     */
    void setSockAddr(const sockaddr_in& addr) { this->addr = addr; }

private:
    sockaddr_in addr;  ///< sockaddr_in结构体
};