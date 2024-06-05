#include <iostream>
#include <thread>
#include <chrono>
#include "timer.h"


void timer::swapNode_(size_t i, size_t j)
{
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
};

void timer::siftUp_(size_t index)
{
    assert(index >= 0 && index < heap_.size());
    size_t parent;

    while (index > 0)
    {
        parent = (index - 1) / 2;
        if (heap_[index] > heap_[parent]) break;
        swapNode_(index, parent);
        index = parent;
    }
};
/* void HeapTimer::siftup_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t j = ( i - 1) / 2;
    while (j >= 0) {
        if (heap_[j] < heap_[i]) { break; }
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
} // 错误  unsigned long long(size_t) 负数 */
bool timer::siftDown_(size_t index, size_t n)
{
    assert(index >= 0 && index < n);
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = 2 * i + 1;
    while (j < n)
    {
        if (j + 1 < n && heap_[j] > heap_[j + 1]) j++;
        if (heap_[i] > heap_[j]) break;
        swapNode_(i, j);
        i = j;
        j = 2 * j + 1;
    }
    return i != index;
};


void timer::del_(size_t index)
{
    assert(index >= 0 && index < heap_.size());

    size_t i = index;
    size_t j = heap_.size() - 1;
    if (i < j) // index 不是最后一个元素
    {
        swapNode_(i, j);
        if (siftDown_(i, j) == false) // 不能下降就上移
            siftUp_(i);
    }
    // 删除末尾节点
    ref_.erase(heap_.back().id);
    heap_.pop_back();
};
void timer::add(int id, int timeOut, const timeOutCallBack& callBack)
{
    assert(id >= 0);

    timeStamp expires = timeClock::now() + MS(timeOut);
    // 新节点
    if (ref_.count(id) == 0)
    {
        heap_.push_back(timerNode{ id,expires, callBack });
        ref_[id] = heap_.size() - 1;
        siftUp_(heap_.size() - 1);
    }
    else { // 已有节点
        int index = ref_[id];
        heap_[index].expireTime = expires;
        heap_[index].callBack = callBack;
        if (siftDown_(index, heap_.size()) == false)
            siftUp_(index);
    }
};
void timer::pop()
{
    assert(!heap_.empty());
    del_(0);
};


void timer::adjust(int id, int newExpireTime)
{
    assert(!heap_.empty() && id >= 0 && ref_.count(id) > 0);

    int index = ref_[id];
    heap_[index].expireTime = timeClock::now() + MS(newExpireTime);
    if (siftDown_(index, heap_.size()) == false)
        siftUp_(index);
};


void timer::tick()
{
    while (!heap_.empty())
    {
        timerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expireTime - timeClock::now()).count() > 0)
            break; // 还不到处理的时候

        // 处理最小节点的响应函数并调整堆结构
        node.callBack();
        pop();
    }
};
int timer::getNextTick()
{
    // 先处理待响应节点
    tick();
    if (heap_.empty())
        return -1;
    else {
        auto res = std::chrono::duration_cast<MS>(heap_.front().expireTime - timeClock::now()).count();
        return res < 0 ? 0 : res;
    }
};


void timer::work(int id)
{
    if (!heap_.empty() && ref_.count(id) > 0)
    {
        size_t index = ref_[id];
        heap_[index].callBack();
        del_(index);
    }
};


void timer::clear()
{
    heap_.clear();
    ref_.clear();
};
