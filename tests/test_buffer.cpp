#include "Buffer.h"
#include <iostream>
#include <string>
#include <cassert>
#include <cstring>

void testBufferEmpty()
{
    Buffer buf;
    assert(buf.empty());
    
    buf.append("Hello");
    assert(!buf.empty());
    
    buf.retrieveAll();
    assert(buf.empty());
}

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

void testBufferRetrieveAsString()
{
    Buffer buf;
    const std::string str = "Hello, World!";
    buf.append(str);
    
    // 测试retrieveAsString(size_t len)
    std::string part1 = buf.retrieveAsString(5);
    assert(part1 == "Hello");
    assert(buf.readableBytes() == str.size() - 5);
    
    std::string part2 = buf.retrieveAsString(2);
    assert(part2 == ", ");
    assert(buf.readableBytes() == str.size() - 7);
    
    std::string part3 = buf.retrieveAsString(buf.readableBytes());
    assert(part3 == "World!");
    assert(buf.empty());
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

void testBufferAppendVoidPtr()
{
    Buffer buf;
    const char* data = "Test data";
    buf.append(static_cast<const void*>(data), strlen(data));
    
    assert(buf.readableBytes() == strlen(data));
    assert(std::string(buf.peek(), buf.readableBytes()) == data);
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
    
    // 测试带reserve参数的shrink
    buf.shrink(100);
    assert(buf.readableBytes() == 500);
    // shrink后buffer大小为8+500+100=608字节
    assert(buf.internalCapacity() == 608);
    assert(buf.writableBytes() == 100);
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
    
    // 测试prepend超过kCheapPrepend大小的数据
    Buffer buf2;
    buf2.append("Original data", 13);  // 明确指定13字节
    buf2.retrieve(5);  // 消耗5字节
    
    // 使用strlen获取真实长度
    const char* prepend_data = "This is a long prepended string";
    size_t prepend_len = strlen(prepend_data);  // 得到31

    printf("Before prepend: readableBytes=%zu, prependableBytes=%zu, prepend_len=%zu\n",
           buf2.readableBytes(), buf2.prependableBytes(), prepend_len);

    buf2.prepend(prepend_data, prepend_len);
    printf("After prepend: readableBytes=%zu\n", buf2.readableBytes());
    // 现在应该是31 + 8 = 39字节
    assert(buf2.readableBytes() == prepend_len + 8);

    // 验证前31字节
    std::string actual(buf2.peek(), prepend_len);
    printf("Expected: %s\n", prepend_data);
    printf("Actual:   %s\n", actual.c_str());
    assert(actual == prepend_data);  // 完整的31字符字符串

    // 验证后面的8字节
    assert(std::string(buf2.peek() + prepend_len, 8) == "nal data");
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
    
    // 测试找不到CRLF的情况
    Buffer buf2;
    buf2.append("No CRLF here");
    assert(buf2.findCRLF() == nullptr);
    
    // 测试空buffer的findCRLF
    Buffer buf3;
    assert(buf3.findCRLF() == nullptr);
}

int main()
{
    std::cout << "Running Buffer tests..." << std::endl;

    testBufferEmpty();
    std::cout << "testBufferEmpty passed" << std::endl;

    testBufferAppendRetrieve();
    std::cout << "testBufferAppendRetrieve passed" << std::endl;

    testBufferRetrieveAsString();
    std::cout << "testBufferRetrieveAsString passed" << std::endl;

    testBufferAppendVoidPtr();
    std::cout << "testBufferAppendVoidPtr passed" << std::endl;

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
