#include "timer.h"


void timer::siftUp_(size_t index)
{
    assert(index > 0 && index < heap_.size());
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
    size_t j = (static_cast<int> (i) - 1) / 2;
    while (j >= 0) {
        if (heap_[j] < heap_[i]) { break; }
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
} // 错误  unsigned long long(size_t) 负数 */
bool timer::siftDown_(size_t index, size_t n)
{
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());

};

void timer::swapNode_(size_t i, size_t j)
{
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
};
void timer::del_(size_t index);

void timer::add(int id, int timeOut, const timeOutCallBack& callBack);
void timer::pop();

void timer::addjust(int id, int newExpireTime);

void timer::tick();
int timer::getNextTick();

void timer::work(int id);

void timer::clear();