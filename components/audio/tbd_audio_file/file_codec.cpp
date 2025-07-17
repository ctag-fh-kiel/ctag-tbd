#include <tbd/drivers/common/file_codec.hpp>

#include <tbd/drivers/common/utils/file_audio_source.hpp>
#include <tbd/drivers/common/utils/file_audio_sink.hpp>

namespace {
    // TODO: make these runtime parameters
    std::string _source_path = "simulator/wavInput.wav";
    std::string _sink_path = "sim_out.wav";


    tbd::common::FileAudioSource input_file;
    tbd::common::FileAudioSink output_file;
}

namespace tbd::driver::file_codec {

void set_source(const std::string& source_path) {
    _source_path = source_path;
}

void set_sink(const std::string& sink_path) {
    _sink_path = sink_path;
}

}

namespace tbd::drivers {
void Codec::init() {
    input_file.open(_source_path);
    output_file.open(_sink_path);
}

void Codec::deinit() {
    input_file.close();
    output_file.close();
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
    output_file.write_chunk(buf, sz);
}

}
