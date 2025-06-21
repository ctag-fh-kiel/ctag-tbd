#include <gtest/gtest.h>

#include <tbd/sound_processor/samples.hpp>

using namespace tbd::sound_processor;


TEST(MonoSampleView, spans_ranges_of_samples) {
    float data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

    MonoSampleView view(data + 3, 3);
    EXPECT_EQ(view.end() - view.begin(), 4);
}

TEST(MonoSampleView, can_be_used_in_range_for) {
    float data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

    // left channel
    {
        auto pos = data + 2;
        size_t count = 0;
        for (MonoSampleView view(data + 2, 4); auto& value : view) {
            EXPECT_EQ(value, *pos);
            pos += TBD_AUDIO_CHANNELS;
            count++;
        }
        EXPECT_EQ(count, 5);
    }

    // right channel
    {
        auto pos = data + 3;
        size_t count = 0;
        for (MonoSampleView view(data + 3, 4); auto& value : view) {
            EXPECT_EQ(value, *pos);
            pos += TBD_AUDIO_CHANNELS;
            count++;
        }
        EXPECT_EQ(count, 5);
    }
}

TEST(MonoSampleView, leaves_other_channel_unaffected) {
    // left channel
    {
        float data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
        float input [] = {80, 81};

        MonoSampleView view(data + 2, 4);
        std::copy_n(input, 2, view.begin());

        EXPECT_EQ(data[0], 1);
        EXPECT_EQ(data[1], 2);
        EXPECT_EQ(data[2], 80);
        EXPECT_EQ(data[3], 4);
        EXPECT_EQ(data[4], 81);
        EXPECT_EQ(data[5], 6);
        EXPECT_EQ(data[6], 7);
        EXPECT_EQ(data[7], 8);
    }

    // right channel
    {
        float data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
        float input [] = {51, 52};

        MonoSampleView view(data + 3, 4);
        std::copy_n(input, 2, view.begin());

        EXPECT_EQ(data[0], 1);
        EXPECT_EQ(data[1], 2);
        EXPECT_EQ(data[2], 3);
        EXPECT_EQ(data[3], 51);
        EXPECT_EQ(data[4], 5);
        EXPECT_EQ(data[5], 52);
        EXPECT_EQ(data[6], 7);
        EXPECT_EQ(data[7], 8);
    }
}
