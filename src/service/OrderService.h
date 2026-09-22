#pragma once

#include "../model/CartItem.h"
#include "../model/Order.h"
#include "../repository/CartRepository.h"
#include "../repository/OrderRepository.h"
#include "../repository/ProductRepository.h"

#include <cstdint>
#include <string>
#include <vector>

namespace anuja::anujamart
{

class OrderService
{
public:
    OrderService(
        OrderRepository& orderRepository,
        CartRepository& cartRepository,
        ProductRepository& productRepository);

    bool checkout(
        std::int64_t buyerId,
        std::int64_t& orderId,
        std::int64_t& totalAmountCents);

    std::vector<Order> getBuyerOrders(
        std::int64_t buyerId);

    std::vector<Order> getSellerOrders(
        std::int64_t sellerId);

    bool updateSellerOrderStatus(
        std::int64_t sellerId,
        std::int64_t orderId,
        const std::string& status);

    std::vector<Order> getAllOrders(
        std::int64_t adminId);

private:
    OrderRepository& orderRepository_;
    CartRepository& cartRepository_;
    ProductRepository& productRepository_;
};

}
