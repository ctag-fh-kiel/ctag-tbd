#pragma once

#include <concepts>


namespace tbd::system {

template<class LockT>
concept LockType = requires(LockT lock, uint32_t _uint32_t) {
    // deviation from std::mutex: lock timeout
    { lock.is_valid() } -> std::same_as<bool>;

    /** @brief try to acquire lock without waiting
     *
     *  Will not block, but instead notify the caller if the lock was acquired.
     *
     *  @arg wait_time_ms   time to wait in ms, 0 means wait indefinitely
     *  @return  true if lock acquired false if lock is taken
     */
    { lock.try_lock() } -> std::same_as<bool>;

    /** @brief acquire lock
     *
     *  If the lock is held by another task, the calling task sleeps until the lock is
     *  released or the opration times out.
     *
     *  @arg wait_time_ms   time to wait in ms, 0 means wait indefinitely
     */
    { lock.lock(_uint32_t) } -> std::same_as<bool>;
    { lock.lock() } -> std::same_as<bool>;

    /** @brief release the lock
     *
     */
    { lock.unlock() } -> std::same_as<void>;
};


/** @brief scope guard for lock usage
 *
 *  The guard acquires the lock on creation and releases it
 */
template<LockType LockT>
struct LockGuardTemplate {
    LockGuardTemplate(LockT& lock, uint32_t timeout_ms = 0) : _lock(&lock) {
        if (!_lock->lock(timeout_ms)) {
            _lock = nullptr;
        }
    }

    ~LockGuardTemplate() {
        if (! *this) {
            return;
        }
        _lock->unlock();
    }

    LockGuardTemplate& operator=(const LockGuardTemplate&) = delete;

    /** @brief check for failure to acquire lock
     *
     *  @return true if lock was acquired false if acquiring failed
     */
    operator bool()  const {
        return _lock != nullptr;   
    }

    private:
        LockT* _lock;
};

/** @brief add functionality to locks, that is not port specific
 *
 */
template<LockType LockT>
struct LockDecorator : LockT {
    [[nodiscard("scope guard destruction unlocks lock")]] LockGuardTemplate<LockT> guard() {
        return LockGuardTemplate<LockT>(*this);
    }
};

}
