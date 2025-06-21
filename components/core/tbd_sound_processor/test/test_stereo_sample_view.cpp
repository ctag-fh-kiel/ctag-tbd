#include <tbd/unittests.hpp>
#include <gtest/gtest.h>

#include <tbd/sound_processor/samples.hpp>

using namespace tbd::sound_processor;

TEST(StereoSampleView, spans_ranges_of_samples) {
    float data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

    StereoSampleView view(data + 3, 3);
    EXPECT_EQ(view.end() - view.begin(), 4);
}

TEST(StereoSampleView, can_be_used_in_range_for) {
    float data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

    auto pos = data + 2;
    size_t count = 0;
    for (StereoSampleView view(data + 2, 4); auto& value : view) {
        EXPECT_EQ(value.left, pos[0]);
        EXPECT_EQ(value.right, pos[1]);
        pos += TBD_AUDIO_CHANNELS;
        count++;
    }
    EXPECT_EQ(count, 5);
}
