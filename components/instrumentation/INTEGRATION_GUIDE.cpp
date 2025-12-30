/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

Example integration of PerfMonitor into SPManager

This file shows how to add performance monitoring to your existing code.

(c) 2025 by Robert Manzke. All rights reserved.
***************/

// =============================================================================
// HOW TO INTEGRATE PERF_MONITOR INTO SPMANAGER.CPP
// =============================================================================

// STEP 1: Add include at top of SPManager.cpp
// --------------------------------------------
#include "perf_monitor.hpp"

using namespace CTAG::INSTRUMENTATION;


// STEP 2: Enable logging in SPManager::Init() or similar
// -------------------------------------------------------
void SPManager::Init() {
    // ... existing initialization ...

    // Enable performance monitoring (log every 5 seconds)
    PerfMonitor::EnableLogging(true, 5000, 0, 1);

    ESP_LOGI("SPManager", "Performance monitoring enabled");
}


// STEP 3: Add measurements to your audio processing loop
// -------------------------------------------------------
void SPManager::ProcessAudio() {
    PERF_MEASURE_FUNCTION(); // Measure entire function

    // Measure control update
    {
        PERF_MEASURE("control_update");
        CTAG::CTRL::Control::Update(&pd.controlData, ledStatus);
        pd.cv = (float*) pd.controlData;
        pd.trig = (uint8_t*) pd.controlData + N_CVS * sizeof(float);
    }

    // Measure codec read
    {
        PERF_MEASURE("codec_read");
        DRIVERS::Codec::ReadBuffer(fbuf, BUF_SZ);
    }

    // Measure peak detection
    {
        PERF_MEASURE("peak_detection");
        float max = 0.f;
        max = fabsf(fbuf[0] + fbuf[1]) / 2.f;
        peakIn = 0.95f * peakIn + 0.05f * max;
        // ... LED update ...
    }

    // Measure sound processor CH0
    if (sp[0] != nullptr) {
        PERF_MEASURE("sound_processor_ch0");
        isStereoCH0 = sp[0]->GetIsStereo();
        sp[0]->Process(pd);
    }

    // Measure sound processor CH1
    if (!isStereoCH0 && sp[1] != nullptr) {
        PERF_MEASURE("sound_processor_ch1");
        sp[1]->Process(pd);
    }

    // Measure stereo conversion
    if (!isStereoCH0 && (toStereoCH0 || toStereoCH1)) {
        PERF_MEASURE("stereo_conversion");
        // ... stereo conversion code ...
    }

    // Measure codec write
    {
        PERF_MEASURE("codec_write");
        DRIVERS::Codec::WriteBuffer(fbuf, BUF_SZ);
    }
}


// STEP 4: Example of selective measurement based on conditions
// -------------------------------------------------------------
void SPManager::ProcessWithConditionalProfiling(bool enableProfiling) {
    auto& audio_section = PerfMonitor::GetSection("audio_full");
    PerfMonitor::EnableSection(audio_section, enableProfiling);

    PERF_MEASURE("audio_full");

    // When enableProfiling is false, overhead is only ~3 cycles
    ProcessAudio();
}


// STEP 5: Query performance stats programmatically
// -------------------------------------------------
void SPManager::CheckPerformance() {
    auto& codec_read = PerfMonitor::GetSection("codec_read");

    uint64_t avg, min, max;
    uint32_t count;
    PerfMonitor::GetStats(codec_read, avg, min, max, count);

    float avg_us = PerfMonitor::CyclesToMicroseconds(avg);

    if (avg_us > 50.0f) { // Threshold: 50 microseconds
        ESP_LOGW("SPManager", "Codec read taking too long: %.2f µs", avg_us);
    }
}


