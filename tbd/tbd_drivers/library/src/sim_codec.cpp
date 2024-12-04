#include <tbd/drivers/codec.hpp>

namespace {

constexpr char* tag = "sim_codec";

}

namespace tbd::drivers {

void Codec::init() {

}

void Codec::deinit() {

}

void Codec::HighPassEnable() {

}

void Codec::HighPassDisable() {

}

void Codec::RecalibDCOffset() {

}

void Codec::SetOutputLevels(uint32_t left, uint32_t right) {

}

void Codec::ReadBuffer(float *buf, uint32_t sz) {

}

void Codec::WriteBuffer(float *buf, uint32_t sz) {

}

}