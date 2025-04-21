// this is based on Mutable Instruments

#include <tbd/sound_utils/polypad/MiSuperSawOsc.hpp>
#include <cstring>
#include <cmath>


using namespace CTAG::SP;

void MiSuperSawOsc::Init() {
    memset(&phase, 0, sizeof(phase));
    phase_ = 0;
    strike_ = true;
}

void MiSuperSawOsc::SetPitch(const int16_t &pitch) {
    // Smooth HF noise when the pitch CV is noisy.
    if (pitch_ > (90 << 7) && pitch > (90 << 7)) {
        pitch_ = (static_cast<int32_t>(pitch_) + pitch) >> 1;
    } else {
        pitch_ = pitch;
    }
}

void MiSuperSawOsc::Render(
        int16_t *buffer,
        size_t size) {
    int32_t detune = detune_ + 1024;
    detune = (detune * detune) >> 9;
    uint32_t increments[7];
    for (int16_t i = 0; i < 7; ++i) {
        int32_t saw_detune = detune * (i - 3);
        int32_t detune_integral = saw_detune >> 16;
        int32_t detune_fractional = saw_detune & 0xffff;
        int32_t increment_a = ComputePhaseIncrement(pitch_ + detune_integral);
        int32_t increment_b = ComputePhaseIncrement(pitch_ + detune_integral + 1);
        increments[i] = increment_a + \
        (((increment_b - increment_a) * detune_fractional) >> 16);
    }
    if (strike_) {
        for (size_t i = 0; i < 6; ++i) {
            phase[i] = Random::GetWord();
        }
        strike_ = false;
    }

    while (size--) {
        int32_t sample;

        phase_ += increments[0];
        phase[0] += increments[1];
        phase[1] += increments[2];
        phase[2] += increments[3];
        phase[3] += increments[4];
        phase[4] += increments[5];
        phase[5] += increments[6];

        // Compute a sample.
        sample = -28672;
        sample += phase_ >> 19;
        sample += phase[0] >> 19;
        sample += phase[1] >> 19;
        sample += phase[2] >> 19;
        sample += phase[3] >> 19;
        sample += phase[4] >> 19;
        sample += phase[5] >> 19;
        sample = Interpolate88(ws_moderate_overdrive, sample + 32768);
        sample >>= damp_;
        *buffer++ += sample;
    }
}

uint32_t MiSuperSawOsc::ComputePhaseIncrement(int16_t midi_pitch) {
    if (midi_pitch >= kPitchTableStart) {
        midi_pitch = kPitchTableStart - 1;
    }

    int32_t ref_pitch = midi_pitch;
    ref_pitch -= kPitchTableStart;

    size_t num_shifts = 0;
    while (ref_pitch < 0) {
        ref_pitch += kOctave;
        ++num_shifts;
    }

    uint32_t a = lut_oscillator_increments[ref_pitch >> 4];
    uint32_t b = lut_oscillator_increments[(ref_pitch >> 4) + 1];
    uint32_t phase_increment = a + \
      (static_cast<int32_t>(b - a) * (ref_pitch & 0xf) >> 4);
    phase_increment >>= num_shifts;
    return phase_increment;
}

uint32_t MiSuperSawOsc::ComputeDelay(int16_t midi_pitch) {
    if (midi_pitch >= kHighestNote - kOctave) {
        midi_pitch = kHighestNote - kOctave;
    }

    int32_t ref_pitch = midi_pitch;
    ref_pitch -= kPitchTableStart;

    size_t num_shifts = 0;
    while (ref_pitch < 0) {
        ref_pitch += kOctave;
        ++num_shifts;
    }

    uint32_t a = lut_oscillator_delays[ref_pitch >> 4];
    uint32_t b = lut_oscillator_delays[(ref_pitch >> 4) + 1];
    uint32_t delay = a + (static_cast<int32_t>(b - a) * (ref_pitch & 0xf) >> 4);
    delay >>= 12 - num_shifts;
    return delay;
}

void MiSuperSawOsc::SetDetune(const int16_t &detune) {
    detune_ = detune;
}

void MiSuperSawOsc::SetDamp(const uint16_t &damp) {
    damp_ = damp;
}
