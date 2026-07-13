#include <gtest/gtest.h>

#include "core/matching_engine.hpp"

using namespace bookforge;

TEST(MatchingEngineTest, NonCrossingLimitOrderRestsInBook) {
    MatchingEngine engine;

    Order buy{1, Side::Buy, 100.00, 10, 1};

    auto result = engine.MatchLimitOrder(buy);
    EXPECT_TRUE(result.trades.empty());
    EXPECT_EQ(result.remaining_quantity, 0u);

    auto best_bid = engine.Book().GetBestBid();
    ASSERT_TRUE(best_bid.has_value());
    EXPECT_DOUBLE_EQ(*best_bid, 100.00);
}

TEST(MatchingEngineTest, CrossingBuyMatchesBestAskAtOrBetterPrice) {
    MatchingEngine engine;

    // Seed book with resting ask.
    EXPECT_TRUE(engine.Book().AddOrder(Order{1, Side::Sell, 100.50, 10, 1}));

    // Incoming buy crosses the ask.
    Order incoming{2, Side::Buy, 101.00, 5, 2};

    auto result = engine.MatchLimitOrder(incoming);

    ASSERT_EQ(result.trades.size(), 1u);
    const auto& trade = result.trades[0];
    EXPECT_EQ(trade.taker_order_id, 2u);
    EXPECT_EQ(trade.side, Side::Buy);
    EXPECT_DOUBLE_EQ(trade.price, 100.50);
    EXPECT_EQ(trade.quantity, 5u);

    // Remaining ask volume should be reduced.
    auto volume = engine.Book().GetLevelVolume(Side::Sell, 100.50);
    ASSERT_TRUE(volume.has_value());
    EXPECT_EQ(*volume, 5u);
}

TEST(MatchingEngineTest, CrossingSellMatchesBestBidAtOrBetterPrice) {
    MatchingEngine engine;

    EXPECT_TRUE(engine.Book().AddOrder(Order{1, Side::Buy, 100.00, 10, 1}));

    Order incoming{2, Side::Sell, 99.50, 4, 2};

    auto result = engine.MatchLimitOrder(incoming);

    ASSERT_EQ(result.trades.size(), 1u);
    const auto& trade = result.trades[0];
    EXPECT_EQ(trade.taker_order_id, 2u);
    EXPECT_EQ(trade.side, Side::Sell);
    EXPECT_DOUBLE_EQ(trade.price, 100.00);
    EXPECT_EQ(trade.quantity, 4u);

    auto volume = engine.Book().GetLevelVolume(Side::Buy, 100.00);
    ASSERT_TRUE(volume.has_value());
    EXPECT_EQ(*volume, 6u);
}

TEST(MatchingEngineTest, CrossingBuyPartiallyFillsAndRestsRemainder) {
    MatchingEngine engine;

    EXPECT_TRUE(engine.Book().AddOrder(Order{1, Side::Sell, 100.50, 5, 1}));

    Order incoming{2, Side::Buy, 101.00, 10, 2};

    auto result = engine.MatchLimitOrder(incoming);

    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].quantity, 5u);

    // After matching 5, we should have resting quantity (implementation-dependent).
    auto best_bid = engine.Book().GetBestBid();
    ASSERT_TRUE(best_bid.has_value());
    EXPECT_DOUBLE_EQ(*best_bid, 101.00);

    auto volume = engine.Book().GetLevelVolume(Side::Buy, 101.00);
    ASSERT_TRUE(volume.has_value());
    EXPECT_EQ(*volume, 5u);
}

TEST(MatchingEngineTest, CrossingBuyMatchesAcrossMultipleAskLevels) {
    MatchingEngine engine;

    EXPECT_TRUE(engine.Book().AddOrder(Order{1, Side::Sell, 100.50, 5, 1}));
    EXPECT_TRUE(engine.Book().AddOrder(Order{2, Side::Sell, 100.75, 4, 2}));

    Order incoming{3, Side::Buy, 101.00, 8, 3};

    auto result = engine.MatchLimitOrder(incoming);

    ASSERT_EQ(result.trades.size(), 2u);

    EXPECT_EQ(result.trades[0].taker_order_id, 3u);
    EXPECT_EQ(result.trades[0].side, Side::Buy);
    EXPECT_DOUBLE_EQ(result.trades[0].price, 100.50);
    EXPECT_EQ(result.trades[0].quantity, 5u);

    EXPECT_EQ(result.trades[1].taker_order_id, 3u);
    EXPECT_EQ(result.trades[1].side, Side::Buy);
    EXPECT_DOUBLE_EQ(result.trades[1].price, 100.75);
    EXPECT_EQ(result.trades[1].quantity, 3u);

    EXPECT_FALSE(engine.Book().GetLevelVolume(Side::Sell, 100.50).has_value());

    auto remaining_ask = engine.Book().GetLevelVolume(Side::Sell, 100.75);
    ASSERT_TRUE(remaining_ask.has_value());
    EXPECT_EQ(*remaining_ask, 1u);
}

TEST(MatchingEngineTest, CrossingSellMatchesAcrossMultipleBidLevels) {
    MatchingEngine engine;

    EXPECT_TRUE(engine.Book().AddOrder(Order{1, Side::Buy, 100.00, 5, 1}));
    EXPECT_TRUE(engine.Book().AddOrder(Order{2, Side::Buy, 99.75, 4, 2}));

    Order incoming{3, Side::Sell, 99.50, 8, 3};

    auto result = engine.MatchLimitOrder(incoming);

    ASSERT_EQ(result.trades.size(), 2u);

    EXPECT_EQ(result.trades[0].taker_order_id, 3u);
    EXPECT_EQ(result.trades[0].side, Side::Sell);
    EXPECT_DOUBLE_EQ(result.trades[0].price, 100.00);
    EXPECT_EQ(result.trades[0].quantity, 5u);

    EXPECT_EQ(result.trades[1].taker_order_id, 3u);
    EXPECT_EQ(result.trades[1].side, Side::Sell);
    EXPECT_DOUBLE_EQ(result.trades[1].price, 99.75);
    EXPECT_EQ(result.trades[1].quantity, 3u);

    EXPECT_FALSE(engine.Book().GetLevelVolume(Side::Buy, 100.00).has_value());

    auto remaining_bid = engine.Book().GetLevelVolume(Side::Buy, 99.75);
    ASSERT_TRUE(remaining_bid.has_value());
    EXPECT_EQ(*remaining_bid, 1u);
}

TEST(MatchingEngineTest, ResidualBuyRestsAfterExhaustingMarketableAskLevels) {
    MatchingEngine engine;

    EXPECT_TRUE(engine.Book().AddOrder(Order{1, Side::Sell, 100.50, 5, 1}));
    EXPECT_TRUE(engine.Book().AddOrder(Order{2, Side::Sell, 100.75, 4, 2}));

    Order incoming{3, Side::Buy, 100.75, 12, 3};

    auto result = engine.MatchLimitOrder(incoming);

    ASSERT_EQ(result.trades.size(), 2u);
    EXPECT_DOUBLE_EQ(result.trades[0].price, 100.50);
    EXPECT_EQ(result.trades[0].quantity, 5u);
    EXPECT_DOUBLE_EQ(result.trades[1].price, 100.75);
    EXPECT_EQ(result.trades[1].quantity, 4u);

    auto resting_bid = engine.Book().GetLevelVolume(Side::Buy, 100.75);
    ASSERT_TRUE(resting_bid.has_value());
    EXPECT_EQ(*resting_bid, 3u);

    EXPECT_FALSE(engine.Book().GetBestAsk().has_value());
}