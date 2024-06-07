#include "buffer.h"


Buffer::Buffer(int initBufferSize) :buffer_(initBufferSize), readIndex_(0), writeIndex_(0) {};

size_t Buffer::readableBytes() const
{
    return writeIndex_ - readIndex_;
}; // 获取缓冲区可读区域的大小


size_t Buffer::writeableBytes() const
{
    return buffer_.size() - writeIndex_;
}; // 获取缓冲区可写区域的大小


size_t Buffer::prependableBytes() const
{
    return readIndex_;
}; // 获取缓冲区预置区域的大小



const char* Buffer::readPtr() const
{
    return beginPtr_() + readIndex_;
}; // 获取当前读指针位置

const char* Buffer::writePtr() const
{
    return beginPtr_() + writeIndex_;
};

char* Buffer::writePtr()
{
    return beginPtr_() + writeIndex_;
};

void Buffer::ensureWriteable(size_t len)
{
    if (writeableBytes() < len)
        makeSpace_(len);
}; // 确保有足够的可读区域


void Buffer::hasWritten(size_t len)
{
    writeIndex_ += len;
}; // 更新writeIndex_



void Buffer::retrieve(size_t len)
{
    assert(len < readableBytes());
    readIndex_ += len;
}; // 更新readIndex_


void Buffer::retrieveUntil(const char* end)
{
    assert(readPtr() < end);
    retrieve(end - readPtr());
}; // 更新readIndex_


void Buffer::retrieveAll()
{
    memset(buffer_.data(), 0, buffer_.size());
    writeIndex_ = 0;
    readIndex_ = 0;
}; // 更新


std::string Buffer::retrieveAllToString()
{
    std::string str(buffer_.begin() + readIndex_, buffer_.begin() + writeIndex_);
    retrieveAll();
    return str;
}; // 更新



// 写入字符
void Buffer::append(const char* str, size_t len)
{
    assert(str);
    ensureWriteable(len);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        std::copy(str, str + len, buffer_.data() + writeIndex_);
    }
    hasWritten(len);
};


void Buffer::append(const std::string& str)
{
    append(str.data(), str.size());
};


void Buffer::append(const Buffer& anotherBuff)
{
    append(anotherBuff.readPtr(), anotherBuff.readableBytes());
};


void Buffer::append(const void* data, size_t len)
{
    assert(data);
    // append((const char*)(data), len);
    append(static_cast<const char*>(data), len);
};



ssize_t Buffer::readFd(int fd, int* errNo)
{
    assert(fd >= 0);
    /* STDIN_FILENO (0) - 标准输入
    STDOUT_FILENO (1) - 标准输出
    STDERR_FILENO (2) - 标准错误输出 */
    char buffer_2[65535];
    struct iovec iov[2];
    const size_t writeable = writeableBytes();

    iov[0].iov_base = beginPtr_() + writeIndex_;
    iov[0].iov_len = writeable;
    iov[1].iov_base = buffer_2;
    iov[1].iov_len = sizeof(buffer_2);

    const ssize_t len = readv(fd, iov, 2);

    if (len < 0)
        *errNo = errno;
    else if (static_cast<size_t> (len) <= writeable)
    {
        hasWritten(len);
    }
    else {
        writeIndex_ = buffer_.size();
        append(buffer_2, len - writeable);
    }
    return len;
}; // 从文件描述符里读


ssize_t Buffer::writeFd(int fd, int* errNo)
{
    size_t readable = readableBytes();
    ssize_t len = write(fd, readPtr(), readable);
    if (len < 0)
        *errNo = errno;
    else readIndex_ += len;
    return len;
}; // 向文件描述符里写

char* Buffer::beginPtr_()
{
    return buffer_.data();
}; // 返回缓冲区起始指针

const char* Buffer::beginPtr_() const
{
    return buffer_.data();
};

void Buffer::makeSpace_(size_t len)
{
    std::lock_guard<std::mutex> lock(mtx_);

    char* begin = beginPtr_();
    if (len > writeableBytes() + prependableBytes())
        buffer_.resize(len + writeIndex_ + 1);
    else {
        size_t readable = readableBytes();
        std::copy(begin + readIndex_, begin + writeIndex_, begin);
        readIndex_ = 0;
        writeIndex_ = readable;
    }
}; // 确保缓冲区空间充足