#include <vector>
#include <numeric>

#include <gtest/gtest.h>
#include <BitBuffer.h>

// using namespace app;

TEST(TestBitBiffer, Init) {
    std::vector<char> vect = { 0x1, 0x2, 0x3, 0x4, 0x5 };
    auto bbuf = app::BitBuffer([](){ return 0;},
                               [](char){ return;});
}
