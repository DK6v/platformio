#include <array>
#include <numeric>

#include <gtest/gtest.h>
#include <SortHelper.h>

TEST(TestChecksum, BSort001) {
    char data[] = { 5, 2, 4, 1, 3, 0 };
    app::bsort(data, sizeof(data));

    for (char ix = 0; ix < sizeof(data); ++ix) {
        EXPECT_EQ(ix, data[ix]);
    }
}

TEST(TestChecksum, QSort001) {
    char data[] = { 5, 2, 2, 4, 1, 3, 0 };
    char sort[] = { 0, 1, 2, 2, 3, 4, 5 };

    app::qsort(data, sizeof(data));

    for (char ix = 0; ix < sizeof(data); ++ix) {
        EXPECT_EQ(data[ix], sort[ix]);
    }
}

TEST(TestChecksum, QSort002) {
    char data[] = { 5, 2, 2, 4, 1, 3, 0 };
    char sort[] = { 5, 2, 2, 4, 1, 3, 0 };

    app::qsort(data, 0);

    for (char ix = 0; ix < sizeof(data); ++ix) {
        EXPECT_EQ(data[ix], sort[ix]);
    }

    app::qsort(data, 1);

    for (char ix = 0; ix < sizeof(data); ++ix) {
        EXPECT_EQ(data[ix], sort[ix]);
    }
}

TEST(TestChecksum, QSort003) {
    char data[] = { 3, 1, 2 };
    char sort[] = { 1, 2, 3 };

    app::qsort(data, 3);

    for (char ix = 0; ix < sizeof(data); ++ix) {
        EXPECT_EQ(data[ix], sort[ix]);
    }
}

TEST(TestChecksum, QSort004) {
    char data[] = { 1, 1, 1 };
    char sort[] = { 1, 1, 1 };

    app::qsort(data, 3);

    for (char ix = 0; ix < sizeof(data); ++ix) {
        EXPECT_EQ(data[ix], sort[ix]);
    }
}