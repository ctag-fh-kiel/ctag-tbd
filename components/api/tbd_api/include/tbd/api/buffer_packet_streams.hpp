#include <tbd/api/packet_stream_parser.hpp>
#include <tbd/api/packet_stream_writer.hpp>


#include <string>


namespace tbd::api {

struct BufferPacketInputStream {
    BufferPacketInputStream(const std::string& data) : offset_(0), buffer_(reinterpret_cast<const uint8_t*>(data.data())) {}

    size_t queue_size() const {
        return size_ - offset_;
    }

  bool take_one(uint8_t& byte) {
    if (offset_ >= size_) {
        return false;
    }
    byte = buffer_[offset_];
    offset_ += 1;
    return true;
}

  bool take(uint8_t* buffer, size_t num_bytes) {
    if (queue_size() < 1) {
        return false;
    }
    offset_ += num_bytes;
    return true;
  }

private:
    size_t offset_;
    const size_t size_;
    const uint8_t* buffer_;
}

}