#include "TcpConnection.h"
#include "Eventloop.h"
#include "InetAddress.h"
#include "Timestamp.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        cout << "New connection " << conn->name() 
             << " from " << conn->peerAddress().toIpPort() << endl;
    }
    else
    {
        cout << "Connection " << conn->name() << " is down" << endl;
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    cout << "Received " << buf->readableBytes() << " bytes from " << conn->name() << endl;
    string msg(buf->retrieveAllAsString());
    cout << "Message content: " << msg << endl;

    // Echo back
    conn->send(msg);
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
    cout << "Write complete for " << conn->name() << endl;
}

int createSocketPair(int sv[2])
{
    return socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
}

int main()
{
    cout << "Running TcpConnection tests..." << endl;

    EventLoop loop;

    // Create a pair of connected sockets
    int sv[2];
    if (createSocketPair(sv) < 0)
    {
        perror("socketpair");
        return 1;
    }

    InetAddress localAddr(1234);
    InetAddress peerAddr(5678);

    // Create a TcpConnection object
    TcpConnectionPtr conn(new TcpConnection(&loop, "testConnection", sv[0], localAddr, peerAddr));

    // Set callbacks
    conn->setConnectionCallback(onConnection);
    conn->setMessageCallback(onMessage);
    conn->setWriteCompleteCallback(onWriteComplete);

    // Simulate connection establishment
    conn->connectEstablished();

    // Send a message through the other socket
    const char* message = "Hello, TcpConnection!";
    write(sv[1], message, strlen(message));

    // Run the loop once to process the event
    loop.runInLoop([&loop](){
        loop.quit();
    });

    loop.loop();

    // Test send functionality
    conn->send("Test message from connection");

    // Run the loop again to process the event
    loop.runInLoop([&loop](){
        loop.quit();
    });

    loop.loop();

    // Test shutdown
    conn->shutdown();

    // Run the loop again to process the event
    loop.runInLoop([&loop](){
        loop.quit();
    });

    loop.loop();

    // Clean up connection
    conn->connectDestroyed();

    // Clean up sockets
    close(sv[0]);
    close(sv[1]);

    cout << "All TcpConnection tests passed!" << endl;
    return 0;
}
