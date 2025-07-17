#pragma once

#include <string>
#include <tinywav.h>

#include <tbd/logging.hpp>
#include <tbd/common/audio.hpp>



namespace tbd::common {

struct FileAudioSink {
    ~FileAudioSink() {
        close();
    }

    bool open(const std::string& file_name) {
        return tinywav_open_write(&_file, 2, TBD_SAMPLE_RATE,
            TW_FLOAT32, TW_INTERLEAVED, file_name.c_str()) == 0;
    }

    void close() {
        if (_file.f == nullptr) {
            return;
        }
        return tinywav_close_write(&_file);
    }

    bool write_chunk(float* sample_buffer, size_t chunk_size) {
        auto nwrite = tinywav_write_f(&_file, sample_buffer, chunk_size);
        if (nwrite != (chunk_size * _file.numChannels)) {
            TBD_LOGV("portutils", "error writing to audio file");
        }
        return true;
    }

private:
    TinyWav _file;
};

}