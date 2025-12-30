# Audio Budget Tracking - Usage Guide

## Overview

The Performance Monitor now includes **audio budget tracking** to compare total CPU usage against available audio processing time. This helps identify if your real-time audio code is at risk of buffer underruns.

---

## Quick Start

### Enable Audio Budget Tracking

```cpp
#include "perf_monitor.hpp"

using namespace CTAG::INSTRUMENTATION;

void audio_task_init() {
    // Enable standard performance logging
    PerfMonitor::EnableLogging(true, 5000);  // Log every 5 seconds
    
    // Enable audio budget tracking
    // Parameters: enable, buffer_size, sample_rate
    PerfMonitor::EnableAudioBudgetTracking(true, 32, 44100);
    //                                            ^^  ^^^^^
    //                                            |   Sample rate (Hz)
    //                                            Buffer size (samples)
}
```

---

## Example Output

### Without Budget Tracking (Default)

```
I (12345) PERF_MONITOR: === Performance Statistics (CPU: 360 MHz) ===
I (12345) PERF_MONITOR:   codec_read              : avg=  12.30 µs (  4428 cyc)  min=  11.50 µs  max=  15.20 µs  n=6887
I (12345) PERF_MONITOR:   audio_dsp               : avg=  45.30 µs ( 16308 cyc)  min=  43.10 µs  max=  48.20 µs  n=6887
I (12345) PERF_MONITOR:   codec_write             : avg=  11.80 µs (  4248 cyc)  min=  11.20 µs  max=  14.50 µs  n=6887
I (12345) PERF_MONITOR: ============================================
```

### With Budget Tracking Enabled

```
I (12345) PERF_MONITOR: === Performance Statistics (CPU: 360 MHz) ===
I (12345) PERF_MONITOR:   codec_read              : avg=  12.30 µs (  4428 cyc)  min=  11.50 µs  max=  15.20 µs  n=6887
I (12345) PERF_MONITOR:   audio_dsp               : avg=  45.30 µs ( 16308 cyc)  min=  43.10 µs  max=  48.20 µs  n=6887
I (12345) PERF_MONITOR:   codec_write             : avg=  11.80 µs (  4248 cyc)  min=  11.20 µs  max=  14.50 µs  n=6887
I (12345) PERF_MONITOR: --------------------------------------------
I (12345) PERF_MONITOR: Audio Budget: 9.5% (69.4 µs / 725.6 µs available)
I (12345) PERF_MONITOR: CPU usage healthy
I (12345) PERF_MONITOR: ============================================
```

---

## Understanding the Output

### Budget Line Breakdown

```
Audio Budget: 42.3% (307.2 µs / 725.6 µs available)
              ^^^^   ^^^^^^   ^^^^^^
              |      |        Available time for 32 samples @ 44.1kHz
              |      Total CPU time used (sum of all avg times)
              CPU utilization percentage
```

### Status Messages

| Utilization | Status | Meaning |
|-------------|--------|---------|
| **< 50%** | `CPU usage healthy` ✅ | Plenty of headroom |
| **50-80%** | `CPU usage moderate` ⚠️ | Acceptable but monitor closely |
| **80-100%** | `High CPU usage - close to limit` ⚠️ | Risk of underruns |
| **> 100%** | `⚠️ OVERRUN! Audio processing exceeds available time!` ❌ | **Critical - will drop samples!** |

---

## Calculation Details

### Available Time Budget

```
Available Time (µs) = (Buffer Size × 1,000,000) / Sample Rate

Example (32 samples @ 44.1kHz):
  = (32 × 1,000,000) / 44,100
  = 725.6 µs
```

### Total CPU Time Used

The monitor **sums the average execution time** of all measured sections:

```
Total Time = avg(codec_read) + avg(audio_dsp) + avg(codec_write) + ...
```

**Important:** This assumes sections don't overlap in time. If sections run in parallel or nested, the sum may overestimate.

### CPU Utilization

```
Utilization (%) = (Total Time Used / Available Time) × 100
```

---

## Common Configurations

### CD Quality Audio (44.1kHz)

```cpp
// 32 samples @ 44.1kHz = 725.6 µs
PerfMonitor::EnableAudioBudgetTracking(true, 32, 44100);

// 64 samples @ 44.1kHz = 1451.2 µs
PerfMonitor::EnableAudioBudgetTracking(true, 64, 44100);

// 128 samples @ 44.1kHz = 2902.4 µs
PerfMonitor::EnableAudioBudgetTracking(true, 128, 44100);
```

