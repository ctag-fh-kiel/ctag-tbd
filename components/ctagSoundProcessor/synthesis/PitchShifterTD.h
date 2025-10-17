#pragma once
#include <cmath>
#include <algorithm>

/// Time-domain pitch shifter following the "two sliding heads + Hann crossfade"
/// algorithm popularized by Katja https://www.katjaas.nl/pitchshift/pitchshift.html
/// Mono in/out. Use one instance per channel for stereo.
class PitchShifterTD {
public:
    static constexpr int MAX_BUF_SIZE = 4096;      // Set as needed
    static constexpr int MAX_WINDOW_SIZE = 512;    // Set as needed

    /// @param maxDelaySamples Size of circular buffer (must be >= 2*windowSize + margin)
    /// @param windowSize Number of samples per Hann window cycle (even number recommended)
    /// @param sampleRate Audio sample rate
    PitchShifterTD(int maxDelaySamples, int windowSize, float sampleRate);

    /// Reset internal state (clears buffer, resets phases)
    void reset();

    /// Process a block of mono samples.
    /// @param in  input buffer (mono)
    /// @param out output buffer (mono)
    /// @param n   number of samples
    /// @param pitchRatio Desired pitch shift ratio (e.g., 1.0 no shift, 1.5 up a fifth, 0.5 down an octave)
    void process(const float* in, float* out, unsigned int n, float pitchRatio);

    /// Set window size on the fly (will reinit windows; slight click possible)
    void setWindowSize(int newWin);

    int getWindowSize() const { return windowSize; }
    int getBufferSize() const { return bufSize; }
    float getSampleRate() const { return fs; }

    /// Algorithmic latency in samples (number of input samples to prime before valid output)
    int getLatencySamples() const { return windowSize; }

private:
    int bufSize;
    int windowSize;
    int halfWindow;
    float fs;

    float buffer[MAX_BUF_SIZE];
    int writeIndex;

    // Two delay values in samples (floating), swept linearly
    float dA;
    float dB;

    // Sweep range and bounds
    float dMin;
    float sweepRange;

    // Precomputed Hann window over [0..windowSize)
    float window[MAX_WINDOW_SIZE];

    void initWindowsAndDelays();
    float hannAtPhase(float phase) const { return 0.5f * (1.0f - cosf(2.0f * 3.14159265359f * phase)); }
    float readInterp(float index) const;
};
