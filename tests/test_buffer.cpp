#include "Buffer.h"
#include <iostream>
#include <string>
#include <cassert>

void testBufferAppendRetrieve()
{
    Buffer buf;
    assert(buf.readableBytes() == 0);
    assert(buf.writableBytes() == 1024);
    assert(buf.prependableBytes() == 8);

    const std::string str1 = "Hello, world!";
    buf.append(str1);
    assert(buf.readableBytes() == str1.size());
    assert(buf.writableBytes() == 1024 - str1.size());
    assert(buf.prependableBytes() == 8);

    assert(std::string(buf.peek(), buf.readableBytes()) == str1);
    buf.retrieve(str1.size());
    assert(buf.readableBytes() == 0);
    assert(buf.writableBytes() == 1024);
    assert(buf.prependableBytes() == 8);

    buf.append(str1);
    assert(buf.readableBytes() == str1.size());
    assert(buf.writableBytes() == 1024 - str1.size());
    assert(buf.prependableBytes() == 8);

    const std::string str2 = buf.retrieveAllAsString();
    assert(str2 == str1);
    assert(buf.readableBytes() == 0);
    assert(buf.writableBytes() == 1024);
    assert(buf.prependableBytes() == 8);
}

void testBufferGrow()
{
    Buffer buf;
    buf.append(std::string(400, 'y'));
    assert(buf.readableBytes() == 400);
    assert(buf.writableBytes() == 1024 - 400);

    buf.append(std::string(1000, 'z'));
    assert(buf.readableBytes() == 1400);
    assert(buf.writableBytes() == 0);

    buf.retrieveAll();
    assert(buf.readableBytes() == 0);
    assert(buf.writableBytes() == 1400);
}

void testBufferInsideGrow()
{
    Buffer buf;
    buf.append(std::string(800, 'x'));
    assert(buf.readableBytes() == 800);
    assert(buf.writableBytes() == 1024 - 800);

    buf.retrieve(500);
    assert(buf.readableBytes() == 300);
    assert(buf.writableBytes() == 1024 - 800);

    buf.append(std::string(600, 'y'));
    assert(buf.readableBytes() == 900);
    assert(buf.writableBytes() == 124);
}

void testBufferShrink()
{
    Buffer buf;
    buf.append(std::string(2000, 'x'));
    buf.retrieve(1500);
    assert(buf.readableBytes() == 500);
    // 此时buffer已经扩容到2000+8=2008字节，writerIndex_为2008，可写字节为2008-2008=0
    assert(buf.writableBytes() == 0);

    buf.shrink(0);
    assert(buf.readableBytes() == 500);
    // shrink后buffer缩小到508字节(8+500)，可写字节为508-508=0
    assert(buf.writableBytes() == 0);
    assert(buf.internalCapacity() == 508);
}

void testBufferPrepend()
{
    Buffer buf;
    const std::string str1 = "Hello";
    const std::string str2 = "World";

    buf.append(str1);
    buf.append(str2);
    assert(buf.readableBytes() == 10);

    buf.retrieve(5);
    assert(buf.readableBytes() == 5);
    assert(std::string(buf.peek(), 5) == str2);

    buf.prepend("Prepended", 9);
    assert(buf.readableBytes() == 14);
    assert(std::string(buf.peek(), 9) == "Prepended");
    assert(std::string(buf.peek() + 9, 5) == str2);
}

void testBufferReadInt()
{
    Buffer buf;
    buf.append("HTTP/1.1 200 OK\r\n");
    assert(buf.readableBytes() == 17);

    const char* crlf = buf.findCRLF();
    assert(crlf != nullptr);
    assert(crlf - buf.peek() == 15);

    std::string line(buf.peek(), crlf);
    assert(line == "HTTP/1.1 200 OK");

    buf.retrieveUntil(crlf + 2);
    assert(buf.readableBytes() == 0);
}

void testBufferFindCRLF()
{
    Buffer buf;
    buf.append("Hello\r\nWorld\r\n");

    const char* crlf1 = buf.findCRLF();
    assert(crlf1 != nullptr);
    assert(crlf1 - buf.peek() == 5);

    const char* crlf2 = buf.findCRLF(crlf1 + 2);
    assert(crlf2 != nullptr);
    assert(crlf2 - buf.peek() == 12);
}

int main()
{
    std::cout << "Running Buffer tests..." << std::endl;

    testBufferAppendRetrieve();
    std::cout << "testBufferAppendRetrieve passed" << std::endl;

    testBufferGrow();
    std::cout << "testBufferGrow passed" << std::endl;

    testBufferInsideGrow();
    std::cout << "testBufferInsideGrow passed" << std::endl;

    testBufferShrink();
    std::cout << "testBufferShrink passed" << std::endl;

    testBufferPrepend();
    std::cout << "testBufferPrepend passed" << std::endl;

    testBufferReadInt();
    std::cout << "testBufferReadInt passed" << std::endl;

    testBufferFindCRLF();
    std::cout << "testBufferFindCRLF passed" << std::endl;

    std::cout << "All Buffer tests passed!" << std::endl;
    return 0;
}
