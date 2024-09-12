#include <vector>
#include <numeric>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <Watchdog.h>

using namespace app;
using ::testing::Return;

class TestPowerDownMock : public WatchdogImpl {
 public:
  using WatchdogImpl::powerDown;
  MOCK_METHOD(void, powerDown, (secs_t));
  MOCK_METHOD0(getCurrentTime, secs_t());
};

TEST(TestWatchdog, PowerDown001) {

    secs_t currentTimeSecs = 1000;
    secs_t intervalSecs = 100;
    secs_t roundSecs = 100;
    secs_t timeShift = 0;

    TestPowerDownMock wd_mock;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs));
    EXPECT_CALL(wd_mock, powerDown(roundSecs));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs - timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs - 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + (roundSecs - timeShift)));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs/2 + 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + (roundSecs - timeShift)));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs/2 - 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs - timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
}

TEST(TestWatchdog, PowerDown002) {

    secs_t currentTimeSecs = 1000;
    secs_t intervalSecs = 98;
    secs_t roundSecs = 100;
    secs_t timeShift = 0;

    TestPowerDownMock wd_mock;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs));
    EXPECT_CALL(wd_mock, powerDown(roundSecs));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs - timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs - 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + (roundSecs - timeShift)));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs/2 + 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs - timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs/2 - 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs - timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
}

TEST(TestWatchdog, PowerDown003) {

    secs_t currentTimeSecs = 1000;
    secs_t intervalSecs = 102;
    secs_t roundSecs = 100;
    secs_t timeShift = 0;

    TestPowerDownMock wd_mock;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs));
    EXPECT_CALL(wd_mock, powerDown(roundSecs));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs - timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs - 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + (roundSecs - timeShift)));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs/2 + 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + (roundSecs - timeShift)));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs/2 - 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(roundSecs + (roundSecs - timeShift)));

    wd_mock.powerDown(intervalSecs, roundSecs);
}

TEST(TestWatchdog, PowerDown004) {

    secs_t currentTimeSecs = 1000;
    secs_t intervalSecs = 100;
    secs_t roundSecs = 10;
    secs_t timeShift = 0;

    TestPowerDownMock wd_mock;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs)).WillOnce(Return());
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs + timeShift)).WillOnce(Return());
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs - timeShift)).WillOnce(Return());
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs - 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs - (roundSecs - timeShift))).WillOnce(Return());
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs + (roundSecs - timeShift)));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs/2 + 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs - (roundSecs - timeShift))).WillOnce(Return());
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs + (roundSecs - timeShift)));
    wd_mock.powerDown(intervalSecs, roundSecs);

    timeShift = roundSecs/2 - 1;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs - timeShift));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs + timeShift)).WillOnce(Return());
    wd_mock.powerDown(intervalSecs, roundSecs);
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs + timeShift));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs - timeShift));
    wd_mock.powerDown(intervalSecs, roundSecs);
}

TEST(TestWatchdog, PowerDown005) {

    secs_t currentTimeSecs = 1000;
    secs_t intervalSecs = 100;
    secs_t roundSecs = 0;

    TestPowerDownMock wd_mock;
    EXPECT_CALL(wd_mock, getCurrentTime()).WillOnce(Return(currentTimeSecs));
    EXPECT_CALL(wd_mock, powerDown(intervalSecs)).WillOnce(Return());
    wd_mock.powerDown(intervalSecs, roundSecs);
}