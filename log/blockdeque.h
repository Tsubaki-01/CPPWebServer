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

    void pop(T& item); // 队列头pop到item里
    void pop(T& item, int timeout); // 队列头pop到item里，超时则退出

    void flush();

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














#endif  // ~BLOCKDEQUEUE_H