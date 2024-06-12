#ifndef THREADPOOL_H

#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <exception>
#include <cassert>
#include <thread>


class ThreadPool
{
public:
    explicit ThreadPool(size_t threadNum = 8); // 防止隐式转换（与复制列表初始化）
    ThreadPool(ThreadPool&&) = default;
    ~ThreadPool();

    void worker();

    // 添加任务到任务队列
    template<class T>
    void addTask(T&& task);


private:
    std::mutex mtx_; // 管理tasks的互斥锁
    std::condition_variable cv_;
    bool isClosed_; // 关闭线程  // 多线程情况下，初始化线程池时后面的线程创建时，线程池可能已经被释放删除，需要检查

    std::vector<std::thread> threadPool;
    std::queue < std::function <void()>> tasks_; // 任务队列

};


#endif // ~THREADPOOL_H
