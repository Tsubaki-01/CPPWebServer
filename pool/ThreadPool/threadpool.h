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

    void worker(void* arg);

    // 添加任务到任务队列
    template<class T>
    void addTask(T&& task);


private:
    std::mutex mtx_; // 管理tasks的互斥锁
    std::condition_variable cv_;
    bool isClosed_; // 多线程情况下，初始化线程池时后面的线程创建时，线程池可能已经被释放删除，需要检查

    std::vector<std::thread*> threadPool;
    std::queue < std::function <void()>> tasks_; // 任务队列

};


ThreadPool::ThreadPool(size_t threadNum)
{
    assert(threadNum > 0);
    threadPool.resize(threadNum);
    // 创建线程池
    for (size_t i = 0;i < threadNum;i++)
    {
        // std::lock_guard<std::mutex> lock(mtx_);
        threadPool[i] = &std::thread(worker, this);
        threadPool[i]->detach();
    }
}

ThreadPool::~ThreadPool()
{
    std::lock_guard<std::mutex> lock(mtx_);
    isClosed_ = true;
    cv_.notify_all(); // 执行任务队列中的剩余任务
}

void ThreadPool::worker(void* arg)
{
    ThreadPool* pool = reinterpret_cast<ThreadPool*> (arg);
    std::unique_lock<std::mutex> lock(pool->mtx_);

    while (true)
    {
        if (!pool->tasks_.empty())
        {
            auto task = pool->tasks_.front();
            pool->tasks_.pop();
            lock.unlock(); // 待任务执行完成后再加锁
            task(); // 执行任务
            lock.lock();
        }
        else if (pool->isClosed_) break; // 1.初始化线程池时后面的线程创建时，线程池可能已经被释放删除 2.关闭线程
        else // 任务队列为空，进入等待
        {
            pool->cv_.wait(lock);
        }
    }
}

template<class T>
void ThreadPool::addTask(T&& task)
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        tasks_.emplace(std::forward<T>(task));
    }
    cv_.notify_one();
}

#endif // ~THREADPOOL_H