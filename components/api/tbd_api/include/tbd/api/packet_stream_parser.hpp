#pragma once

#include <tbd/api/module.hpp>
#include <tbd/api/packet_parser.hpp>

#include <tbd/logging.hpp>
#include <tbd/errors.hpp>

#ifdef __cpp_concepts
#include <concepts>
#endif


// on success continue with next step without turning back control to main loop
#define tbd_packet_parser_step(state, new_state, call) do { \
    if(state_ == state) { \
        auto status = call; \
        if (status == WAIT) { \
            return false; \
        } else if (status == DONE) { \
            state_ = new_state; \
        } else { \
            reset_state(); \
            return false; \
        } \
    } \
} while(0)

namespace tbd::api {

#ifdef __cpp_concepts

/**
 *
 */
template<class ImplT>
concept PacketInputStream = requires(
    ImplT impl,
    const ImplT const_impl,
    size_t _size_t,
    uint8_t _uint8_t,
    uint8_t* _uint8_t_ptr
) {
    { const_impl.queue_size() } -> std::same_as<size_t>;
    { impl.skip_until(_uint8_t) } -> std::same_as<bool>;
    { impl.take(_uint8_t_ptr, _size_t) } -> std::same_as<bool>;
};
#else
#define PacketInputStream class
#endif

template<PacketInputStream StreamT>
struct PacketStreamParser : PacketBufferParser {
    explicit PacketStreamParser(StreamT& stream) : PacketBufferParser(buffer_, Packet::BUFFER_SIZE), stream_(stream) {
        reset_state();
    }

    bool do_work() {
        return _do_work();
    }

private:
    enum StepStatus {
        DONE,
        WAIT,
        FAILED,
    };

    enum ReadState {
        UNKNOWN              = 0,
        EXPECT_PACKET_START  = 1,
        READ_PAYLOAD         = 2,
        EXPECT_PACKET_END    = 3,
        INVALID              = 4,
    } state_;

    StreamT& stream_;

    bool _do_work() {
        if (state_ >= INVALID) {
            TBD_LOGE(tag, "invalid state %i", state_);
            state_ = UNKNOWN;
        }

        if (state_ == UNKNOWN) {
            if (!stream_.skip_until(Packet::START_BYTE)) {
                return false;
            }
            state_ = EXPECT_PACKET_START;
        }

        tbd_packet_parser_step(EXPECT_PACKET_START, READ_PAYLOAD, fetch_header());
        tbd_packet_parser_step(READ_PAYLOAD, EXPECT_PACKET_END, fetch_payload());
        tbd_packet_parser_step(EXPECT_PACKET_END, EXPECT_PACKET_START, check_end());

        // yield with new parsed packet
        return true;
    }

    void reset_state() {
        state_ = UNKNOWN;
        packet_.type = Packet::TYPE_INVALID;
    }

    StepStatus fetch_header() {
        if (stream_.queue_size() < Packet::HEADER_SIZE) {
            return WAIT;
        }

        if (uint8_t* buffer = buffer_; !stream_.take(buffer, Packet::HEADER_SIZE)) {
            TBD_LOGE(tag, "failed to read header data from stream");
            return FAILED;
        }

        if (!parse_header()) {
            TBD_LOGE(tag, "parse error in header");
            return FAILED;
        }
        return DONE;
    }

    StepStatus fetch_payload() {
        if (stream_.queue_size() < packet_.payload_length) {
            return WAIT;
        }

        if (packet_.payload_length == 0) {
            packet_.payload = nullptr;
            return DONE;
        }

        uint8_t* buffer = buffer_ + Packet::HEADER_SIZE;
        if (!stream_.take(buffer, packet_.payload_length)) {
            TBD_LOGE(tag, "failed to read header data from stream");
            return FAILED;
        }
        packet_.payload = buffer;
        return DONE;
    }

    StepStatus check_end() {
        if (stream_.queue_size() < 1) {
            return WAIT;
        }

        uint8_t next_byte;
        if (stream_.take(&next_byte, 1) && next_byte == Packet::END_BYTE) {
            return DONE;
        }
        TBD_LOGE(tag, "expected end byte, got %i", next_byte);
        return FAILED;
    }

    Packet::PayloadBuffer buffer_;
};

#ifdef PacketInputStream
#undef PacketInputStream
#endif

}

