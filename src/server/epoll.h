#ifndef EPOLL_H

#define EPOLL_H

#include <vector>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

class Epoller
{
private:
    int epollFd_; // epoll 实例的文件描述符

    std::vector<struct epoll_event> events_; // 存储从 epoll 等待（epoll_wait）调用返回的事件。
public:
    explicit Epoller(int maxEvent = 1024); // 创建 epoll 实例并初始化事件向量
    ~Epoller(); // 关闭 epoll 文件描述符，释放资源

    bool addFd(int fd, uint32_t events); // 将文件描述符 fd 添加到 epoll 实例中，并指定要监听的事件类型
    bool delFd(int fd); // 从 epoll 实例中删除文件描述符 fd
    bool modFd(int fd, uint32_t events); // 修改已经添加到 epoll 实例中的文件描述符 fd 的监听事件

    int getEventFd(size_t i) const; // 返回第 i 个事件对应的文件描述符
    uint32_t getEvents(size_t i) const; // 返回第 i 个事件对应的事件类型

    int wait(int timeoutMs = -1); // 等待事件的发生。timeoutMs 指定超时时间，默认是无限等待。返回值是发生事件的数量
};

Epoller::Epoller(int maxEvent) :epollFd_(epoll_create1(0)), events_(maxEvent)
{
    assert(epollFd_ >= 0 && "epoll_create1 failed");
    assert(events_.size() > 0 && "events creation failed");
}

Epoller::~Epoller()
{
    if (epollFd_ >= 0)
        close(epollFd_);
}

#endif  // ~EPOLL_H