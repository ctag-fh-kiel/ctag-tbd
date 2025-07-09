#include <gtest/gtest.h>

#include <tbd/serialization/message_transcoding.hpp>

using namespace tbd;
using namespace tbd::serialization;

TEST(WriteProto, can_serialize_int_params) {
    uint8_t buffer[64];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, 64);

    const int_par par = 12;
    write_par<par_tags::INT_PARAM>(1, par, stream);

    EXPECT_EQ(stream.bytes_written, 2);
    EXPECT_EQ(buffer[0], 0x08);
    EXPECT_EQ(buffer[1], 0x8c);
}

TEST(WriteProto, can_serialize_trigger_params) {
    uint8_t buffer[64];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, 64);

    const trigger_par yes = true;
    const trigger_par no = false;
    write_par<par_tags::TRIGGER_PARAM>(1, no, stream);
    write_par<par_tags::TRIGGER_PARAM>(2, yes, stream);

    EXPECT_EQ(stream.bytes_written, 4);
    EXPECT_EQ(buffer[0], 0x08);
    EXPECT_EQ(buffer[1], 0x80);
    EXPECT_EQ(buffer[2], 0x10);
    EXPECT_EQ(buffer[3], 0x81);
}
