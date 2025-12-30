// Audio Budget Tracking - Integration Example
// Add this to your audio processing code

#include "perf_monitor.hpp"

using namespace CTAG::INSTRUMENTATION;

// Example: Audio callback with budget tracking
void audio_callback_example() {
    // Configure at startup
    PerfMonitor::EnableLogging(true, 5000);  // Log every 5 seconds

    // Enable audio budget tracking for 32 samples @ 44.1kHz
    PerfMonitor::EnableAudioBudgetTracking(true, 32, 44100);

    // Your existing audio processing loop
    float buffer[32];

    while (true) {
        {
            PERF_MEASURE("codec_read");
            // Read from codec
            Codec::ReadBuffer(buffer, 32);
        }

        {
            PERF_MEASURE("audio_dsp");
            // Process audio
            processSynth(buffer, 32);
            applyEffects(buffer, 32);
        }

        {
            PERF_MEASURE("codec_write");
            // Write to codec
            Codec::WriteBuffer(buffer, 32);
        }

        // Every 5 seconds you'll see:
        // === Performance Statistics (CPU: 360 MHz) ===
        //   codec_read              : avg=  12.30 µs (  4428 cyc)  min=  11.50 µs  max=  15.20 µs  n=6887
        //   audio_dsp               : avg=  45.30 µs ( 16308 cyc)  min=  43.10 µs  max=  48.20 µs  n=6887
        //   codec_write             : avg=  11.80 µs (  4248 cyc)  min=  11.20 µs  max=  14.50 µs  n=6887
        // --------------------------------------------
        // Audio Budget: 9.5% (69.4 µs / 725.6 µs available)
        // CPU usage healthy
        // ============================================
    }
}

// Example: Different buffer sizes
void configure_for_different_scenarios() {
    // Low latency (smaller buffer, tighter budget)
    PerfMonitor::EnableAudioBudgetTracking(true, 32, 44100);   // 725.6 µs

    // Medium latency (more headroom)
    PerfMonitor::EnableAudioBudgetTracking(true, 64, 44100);   // 1451.2 µs

    // Higher latency (lots of headroom)
    PerfMonitor::EnableAudioBudgetTracking(true, 128, 44100);  // 2902.4 µs

    // Professional audio rate
    PerfMonitor::EnableAudioBudgetTracking(true, 32, 48000);   // 666.7 µs

    // High-res audio (very tight!)
    PerfMonitor::EnableAudioBudgetTracking(true, 32, 96000);   // 333.3 µs
}

// Example: Disable tracking
void disable_tracking() {
    PerfMonitor::EnableAudioBudgetTracking(false);
}

