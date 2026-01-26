
#include "InetAddress.h"
#include <iostream>
#include <cassert>

void test_inetaddress_with_port() {
    uint16_t port = 8080;
    InetAddress addr(port);
    std::string ip = addr.toIp();
    std::string ipPort = addr.toIpPort();
    uint16_t portOut = addr.toPort();

    std::cout << "IP: " << ip << std::endl;
    std::cout << "IP:Port: " << ipPort << std::endl;
    std::cout << "Port: " << portOut << std::endl;

    assert(portOut == port);
    assert(ipPort.find(":") != std::string::npos);
    std::cout << "InetAddress with port test passed" << std::endl;
}

void test_inetaddress_with_ip_and_port() {
    std::string ip = "192.168.1.1";
    uint16_t port = 9000;
    InetAddress addr(port, ip);

    std::string ipOut = addr.toIp();
    std::string ipPort = addr.toIpPort();
    uint16_t portOut = addr.toPort();

    std::cout << "IP: " << ipOut << std::endl;
    std::cout << "IP:Port: " << ipPort << std::endl;
    std::cout << "Port: " << portOut << std::endl;

    assert(ipOut == ip);
    assert(portOut == port);
    assert(ipPort == ip + ":" + std::to_string(port));
    std::cout << "InetAddress with IP and port test passed" << std::endl;
}

void test_inetaddress_get_sockaddr() {
    uint16_t port = 8080;
    InetAddress addr(port);

    const sockaddr_in* sockAddr = addr.getSockAddr();
    assert(sockAddr != nullptr);
    std::cout << "InetAddress getSockAddr test passed" << std::endl;
}

int main() {
    std::cout << "=== InetAddress Tests ===" << std::endl;
    test_inetaddress_with_port();
    test_inetaddress_with_ip_and_port();
    test_inetaddress_get_sockaddr();
    std::cout << "=== All InetAddress Tests Passed ===" << std::endl;
    return 0;
}
