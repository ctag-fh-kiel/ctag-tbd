#pragma once

#include <algorithm>
#include <tbd/errors.hpp>

#if USE_TBD_ERR
#define TBD_FAIL(err) { ::tbd::errors::err }
#else
#define TBD_FAIL(err) { ::tbd::errors::FAILURE }
#endif

namespace tbd {

template<typename T>
struct Maybe;

template<typename ValueT>
struct Maybe {
    Maybe(const errors::Errors error) : has_err_(true), content_({.err = error}) {}
    Maybe(const ValueT& value) : has_err_(false), content_({.value = value}) {}
    Maybe(ValueT&& value) : has_err_(false), content_({.value = std::move(value)}) {}
    Maybe(const Maybe& other) = default;
    Maybe(Maybe&& other) = default;

    operator bool() const { return !has_err_; }

    ValueT& value() { return content_.value; }
    const ValueT& value() const { return content_.value; }

    ValueT& operator* () { return content_.value; }
    const ValueT& operator* () const { return content_.value; }

    ValueT* operator-> () { return &content_.value; }
    const ValueT* operator-> () const { return &content_.value; }

    errors::Errors error() const { return has_err_ ? content_.err : TBD_OK; }

    static Maybe fail(const Error error) { return {error, false}; }

private:
    const bool has_err_;
    union {
        ValueT value;
        Error err;
    } const content_;
};

template<typename ValueT>
struct PtrMaybeBase {
    PtrMaybeBase(const errors::Errors error) : has_err_(true), content_({.err = error}) {}
    PtrMaybeBase(ValueT* value) : has_err_(false), content_({.value = value}) {}
    PtrMaybeBase(const PtrMaybeBase& other) = default;
    PtrMaybeBase(PtrMaybeBase&& other) = default;

    operator bool() const { return !has_err_; }

    ValueT* value() { return content_.value; }
    const ValueT* value() const { return content_.value; }

    ValueT* operator-> () { return content_.value; }
    const ValueT* operator-> () const { return content_.value; }

    Error error() const { return has_err_ ? content_.err : TBD_OK; }

    static PtrMaybeBase fail(const Error error) { return PtrMaybeBase(error); }

private:
    const bool has_err_;
    union {
        ValueT* value;
        Error err;
    } const content_;
};

template<typename ValueT>
struct Maybe<ValueT*> : PtrMaybeBase<ValueT> {
    ValueT& operator* () { return *PtrMaybeBase<ValueT>::content_.value; }
    const ValueT& operator* () const { return *PtrMaybeBase<ValueT>::content_.value; }
};

template<>
struct Maybe<void*> : PtrMaybeBase<void> {};

template<typename T>
Maybe<T> return_err(const Error error) { return Maybe<T>(error); }

}
