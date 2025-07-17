#include <gtest/gtest.h>

#include <tbd/api/endpoint_index.hpp>
#include <tbd/api/events.hpp>
#include <tbd/errors.hpp>


TEST(BaseEndpoints, has_api_info_endpoint) {
    tbd::api::ApiVersion output;

    EXPECT_EQ(tbd::api::get_api_version(output), TBD_OK);
    EXPECT_EQ(output.version, 1);
    EXPECT_EQ(output.core_hash, 1011701202);
    EXPECT_EQ(output.base_hash, 636863946);
    EXPECT_NE(output.api_hash, 0);
}

TEST(BaseEndpoints, endpoint_api_info_has_id_0) {
    uint8_t output_buffer[TBD_API_MAX_PAYLOAD_SIZE];
    tbd::api::Packet dummy_request;
    size_t output_length = TBD_API_MAX_PAYLOAD_SIZE;

    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[0].path, "get_api_version");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[0].callback(dummy_request, output_buffer, output_length), TBD_OK);

    tbd::api::ApiVersion output;
    pb_istream_t stream = pb_istream_from_buffer(output_buffer, TBD_API_MAX_PAYLOAD_SIZE);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);

    EXPECT_EQ(output.version, 1);
    EXPECT_EQ(output.core_hash, 1011701202);
    EXPECT_EQ(output.base_hash, 636863946);
    EXPECT_NE(output.api_hash, 0);
}

TEST(BaseEndpoints, endpoint_update_has_id_1) {
    uint8_t output_buffer[TBD_API_MAX_PAYLOAD_SIZE];
    tbd::api::Packet dummy_request;
    size_t output_length = TBD_API_MAX_PAYLOAD_SIZE;

    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[1].path, "update_device");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[1].callback(dummy_request, output_buffer, output_length), TBD_OK);
    EXPECT_EQ(output_length, 0);
}

TEST(BaseEndpoints, endpoint_reset_has_id_2) {
    uint8_t output_buffer[TBD_API_MAX_PAYLOAD_SIZE];
    tbd::api::Packet dummy_request;
    size_t output_length = TBD_API_MAX_PAYLOAD_SIZE;

    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[2].path, "reset_device");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[2].callback(dummy_request, output_buffer, output_length), TBD_OK);
    EXPECT_EQ(output_length, 0);
}

TEST(BaseEndpoints, has_get_num_endpoints) {
    tbd::uint_par output;
    EXPECT_EQ(tbd::api::get_num_endpoints(output), TBD_OK);
    EXPECT_EQ(output, tbd::api::NUM_ENDPOINTS);
}

TEST(BaseEndpoints, endpoint_get_num_endpoints_has_id_3) {
    uint8_t output_buffer[TBD_API_MAX_PAYLOAD_SIZE];
    tbd::api:: Packet dummy_request;
    size_t output_length = TBD_API_MAX_PAYLOAD_SIZE;

    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[3].path, "get_num_endpoints");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[3].callback(dummy_request, output_buffer, output_length), TBD_OK);

    tbd::UintParWrapper output;
    pb_istream_t stream = pb_istream_from_buffer(output_buffer, TBD_API_MAX_PAYLOAD_SIZE);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);
    EXPECT_EQ(output.value, tbd::api::NUM_ENDPOINTS);
}

TEST(BaseEndpoints, has_get_endpoint_name) {
    std::string output;
    EXPECT_EQ(tbd::api::get_endpoint_name(3, output), TBD_OK);
    EXPECT_STREQ(output.c_str(), "get_num_endpoints");
}

TEST(BaseEndpoints, endpoint_get_endpoint_name_has_id_4) {
    uint8_t buffer[TBD_API_MAX_PAYLOAD_SIZE];

    size_t req_length = TBD_API_MAX_PAYLOAD_SIZE;
    const tbd::UintParWrapper req = {.value = 7};
    EXPECT_EQ(tbd::api::encode_packet(req, buffer, req_length), TBD_OK);
    tbd::api::Packet dummy_request = {};
    dummy_request.payload = buffer;
    dummy_request.payload_length = req_length;

    size_t res_length = TBD_API_MAX_PAYLOAD_SIZE;
    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[4].path, "get_endpoint_name");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[4].callback(dummy_request, buffer, res_length), TBD_OK);

    tbd::StrParWrapper output;
    pb_istream_t stream = pb_istream_from_buffer(buffer, res_length);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);
    EXPECT_STREQ(output.value.c_str(), "get_num_errors");
}

TEST(BaseEndpoints, has_get_num_events) {
    tbd::uint_par output;
    EXPECT_EQ(tbd::api::get_num_events(output), TBD_OK);
    EXPECT_EQ(output, tbd::api::NUM_EVENTS);
}

TEST(BaseEndpoints, endpoint_get_num_events_has_id_5) {
    uint8_t output_buffer[TBD_API_MAX_PAYLOAD_SIZE];
    tbd::api:: Packet dummy_request;
    size_t output_length = TBD_API_MAX_PAYLOAD_SIZE;

    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[5].path, "get_num_events");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[5].callback(dummy_request, output_buffer, output_length), TBD_OK);

    tbd::UintParWrapper output;
    pb_istream_t stream = pb_istream_from_buffer(output_buffer, TBD_API_MAX_PAYLOAD_SIZE);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);
    EXPECT_GE(output.value, tbd::api::NUM_EVENTS);
}

TEST(BaseEndpoints, has_get_event_name) {
    std::string output;
    EXPECT_EQ(tbd::api::get_event_name(0, output), TBD_OK);
    EXPECT_GT(output.length(), 0);
}

