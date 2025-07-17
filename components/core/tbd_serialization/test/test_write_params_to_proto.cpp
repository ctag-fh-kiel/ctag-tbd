#include <gtest/gtest.h>

#include <tbd/serialization/message_encoding.hpp>
#include <tbd/errors.hpp>

using namespace tbd;
using namespace tbd::serialization;

namespace {

const uint8_t INT_POS_DATA[] = {0x40, 0xa};
const size_t INT_POS_LENGTH = 2;
const uint8_t INT_NEG_DATA[] = {0xe0, 0x1, 0xcb, 0x24};
const size_t INT_NEG_LENGTH = 4;
const uint8_t UINT_DATA[] = {0x78, 0xb1, 0xc1, 0x5};
const size_t UINT_LENGTH = 4;
const uint8_t FLOAT_POS_DATA[] = {0x25, 0xa0, 0xfd, 0x4e, 0x40};
const size_t FLOAT_POS_LENGTH = 5;
const uint8_t FLOAT_NEG_DATA[] = {0x8d, 0x2, 0x9a, 0x99, 0xd9, 0xbf};
const size_t FLOAT_NEG_LENGTH = 6;
const uint8_t BOOL_TRUE_DATA[] = {0x60, 0x1};
const size_t BOOL_TRUE_LENGTH = 2;
const uint8_t BOOL_FALSE_DATA[] = {0x60, 0x0};
const size_t BOOL_FALSE_LENGTH = 2;
const uint8_t STRING_DATA[] = {0xaa, 0x1, 0x1a, 0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x73, 0x6f, 0x6d, 0x65, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x20, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x79};
const size_t STRING_LENGTH = 29;
const uint8_t STRING_EMPTY_DATA[] =  {0x3a, 0x0};
const size_t STRING_EMPTY_LENGTH = 2;

}


TEST(WriteParamsToProto, can_serialize_positive_int_params) {
    const size_t buffer_size = INT_POS_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::INT_PARAM>(8, 5, stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, INT_POS_LENGTH);
    for (size_t i = 0; i < INT_POS_LENGTH; i++) {
        EXPECT_EQ(buffer[i], INT_POS_DATA[i]);
    }
}


TEST(WriteParamsToProto, can_serialize_negative_int_params) {
    const size_t buffer_size = INT_NEG_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::INT_PARAM>(28, -2342, stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, INT_NEG_LENGTH);
    for (size_t i = 0; i < INT_NEG_LENGTH; i++) {
        EXPECT_EQ(buffer[i], INT_NEG_DATA[i]);
    }
}


TEST(WriteParamsToProto, can_serialize_uint_params) {
    const size_t buffer_size = UINT_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::UINT_PARAM>(15, 90289, stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, UINT_LENGTH);
    for (size_t i = 0; i < UINT_LENGTH; i++) {
        EXPECT_EQ(buffer[i], UINT_DATA[i]);
    }
}

TEST(WriteParamsToProto, can_serialize_positive_float_params) {
    const size_t buffer_size = FLOAT_POS_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::FLOAT_PARAM>(4, 3.23423, stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, FLOAT_POS_LENGTH);
    for (size_t i = 0; i < FLOAT_POS_LENGTH; i++) {
        EXPECT_EQ(buffer[i], FLOAT_POS_DATA[i]);
    }
}

TEST(WriteParamsToProto, can_serialize_negative_float_params) {
    const size_t buffer_size = FLOAT_NEG_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::FLOAT_PARAM>(33, -1.7, stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, FLOAT_NEG_LENGTH);
    for (size_t i = 0; i < FLOAT_NEG_LENGTH; i++) {
        EXPECT_EQ(buffer[i], FLOAT_NEG_DATA[i]);
    }
}

TEST(WriteParamsToProto, can_serialize_true_value_bool_params) {
    const size_t buffer_size = BOOL_TRUE_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::TRIGGER_PARAM>(12, true, stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, BOOL_TRUE_LENGTH);
    for (size_t i = 0; i < BOOL_TRUE_LENGTH; i++) {
        EXPECT_EQ(buffer[i], BOOL_TRUE_DATA[i]);
    }
}

TEST(WriteParamsToProto, can_serialize_false_value_bool_params) {
    const size_t buffer_size = BOOL_FALSE_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::TRIGGER_PARAM>(12, false, stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, BOOL_FALSE_LENGTH);
    for (size_t i = 0; i < BOOL_FALSE_LENGTH; i++) {
        EXPECT_EQ(buffer[i], BOOL_FALSE_DATA[i]);
    }
}

TEST(WriteParamsToProto, can_serialize_string_params) {
    const size_t buffer_size = STRING_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::STR_PARAM>(21, "this is some string thingy", stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, STRING_LENGTH);
    for (size_t i = 0; i < STRING_LENGTH; i++) {
        EXPECT_EQ(buffer[i], STRING_DATA[i]);
    }
}

TEST(WriteParamsToProto, can_serialize_empty_string_params) {
    const size_t buffer_size = STRING_EMPTY_LENGTH + 64;
    uint8_t buffer[buffer_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    EXPECT_EQ(encode_par_field<par_tags::STR_PARAM>(7, "", stream), TBD_OK);

    EXPECT_EQ(stream.bytes_written, STRING_EMPTY_LENGTH);
    for (size_t i = 0; i < STRING_EMPTY_LENGTH; i++) {
        EXPECT_EQ(buffer[i], STRING_EMPTY_DATA[i]);
    }
}