// =============================================================================
// EXPECTED OUTPUT (every 5 seconds):
// =============================================================================
/*
I (12345) PERF_MONITOR: === Performance Statistics (CPU: 360 MHz) ===
I (12345) PERF_MONITOR:   ProcessAudio            : avg= 245.30 µs ( 88308 cyc)  min= 220.15 µs  max= 310.50 µs  n=6887
I (12345) PERF_MONITOR:   control_update          : avg=  12.45 µs (  4482 cyc)  min=  10.20 µs  max=  25.30 µs  n=6887
I (12345) PERF_MONITOR:   codec_read              : avg=   2.04 µs (   735 cyc)  min=   0.89 µs  max=  85.40 µs  n=6887
I (12345) PERF_MONITOR:   peak_detection          : avg=   1.50 µs (   540 cyc)  min=   1.20 µs  max=   3.80 µs  n=6887
I (12345) PERF_MONITOR:   sound_processor_ch0     : avg= 180.20 µs ( 64872 cyc)  min= 165.30 µs  max= 220.40 µs  n=6887
I (12345) PERF_MONITOR:   sound_processor_ch1     : avg=  35.60 µs ( 12816 cyc)  min=  32.10 µs  max=  45.20 µs  n=3443
I (12345) PERF_MONITOR:   stereo_conversion       : avg=   5.80 µs (  2088 cyc)  min=   5.40 µs  max=   8.90 µs  n=2205
I (12345) PERF_MONITOR:   codec_write             : avg=   1.93 µs (   696 cyc)  min=   1.69 µs  max=  77.10 µs  n=6887
I (12345) PERF_MONITOR: ============================================
*/


// =============================================================================
// MINIMAL CHANGES NEEDED TO YOUR EXISTING CODE:
// =============================================================================

// Before (line ~85 in SPManager.cpp):
void audio_task(void *pvParameter) {
    while (true) {
        // ... control update ...
        DRIVERS::Codec::ReadBuffer(fbuf, BUF_SZ);
        // ... processing ...
        DRIVERS::Codec::WriteBuffer(fbuf, BUF_SZ);
    }
}

// After (just add 3 lines!):
void audio_task(void *pvParameter) {
    while (true) {
        PERF_MEASURE("audio_task_loop"); // ← ADD THIS

        {
            PERF_MEASURE("codec_read"); // ← ADD THIS
            DRIVERS::Codec::ReadBuffer(fbuf, BUF_SZ);
        }

        // ... processing ...

        {
            PERF_MEASURE("codec_write"); // ← ADD THIS
            DRIVERS::Codec::WriteBuffer(fbuf, BUF_SZ);
        }
    }
}


// =============================================================================
// ADVANCED: Measure specific plugin performance
// =============================================================================

class MySynthPlugin : public SoundProcessor {
public:
    void Process(ProcessData &pd) override {
        PERF_MEASURE_FUNCTION(); // Measures "MySynthPlugin::Process"

        {
            PERF_MEASURE("osc_generation");
            generateOscillators(pd);
        }

        {
            PERF_MEASURE("filter_processing");
            applyFilter(pd);
        }

        {
            PERF_MEASURE("envelope_processing");
            applyEnvelope(pd);
        }
    }
};


// =============================================================================
// TIPS FOR BEST RESULTS:
// =============================================================================

// 1. Measure complete operations, not individual instructions
//    ✅ GOOD: PERF_MEASURE("reverb_process");
//    ❌ BAD:  PERF_MEASURE("add_one_sample");

// 2. Use meaningful names
//    ✅ GOOD: "codec_read_i2s_32samples"
//    ❌ BAD:  "func1"

// 3. Don't measure inside very tight loops
//    ❌ BAD:
//    for (int i = 0; i < 10000; i++) {
//        PERF_MEASURE("loop_iteration"); // Too many measurements!
//    }
//
//    ✅ GOOD:
//    PERF_MEASURE("entire_loop");
//    for (int i = 0; i < 10000; i++) {
//        // ... work ...
//    }

// 4. Use conditional profiling in production
//    #ifdef CONFIG_ENABLE_PROFILING
//        PerfMonitor::EnableLogging(true, 5000);
//    #endif

// 5. Remember to disable logging in release builds
//    PerfMonitor::EnableLogging(false); // Saves CPU cycles


