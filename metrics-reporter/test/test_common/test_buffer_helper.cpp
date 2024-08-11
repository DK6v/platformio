#include <vector>
#include <numeric>
#include <ranges>

#include <gtest/gtest.h>
#include <BufferHelper.h>

// using namespace app;

TEST(TestBufferWrapper, GetTest001) {
    std::vector<char> vect(10);
    std::iota(vect.begin(), vect.end(), 1);

    auto it = vect.begin();
    auto rbuf = app::RBufferHelper([&it]() {
        return *it++;
    });

    EXPECT_EQ(rbuf.getByte(), 0x01);
    EXPECT_EQ(rbuf.getByte(), 0x02);
}

TEST(TestBufferWrapper, GetTest002) {

    std::vector<char> vect(10);
    std::iota(vect.begin(), vect.end(), 1);

    auto it = vect.begin();
    auto rbuf = app::RBufferHelper([&it]() {
        return *it++;
    });

    EXPECT_EQ(rbuf.getBytes(2), 0x0201);
    EXPECT_EQ(rbuf.getBytes(2, true), 0x0304);
    EXPECT_EQ(rbuf.getBytes(2, false), 0x0605);
}

TEST(TestBufferWrapper, GetTest003) {

    std::vector<uint8_t> vect{ 0xAA, /* 1010'1010 */
                               0x55  /* 0101'0101 */ };

    auto it = vect.begin();
    auto rbuf = app::RBufferHelper([&it]() {
        return *it++;
    });

    for (int i : std::views::iota(0, 4)) {
        EXPECT_TRUE(rbuf.getBit());
        EXPECT_FALSE(rbuf.getBit());
    }

    for (int i : std::views::iota(0, 4)) {
        EXPECT_FALSE(rbuf.getBit());
        EXPECT_TRUE(rbuf.getBit());
    }
}

TEST(TestBufferWrapper, GetTest004) {

    std::vector<uint8_t> vect{ 0xAB, 0xCD };

    auto it = vect.begin();
    auto rbuf = app::RBufferHelper([&it]() {
        return *it++;
    });

    EXPECT_EQ(rbuf.getBits(4), 0x0A);
    EXPECT_EQ(rbuf.getBits(4), 0x0B);
    EXPECT_EQ(rbuf.getBits(4), 0x0C);
    EXPECT_EQ(rbuf.getBits(4), 0x0D);
}