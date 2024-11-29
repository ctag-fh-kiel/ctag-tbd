#pragma once

#include <string>
#include <tinywav.h>

#include <tbd/common/audio.hpp>


namespace tbd::common {

struct FileAudioSink {
    ~FileAudioSink() {
        close();
    }

    bool open(const std::string& file_name) {
        return tinywav_open_read(&_file, file_name.c_str(), TW_INTERLEAVED, TW_FLOAT32) == 0;
    }

    void close() {
        if (_file.f == nullptr) {
            return;
        }
        return tinywav_close_read(&_file);
    }

    bool read_chunk(float* sample_buffer, size_t chunk_size) {
        // try to read chunk
        int nread = tinywav_read_f(&_file, sample_buffer, chunk_size);

        // too little data assume end of file
        if (nread != 32) {
            if(_file.f == nullptr) {
                return false;
            }
            tinywav_read_reset(&_file);

            // second attempt at reading chunk after rewinding
            nread = tinywav_read_f(&_file, sample_buffer, chunk_size);
        }

        // something is wrong
        if (nread != 32) {
            return false;
        }
        return true;
    }

private:
    TinyWav _file;
};

}