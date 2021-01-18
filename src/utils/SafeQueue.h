//
// Created by dszhdankin on 13.01.2021.
//

#ifndef NAVY_COMBAT_SAFEQUEUE_H
#define NAVY_COMBAT_SAFEQUEUE_H
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cassert>

template<class T>
class SafeQueue {
private:
    std::queue<T> _que;
    std::mutex _mtx;
    std::condition_variable _cond;

public:
    SafeQueue(): _que(), _mtx(), _cond() {}

    void push(T val) {
        {
            std::lock_guard<std::mutex> lock(_mtx);
            que.push(val);
        }
        _cond.notify_one();
    }

    T pop_front() {
        std::unique_lock<std::mutex> lock(_mtx);

        _cond.wait(lock, [this]->{ return !_que.empty() });
        assert(!_que.empty());
        T front(std::move(_que.front()));
        que.pop();

        return front;
    }
};

#endif //NAVY_COMBAT_SAFEQUEUE_H
