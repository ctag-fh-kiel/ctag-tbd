/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

Lock-free atomic performance monitoring component - Implementation

(c) 2025 by Robert Manzke. All rights reserved.
***************/

#include "perf_monitor.hpp"
#include "esp_cpu.h"
#include "esp_log.h"
#include "esp_private/esp_clk.h"
#include <cstring>

namespace CTAG {
namespace INSTRUMENTATION {

static const char* TAG = "PERF_MONITOR";

// Static registry initialization
PerfMonitor::SectionRegistry PerfMonitor::registry_;

// Section constructor
PerfMonitor::Section::Section(const char* section_name) {
    strncpy(name, section_name, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
}

// Get or create a named section
PerfMonitor::Section& PerfMonitor::GetSection(const char* name) {
    // First, try to find existing section
    uint32_t count = registry_.count.load(std::memory_order_relaxed);
    for (uint32_t i = 0; i < count; i++) {
        if (strcmp(registry_.sections[i].name, name) == 0) {
            return registry_.sections[i];
        }
    }

    // Not found, create new section
    uint32_t old_count = count;
    uint32_t new_count = count + 1;

    // Try to atomically increment count
    if (registry_.count.compare_exchange_strong(old_count, new_count,
                                               std::memory_order_relaxed)) {
        if (new_count <= MAX_SECTIONS) {
            // Successfully reserved a slot
            Section& section = registry_.sections[old_count];
            strncpy(section.name, name, sizeof(section.name) - 1);
            section.name[sizeof(section.name) - 1] = '\0';
            section.stats.Reset();
            return section;
        } else {
            // Too many sections
            ESP_LOGE(TAG, "Maximum sections (%d) exceeded! Cannot create '%s'",
                     MAX_SECTIONS, name);
            registry_.count.fetch_sub(1, std::memory_order_relaxed);
            // Return first section as fallback
            return registry_.sections[0];
        }
    } else {
        // Another thread created a section, try again
        return GetSection(name);
    }
}

// Enable/disable all sections
void PerfMonitor::EnableAll(bool enable) {
    uint32_t count = registry_.count.load(std::memory_order_relaxed);
    for (uint32_t i = 0; i < count; i++) {
        registry_.sections[i].stats.enabled.store(enable, std::memory_order_relaxed);
    }
    ESP_LOGI(TAG, "All sections %s", enable ? "ENABLED" : "DISABLED");
}

// Reset all sections
void PerfMonitor::ResetAll() {
    uint32_t count = registry_.count.load(std::memory_order_relaxed);
    for (uint32_t i = 0; i < count; i++) {
        registry_.sections[i].stats.Reset();
    }
    ESP_LOGI(TAG, "All sections reset");
}

// Get statistics (non-blocking atomic read)
void PerfMonitor::GetStats(const Section& section,
                           uint32_t& avg_cycles,
                           uint32_t& min_cycles,
                           uint32_t& max_cycles,
                           uint32_t& count) {
    // Read atomically (truly atomic on 32-bit RISC-V)
    uint32_t total = section.stats.total_cycles.load(std::memory_order_relaxed);
    count = section.stats.call_count.load(std::memory_order_relaxed);
    min_cycles = section.stats.min_cycles.load(std::memory_order_relaxed);
    max_cycles = section.stats.max_cycles.load(std::memory_order_relaxed);

    // Calculate average
    avg_cycles = (count > 0) ? (total / count) : 0;
}

// Convert cycles to microseconds
float PerfMonitor::CyclesToMicroseconds(uint32_t cycles) {
    uint32_t freq = esp_clk_cpu_freq();
    return (float)cycles * 1000000.0f / (float)freq;
}

// Get CPU frequency
uint32_t PerfMonitor::GetCpuFrequency() {
    return esp_clk_cpu_freq();
}

// Logging task implementation
void PerfMonitor::LoggingTask(void* arg) {
    uint32_t interval_ms = registry_.log_interval_ms;
    const TickType_t interval_ticks = pdMS_TO_TICKS(interval_ms);

    ESP_LOGI(TAG, "Logging task started (interval: %lu ms, core: %d)",
             interval_ms, xPortGetCoreID());

    while (registry_.logging_enabled.load(std::memory_order_relaxed)) {
        vTaskDelay(interval_ticks);

        uint32_t section_count = registry_.count.load(std::memory_order_relaxed);

        if (section_count == 0) {
            ESP_LOGW(TAG, "No sections registered");
            continue;
        }

        // Print header
        ESP_LOGI(TAG, "=== Performance Statistics (CPU: %lu MHz) ===",
                 GetCpuFrequency() / 1000000);

        // Print each section
        for (uint32_t i = 0; i < section_count; i++) {
            Section& section = registry_.sections[i];

            if (!section.stats.enabled.load(std::memory_order_relaxed)) {
                continue; // Skip disabled sections
            }

            uint32_t avg, min, max;
            uint32_t count;
            GetStats(section, avg, min, max, count);

            if (count > 0) {
                float avg_us = CyclesToMicroseconds(avg);
                float min_us = CyclesToMicroseconds(min);
                float max_us = CyclesToMicroseconds(max);

                ESP_LOGI(TAG, "  %-24s: avg=%7.2f µs (%6lu cyc)  "
                         "min=%7.2f µs  max=%7.2f µs  n=%lu",
                         section.name, avg_us, avg, min_us, max_us, count);
            } else {
                ESP_LOGI(TAG, "  %-24s: (no data)", section.name);
            }
        }

        ESP_LOGI(TAG, "============================================");

        // Reset statistics for next window
        for (uint32_t i = 0; i < section_count; i++) {
            registry_.sections[i].stats.Reset();
        }
    }

    ESP_LOGI(TAG, "Logging task stopped");
    registry_.logging_task = nullptr;
    vTaskDelete(nullptr);
}

// Enable/disable logging
void PerfMonitor::EnableLogging(bool enable,
                               uint32_t interval_ms,
                               BaseType_t core_id,
                               UBaseType_t priority) {
    if (enable && !registry_.logging_enabled.load(std::memory_order_relaxed)) {
        // Start logging
        registry_.log_interval_ms = interval_ms;
        registry_.logging_enabled.store(true, std::memory_order_relaxed);

        if (registry_.logging_task == nullptr) {
            BaseType_t result = xTaskCreatePinnedToCore(
                LoggingTask,
                "perf_log",
                4096,
                nullptr,
                priority,
                &registry_.logging_task,
                core_id
            );

            if (result == pdPASS) {
                ESP_LOGI(TAG, "Performance logging ENABLED (interval: %lu ms, core: %ld, priority: %lu)",
                         interval_ms, core_id, priority);
            } else {
                ESP_LOGE(TAG, "Failed to create logging task!");
                registry_.logging_enabled.store(false, std::memory_order_relaxed);
            }
        }
    } else if (!enable && registry_.logging_enabled.load(std::memory_order_relaxed)) {
        // Stop logging
        registry_.logging_enabled.store(false, std::memory_order_relaxed);
        ESP_LOGI(TAG, "Performance logging DISABLED");
        // Task will exit on next iteration
    }
}

} // namespace INSTRUMENTATION
} // namespace CTAG

