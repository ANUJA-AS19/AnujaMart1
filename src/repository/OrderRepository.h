#pragma once

#include "../model/Order.h"

#include <cstdint>
#include <string>
#include <vector>

namespace anuja::anujamart
{
class OrderRepository
{
public:
    bool createOrder(
        std::int64_t buyerId,
        std::int64_t totalAmountCents,
        std::int64_t& orderId);

    bool addOrderItem(
        std::int64_t orderId,
        std::int64_t productId,
        std::int64_t quantity,
        std::int64_t unitPriceCents);

    std::vector<Order> findByBuyer(
        std::int64_t buyerId);

    std::vector<Order> findBySeller(
        std::int64_t sellerId);
bool updateStatus(
    std::int64_t orderId,
    std::int64_t sellerId,
    const std::string& status);

    std::vector<Order> findAll();
};
} // namespace anuja::anujamart
