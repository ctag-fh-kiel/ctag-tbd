#include "PitchShifterTD.h"
#include <algorithm>
#include <cmath>
#include "esp_heap_caps.h"
#include <cassert>

PitchShifterTD::PitchShifterTD(int maxDelaySamples, int windowSize_, float sampleRate)
: bufSize(std::max(maxDelaySamples, windowSize_*3)),
  windowSize(std::max(8, windowSize_)),
  fs(sampleRate),
  writeIndex(0),
  dA(0.0f), dB(0.0f), dMin(0.0f), sweepRange(0.0f)
{
    if (bufSize > MAX_BUF_SIZE) bufSize = MAX_BUF_SIZE;
    if (windowSize > MAX_WINDOW_SIZE) windowSize = MAX_WINDOW_SIZE;
    halfWindow = windowSize / 2;

    // allocate buffer in PSRAM
    buffer = (float*)heap_caps_malloc(MAX_BUF_SIZE * sizeof(float), MALLOC_CAP_SPIRAM);
    assert(nullptr != buffer);

    std::fill(buffer, buffer + bufSize, 0.0f);
    initWindowsAndDelays();
}

PitchShifterTD::~PitchShifterTD(){
    heap_caps_free(buffer);
    buffer = nullptr;
}

void PitchShifterTD::setWindowSize(int newWin) {
    windowSize = std::max(8, newWin);
    if (windowSize > MAX_WINDOW_SIZE) windowSize = MAX_WINDOW_SIZE;
    halfWindow = windowSize / 2;
    if (bufSize < windowSize*3) {
        bufSize = windowSize*3;
        if (bufSize > MAX_BUF_SIZE) bufSize = MAX_BUF_SIZE;
        std::fill(buffer, buffer + bufSize, 0.0f);
        writeIndex = 0;
    }
    initWindowsAndDelays();
}

void PitchShifterTD::initWindowsAndDelays() {
    // Delay bounds: sweep over exactly one window size for perfect overlap-add
    sweepRange = float(windowSize);
    dMin = float(windowSize); // keep at least one window delay for safety
    dA = dMin;                // Head A starts at beginning of its window
    dB = dMin + sweepRange*0.5f; // Head B half a cycle offset
}

inline float PitchShifterTD::readInterp(float index) const {
    // Wrap circularly
    while (index < 0.0f) index += float(bufSize);
    while (index >= float(bufSize)) index -= float(bufSize);
    int i0 = int(index);
    int i1 = (i0 + 1) % bufSize;
    float frac = index - float(i0);
    return buffer[i0] + (buffer[i1] - buffer[i0]) * frac;
}

void PitchShifterTD::process(const float* in, float* out, unsigned int n, float pitchRatio) {
    float r = std::max(0.01f, pitchRatio);
    float m = 1.0f - r; // samples of delay change per output sample
    float dMax = dMin + sweepRange;
    for (unsigned int k = 0; k < n; ++k) {
        buffer[writeIndex] = in[k];
        float readPosA = float(writeIndex) - dA;
        float readPosB = float(writeIndex) - dB;
        float sA = readInterp(readPosA);
        float sB = readInterp(readPosB);
        float phaseA = (dA - dMin) / sweepRange;
        float phaseB = phaseA + 0.5f;
        if (phaseB >= 1.0f) phaseB -= 1.0f;
        //float wA = hannAtPhase(phaseA);
        float wA = triAtPhase(phaseA);
        //float wB = hannAtPhase(phaseB);
        float wB = triAtPhase(phaseB);
        float y = wA * sA + wB * sB;
        out[k] = y;
        writeIndex++;
        if (writeIndex >= bufSize) writeIndex = 0;
        dA += m;
        dB += m;
        if (dA < dMin)         dA += sweepRange;
        else if (dA >= dMax)   dA -= sweepRange;
        if (dB < dMin)         dB += sweepRange;
        else if (dB >= dMax)   dB -= sweepRange;
    }
}
