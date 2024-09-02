#include <vector>
#include <numeric>
#include <ranges>

#include <gtest/gtest.h>
#include "RBufferHelper.h"
#include "WBufferHelper.h"

// using namespace app;

TEST(TestBufferWrapper, GetTest001) {
    std::vector<uint8_t> vect(10);
    std::iota(vect.begin(), vect.end(), 1);

    auto it = vect.begin();
    auto cb = +[](decltype(it)*it) -> char {
        return *(*it)++;
    };
    auto rbuf = app::RBufferHelper<decltype(it)*>(cb, &it);

    EXPECT_EQ(rbuf.getByte(), 0x01);
    EXPECT_EQ(rbuf.getByte(), 0x02);
}

TEST(TestBufferWrapper, GetTest002) {

    std::vector<uint8_t> vect(10);
    std::iota(vect.begin(), vect.end(), 1);

    auto it = vect.begin();
    auto cb = +[](decltype(it)*it) -> char {
        return *(*it)++;
    };
    auto rbuf = app::RBufferHelper<decltype(it)*>(cb, &it);

    EXPECT_EQ(rbuf.getBytes(2), 0x0201);
    EXPECT_EQ(rbuf.getBytes(2, true), 0x0304);
    EXPECT_EQ(rbuf.getBytes(2, false), 0x0605);
}

TEST(TestBufferWrapper, GetTest003) {

    std::vector<uint8_t> vect{ 0xAA, /* 1010'1010 */
                               0x55  /* 0101'0101 */ };
    auto it = vect.begin();
    auto cb = +[](decltype(it)*it) -> char {
        return *(*it)++;
    };
    auto rbuf = app::RBufferHelper<decltype(it)*>(cb, &it);

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
    auto cb = +[](decltype(it)*it) -> char {
        return *(*it)++;
    };
    auto rbuf = app::RBufferHelper<decltype(it)*>(cb, &it);

    EXPECT_EQ(rbuf.getBits(4), 0x0A);
    EXPECT_EQ(rbuf.getBits(4), 0x0B);
    EXPECT_EQ(rbuf.getBits(4), 0x0C);
    EXPECT_EQ(rbuf.getBits(4), 0x0D);
}

TEST(TestBufferWrapper, SetTest001) {

    std::vector<uint8_t> vect(10);
    std::fill(vect.begin(), vect.end(), 0);

    auto it = vect.begin();
    auto cb = +[](char value, decltype(it)*it) -> void {
        *(*it)++ = value;
    };
    auto wbuf = app::WBufferHelper<decltype(it)*>(cb, &it);

    wbuf.setByte(0x01);
    wbuf.setByte(0x02);

    EXPECT_EQ(vect[0], 0x01);
    EXPECT_EQ(vect[1], 0x02);
}

TEST(TestBufferWrapper, SetTest002) {

    std::vector<uint8_t> vect(10);
    std::fill(vect.begin(), vect.end(), 0);

    auto it = vect.begin();
    auto cb = +[](char value, decltype(it)*it) -> void {
        *(*it)++ = value;
    };
    auto wbuf = app::WBufferHelper<decltype(it)*>(cb, &it);

    wbuf.setBytes(0x01020304, 4);

    EXPECT_EQ(vect[0], 0x04);
    EXPECT_EQ(vect[1], 0x03);
    EXPECT_EQ(vect[2], 0x02);
    EXPECT_EQ(vect[3], 0x01);
}

TEST(TestBufferWrapper, SetTest004) {

    std::vector<uint8_t> vect(10);
    std::fill(vect.begin(), vect.end(), 0);

    auto it = vect.begin();
    auto cb = +[](char value, decltype(it)*it) -> void {
        *(*it)++ = value;
    };
    auto wbuf = app::WBufferHelper<decltype(it)*>(cb, &it);

    wbuf.setBytes(0x01020304, 4);
    wbuf.setBytes(0x05060708, 4, true);

    EXPECT_EQ(0x01020304, reinterpret_cast<uint32_t*>(vect.data())[0]);
    EXPECT_EQ(0x08070605, reinterpret_cast<uint32_t*>(vect.data())[1]);
}

TEST(TestBufferWrapper, SetTest005) {

    std::vector<uint8_t> vect(10);
    std::fill(vect.begin(), vect.end(), 0);

    auto it = vect.begin();
    auto cb = +[](char value, decltype(it)*it) -> void {
        *(*it)++ = value;
    };
    auto wbuf = app::WBufferHelper<decltype(it)*>(cb, &it);

    wbuf.setBit(true);
    wbuf.setBit(true);
    wbuf.flush();

    EXPECT_EQ(vect[0], 0x03);
}

TEST(TestBufferWrapper, SetTest006) {

    std::vector<uint8_t> vect(10);
    std::fill(vect.begin(), vect.end(), 0);

    auto it = vect.begin();
    auto cb = +[](char value, decltype(it)*it) -> void {
        *(*it)++ = value;
    };
    auto wbuf = app::WBufferHelper<decltype(it)*>(cb, &it);

    for (int i : std::views::iota(0, 7)) {
        wbuf.setBit(true);
    }
    EXPECT_EQ(vect[0], 0x00);

    wbuf.setBit(true);
    EXPECT_EQ(vect[0], 0xFF);
}

TEST(TestBufferWrapper, SetTest007) {

    std::vector<uint8_t> vect(10);
    std::fill(vect.begin(), vect.end(), 0);

    auto it = vect.begin();
    auto cb = +[](char value, decltype(it)*it) -> void {
        *(*it)++ = value;
    };
    auto wbuf = app::WBufferHelper<decltype(it)*>(cb, &it);

    wbuf.setBits(0xAA, 8);
    wbuf.setBits(0xBB, 4);
    wbuf.flush();

    EXPECT_EQ(vect[0], 0xAA);
    EXPECT_EQ(vect[1], 0x0B);
}

TEST(TestBufferWrapper, SetTest008) {

    std::vector<uint8_t> vect(10);
    std::fill(vect.begin(), vect.end(), 0);

    auto it = vect.begin();
    auto cb = +[](char value, decltype(it)*it) -> void {
        *(*it)++ = value;
    };
    auto wbuf = app::WBufferHelper<decltype(it)*>(cb, &it);

    wbuf.setBits(0x0A, 4);
    wbuf.setByte(0xBC);
    wbuf.spoof();

    EXPECT_EQ(vect[0], 0xAB);
    EXPECT_EQ(vect[1], 0xC0);
}
