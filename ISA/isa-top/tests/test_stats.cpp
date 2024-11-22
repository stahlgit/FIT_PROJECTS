#include <gtest/gtest.h>
#include "../src/include/stats.h"
class StatsTest : public ::testing::Test {
protected:
    Stats stats;

    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(StatsTest, UpdateAndRetrieveStats) {
    stats.update("192.168.1.1", "192.168.1.2", "tcp", 500, 2, true);
    stats.update("192.168.1.1", "192.168.1.2", "tcp", 300, 1, false);

    auto snapshot = stats.get_stats_snapshot();
    ConnectionKey key = {"192.168.1.1", "192.168.1.2", "tcp"};

    ASSERT_TRUE(snapshot.find(key) != snapshot.end());
    EXPECT_EQ(snapshot[key].tx_bytes, 500);
    EXPECT_EQ(snapshot[key].rx_bytes, 300);
    EXPECT_EQ(snapshot[key].tx_packets, 2);
    EXPECT_EQ(snapshot[key].rx_packets, 1);
}

TEST_F(StatsTest, NoStatsInitially) {
    auto snapshot = stats.get_stats_snapshot();
    EXPECT_TRUE(snapshot.empty());
}