#ifndef TIMER_H

#define TIMER_H

#include <chrono> // 管理时间
#include <queue>
#include <unordered_map>
#include <cassert>
#include <functional>
#include <vector>
#include <algorithm>



typedef std::function<void()> timeOutCallBack;
typedef std::chrono::high_resolution_clock timeClock;
typedef std::chrono::time_point<timeClock> timeStamp;

struct timerNode
{
    int id;
    timeStamp expireSeconds; // 过期时间
    timeOutCallBack callBack; // 响应函数

    bool operator<(timerNode& t)
    {
        return expireSeconds < t.expireSeconds;
    }
    bool operator>(timerNode& t)
    {
        return expireSeconds > t.expireSeconds;
    }
};

class timer
{
public:
    timer() { heap_.resize(64); }
    ~timer() { clear(); }

    void add(int id, int timeOut, const timeOutCallBack& callBack);
    void pop();

    void addjust(int id, int newExpireTime);

    void tick();
    int getNextTick();

    void work(int id);

    void clear();

private:
    void siftUp_(size_t index);
    bool siftDown_(size_t index, size_t n);

    void swapNode_(size_t i, size_t j);
    void del_(size_t index);


    std::vector<timerNode> heap_; // 手动实现小顶堆 // 用priority_queue直接实现？
    std::unordered_map<int, size_t> ref_; // 存储计时器节点ID到其在堆中的索引的映射，便于快速访问和更新
};


#endif // ~TIMER_H