#pragma once

#include <cuchar>
#include <cinttypes>


#ifndef TBD_API_MAX_PAYLOAD_SIZE
    #error "no upper limit for api payloads set"
#endif


namespace tbd::api {

/** TBD protocol packet representation.
 *
 *  Overview
 *  --------
 *
 *  All TBD control communication over serial data stream should be wrapped in this custom packet format. Packages
 *  represent the following entities:
 *
 *  - remote procedure calls (RPCs): Requests that expect responses and optional arguments payload.
 *  - response: Responses to specific RPC invocation with error/success status and optional result payload. Responses
                can be result responses with optional payload or errors with no payload.

 *  - actions: Requests for state changes without acknowledgement/response. Actions can be dispatched to multiple
 *             listeners across multiple media.
 *  - events: Information about state changes that can be dispatched to multiple listeners on multiple serial media
 *            and thus have no dedicated response. Event can have an optional state change payload.
 *
 *  This allows for two kinds of communication:
 *
 *  1. synchronous: Invoke RPC and wait for response. Allows waiting for completion and retrying on failure.
 *  2. asynchronous: Send action and if resulting state changes need to be reflected in client state, respond to
 *                   matching events. No direct correlation or waiting for specific state changes possible.
 *
 *  Transport Format
 *  ----------------
 *
 *  For sending packets over any bi-directional binary data stream the following binary representation is used:
 *
 *  [0]: packet start (0xf0)
 *  [1]: packet type and flags
 *  [2]: handler/error (low)
 *  [3]: handler/error  (high)
 *  [4]: checksum (CRC)
 *  [5]: payload length lower
 *  [6]: payload length higher
 *  [7]: ID (low)
 *  [8]: ID (high)
 *  [9]: header end (0xe7)
 *  [10:n]: payload
 *  [n+1]: packet end (0xff)
 *
 */
struct Header {
    static constexpr uint8_t START_BYTE = 0xf0;
    static constexpr uint8_t HEADER_END_BYTE = 0xe7;
    static constexpr uint8_t END_BYTE = 0xff;

    enum HeaderBytes {
        OFFSET_START        = 0,
        OFFSET_TYPE         = 1,
        OFFSET_HANDLER_LOW  = 2,
        OFFSET_HANDLER_HIGH = 3,
        OFFSET_CRC          = 4,
        OFFSET_LENGTH_LOW   = 5,
        OFFSET_LENGTH_HIGH  = 6,
        OFFSET_ID_LOW       = 7,
        OFFSET_ID_HIGH      = 8,
        OFFSET_HEADER_END   = 9,
    };
    static constexpr size_t HEADER_SIZE = 10;

    enum EndBytes {
        OFFSET_END = 0,
    };
    static constexpr size_t END_SIZE = 1;

    static constexpr size_t CONTROL_SIZE = HEADER_SIZE + END_SIZE;
    static constexpr size_t MAX_PAYLOAD_SIZE = TBD_API_MAX_PAYLOAD_SIZE;
    static constexpr size_t BUFFER_SIZE = MAX_PAYLOAD_SIZE + CONTROL_SIZE;
    using HeaderBuffer = uint8_t[HEADER_SIZE + 1];
    using PayloadBuffer = uint8_t[MAX_PAYLOAD_SIZE + 1];
    using PacketBuffer = uint8_t[BUFFER_SIZE + 1];

    enum PacketType {
        TYPE_RPC      = 0,
        TYPE_RESPONSE = 1,
        TYPE_ERROR    = 2,
        TYPE_ACTION   = 3,
        TYPE_EVENT    = 4,
        TYPE_INVALID  = 5,
    };

    static constexpr uint8_t TYPE_MASK = 0b00000111;
    static constexpr uint8_t FLAG_MASK = 0b11111000;
    static_assert(TYPE_MASK >= TYPE_INVALID);

    PacketType type;
    uint16_t handler;
    uint8_t crc;
    uint16_t payload_length;
    uint16_t id;

    size_t total_size() const {
        return HEADER_SIZE + payload_length + END_SIZE;
    }
};

struct Packet : Header {
    const uint8_t* payload;

    void set_error(const uint16_t error, const uint16_t request_id) {
        type = TYPE_ERROR;
        handler = error;
        crc = 0;
        payload_length = 0;
        id = request_id;
        payload = nullptr;
    }
};

}