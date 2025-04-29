#pragma once

#ifndef TBD_IS_AUDIO_LOOP_COMPILATION_UNIT
    #error "audio loop is performance critical, compile in a single compilation unit"
#endif

#include <tbd/logging.hpp>
#include <tbd/system/task_module.hpp>
#include <tbd/indicator.hpp>

#include <atomic>


namespace tbd::audio {

struct SoundLevelWorker {
    void set_color(uint32_t color) {
        _color = color;
    }

    void set_blink_duration(uint32_t blink_duration) {
        _blink_duration = blink_duration;
    }

    uint32_t do_work() {
        uint32_t color = _color;
        auto r = color & 0x00FF0000;
        r >>= 16;
        auto g = color & 0x0000FF00;
        g >>= 8;
        if ((_blink_duration % 2) == 1) {
            tbd::drivers::Indicator::SetLedRGB(r, g, 0);
        } else {
            tbd::drivers::Indicator::SetLedRGB(r, g, 255);
        }
        if (_blink_duration > 1 && _blink_duration != 42) _blink_duration--; // >= 42 led blink doesn't stop
        if (_blink_duration == 42) _blink_duration = 44;
        system::Task::sleep(50); // 50ms refresh rate for led
        return 0;
    }

    uint32_t do_begin() {
        TBD_LOGI(tag, "initializing sound level worker");
        return 0;
    }

    uint32_t do_cleanup() {
        TBD_LOGI(tag, "shutting down sound level worker");
        return 0;
    }

private:
    uint32_t _data = 0;

    std::atomic<uint32_t> _blink_duration;
    std::atomic<uint32_t> _color;
};

/**
 * @brief simple scope guard that gathers sound levels and flushes them to worker task
 */
struct SoundLevel {
    SoundLevel(SoundLevelWorker& task) : _task(task), _color(0), _warning_set(false) {}

    ~SoundLevel() {
        if (_warning_set) {
            // set orange warning color
            _task.set_color(0xB39134);
            return;
        }
        // set color and intensity for next hardware refresh cycle
        _task.set_color(_color);
    }

    void add_input_level(uint32_t level) {
        _color |= level << 8; // green
    }

    void add_output_level(uint32_t level) {
        _color |= level << 16; // red
    }

    void set_warning() {

    }

    private:
        SoundLevelWorker& _task;
        uint32_t _color;
        bool _warning_set;
};

system::TaskModule<SoundLevelWorker, system::CpuCore::audio>
    sound_level_worker("sound_level_worker");    

}
