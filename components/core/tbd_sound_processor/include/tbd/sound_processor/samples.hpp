#pragma once

#include "tbd/audio_device/audio_settings.hpp"


namespace tbd::sound_processor {

template<class IterT>
struct SampleIterator : IterT {
    explicit SampleIterator(float* pos) : IterT(pos) {}

    SampleIterator& operator++ () {
        IterT::pos_ += TBD_AUDIO_CHANNELS;
        return *this;
    }

    SampleIterator operator++ (int) {
        const SampleIterator<IterT> pre_increment = *this;
        IterT::pos_ += TBD_AUDIO_CHANNELS;
        return pre_increment;
    }

    SampleIterator& operator-- () {
        IterT::pos_ -= TBD_AUDIO_CHANNELS;
        return *this;
    }

    SampleIterator operator-- (int) {
        const SampleIterator<IterT> pre_increment = *this;
        IterT::pos_ -= TBD_AUDIO_CHANNELS;
        return pre_increment;
    }

    SampleIterator& operator+= (const int steps) {
        IterT::pos_ += (TBD_AUDIO_CHANNELS * steps);
        return *this;
    }

    SampleIterator<IterT> operator+ (const int steps) const {
        SampleIterator<IterT> res = *this;
        res += steps;
        return res;
    }

    SampleIterator operator-= (const int steps) {
        IterT::pos_ -= TBD_AUDIO_CHANNELS * steps;
        return *this;
    }

    SampleIterator operator- (const int steps) const {
        SampleIterator res = *this;
        res += steps;
        return res;
    }


};

struct MonoSampleIteratorMixin {
    explicit MonoSampleIteratorMixin(float* pos) : pos_(pos) {}

    float& operator*() { return *pos_; }
    float operator*() const { return *pos_; }

protected:
    float* pos_;
};

using MonoSampleIterator = SampleIterator<MonoSampleIteratorMixin>;

struct MonoSampleView {
    explicit MonoSampleView(float* data) : data_(data) {}

    float& operator[] (const size_t index) { return data_[index * 2]; }
    float operator[] (const size_t index) const { return data_[index * 2]; }

    // SampleIterator begin();
    // SampleIterator begin() const;


private:
    float* data_;
};



struct StereoSampleView {
    explicit StereoSampleView(float* data) : data_(data) {}

    float* operator[] (const size_t index) { return data_ + (index * 2); }
    const float* operator[] (const size_t index) const { return data_ + (index * 2); }

private:
    float* data_;
};

}
