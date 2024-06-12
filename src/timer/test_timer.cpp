#include <iostream>
#include <thread>
#include <chrono>
#include "timer.h"

void testAddAndPop() {
    timer t;

    t.add(1, 500, []() {
        std::cout << "Timer 1 expired after 500ms" << std::endl;
        });

    t.add(2, 1000, []() {
        std::cout << "Timer 2 expired after 1000ms" << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    t.tick(); // Should trigger Timer 1

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    t.tick(); // Should trigger Timer 2
}

void testAdjust() {
    timer t;

    t.add(1, 500, []() {
        std::cout << "Timer 1 expired after 500ms" << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    t.adjust(1, 1000); // Adjust Timer 1 to expire after 1000ms from now

    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    t.tick(); // Should not trigger Timer 1

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    t.tick(); // Should trigger Timer 1
}

void testWorkAndClear() {
    timer t;

    t.add(1, 500, []() {
        std::cout << "Timer 1 expired after 500ms" << std::endl;
        });

    t.add(2, 1000, []() {
        std::cout << "Timer 2 expired after 1000ms" << std::endl;
        });

    t.work(1); // Manually trigger Timer 1

    std::this_thread::sleep_for(std::chrono::milliseconds(1600));
    t.tick(); // Should trigger Timer 2

    t.clear(); // Clear all timers

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    t.tick(); // Should not trigger any timer
}

void testGetNextTick() {
    timer t;

    t.add(1, 500, []() {
        std::cout << "Timer 1 expired after 500ms" << std::endl;
        });

    t.add(2, 1000, []() {
        std::cout << "Timer 2 expired after 1000ms" << std::endl;
        });

    int nextTick = t.getNextTick(); // Should return the time until the next timer expires
    std::cout << "Next tick in " << nextTick << "ms" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    t.tick(); // Should trigger Timer 1

    nextTick = t.getNextTick(); // Should return the time until the next timer expires
    std::cout << "Next tick in " << nextTick << "ms" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    t.tick(); // Should trigger Timer 2

    nextTick = t.getNextTick(); // Should return -1 as there are no timers left
    std::cout << "Next tick in " << nextTick << "ms" << std::endl;
}

int main() {
    std::cout << "Testing add and pop:" << std::endl;
    testAddAndPop();

    std::cout << "Testing adjust:" << std::endl;
    testAdjust();

    std::cout << "Testing work and clear:" << std::endl;
    testWorkAndClear();

    std::cout << "Testing getNextTick:" << std::endl;
    testGetNextTick();

    return 0;
}
