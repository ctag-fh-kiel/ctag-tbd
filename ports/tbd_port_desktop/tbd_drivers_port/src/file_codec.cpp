#include <tbd/drivers/common/codec.hpp>

#include <tbd/portutils/file_audio_source.hpp>
#include <tbd/portutils/file_audio_sink.hpp>

namespace {
    // TODO: make these runtime parameters
    std::string INPUT_FILE_PATH = "simulator/wavInput.wav";
    std::string OUTPUT_FILE_PATH = "sim_out.wav";


    tbd::common::FileAudioSource input_file;
    tbd::common::FileAudioSink output_file;
}

namespace tbd::drivers {
void Codec::init() {
    input_file.open(INPUT_FILE_PATH);
    output_file.open(OUTPUT_FILE_PATH);
}

void Codec::deinit() {
    input_file.close();
}

void Codec::HighPassEnable() {

}

void Codec::HighPassDisable() {

}

void Codec::RecalibDCOffset() {

}

void Codec::SetOutputLevels(uint32_t left, uint32_t right) {

}

void Codec::ReadBuffer(float* buf, uint32_t sz) {
    input_file.read_chunk(buf, sz);
}

void Codec::WriteBuffer(float* buf, uint32_t sz) {
    // no output
}

}
