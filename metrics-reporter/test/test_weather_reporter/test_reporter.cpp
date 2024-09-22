#include <vector>
#include <numeric>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <Client.h>
#include <RetryHelper.h>
#include <reporter/InfluxReporter.h>

using namespace app;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Invoke;

class TestClientMock : public Client {
public:
  MOCK_METHOD(int, connect, (const char *host, uint16_t port), ());
  MOCK_METHOD(size_t, write, (const uint8_t *, size_t));
  MOCK_METHOD(void, flush, ());
  MOCK_METHOD(void, stop, ());
};

TEST(TestInfluxReporter, InfluxReporter001) {

    TestClientMock client_mock;

    ON_CALL(client_mock, connect(_, _)).WillByDefault(Return(true));

    std::string buffer;
    ON_CALL(client_mock, write(_, _)).WillByDefault(
        Invoke([&buffer](const uint8_t *buf, size_t size) -> size_t {
            buffer = std::string(reinterpret_cast<const char*>(buf));
            return buffer.length();
    }));

    const char* hostname = "hostname";
    uint16_t port = 1;
    auto reporter = InfluxReporter(hostname, port);

    reporter.clear();
    reporter.send(client_mock);
    EXPECT_STREQ(buffer.c_str(), "");
}

TEST(TestInfluxReporter, InfluxReporter002) {

    TestClientMock client_mock;

    ON_CALL(client_mock, connect(_, _)).WillByDefault(Return(true));

    std::string buffer;
    ON_CALL(client_mock, write(_, _)).WillByDefault(
        Invoke([&buffer](const uint8_t *buf, size_t size) -> size_t {
            buffer = std::string(reinterpret_cast<const char*>(buf));
            return buffer.length();
    }));

    const char* hostname = "hostname";
    uint16_t port = 1;
    auto reporter = InfluxReporter(hostname, port);

    EXPECT_CALL(client_mock, connect(_, _));
    EXPECT_CALL(client_mock, write(_, _));
    EXPECT_CALL(client_mock, flush());
    EXPECT_CALL(client_mock, stop());

    reporter.clear();
    reporter.create("metric");
    reporter.addTag("tag1", 1);
    reporter.addTag("tag2", std::string("2"));
    reporter.send(client_mock);
    EXPECT_STREQ(buffer.c_str(), "metric,tag1=1,tag2=2\n");
}

TEST(TestInfluxReporter, InfluxReporter003) {

    TestClientMock client_mock;

    ON_CALL(client_mock, connect(_, _)).WillByDefault(Return(true));

    std::string buffer;
    ON_CALL(client_mock, write(_, _)).WillByDefault(
        Invoke([&buffer](const uint8_t *buf, size_t size) -> size_t {
            buffer = std::string(reinterpret_cast<const char*>(buf));
            return buffer.length();
    }));

    const char* hostname = "hostname";
    uint16_t port = 1;
    auto reporter = InfluxReporter(hostname, port);

    EXPECT_CALL(client_mock, connect(_, _));
    EXPECT_CALL(client_mock, write(_, _));
    EXPECT_CALL(client_mock, flush());
    EXPECT_CALL(client_mock, stop());

    reporter.clear();
    reporter.create("metric");
    reporter.addField("field1", 1);
    reporter.addField("field2", std::string("2"));
    reporter.send(client_mock);
    EXPECT_STREQ(buffer.c_str(), "metric field1=1,field2=2\n");
}

TEST(TestInfluxReporter, InfluxReporter004) {

    TestClientMock client_mock;

    ON_CALL(client_mock, connect(_, _)).WillByDefault(Return(true));

    std::string buffer;
    ON_CALL(client_mock, write(_, _)).WillByDefault(
        Invoke([&buffer](const uint8_t *buf, size_t size) -> size_t {
            buffer = std::string(reinterpret_cast<const char*>(buf));
            return buffer.length();
    }));

    const char* hostname = "hostname";
    uint16_t port = 1;
    auto reporter = InfluxReporter(hostname, port);

    EXPECT_CALL(client_mock, connect(_, _));
    EXPECT_CALL(client_mock, write(_, _));
    EXPECT_CALL(client_mock, flush());
    EXPECT_CALL(client_mock, stop());

    reporter.clear();
    reporter.create("metric");
    reporter.addTag("tag1", 1);
    reporter.addField("field1", 1);
    reporter.send(client_mock);
    EXPECT_STREQ(buffer.c_str(), "metric,tag1=1 field1=1\n");

    EXPECT_TRUE(true);
}

TEST(TestInfluxReporter, InfluxReporter005) {

    TestClientMock client_mock;

    ON_CALL(client_mock, connect(_, _)).WillByDefault(Return(true));

    std::string buffer;
    ON_CALL(client_mock, write(_, _)).WillByDefault(
        Invoke([&buffer](const uint8_t *buf, size_t size) -> size_t {
            buffer = std::string(reinterpret_cast<const char*>(buf));
            return buffer.length();
    }));

    const char* hostname = "hostname";
    uint16_t port = 1;
    auto reporter = InfluxReporter(hostname, port);

    EXPECT_CALL(client_mock, connect(_, _)).Times(2);
    EXPECT_CALL(client_mock, write(_, _)).Times(2);
    EXPECT_CALL(client_mock, flush()).Times(2);
    EXPECT_CALL(client_mock, stop()).Times(2);

    reporter.clear();
    reporter.create("metric");
    reporter.addTag("tag1", 1);

    RETRY(2) {
      reporter.send(client_mock);
    } DONE(false);

    EXPECT_EQ(g_delay, RETRY_TIMEOUT);
    EXPECT_STREQ(buffer.c_str(), "metric,tag1=1\n");
}

TEST(TestInfluxReporter, InfluxReporter006) {

    TestClientMock client_mock;

    ON_CALL(client_mock, connect(_, _)).WillByDefault(Return(true));

    std::string buffer;
    ON_CALL(client_mock, write(_, _)).WillByDefault(
        Invoke([&buffer](const uint8_t *buf, size_t size) -> size_t {
            buffer = std::string(reinterpret_cast<const char*>(buf));
            return buffer.length();
    }));

    const char* hostname = "hostname";
    uint16_t port = 1;
    auto reporter = InfluxReporter(hostname, port);

    EXPECT_CALL(client_mock, connect(_, _)).Times(1);
    EXPECT_CALL(client_mock, write(_, _)).Times(1);
    EXPECT_CALL(client_mock, flush()).Times(1);
    EXPECT_CALL(client_mock, stop()).Times(1);

    reporter.clear();
    reporter.create("metric");
    reporter.addTag("tag1", 1);

    size_t lenght = 0;
    RETRY_W_TMO(2, 200) {
      lenght = reporter.send(client_mock);
    } DONE(lenght != 0);

    EXPECT_NE(lenght, 0);
    EXPECT_EQ(g_delay, 200);
    EXPECT_STREQ(buffer.c_str(), "metric,tag1=1\n");
}