#pragma once

// FIXME: sound processor should not know about audio device
#include <tbd/audio_device/audio_settings.hpp>

#include <concepts>
#include <iterator>
#include <cstddef>

namespace tbd::sound_processor {

template<class ImplT>
concept SampleHopperType =
    std::is_constructible_v<ImplT, typename ImplT::buffer_type>
    && requires(ImplT impl, const ImplT const_impl, int _int) {
        {*impl} -> std::same_as<std::iter_reference_t<ImplT>>;
        {impl.operator->()} -> std::same_as<std::add_pointer_t<std::iter_value_t<ImplT>>>;
        {const_impl.diff(const_impl)} -> std::same_as<std::iter_difference_t<ImplT>>;
        {impl.move(_int)} -> std::same_as<void>;
    };


template<SampleHopperType IterT>
struct SampleIteratorDecorator : IterT, std::contiguous_iterator_tag {
    using value_type = typename IterT::value_type;
    using element_type = typename IterT::value_type;
    using difference_type = typename IterT::difference_type;
    using iterator_category = std::contiguous_iterator_tag;

    SampleIteratorDecorator() = default;
    // SampleIteratorDecorator(const SampleIteratorDecorator& other) = default;
    explicit SampleIteratorDecorator(typename IterT::buffer_type buffer) : IterT(buffer) {}

    bool operator== (const SampleIteratorDecorator& other) const { return IterT::diff(other) == 0; }
    bool operator!= (const SampleIteratorDecorator& other) const { return IterT::diff(other) != 0; }
    bool operator>(const SampleIteratorDecorator& rhs) const { return IterT::diff(rhs) > 0; }
    bool operator<(const SampleIteratorDecorator& rhs) const { return IterT::diff(rhs) < 0; }
    bool operator>=(const SampleIteratorDecorator& rhs) const { return IterT::diff(rhs) >= 0; }
    bool operator<=(const SampleIteratorDecorator& rhs) const { return IterT::diff(rhs) <= 0; }

    difference_type operator-(const SampleIteratorDecorator& rhs) const { return IterT::diff(rhs); }

    SampleIteratorDecorator& operator++ () {
        IterT::move(1);
        return *this;
    }

    SampleIteratorDecorator operator++ (int) {
        const SampleIteratorDecorator pre_increment = *this;
        IterT::move(1);
        return pre_increment;
    }

    SampleIteratorDecorator& operator-- () {
        IterT::move(-1);
        return *this;
    }

    SampleIteratorDecorator operator-- (int) {
        const SampleIteratorDecorator<IterT> pre_increment = *this;
        IterT::move(-1);
        return pre_increment;
    }

    SampleIteratorDecorator& operator+= (const difference_type distance) {
        IterT::move(distance);
        return *this;
    }

    SampleIteratorDecorator operator+ (const difference_type rhs) const {
        SampleIteratorDecorator res = *this;
        res += rhs;
        return res;
    }

    SampleIteratorDecorator& operator-= (const difference_type distance) {
        IterT::move(-distance);
        return *this;
    }

    SampleIteratorDecorator operator- (const difference_type rhs) const {
        SampleIteratorDecorator res = *this;
        res -= rhs;
        return res;
    }

    friend SampleIteratorDecorator operator+(difference_type lhs, const SampleIteratorDecorator& rhs) {
        return SampleIteratorDecorator(lhs + rhs._ptr);
    }

    friend SampleIteratorDecorator operator-(difference_type lhs, const SampleIteratorDecorator& rhs) {
        return SampleIteratorDecorator(lhs - rhs._ptr);
    }
};

// template<SampleHopperType IterT>
// struct EnumeratorHopper {
//     using value_type = typename IterT::value_type;
//
//     explicit EnumeratorHopper(const IterT& iter) : iter_(iter), pos_(0) {}
//
//     value_type& operator*() { return *iter_; }
//
//     void move(const int distance) { iter_.move(distance); pos_ += distance; }
//     bool compare(const EnumeratorHopper& rhs) const { return pos_ - rhs.pos_; }
//
// protected:
//     IterT* iter_;
//     int pos_;
// };

struct SampleHopperBase {
    using difference_type = int;
    using buffer_type = float*;

