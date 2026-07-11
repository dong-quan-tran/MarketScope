#include <gtest/gtest.h>

#include "core/order_book.hpp"

using namespace bookforge;

TEST(OrderBookTest, EmptyBookHasNoBestBidOrAsk) {
    OrderBook book;

    EXPECT_FALSE(book.GetBestBid().has_value());
    EXPECT_FALSE(book.GetBestAsk().has_value());
    EXPECT_FALSE(book.GetMidPrice().has_value());
    EXPECT_FALSE(book.GetSpread().has_value());
}

TEST(OrderBookTest, AddSingleBid) {
    OrderBook book;
    Order order{1, Side::Buy, 100.25, 10, 1};

    book.AddOrder(order);

    ASSERT_TRUE(book.GetBestBid().has_value());
    EXPECT_DOUBLE_EQ(*book.GetBestBid(), 100.25);
    EXPECT_EQ(book.BidLevelCount(), 1);
}

TEST(OrderBookTest, AddBidAndAskComputesMidAndSpread) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 100.00, 10, 1});
    book.AddOrder(Order{2, Side::Sell, 100.50, 12, 2});

    ASSERT_TRUE(book.GetBestBid().has_value());
    ASSERT_TRUE(book.GetBestAsk().has_value());
    ASSERT_TRUE(book.GetMidPrice().has_value());
    ASSERT_TRUE(book.GetSpread().has_value());

    EXPECT_DOUBLE_EQ(*book.GetBestBid(), 100.00);
    EXPECT_DOUBLE_EQ(*book.GetBestAsk(), 100.50);
    EXPECT_DOUBLE_EQ(*book.GetMidPrice(), 100.25);
    EXPECT_DOUBLE_EQ(*book.GetSpread(), 0.50);
}

TEST(OrderBookTest, CancelOrderRemovesLevelWhenEmpty) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 99.50, 20, 1});

    EXPECT_TRUE(book.CancelOrder(1));
    EXPECT_EQ(book.BidLevelCount(), 0);
    EXPECT_FALSE(book.GetBestBid().has_value());
}

TEST(OrderBookTest, MultipleOrdersSameLevelAggregateVolume) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 100.00, 10, 1});
    book.AddOrder(Order{2, Side::Buy, 100.00, 15, 2});

    auto volume = book.GetLevelVolume(Side::Buy, 100.00);
    ASSERT_TRUE(volume.has_value());
    EXPECT_EQ(*volume, 25);
}

TEST(OrderBookTest, BestBidTracksHighestPrice) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 99.00, 10, 1});
    book.AddOrder(Order{2, Side::Buy, 101.00, 10, 2});
    book.AddOrder(Order{3, Side::Buy, 100.00, 10, 3});

    ASSERT_TRUE(book.GetBestBid().has_value());
    EXPECT_DOUBLE_EQ(*book.GetBestBid(), 101.00);
}

TEST(OrderBookTest, BestAskTracksLowestPrice) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Sell, 102.00, 10, 1});
    book.AddOrder(Order{2, Side::Sell, 100.50, 10, 2});
    book.AddOrder(Order{3, Side::Sell, 101.00, 10, 3});

    ASSERT_TRUE(book.GetBestAsk().has_value());
    EXPECT_DOUBLE_EQ(*book.GetBestAsk(), 100.50);
}

TEST(OrderBookTest, BidDepthReturnsSortedLevels) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 99.00, 10, 1});
    book.AddOrder(Order{2, Side::Buy, 101.00, 20, 2});
    book.AddOrder(Order{3, Side::Buy, 100.00, 30, 3});

    auto depth = book.GetBidDepth(2);
    ASSERT_EQ(depth.size(), 2);
    EXPECT_DOUBLE_EQ(depth[0].first, 101.00);
    EXPECT_EQ(depth[0].second, 20);
    EXPECT_DOUBLE_EQ(depth[1].first, 100.00);
    EXPECT_EQ(depth[1].second, 30);
}

TEST(OrderBookTest, AskDepthReturnsSortedLevels) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Sell, 102.00, 10, 1});
    book.AddOrder(Order{2, Side::Sell, 100.50, 20, 2});
    book.AddOrder(Order{3, Side::Sell, 101.00, 30, 3});

    auto depth = book.GetAskDepth(2);
    ASSERT_EQ(depth.size(), 2);
    EXPECT_DOUBLE_EQ(depth[0].first, 100.50);
    EXPECT_EQ(depth[0].second, 20);
    EXPECT_DOUBLE_EQ(depth[1].first, 101.00);
    EXPECT_EQ(depth[1].second, 30);
}

TEST(OrderBookTest, PartialExecutionReducesFrontOrderVolume) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 100.00, 20, 1});

    EXPECT_TRUE(book.ExecuteTopOrder(Side::Buy, 100.00, 5));

    auto volume = book.GetLevelVolume(Side::Buy, 100.00);
    ASSERT_TRUE(volume.has_value());
    EXPECT_EQ(*volume, 15);
}

