#include <gtest/gtest.h>

#include "HyperliquidCsvReader.hpp"
#include "HyperliquidMatchingEngineAdapter.hpp"
#include "core/matching_engine.hpp"
#include "replay/ReplayConfig.hpp"
#include "replay/ReplayRunner.hpp"

using namespace bookforge;

namespace {

std::string FixturePath(const std::string& filename) {
    return "tests/fixtures/" + filename;
}

class ReplayRegressionTest : public ::testing::Test {
protected:
    MatchingEngine engine;
    HyperliquidMatchingEngineAdapter adapter{engine};
};

} // namespace

TEST_F(ReplayRegressionTest, BasicFixtureProducesStableReplayOutcome) {
    ReplayConfig config;
    config.path = FixturePath("hyperliquid_replay_fixture_basic.csv");
    config.symbol = "BTCUSDT.P";
    config.source = ReplaySource::Hyperliquid;
    config.max_events = 0;
    config.start_offset = 0;
    config.log_every_n = 0;
    config.log_summary = false;
    config.log_errors = false;
    config.strict_mode = true;

    HyperliquidCsvReader reader(config.path);
    const auto events = reader.read_all(config.strict_mode, config.log_errors);

    ASSERT_EQ(events.size(), 4u);

    ReplayRunner runner(config);
    EXPECT_TRUE(runner.Run(adapter, events));

    const auto& stats = adapter.Stats();
    EXPECT_EQ(stats.totalEvents, 4u);
    EXPECT_EQ(stats.newCount, 3u);
    EXPECT_EQ(stats.rejectCount, 1u);
    EXPECT_EQ(stats.submittedOrders, 3u);
    EXPECT_EQ(stats.ignoredEvents, 1u);
    EXPECT_EQ(stats.generatedTrades, 1u);

    const auto& trades = adapter.Trades();
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_DOUBLE_EQ(trades[0].price, 100.50);
    EXPECT_EQ(trades[0].side, Side::Buy);

    auto bestBid = engine.Book().GetBestBid();
    ASSERT_TRUE(bestBid.has_value());
    EXPECT_DOUBLE_EQ(*bestBid, 99.50);

    EXPECT_FALSE(engine.Book().GetBestAsk().has_value());
    EXPECT_FALSE(engine.Book().GetMidPrice().has_value());
    EXPECT_FALSE(engine.Book().GetSpread().has_value());
}

TEST_F(ReplayRegressionTest, BoundedFixtureReplayProducesStableSubrangeOutcome) {
    ReplayConfig config;
    config.path = FixturePath("hyperliquid_replay_fixture_bounded.csv");
    config.symbol = "BTCUSDT.P";
    config.source = ReplaySource::Hyperliquid;
    config.start_offset = 1;
    config.max_events = 2;
    config.log_every_n = 0;
    config.log_summary = false;
    config.log_errors = false;
    config.strict_mode = true;

    HyperliquidCsvReader reader(config.path);
    const auto events = reader.read_all(config.strict_mode, config.log_errors);

    ASSERT_EQ(events.size(), 4u);

    ReplayRunner runner(config);
    EXPECT_TRUE(runner.Run(adapter, events));

    const auto& stats = adapter.Stats();
    EXPECT_EQ(stats.totalEvents, 2u);
    EXPECT_EQ(stats.newCount, 1u);
    EXPECT_EQ(stats.rejectCount, 1u);
    EXPECT_EQ(stats.submittedOrders, 1u);
    EXPECT_EQ(stats.ignoredEvents, 1u);
    EXPECT_EQ(stats.generatedTrades, 0u);

    EXPECT_FALSE(engine.Book().GetBestBid().has_value());

    auto bestAsk = engine.Book().GetBestAsk();
    ASSERT_TRUE(bestAsk.has_value());
    EXPECT_DOUBLE_EQ(*bestAsk, 100.50);
}