    SampleHopperBase() : pos_(nullptr) {}
    explicit SampleHopperBase(float* pos) : pos_(pos) {}

    void move(const difference_type distance) { pos_ = at(distance); }
    difference_type diff(const SampleHopperBase& other) const { return (pos_ - other.pos_) / TBD_AUDIO_CHANNELS; }
    float* at(const difference_type distance) const { return pos_ + distance * TBD_AUDIO_CHANNELS; }

protected:
    float* pos_;
};

struct MonoSampleHopper : SampleHopperBase {
    using value_type = float;

    MonoSampleHopper() : SampleHopperBase(nullptr) {}
    explicit MonoSampleHopper(float* pos) : SampleHopperBase(pos) {}

    float& operator*() const { return *at(0); }
    float* operator->() const { return at(0); }
    float& operator[] (const size_t index) const { return *at(index); }
};

static_assert(SampleHopperType<MonoSampleHopper>);
using MonoSampleIterator = SampleIteratorDecorator<MonoSampleHopper>;
static_assert(std::contiguous_iterator<MonoSampleIterator>);


struct MonoSampleView {
    explicit MonoSampleView(float* data, const size_t size) : data_(data), size_(size) {}

    float& operator[] (const size_t index) { return data_[index * 2]; }

    MonoSampleIterator begin() { return MonoSampleIterator(data_); }
    MonoSampleIterator end() { return MonoSampleIterator(data_ + TBD_AUDIO_CHANNELS * (size_+ 1)); };

private:
    float* data_;
    size_t size_;
};
static_assert(std::ranges::contiguous_range<MonoSampleView>);

struct alignas(float) SampleProxy final {
    SampleProxy() = delete;

    float left;
    float right;
    float* data() { return &left; }

    static SampleProxy* alias_at(float* buffer, int distance) {
        return reinterpret_cast<SampleProxy*>(buffer + distance * TBD_AUDIO_CHANNELS);
    }
};
static_assert(offsetof(SampleProxy, left) == 0);
static_assert(offsetof(SampleProxy, right) == sizeof(float));


struct StereoSampleHopper : MonoSampleHopper {
    using value_type = SampleProxy;
    using difference_type = int;
    using buffer_type = float*;

    StereoSampleHopper() : MonoSampleHopper(nullptr) {}
    // StereoSampleHopper(const MonoSampleHopper& other) : pos_(other.pos_) {}
    explicit StereoSampleHopper(float* pos) : MonoSampleHopper(pos) {}

    SampleProxy& operator*() const { return *SampleProxy::alias_at(pos_, 0); }
    SampleProxy* operator->() const { return SampleProxy::alias_at(pos_, 0); }
    SampleProxy& operator[] (const size_t index) const { return *SampleProxy::alias_at(pos_, index); }
};
static_assert(SampleHopperType<StereoSampleHopper>);
using StereoSampleIterator = SampleIteratorDecorator<StereoSampleHopper>;
static_assert(std::contiguous_iterator<StereoSampleIterator>);


struct StereoSampleView {
    explicit StereoSampleView(float* data, size_t size) : data_(data), size_(size) {}

    SampleProxy& operator[] (const size_t index) { return *reinterpret_cast<SampleProxy*>(data_ + index * TBD_AUDIO_CHANNELS); }
    StereoSampleIterator begin() { return StereoSampleIterator(data_); }
    StereoSampleIterator end() { return StereoSampleIterator(data_ + TBD_AUDIO_CHANNELS * (size_ + 1)); }
private:
    float* data_;
    size_t size_;
};
static_assert(std::ranges::contiguous_range<StereoSampleView>);


}