TEST(OrderBookTest, FullExecutionRemovesPriceLevel) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Sell, 101.00, 20, 1});

    EXPECT_TRUE(book.ExecuteTopOrder(Side::Sell, 101.00, 20));

    EXPECT_FALSE(book.GetLevelVolume(Side::Sell, 101.00).has_value());
    EXPECT_EQ(book.AskLevelCount(), 0);
}

TEST(OrderBookTest, CancelMissingOrderReturnsFalse) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 100.00, 10, 1});

    EXPECT_FALSE(book.CancelOrder(999));
}

TEST(OrderBookTest, ExecuteMissingLevelReturnsFalse) {
    OrderBook book;
    EXPECT_FALSE(book.ExecuteTopOrder(Side::Buy, 123.45, 10));
}

TEST(OrderBookTest, ExecuteTopOrderConsumesOldestOrderFirst) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 100.00, 10, 1});
    book.AddOrder(Order{2, Side::Buy, 100.00, 15, 2});

    EXPECT_TRUE(book.ExecuteTopOrder(Side::Buy, 100.00, 10));

    auto volume = book.GetLevelVolume(Side::Buy, 100.00);
    ASSERT_TRUE(volume.has_value());
    EXPECT_EQ(*volume, 15);

    EXPECT_FALSE(book.CancelOrder(1));
    EXPECT_TRUE(book.CancelOrder(2));
    EXPECT_FALSE(book.GetLevelVolume(Side::Buy, 100.00).has_value());
}

TEST(OrderBookTest, ExecuteTopOrderPartiallyConsumesOldestBeforeNextOrder) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Sell, 101.00, 20, 1});
    book.AddOrder(Order{2, Side::Sell, 101.00, 30, 2});

    EXPECT_TRUE(book.ExecuteTopOrder(Side::Sell, 101.00, 5));

    auto volume = book.GetLevelVolume(Side::Sell, 101.00);
    ASSERT_TRUE(volume.has_value());
    EXPECT_EQ(*volume, 45);

    EXPECT_TRUE(book.CancelOrder(1));
    auto remaining = book.GetLevelVolume(Side::Sell, 101.00);
    ASSERT_TRUE(remaining.has_value());
    EXPECT_EQ(*remaining, 30);
}

TEST(OrderBookTest, ExecuteZeroQuantityLeavesVolumeUnchanged) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 100.00, 20, 1});

    EXPECT_TRUE(book.ExecuteTopOrder(Side::Buy, 100.00, 0));

    auto volume = book.GetLevelVolume(Side::Buy, 100.00);
    ASSERT_TRUE(volume.has_value());
    EXPECT_EQ(*volume, 20);
}

TEST(OrderBookTest, CancelOrderByIdRemovesAsk) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Sell, 101.50, 25, 1});

    EXPECT_TRUE(book.CancelOrder(1));
    EXPECT_EQ(book.AskLevelCount(), 0);
    EXPECT_FALSE(book.GetBestAsk().has_value());
}

TEST(OrderBookTest, FullExecutionRemovesOrderFromIndex) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 100.00, 10, 1});

    EXPECT_TRUE(book.ExecuteTopOrder(Side::Buy, 100.00, 10));
    EXPECT_FALSE(book.CancelOrder(1));
}

TEST(OrderBookTest, BidDepthRequestLargerThanAvailableReturnsAllLevels) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 101.00, 10, 1});
    book.AddOrder(Order{2, Side::Buy, 100.00, 20, 2});

    auto depth = book.GetBidDepth(10);
    ASSERT_EQ(depth.size(), 2);
    EXPECT_DOUBLE_EQ(depth[0].first, 101.00);
    EXPECT_EQ(depth[0].second, 10);
    EXPECT_DOUBLE_EQ(depth[1].first, 100.00);
    EXPECT_EQ(depth[1].second, 20);
}

TEST(OrderBookTest, AskDepthRequestLargerThanAvailableReturnsAllLevels) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Sell, 100.50, 10, 1});
    book.AddOrder(Order{2, Side::Sell, 101.50, 20, 2});

    auto depth = book.GetAskDepth(10);
    ASSERT_EQ(depth.size(), 2);
    EXPECT_DOUBLE_EQ(depth[0].first, 100.50);
    EXPECT_EQ(depth[0].second, 10);
    EXPECT_DOUBLE_EQ(depth[1].first, 101.50);
    EXPECT_EQ(depth[1].second, 20);
}

TEST(OrderBookTest, MidPriceUnavailableWhenOnlyBidExists) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Buy, 100.00, 10, 1});

    EXPECT_FALSE(book.GetMidPrice().has_value());
}

TEST(OrderBookTest, SpreadUnavailableWhenOnlyAskExists) {
    OrderBook book;
    book.AddOrder(Order{1, Side::Sell, 100.50, 10, 1});

    EXPECT_FALSE(book.GetSpread().has_value());
}