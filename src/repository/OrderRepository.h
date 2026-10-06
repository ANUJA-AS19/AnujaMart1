#pragma once

#include "../model/Order.h"
#include "../model/CartItem.h"

#include <cstdint>
#include <json/json.h>
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

    bool checkoutTransaction(
        std::int64_t buyerId,
        const std::vector<CartItem>& cartItems,
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

    Json::Value findItemsJson(std::int64_t orderId, std::int64_t sellerId);
};
} // namespace anuja::anujamart
