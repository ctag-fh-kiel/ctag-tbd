#include <gtest/gtest.h>

#include <tbd/api/api_adapters.hpp>

#include "tbd/api/endpoints.hpp"
#include "tbd/api/packet_transcoding.hpp"

TEST(TestApiAdapter, checks_request_type) {
    const tbd::api::Packet request {
                {
                    .type = tbd::api::Header::TYPE_EVENT,
                    .handler = 1,
                    .crc = 0,
                    .payload_length = 0,
                    .id = 23,
                },
                nullptr,
            };
    tbd::api::Packet response = {};

    EXPECT_EQ(tbd::api::impl::handle_rpc(request, response, nullptr, 0), TBD_OK);
    EXPECT_EQ(response.type, tbd::api::Header::TYPE_ERROR);
    EXPECT_EQ(response.handler, TBD_ERR(API_WRONG_PACKET_TYPE));
    EXPECT_EQ(response.crc, 0);
    EXPECT_EQ(response.payload_length, 0);
    EXPECT_EQ(response.id, 23);
}

TEST(TestApiAdapter, checks_request_in_range) {
    const tbd::api::Packet request {
                {
                    .type = tbd::api::Header::TYPE_RPC,
                    .handler = tbd::api::NUM_ENDPOINTS,
                    .crc = 0,
                    .payload_length = 0,
                    .id = 9,
                },
                nullptr,
            };
    tbd::api::Packet response = {};

    EXPECT_EQ(tbd::api::impl::handle_rpc(request, response, nullptr, 0), TBD_OK);
    EXPECT_EQ(response.type, tbd::api::Header::TYPE_ERROR);
    EXPECT_EQ(response.handler, TBD_ERR(API_BAD_ENDPOINT));
    EXPECT_EQ(response.crc, 0);
    EXPECT_EQ(response.payload_length, 0);
    EXPECT_EQ(response.id, 9);
}

TEST(TestApiAdapter, checks_buffer_size_before_writing) {
    tbd::api::Packet request {
                {
                    .type = tbd::api::Header::TYPE_RPC,
                    .handler = 0,
                    .crc = 0,
                    .payload_length = 0,
                    .id = 17,
                },
                nullptr,
            };
    tbd::api::Packet response = {};

    EXPECT_EQ(tbd::api::impl::handle_rpc(request, response, nullptr, 0), TBD_OK);

    EXPECT_EQ(response.type, tbd::api::Header::TYPE_ERROR);
    EXPECT_EQ(response.handler, TBD_ERR(API_RESPONSE_BUFFER_SIZE));
    EXPECT_EQ(response.crc, 0);
    EXPECT_EQ(response.payload_length, 0);
    EXPECT_EQ(response.id, 17);
}

TEST(TestApiAdapter, handles_trigger_rpcs) {
    const tbd::api::Packet request {
            {
                .type = tbd::api::Header::TYPE_RPC,
                .handler = 1,
                .crc = 0,
                .payload_length = 0,
                .id = 17,
            },
            nullptr,
        };
    tbd::api::Packet response = {};

    EXPECT_EQ(tbd::api::impl::handle_rpc(request, response, nullptr, 0), TBD_OK);

    EXPECT_EQ(response.type, tbd::api::Header::TYPE_RESPONSE);
    EXPECT_EQ(response.handler, 0);
    EXPECT_EQ(response.crc, 0);
    EXPECT_EQ(response.payload_length, 0);
    EXPECT_EQ(response.id, 17);
}

TEST(TestApiAdapter, handles_getter_rpcs) {
    const tbd::api::Packet request {
            {
                .type = tbd::api::Header::TYPE_RPC,
                .handler = 0,
                .crc = 0,
                .payload_length = 0,
                .id = 2342,
            },
            nullptr,
        };
    tbd::api::Packet response = {};


    uint8_t buffer[TBD_API_MAX_PAYLOAD_SIZE];
    EXPECT_EQ(tbd::api::impl::handle_rpc(request, response, buffer, TBD_API_MAX_PAYLOAD_SIZE), TBD_OK);

    EXPECT_EQ(response.type, tbd::api::Header::TYPE_RESPONSE);
    EXPECT_EQ(response.handler, 0);
    EXPECT_EQ(response.crc, 0);
    EXPECT_GT(response.payload_length, 0);
    EXPECT_EQ(response.id, 2342);

    tbd::api::ApiVersion resp_dto;
    tbd::api::decode_packet(resp_dto, response);
    EXPECT_EQ(resp_dto.version, tbd::api::API_VERSION);
    EXPECT_EQ(resp_dto.core_hash, tbd::api::CORE_API_HASH);
    EXPECT_EQ(resp_dto.base_hash, tbd::api::BASE_API_HASH);
    EXPECT_EQ(resp_dto.api_hash, tbd::api::API_HASH);
}

TEST(TestApiAdapter, handles_call_rpcs) {
    uint8_t buffer[TBD_API_MAX_PAYLOAD_SIZE];
    size_t length = TBD_API_MAX_PAYLOAD_SIZE;
    const tbd::UintParWrapper req_dto = { .value = 8 };
    EXPECT_EQ(tbd::api::encode_packet(req_dto, buffer, length), TBD_OK);

    const tbd::api::Packet request {
                    {
                        .type = tbd::api::Header::TYPE_RPC,
                        .handler = 4,
                        .crc = 0,
                        .payload_length = static_cast<uint16_t>(length),
                        .id = 9937,
                    },
                    buffer,
                };
    tbd::api::Packet response = {};

    EXPECT_EQ(tbd::api::impl::handle_rpc(request, response, buffer, TBD_API_MAX_PAYLOAD_SIZE), TBD_OK);

    EXPECT_EQ(response.type, tbd::api::Header::TYPE_RESPONSE);
    EXPECT_EQ(response.handler, 0);
    EXPECT_EQ(response.crc, 0);
    EXPECT_GT(response.payload_length, 0);
    EXPECT_EQ(response.id, 9937);

    tbd::StrParWrapper resp_dto;
    tbd::api::decode_packet(resp_dto, response);
    EXPECT_STREQ(resp_dto.value.c_str(), "get_error_name");
}