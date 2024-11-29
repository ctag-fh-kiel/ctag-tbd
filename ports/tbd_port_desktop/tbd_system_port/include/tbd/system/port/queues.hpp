#pragma once

#include <optional>
#include <queue>
#include <condition_variable>


namespace tbd::system {

template<typename ItemT, uint8_t queue_size = 10>
struct Queue {
    Queue() {};

    // no copying
    Queue(const Queue&) = delete;
    Queue(Queue&&) = delete;
    // no assignment
    Queue &operator = (const Queue&) = delete;
    Queue &operator = (Queue&&) = delete;

    bool push(const ItemT& item) {
        std::unique_lock lock(_access_lock);
        _is_not_full.wait(lock, [this] {
            return _impl.size() < queue_size;
        });
        _impl.push(item);
        _is_not_empty.notify_one();
        return true;
    }

    /** @brief take oldest element from queue
     *
     *  If no value is available the task sleeps until an element beomces available
     *  or operation times out.
     *
     *  @arg wait_time_ms   time to wait in ms, 0 means wait indefinitely
     */
    std::optional<ItemT> pop(uint32_t wait_time_ms = 0) {
        std::unique_lock lock(_access_lock);

        auto has_item = [this] {
            return !_impl.empty();
        };

        if (wait_time_ms == 0) {
            _is_not_empty.wait(lock, has_item);
        } else {
            std::chrono::microseconds wait_time(wait_time_ms);
            auto timout = _is_not_empty.wait_for(lock, wait_time, has_item);
            if (timout == std::cv_status::timeout) {
                return {};
            }
        }

        auto item = _impl.front();
        _impl.pop();
        _is_not_full.notify_one();
        return item;
    }

    /** @brief take oldest element from queue
     *
     */
    std::optional<ItemT> try_pop() {
        std::lock_guard lock(_access_lock);
        if (_impl.empty()) {
            return {};
        }
        ItemT item = _impl.front();
        _impl.pop();
        _is_not_full.notify_one();
        return item;
    }

private:
    std::queue<ItemT> _impl;
    std::mutex _access_lock;
    std::condition_variable _is_not_empty;
    std::condition_variable _is_not_full;
};

}
