#include "epoll.h"


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


bool Epoller::addFd(int fd, uint32_t events)
{
    if (fd < 0)
        return false;

    epoll_event ev = { 0 };
    ev.data.fd = fd;
    ev.events = events;

    return epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}; // 将文件描述符 fd 添加到 epoll 实例中，并指定要监听的事件类型

bool Epoller::delFd(int fd)
{
    if (fd < 0)
        return false;

    return epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr) == 0;

}; // 从 epoll 实例中删除文件描述符 fd

bool Epoller::modFd(int fd, uint32_t events)
{
    if (fd < 0)
        return false;

    struct epoll_event ev = { 0 };
    ev.data.fd = fd;
    ev.events = events;

    return epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}; // 修改已经添加到 epoll 实例中的文件描述符 fd 的监听事件


int Epoller::getEventFd(size_t i) const
{
    assert(i < events_.size());
    return events_[i].data.fd;
}; // 返回第 i 个事件对应的文件描述符

uint32_t Epoller::getEvents(size_t i) const
{
    assert(i < events_.size());
    return events_[i].events;
}; // 返回第 i 个事件对应的事件类型


int Epoller::wait(int timeoutMs)
{
    return epoll_wait(epollFd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
}; // 等待事件的发生。timeoutMs 指定超时时间，默认是无限等待。返回值是发生事件的数量