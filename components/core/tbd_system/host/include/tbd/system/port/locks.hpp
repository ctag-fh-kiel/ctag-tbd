#pragma once

#include <tbd/host_only.hpp>

#include <mutex>
#include <chrono>
#include <atomic>

#include <tbd/logging.hpp>

// TBD relies heavily on atomics for thread safety, make sure they are lock free
static_assert(std::atomic<bool>::is_always_lock_free);

namespace tbd::system {

struct LockImpl {

bool is_valid() const {
    return true;
}

bool try_lock() {
    return _mutex.try_lock();
}

bool lock(uint32_t wait_time_ms = 0) {
    if (wait_time_ms == 0) {
        _mutex.lock();
        return true;
    }

    if (!_mutex.try_lock_for(std::chrono::microseconds(wait_time_ms))) {
        TBD_LOGE("tbd_system", "failed to acquire lock");
        return false;
    }   

    return true;
}

void unlock() {
    _mutex.unlock();
}

private:
    std::timed_mutex _mutex;
};

}
