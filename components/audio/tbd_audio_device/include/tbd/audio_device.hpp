#pragma once

#include <tbd/audio_device/need_pull_audio.hpp>

#include <cinttypes>


namespace tbd {

struct AudioDevice {
    AudioDevice() = delete;

    static void init();
    static void deinit();

    static void set_high_pass(bool is_enabled);
    static void set_output_levels(uint32_t left, uint32_t right);

    static void recalib_dc_offset();

    static void read_buffer(float *buf, uint32_t sz);
    static void write_buffer(float *buf, uint32_t sz);
};

}
