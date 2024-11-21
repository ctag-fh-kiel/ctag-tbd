#pragma once


namespace tbd::system {

template<class LockT>
concept LockType = requires(LockT lock, uint32_t _uint32_t) {
    // deviation from std::mutex: lock timeout
    { lock.is_valid() } -> std::same_as<bool>;
    { lock.try_lock() } -> std::same_as<bool>;
    { lock.lock(_uint32_t) } -> std::same_as<bool>;
    { lock.lock() } -> std::same_as<bool>;
    { lock.unlock() } -> std::same_as<void>;
};

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

    LockGuardTemplate<LockT>& operator=(const LockGuardTemplate<LockT>&) = delete;

    /**
     *  check for failure to acquire lock
     */
    operator bool()  const {
        return _lock != nullptr;   
    }

    private:
        LockT* _lock;
};

template<LockType LockT>
struct LockDecorator : public LockT {
    [[nodiscard("scope guard destruction unlocks lock")]] LockGuardTemplate<LockT> guard() {
        return LockGuardTemplate<LockT>(*this);
    }
};

}
