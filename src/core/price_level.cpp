#include "price_level.hpp"

namespace marketscope {

PriceLevel::PriceLevel(double price) : price_(price) {}

void PriceLevel::AddOrder(const Order& order) {
    orders_.push_back(order);
}

bool PriceLevel::RemoveOrder(std::uint64_t order_id) {
    for (auto it = orders_.begin(); it != orders_.end(); ++it) {
        if (it->id == order_id) {
            orders_.erase(it);
            return true;
        }
    }
    return false;
}

std::optional<Order> PriceLevel::PopFrontOrder() {
    if (orders_.empty()) {
        return std::nullopt;
    }

    Order front = orders_.front();
    orders_.pop_front();
    return front;
}

double PriceLevel::GetPrice() const {
    return price_;
}

std::uint32_t PriceLevel::TotalVolume() const {
    std::uint32_t total = 0;
    for (const auto& order : orders_) {
        total += order.quantity;
    }
    return total;
}

bool PriceLevel::Empty() const {
    return orders_.empty();
}

std::size_t PriceLevel::OrderCount() const {
    return orders_.size();
}

}  // namespace marketscope
