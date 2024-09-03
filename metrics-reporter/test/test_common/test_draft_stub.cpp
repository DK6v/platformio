#include <gtest/gtest.h>
#include <draft.h>

TEST(TestDraftStub, Test001) {
    foo();
    bar(0x01);

    EXPECT_TRUE(true);
}
