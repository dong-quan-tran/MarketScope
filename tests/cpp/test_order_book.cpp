#include <gtest/gtest.h>

#include "core/order_book.hpp"

using namespace marketscope;

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

    EXPECT_TRUE(book.CancelOrder(1, Side::Buy, 99.50));
    EXPECT_EQ(book.BidLevelCount(), 0);
    EXPECT_FALSE(book.GetBestBid().has_value());
}
