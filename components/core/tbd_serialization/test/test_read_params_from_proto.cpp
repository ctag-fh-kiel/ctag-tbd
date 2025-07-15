#include <gtest/gtest.h>

#include <tbd/serialization/message_decoding.hpp>
#include <tbd/errors.hpp>

using namespace tbd;
using namespace tbd::serialization;

namespace {

const uint8_t INT_POS_DATA[] = {0xa};
const size_t INT_POS_LENGTH = 1;
const uint8_t INT_NEG_DATA[] = {0xcb, 0x24};
const size_t INT_NEG_LENGTH = 2;
const uint8_t UINT_DATA[] = {0xb1, 0xc1, 0x5};
const size_t UINT_LENGTH = 3;
const uint8_t FLOAT_POS_DATA[] = {0xa0, 0xfd, 0x4e, 0x40};
const size_t FLOAT_POS_LENGTH = 4;
const uint8_t FLOAT_NEG_DATA[] = {0x9a, 0x99, 0xd9, 0xbf};
const size_t FLOAT_NEG_LENGTH = 4;
const uint8_t BOOL_TRUE_DATA[] = {0x1};
const size_t BOOL_TRUE_LENGTH = 1;
const uint8_t BOOL_FALSE_DATA[] = {0x0};
const size_t BOOL_FALSE_LENGTH = 1;
const uint8_t STRING_DATA[] = {0x1a, 0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x73, 0x6f, 0x6d, 0x65, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x20, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x79};
const size_t STRING_LENGTH = 27;
const uint8_t STRING_EMPTY_DATA[] =  {0x0};
const size_t STRING_EMPTY_LENGTH = 1;

}


TEST(ReadParamsFromProto, can_deserialize_positive_int_params) {
    pb_istream_t stream = pb_istream_from_buffer(INT_POS_DATA, INT_POS_LENGTH);

    int_par value;
    EXPECT_EQ(decode_par<par_tags::INT_PARAM>(PB_WT_VARINT, value, stream), TBD_OK);
    EXPECT_EQ(value, 5);
}

TEST(ReadParamsFromProto, can_deserialize_negative_int_params) {
    pb_istream_t stream = pb_istream_from_buffer(INT_NEG_DATA, INT_NEG_LENGTH);

    int_par value;
    EXPECT_EQ(decode_par<par_tags::INT_PARAM>(PB_WT_VARINT, value, stream), TBD_OK);
    EXPECT_EQ(value, -2342);
}

TEST(ReadParamsFromProto, can_deserialize_negative_uint_params) {
    pb_istream_t stream = pb_istream_from_buffer(UINT_DATA, UINT_LENGTH);

    uint_par value;
    EXPECT_EQ(decode_par<par_tags::UINT_PARAM>(PB_WT_VARINT, value, stream), TBD_OK);
    EXPECT_EQ(value, 90289);
}

TEST(ReadParamsFromProto, can_deserialize_positive_float_params) {
    pb_istream_t stream = pb_istream_from_buffer(FLOAT_POS_DATA, FLOAT_POS_LENGTH);

    float_par value;
    EXPECT_EQ(decode_par<par_tags::FLOAT_PARAM>(PB_WT_32BIT, value, stream), TBD_OK);
    EXPECT_FLOAT_EQ(value, 3.23423);
}

TEST(ReadParamsFromProto, can_deserialize_negative_float_params) {
    pb_istream_t stream = pb_istream_from_buffer(FLOAT_NEG_DATA, FLOAT_NEG_LENGTH);

    float_par value;
    EXPECT_EQ(decode_par<par_tags::FLOAT_PARAM>(PB_WT_32BIT, value, stream), TBD_OK);
    EXPECT_FLOAT_EQ(value, -1.7);
}

TEST(ReadParamsFromProto, can_deserialize_true_value_bool_params) {
    pb_istream_t stream = pb_istream_from_buffer(BOOL_TRUE_DATA, BOOL_TRUE_LENGTH);

    trigger_par value;
    EXPECT_EQ(decode_par<par_tags::TRIGGER_PARAM>(PB_WT_VARINT, value, stream), TBD_OK);
    EXPECT_EQ(value, true);
}

TEST(ReadParamsFromProto, can_deserialize_false_value_bool_params) {
    pb_istream_t stream = pb_istream_from_buffer(BOOL_FALSE_DATA, BOOL_FALSE_LENGTH);

    trigger_par value;
    EXPECT_EQ(decode_par<par_tags::TRIGGER_PARAM>(PB_WT_VARINT, value, stream), TBD_OK);
    EXPECT_EQ(value, false);
}

TEST(ReadParamsFromProto, can_deserialize_string_params) {
    pb_istream_t stream = pb_istream_from_buffer(STRING_DATA, STRING_LENGTH);

    str_par value;
    EXPECT_EQ(decode_par<par_tags::STR_PARAM>(PB_WT_STRING, value, stream), TBD_OK);
    EXPECT_STREQ(value.c_str(), "this is some string thingy");
}

TEST(ReadParamsFromProto, can_deserialize_empty_string_params) {
    pb_istream_t stream = pb_istream_from_buffer(STRING_EMPTY_DATA, STRING_EMPTY_LENGTH);

    str_par value;
    EXPECT_EQ(decode_par<par_tags::STR_PARAM>(PB_WT_STRING, value, stream), TBD_OK);
    EXPECT_STREQ(value.c_str(), "");
}