### Professional Audio (48kHz)

```cpp
// 32 samples @ 48kHz = 666.7 µs
PerfMonitor::EnableAudioBudgetTracking(true, 32, 48000);

// 64 samples @ 48kHz = 1333.3 µs
PerfMonitor::EnableAudioBudgetTracking(true, 64, 48000);
```

### High-Res Audio (96kHz)

```cpp
// 32 samples @ 96kHz = 333.3 µs (tight!)
PerfMonitor::EnableAudioBudgetTracking(true, 32, 96000);
```

---

## Integration Example

### Complete Audio Task Setup

```cpp
#include "perf_monitor.hpp"
#include "codec.hpp"

using namespace CTAG::INSTRUMENTATION;

constexpr size_t BUFFER_SIZE = 32;
constexpr size_t SAMPLE_RATE = 44100;

void audio_task(void* params) {
    // Initialize performance monitoring
    PerfMonitor::EnableLogging(true, 5000);  // Log every 5 seconds
    PerfMonitor::EnableAudioBudgetTracking(true, BUFFER_SIZE, SAMPLE_RATE);
    
    float audio_buffer[BUFFER_SIZE];
    
    while (true) {
        // Measure codec read
        {
            PERF_MEASURE("codec_read");
            codec_read(audio_buffer, BUFFER_SIZE);
        }
        
        // Measure DSP processing
        {
            PERF_MEASURE("audio_dsp");
            apply_effects(audio_buffer, BUFFER_SIZE);
            run_synthesizer(audio_buffer, BUFFER_SIZE);
        }
        
        // Measure codec write
        {
            PERF_MEASURE("codec_write");
            codec_write(audio_buffer, BUFFER_SIZE);
        }
        
        // Statistics are logged automatically every 5 seconds
        // showing total budget usage
    }
}
```

---

## Disabling Budget Tracking

```cpp
// Disable budget tracking but keep performance logging
PerfMonitor::EnableAudioBudgetTracking(false);

// Or disable everything
PerfMonitor::EnableLogging(false);
PerfMonitor::EnableAudioBudgetTracking(false);
```

---

## Important Notes

### ⚠️ Limitations

1. **Assumes non-overlapping sections** - If you measure nested or parallel code, the sum will be incorrect
2. **Uses average times** - Peak times (max) are not included in budget calculation
3. **Doesn't account for overhead** - Interrupt latency, context switches not included
4. **Per-call basis** - If sections are called different numbers of times, results may not reflect actual buffer processing time

### 💡 Best Practices

1. **Measure entire audio callback** - Instrument the complete audio processing path
2. **Target < 80% utilization** - Leave headroom for jitter and system overhead
3. **Watch for overruns** - If you see >100%, you **will** get audio glitches
4. **Use larger buffers if needed** - Increases latency but gives more processing time
5. **Profile regularly** - Monitor during development to catch regressions

---

## Troubleshooting

### Problem: Budget Shows >100% But No Audio Glitches

**Possible causes:**
- Sections are nested (double-counting)
- Sections aren't called every buffer
- System is using multiple cores

**Solution:** 
- Only measure top-level functions
- Ensure measurements are per-buffer
- Check if parallel processing is happening

### Problem: Budget Shows Low But Audio Glitches Occur

**Possible causes:**
- ISR overhead not measured
- Cache misses causing occasional spikes
- Other tasks interfering

**Solution:**
- Measure interrupt handlers too
- Check `max` times vs `avg` times
- Use FreeRTOS task priorities correctly

---

## API Reference

```cpp
void PerfMonitor::EnableAudioBudgetTracking(
    bool enable,              // true to enable, false to disable
    uint32_t buffer_size,     // Samples per buffer (e.g., 32)
    uint32_t sample_rate      // Sample rate in Hz (e.g., 44100)
);
```

**Parameters:**
- `enable` - Enable or disable tracking
- `buffer_size` - Number of audio samples processed per callback
- `sample_rate` - Audio sample rate in Hz

**Example:**
```cpp
// Enable for 32-sample buffers at 44.1kHz
PerfMonitor::EnableAudioBudgetTracking(true, 32, 44100);
```

---

**Status:** ✅ Ready to use  
**Overhead:** Negligible (only affects logging, not measurement)  
**Thread-safe:** Yes (atomic operations)

---

*Feature added: December 30, 2025*  
*For use with CTAG TBD audio processing pipeline*

