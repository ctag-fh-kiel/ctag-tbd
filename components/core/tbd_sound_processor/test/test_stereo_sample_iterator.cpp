#include <gtest/gtest.h>

#include <tbd/sound_processor/samples.hpp>

using namespace tbd::sound_processor;

TEST(StereoSampleIterator, can_be_incremented) {
    float data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    float* ptr = data;

    StereoSampleIterator iter(ptr);
    EXPECT_EQ(iter->left, 1);
    EXPECT_EQ(iter->right, 2);
    ++iter;
    EXPECT_EQ(iter->left, 3);
    EXPECT_EQ(iter->right, 4);
    ++iter;
    EXPECT_EQ(iter->left, 5);
    EXPECT_EQ(iter->right, 6);
}

TEST(StereoSampleIterator, can_be_compared) {
    float data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    float* ptr = data;

    StereoSampleIterator iter1(ptr);
    ++iter1;
    ++iter1;
    ++iter1;

    StereoSampleIterator iter2(ptr);
    ++iter2;

    EXPECT_EQ(iter1.diff(iter2), 2);
    EXPECT_EQ(iter2.diff(iter1), -2);
    EXPECT_EQ(iter1 - iter2, 2);
    EXPECT_EQ(iter2 - iter1, -2);
    EXPECT_FALSE(iter1 == iter2);
    EXPECT_TRUE(iter1 != iter2);

    ++iter2;
    ++iter2;

    EXPECT_EQ(iter1.diff(iter2), 0);
    EXPECT_EQ(iter2.diff(iter1), 0);
    EXPECT_EQ(iter1 - iter2, 0);
    EXPECT_EQ(iter2 - iter1, 0);
    EXPECT_TRUE(iter1 == iter2);
    EXPECT_FALSE(iter1 != iter2);
}