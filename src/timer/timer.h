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
typedef std::chrono::milliseconds MS;

struct timerNode
{
    int id;
    timeStamp expireTime; // 过期时间
    timeOutCallBack callBack; // 响应函数

    bool operator<(timerNode& t)
    {
        return expireTime < t.expireTime;
    }
    bool operator>(timerNode& t)
    {
        return expireTime > t.expireTime;
    }
};

class timer
{
public:
    timer() { heap_.reserve(64); } // 不能用resize
    ~timer() { clear(); }

    void add(int id, int timeOut, const timeOutCallBack& callBack);
    void pop();

    void adjust(int id, int newExpireTime);
    void adjust(int index);

    void tick(); // 处理所有已经到期的计时器节点
    int getNextTick(); // 获取下一个计时器到期的时间间隔

    void work(int id); // 处理并移除指定ID的计时器节点，执行其回调函数

    void clear();

private:
    void swapNode_(size_t i, size_t j);

    void siftUp_(size_t index);
    bool siftDown_(size_t index, size_t n); // 左闭右开

    void del_(size_t index);


    std::vector<timerNode> heap_; // 手动实现小顶堆 // 用priority_queue直接实现？
    std::unordered_map<int, size_t> ref_; // 存储计时器节点ID到其在堆中的索引的映射，便于快速访问和更新
};


#endif // ~TIMER_H