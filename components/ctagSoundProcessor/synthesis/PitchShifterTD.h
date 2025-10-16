/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2025 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#pragma once
#include <cmath>

/// Time-domain pitch shifter following the "two sliding heads + tri crossfade"
/// algorithm popularized by Katja https://www.katjaas.nl/pitchshift/pitchshift.html
/// Mono in/out. Use one instance per channel for stereo.
class PitchShifterTD {
public:
    static constexpr int MAX_BUF_SIZE = 4096;      // Set as needed
    static constexpr int MAX_WINDOW_SIZE = 1024;    // Set as needed

    /// @param maxDelaySamples Size of circular buffer (must be >= 2*windowSize + margin)
    /// @param windowSize Number of samples per Hann window cycle (even number recommended)
    /// @param sampleRate Audio sample rate
    PitchShifterTD(int maxDelaySamples, int windowSize, float sampleRate);
    ~PitchShifterTD();

    /// Process a block of mono samples.
    /// @param in  input buffer (mono)
    /// @param out output buffer (mono)
    /// @param n   number of samples
    /// @param pitchRatio Desired pitch shift ratio (e.g., 1.0 no shift, 1.5 up a fifth, 0.5 down an octave)
    void process(const float* in, float* out, unsigned int n, float pitchRatio);

    /// Set window size on the fly (will reinit windows; slight click possible)
    void setWindowSize(int newWin);

private:
    int bufSize;
    int windowSize;
    int halfWindow;
    float fs;

    float *buffer;
    int writeIndex;

    // Two delay values in samples (floating), swept linearly
    float dA;
    float dB;

    // Sweep range and bounds
    float dMin;
    float sweepRange;

    void initWindowsAndDelays();
    float hannAtPhase(float phase) const { return 0.5f * (1.0f - cosf(2.0f * 3.14159265359f * phase)); }
    float triAtPhase(float phase) const { return 2.0f * (phase >= 0.5f ? 1.0f - phase : phase);}
    float readInterp(float index) const;
};
