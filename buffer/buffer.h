#ifndef BUFFER_H

#define BUFFER_H

#include <vector>
#include <atomic>
#include <string>
#include <mutex>
#include <cassert>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/uio.h>




class Buffer
{
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;

    size_t readableBytes() const; // 获取缓冲区可读区域的大小
    size_t writeableBytes() const; // 获取缓冲区可写区域的大小
    size_t prependableBytes() const; // 获取缓冲区预置区域的大小,该区域即是已经读完了的区域

    const char* readPtr() const; // 获取当前读指针位置
    const char* writePtr() const;
    void ensureWriteable(size_t len); // 确保有足够的可读区域
    void hasWritten(size_t len); // 更新writeIndex_

    void retrieve(size_t len); // 更新readIndex_
    void retrieveUntil(const char* end); // 更新readIndex_
    void retrieveAll(); // 更新
    std::string retrieveAllToString(); // 更新

    // 写入字符
    void append(const char* str, size_t len);
    void append(const std::string& str);
    void append(const Buffer& anotherBuff); // 转移另一个缓冲区的可读数据
    void append(const void* data, size_t len);

    ssize_t readFd(int fd, int* errNo); // 从文件描述符里读
    ssize_t writeFd(int fd, int* errNo); // 向文件描述符里写

private:
    char* beginPtr_(); // 返回缓冲区起始指针
    const char* beginPtr_() const;
    void makeSpace_(size_t len); // 确保缓冲区空间充足

    std::vector<char> buffer_;
    std::mutex mtx_; // mutex of buffer_
    std::atomic<size_t> readIndex_;
    std::atomic<size_t> writeIndex_;
};



#endif // ~BUFFER_H