TEST(BaseEndpoints, endpoint_get_event_name_has_id_6) {
    uint8_t buffer[TBD_API_MAX_PAYLOAD_SIZE];

    size_t req_length = TBD_API_MAX_PAYLOAD_SIZE;
    const tbd::UintParWrapper req = {.value = 1};
    EXPECT_EQ(tbd::api::encode_packet(req, buffer, req_length), TBD_OK);
    tbd::api::Packet dummy_request = {};
    dummy_request.payload = buffer;
    dummy_request.payload_length = req_length;

    size_t res_length = TBD_API_MAX_PAYLOAD_SIZE;
    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[6].path, "get_event_name");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[6].callback(dummy_request, buffer, res_length), TBD_OK);

    tbd::StrParWrapper output;
    pb_istream_t stream = pb_istream_from_buffer(buffer, res_length);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);
    EXPECT_GT(output.value.length(), 0);
}

TEST(BaseEndpoints, has_get_num_errors) {
    tbd::uint_par output;
    EXPECT_EQ(tbd::api::get_num_errors(output), TBD_OK);
    EXPECT_EQ(output, tbd::errors::NUM_ERRORS);
}

TEST(BaseEndpoints, endpoint_get_num_errors_has_id_7) {
    uint8_t output_buffer[TBD_API_MAX_PAYLOAD_SIZE];
    tbd::api:: Packet dummy_request;
    size_t output_length = TBD_API_MAX_PAYLOAD_SIZE;

    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[7].path, "get_num_errors");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[7].callback(dummy_request, output_buffer, output_length), TBD_OK);

    tbd::UintParWrapper output;
    pb_istream_t stream = pb_istream_from_buffer(output_buffer, TBD_API_MAX_PAYLOAD_SIZE);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);
    EXPECT_GE(output.value, tbd::errors::NUM_ERRORS);
}

TEST(BaseEndpoints, has_get_error_name) {
    tbd::str_par output1;
    EXPECT_EQ(tbd::api::get_error_name(0, output1), TBD_OK);
    EXPECT_STREQ(output1.c_str(), "SUCCESS");

    tbd::str_par output2;
    EXPECT_EQ(tbd::api::get_error_name(1, output2), TBD_OK);
    EXPECT_STREQ(output2.c_str(), "FAILURE");
}

TEST(BaseEndpoints, endpoint_get_error_name_has_id_8) {
    uint8_t buffer[TBD_API_MAX_PAYLOAD_SIZE];

    size_t req_length = TBD_API_MAX_PAYLOAD_SIZE;
    const tbd::UintParWrapper req = {.value = 0};
    EXPECT_EQ(tbd::api::encode_packet(req, buffer, req_length), TBD_OK);
    tbd::api::Packet dummy_request = {};
    dummy_request.payload = buffer;
    dummy_request.payload_length = req_length;

    size_t res_length = TBD_API_MAX_PAYLOAD_SIZE;
    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[8].path, "get_error_name");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[8].callback(dummy_request, buffer, res_length), TBD_OK);

    tbd::StrParWrapper output;
    pb_istream_t stream = pb_istream_from_buffer(buffer, res_length);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);
    EXPECT_STREQ(output.value.c_str(), "SUCCESS");
}

TEST(BaseEndpoints, has_get_error_message) {
    tbd::str_par output1;
    EXPECT_EQ(tbd::api::get_error_message(0, output1), TBD_OK);
    EXPECT_STREQ(output1.c_str(), "no error");

    tbd::str_par output2;
    EXPECT_EQ(tbd::api::get_error_message(1, output2), TBD_OK);
    EXPECT_STREQ(output2.c_str(), "unknown error");
}

TEST(BaseEndpoints, endpoint_get_error_message_has_id_9) {
    uint8_t buffer[TBD_API_MAX_PAYLOAD_SIZE];

    size_t req_length = TBD_API_MAX_PAYLOAD_SIZE;
    const tbd::UintParWrapper req = {.value = 1};
    EXPECT_EQ(tbd::api::encode_packet(req, buffer, req_length), TBD_OK);
    tbd::api::Packet dummy_request = {};
    dummy_request.payload = buffer;
    dummy_request.payload_length = req_length;

    size_t res_length = TBD_API_MAX_PAYLOAD_SIZE;
    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[9].path, "get_error_message");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[9].callback(dummy_request, buffer, res_length), TBD_OK);

    tbd::StrParWrapper output;
    pb_istream_t stream = pb_istream_from_buffer(buffer, res_length);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);
    EXPECT_STREQ(output.value.c_str(), "unknown error");
}

TEST(BaseEndpoints, has_get_device_info) {
    tbd::api::DeviceInfo output;
    EXPECT_EQ(tbd::api::get_device_info(output), TBD_OK);
}

TEST(BaseEndpoints, endpoint_get_device_info_has_id_10) {
    uint8_t output_buffer[TBD_API_MAX_PAYLOAD_SIZE];
    tbd::api::Packet dummy_request;
    size_t output_length = TBD_API_MAX_PAYLOAD_SIZE;

    EXPECT_STREQ(tbd::api::ENDPOINT_LIST[10].path, "get_device_info");
    EXPECT_EQ(tbd::api::ENDPOINT_LIST[10].callback(dummy_request, output_buffer, output_length), TBD_OK);

    tbd::api::DeviceInfo output;
    pb_istream_t stream = pb_istream_from_buffer(output_buffer, TBD_API_MAX_PAYLOAD_SIZE);
    EXPECT_EQ(tbd::serialization::decode_message(output, stream), TBD_OK);
}