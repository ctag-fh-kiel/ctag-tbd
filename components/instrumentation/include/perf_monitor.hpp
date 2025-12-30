/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

Lock-free atomic performance monitoring component for real-time systems.

(c) 2025 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

CTAG TBD is provided "as is" without any express or implied warranties.
***************/

#pragma once

#include <atomic>
#include <cstdint>
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace CTAG {
namespace INSTRUMENTATION {

/**
 * @brief Lock-free atomic performance monitor for real-time code
 *
 * This class provides cycle-accurate performance measurement using esp_cpu_get_cycle_count()
 * with lock-free atomic operations. Safe to use in IRAM interrupt handlers and real-time
 * audio callbacks.
 *
 * Features:
 * - Zero locks (std::atomic operations only)
 * - Minimal overhead (<10 cycles when enabled, <3 cycles when disabled)
 * - Tracks min/max/average/total cycles
 * - Separate logging thread for non-blocking statistics output
 * - Can monitor multiple code sections independently
 *
 * Usage:
 * @code
 * // Create monitor for a section
 * auto& monitor = PerfMonitor::GetSection("audio_process");
 *
 * // Measure execution
 * {
 *     PerfMonitor::ScopedMeasure measure(monitor);
 *     // ... your code here ...
 * } // Automatically records cycles
 *
 * // Or manual measurement
 * monitor.Begin();
 * // ... your code ...
 * monitor.End();
 *
 * // Enable logging
 * PerfMonitor::EnableLogging(true, 5000); // Log every 5 seconds
 * @endcode
 */
class PerfMonitor {
public:
    /**
     * @brief Statistics for a monitored section
     *
     * NOTE: Uses 32-bit atomics for true lock-free operation on 32-bit RISC-V.
     * On 32-bit architectures, 64-bit atomics require locks/sequences internally.
     */
    struct Stats {
        // Use 32-bit for truly atomic operations on 32-bit architecture
        std::atomic<uint32_t> total_cycles{0};
        std::atomic<uint32_t> min_cycles{UINT32_MAX};
        std::atomic<uint32_t> max_cycles{0};
        std::atomic<uint32_t> call_count{0};
        std::atomic<bool> enabled{true};

        // Thread-local state (not atomic, used only by measuring thread)
        uint32_t start_cycles{0};

        void Reset() {
            total_cycles.store(0, std::memory_order_relaxed);
            min_cycles.store(UINT32_MAX, std::memory_order_relaxed);
            max_cycles.store(0, std::memory_order_relaxed);
            call_count.store(0, std::memory_order_relaxed);
        }
    };

    /**
     * @brief Named section for performance monitoring
     */
    struct Section {
        char name[32];
        Stats stats;

        Section() : name{0} {}
        explicit Section(const char* section_name);
    };

    /**
     * @brief RAII helper for automatic measurement
     *
     * Usage:
     * @code
     * void myFunction() {
     *     PerfMonitor::ScopedMeasure m(PerfMonitor::GetSection("myFunc"));
     *     // ... code to measure ...
     * } // Measurement ends automatically
     * @endcode
     */
    class ScopedMeasure {
    public:
        explicit ScopedMeasure(Section& section)
            : section_(section)
        {
            if (section_.stats.enabled.load(std::memory_order_relaxed)) {
                section_.stats.start_cycles = esp_cpu_get_cycle_count();
            }
        }

        ~ScopedMeasure() {
            if (section_.stats.enabled.load(std::memory_order_relaxed)) {
                uint32_t end = esp_cpu_get_cycle_count();
                uint32_t cycles = end - section_.stats.start_cycles;
                RecordCycles(section_.stats, cycles);
            }
        }

    private:
        Section& section_;

        // Non-copyable
        ScopedMeasure(const ScopedMeasure&) = delete;
        ScopedMeasure& operator=(const ScopedMeasure&) = delete;
    };

    /**
     * @brief Get or create a named section for monitoring
     *
     * @param name Section name (max 31 characters)
     * @return Reference to the section
     *
     * Thread-safe: Yes (uses atomic operations)
     * Can be called from IRAM: Yes
     */
    static Section& GetSection(const char* name);

    /**
     * @brief Manually start measurement for a section
     *
     * @param section Section to measure
     *
     * Must call End() to complete measurement.
     * Use ScopedMeasure for automatic RAII-style measurement.
     */
    static inline void Begin(Section& section) {
        if (section.stats.enabled.load(std::memory_order_relaxed)) {
            section.stats.start_cycles = esp_cpu_get_cycle_count();
        }
    }

    /**
     * @brief Manually end measurement for a section
     *
     * @param section Section being measured
     *
     * Records the elapsed cycles since Begin() was called.
     */
    static inline void End(Section& section) {
        if (section.stats.enabled.load(std::memory_order_relaxed)) {
            uint32_t end = esp_cpu_get_cycle_count();
            uint32_t cycles = end - section.stats.start_cycles;
            RecordCycles(section.stats, cycles);
        }
    }

    /**
     * @brief Enable/disable a specific section
     *
     * @param section Section to configure
     * @param enable true to enable, false to disable
     *
     * When disabled, Begin/End/ScopedMeasure have minimal overhead (~3 cycles).
     */
    static void EnableSection(Section& section, bool enable) {
        section.stats.enabled.store(enable, std::memory_order_relaxed);
    }

    /**
     * @brief Enable/disable all sections
     *
     * @param enable true to enable, false to disable
     */
    static void EnableAll(bool enable);

    /**
     * @brief Reset statistics for a section
     *
     * @param section Section to reset
     */
    static void ResetSection(Section& section) {
        section.stats.Reset();
    }

    /**
     * @brief Reset statistics for all sections
     */
    static void ResetAll();

    /**
     * @brief Enable automatic logging to console
     *
     * @param enable true to start logging thread, false to stop
     * @param interval_ms Logging interval in milliseconds (default: 5000)
     * @param core_id CPU core for logging thread (default: 0)
     * @param priority FreeRTOS priority (default: 1, very low)
     *
     * The logging thread runs independently and never blocks the measurement code.
     * Logs show avg/min/max cycles for each section.
     */
    static void EnableLogging(bool enable,
                            uint32_t interval_ms = 5000,
                            BaseType_t core_id = 0,
                            UBaseType_t priority = 1);

    /**
     * @brief Enable audio budget tracking and reporting
     *
     * @param enable true to enable budget tracking
     * @param buffer_size Number of samples per buffer (e.g., 32)
     * @param sample_rate Sample rate in Hz (e.g., 44100)
     *
     * When enabled, the logging output will show:
     * - Total cycles used by all monitored sections
     * - Total time in microseconds
     * - Available time budget for audio processing
     * - CPU utilization percentage
     * - Budget overrun statistics (count and percentage)
     *
     * Example:
     * @code
     * PerfMonitor::EnableAudioBudgetTracking(true, 32, 44100);
     * // Shows: "Audio Budget: 42.3% (307.2µs / 725.6µs available)"
     * //        "Budget Overruns: 5 / 120 windows (4.2%)"
     * @endcode
     */
    static void EnableAudioBudgetTracking(bool enable, uint32_t buffer_size = 32, uint32_t sample_rate = 44100);

    /**
     * @brief Get audio budget overrun statistics
     *
     * @param[out] overrun_count Number of measurement windows that exceeded budget
     * @param[out] total_windows Total number of measurement windows
     * @return Overrun percentage (0.0 to 100.0)
     *
     * Example:
     * @code
     * uint32_t overruns, total;
     * float pct = PerfMonitor::GetBudgetOverrunStats(overruns, total);
     * ESP_LOGI("APP", "Overruns: %lu/%lu (%.1f%%)", overruns, total, pct);
     * @endcode
     */
    static float GetBudgetOverrunStats(uint32_t& overrun_count, uint32_t& total_windows);

    /**
     * @brief Reset budget overrun statistics
     *
     * Resets the overrun counters to zero. Useful when you want to measure
     * a fresh period after making optimizations.
     */
    static void ResetBudgetOverrunStats();

    /**
     * @brief Get current statistics for a section (non-blocking)
     *
     * @param section Section to query
     * @param[out] avg_cycles Average cycles per call
     * @param[out] min_cycles Minimum cycles observed
     * @param[out] max_cycles Maximum cycles observed
     * @param[out] count Number of calls
     *
     * Values are read atomically without blocking the measurement thread.
     * Uses 32-bit values for true lock-free atomicity on 32-bit RISC-V.
     */
    static void GetStats(const Section& section,
                        uint32_t& avg_cycles,
                        uint32_t& min_cycles,
                        uint32_t& max_cycles,
                        uint32_t& count);

    /**
     * @brief Convert cycles to microseconds
     *
     * @param cycles Cycle count (32-bit for true atomic operations)
     * @return Microseconds (assumes esp_clk_cpu_freq())
     */
    static float CyclesToMicroseconds(uint32_t cycles);

    /**
     * @brief Get CPU frequency in Hz
     *
     * @return CPU frequency
     */
    static uint32_t GetCpuFrequency();

private:
    static constexpr size_t MAX_SECTIONS = 32;

    struct SectionRegistry {
        Section sections[MAX_SECTIONS];
        std::atomic<uint32_t> count{0};
        std::atomic<bool> logging_enabled{false};
        TaskHandle_t logging_task{nullptr};
        uint32_t log_interval_ms{5000};

        // Audio budget tracking (optional)
        std::atomic<bool> track_audio_budget{false};
        uint32_t audio_buffer_size{32};        // Default: 32 samples
        uint32_t audio_sample_rate{44100};     // Default: 44.1kHz

        // Budget overrun statistics
        std::atomic<uint32_t> budget_overrun_count{0};      // Number of windows with overruns
        std::atomic<uint32_t> total_measurement_windows{0}; // Total measurement windows
    };

    static SectionRegistry registry_;

    // Lock-free atomic update of min/max/total (32-bit for true atomicity)
    static void RecordCycles(Stats& stats, uint32_t cycles) {
        // Update total and count
        stats.total_cycles.fetch_add(cycles, std::memory_order_relaxed);
        stats.call_count.fetch_add(1, std::memory_order_relaxed);

        // Update minimum using compare-exchange
        uint32_t current_min = stats.min_cycles.load(std::memory_order_relaxed);
        while (cycles < current_min &&
               !stats.min_cycles.compare_exchange_weak(current_min, cycles,
                   std::memory_order_relaxed));

        // Update maximum using compare-exchange
        uint32_t current_max = stats.max_cycles.load(std::memory_order_relaxed);
        while (cycles > current_max &&
               !stats.max_cycles.compare_exchange_weak(current_max, cycles,
                   std::memory_order_relaxed));
    }

    // Logging task
    static void LoggingTask(void* arg);
};

/**
 * @brief Convenience macro for named scoped measurement
 *
 * Usage:
 * @code
 * void myFunction() {
 *     PERF_MEASURE("myFunction");
 *     // ... code to measure ...
 * }
 * @endcode
 */
#define PERF_MEASURE(name) \
    CTAG::INSTRUMENTATION::PerfMonitor::ScopedMeasure \
    __perf_measure_##__LINE__(CTAG::INSTRUMENTATION::PerfMonitor::GetSection(name))

/**
 * @brief Convenience macro for function-scoped measurement
 *
 * Automatically uses __FUNCTION__ as section name.
 *
 * Usage:
 * @code
 * void myFunction() {
 *     PERF_MEASURE_FUNCTION();
 *     // ... code to measure ...
 * }
 * @endcode
 */
#define PERF_MEASURE_FUNCTION() PERF_MEASURE(__FUNCTION__)

} // namespace INSTRUMENTATION
} // namespace CTAG

