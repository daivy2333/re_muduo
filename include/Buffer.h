#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
    }

    size_t readableBytes() const
    { return writerIndex_ - readerIndex_; }

    size_t writableBytes() const
    { return buffer_.size() - writerIndex_; }

    size_t prependableBytes() const
    { return readerIndex_; }

    bool empty() const
    { return readableBytes() == 0; }

    const char* peek() const
    { return begin() + readerIndex_; }

    const char* findCRLF() const
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end)
    {
        retrieve(end - peek());
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAllAsString()
    {
        std::string result(peek(), readableBytes());
        retrieveAll();
        return result;
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    void append(const std::string& str)
    {
        append(str.data(), str.size());
    }

    void append(const void* data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    void prepend(const void* data, size_t len)
    {
        if (len < prependableBytes())
        {
            // 空间足够，直接使用预留空间
            readerIndex_ -= len;
            std::copy(static_cast<const char*>(data), 
                    static_cast<const char*>(data) + len, 
                    begin() + readerIndex_);
        }
        else
        {
            // 空间不足，需要移动数据
            size_t readable = readableBytes();
            
            // 计算需要的总空间
            size_t needed = kCheapPrepend + len + readable;
            if (buffer_.size() < needed)
            {
                // 需要扩容
                buffer_.resize(needed);
            }
            
            // 1. 首先，将现有数据移动到正确位置
            // 将"nal data"从[8, 16)移动到[kCheapPrepend + len, kCheapPrepend + len + readable)
            std::copy(begin() + readerIndex_, 
                    begin() + writerIndex_, 
                    begin() + kCheapPrepend + len);
            
            // 2. 然后，将prepend的数据复制到kCheapPrepend位置
            std::copy(static_cast<const char*>(data), 
                    static_cast<const char*>(data) + len, 
                    begin() + kCheapPrepend);
            
            // 3. 更新索引
            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend + len + readable;
        }
    }
    void shrink(size_t reserve)
    {
        size_t readable = readableBytes();
        std::vector<char> buf(kCheapPrepend + readable + reserve);
        std::copy(begin() + readerIndex_, begin() + writerIndex_, buf.begin() + kCheapPrepend);
        buffer_.swap(buf);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend + readable;
    }

    size_t internalCapacity() const
    { return buffer_.capacity(); }

    ssize_t readFd(int fd, int* savedErrno);
    ssize_t writeFd(int fd, int* savedErrno);

    char* beginWrite()
    { return begin() + writerIndex_; }

    const char* beginWrite() const
    { return begin() + writerIndex_; }

private:
    char* begin()
    { return &*buffer_.begin(); }

    const char* begin() const
    { return &*buffer_.begin(); }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    static const char kCRLF[];

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};
