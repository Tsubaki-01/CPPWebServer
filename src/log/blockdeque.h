#ifndef BLOCKDEQUEUE_H

#define BLOCKDEQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <cassert>
#include <sys/time.h>

template<class T>
class BlockDeque
{
public:
    explicit BlockDeque(size_t Capacity = 1000);
    ~BlockDeque();

    bool empty();
    bool full();

    size_t size();
    size_t capacity();

    T front();
    T back();

    void push_back(const T& item);
    void push_front(const T& item);

    bool pop(T& item); // 队列头pop到item里
    bool pop(T& item, int timeout); // 队列头pop到item里，超时则退出

    void flush(); // 强制唤醒一个消费者进程(以防有可能一直push，pop进程抢不到互斥锁)

    void clear();

    void close();
private:
    std::deque<T> deq_; // 存储队列中的元素
    size_t capacity_;

    std::mutex mtx_; // 对deq_的互斥锁

    std::condition_variable condConsumer_; // 消费者线程条件变量
    std::condition_variable condProducer_; // 消费者线程条件变量

    bool isClose_;
};


template<class T>
BlockDeque<T>::BlockDeque(size_t Capacity)
    :capacity_(Capacity), isClose_(false)
{
    assert(Capacity > 0);
};

template<class T>
BlockDeque<T>::~BlockDeque()
{
    close();
};

template<class T>
bool BlockDeque<T>::BlockDeque::empty()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return deq_.empty();
};
template<class T>
bool BlockDeque<T>::full()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return deq_.size() >= capacity_;
};

template<class T>
size_t BlockDeque<T>::size()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return deq_.size();
};
template<class T>
size_t BlockDeque<T>::capacity()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return capacity_;
};

template<class T>
T BlockDeque<T>::front()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return deq_.front();
};
template<class T>
T BlockDeque<T>::back()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return deq_.back();
};

template<class T>
void BlockDeque<T>::push_back(const T& item)
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (deq_.size() >= capacity_) // while 保证线程安全
        condProducer_.wait(lock);
    if (isClose_)
        return;
    deq_.push_back(item);
    condConsumer_.notify_one();
};
template<class T>
void BlockDeque<T>::push_front(const T& item)
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (deq_.size() >= capacity_) // while 保证线程安全
        condProducer_.wait(lock);
    if (isClose_)
        return;
    deq_.push_front(item);
    condConsumer_.notify_one();
};

template<class T>
bool BlockDeque<T>::pop(T& item)
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (deq_.empty())
    {
        condConsumer_.wait(lock);
        if (isClose_)
            return false;
    }

    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
};
template<class T>
bool BlockDeque<T>::pop(T& item, int timeout)
{
    std::unique_lock<std::mutex> lock(mtx_);
    while (deq_.empty())
    {
        if (condConsumer_.wait_for(lock, std::chrono::seconds(timeout))
            == std::cv_status::timeout)
        {
            return false;
        }
        if (isClose_)
            return false;
    }

    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
};

template<class T>
void BlockDeque<T>::flush()
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (!deq_.empty())
    {
        condConsumer_.notify_one();
    }
};

template<class T>
void BlockDeque<T>::clear()
{
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
};

template<class T>
void BlockDeque<T>::close()
{
    {
        std::lock_guard<std::mutex> locker(mtx_);
        deq_.clear();
        isClose_ = true;
    }
    condProducer_.notify_all();
    condConsumer_.notify_all();
};
#endif  // ~BLOCKDEQUEUE_H