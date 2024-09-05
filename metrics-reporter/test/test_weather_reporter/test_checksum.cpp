#include <vector>
#include <numeric>

#include <gtest/gtest.h>
#include <Checksum.h>

TEST(TestChecksum, XOR) {
    std::vector<char> vect = { 0x1, 0x2, 0x3, 0x4, 0x5 };
    char checkVal = std::accumulate(vect.cbegin(), vect.cend(), 0, std::bit_xor<char>());

    EXPECT_EQ(checkVal,
        app::Checksum(app::Checksum::XOR)
            .calculate(vect.cbegin(), vect.cend()));

    EXPECT_EQ(checkVal,
        app::Checksum(app::Checksum::XOR)
            .calculate(reinterpret_cast<char *>(vect.data()), vect.size()));
}
