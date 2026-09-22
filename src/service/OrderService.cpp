#include "OrderService.h"

namespace anuja::anujamart
{

OrderService::OrderService(
    OrderRepository& orderRepository,
    CartRepository& cartRepository,
    ProductRepository& productRepository)
    : orderRepository_(orderRepository),
      cartRepository_(cartRepository),
      productRepository_(productRepository)
{
}

bool OrderService::checkout(
    std::int64_t buyerId,
    std::int64_t& orderId,
    std::int64_t& totalAmountCents)
{
    if (buyerId <= 0)
    {
        return false;
    }

    const auto cartItems =
        cartRepository_.findByUser(buyerId);

    if (cartItems.empty())
    {
        return false;
    }

    totalAmountCents = 0;

    for (const auto& item : cartItems)
    {
        const auto product =
            productRepository_.findById(item.productId);

        if (!product.has_value())
        {
            return false;
        }

        if (item.quantity <= 0 ||
            item.quantity > product->stockQty)
        {
            return false;
        }

        totalAmountCents +=
            item.quantity * product->priceCents;
    }

    if (!orderRepository_.createOrder(
            buyerId,
            totalAmountCents,
            orderId))
    {
        return false;
    }

    for (const auto& item : cartItems)
    {
        const auto product =
            productRepository_.findById(item.productId);

        if (!product.has_value())
        {
            return false;
        }

        if (!orderRepository_.addOrderItem(
                orderId,
                item.productId,
                item.quantity,
                product->priceCents))
        {
            return false;
        }

        if (!productRepository_.decreaseStock(
                item.productId,
                item.quantity))
        {
            return false;
        }
    }

    for (const auto& item : cartItems)
    {
        cartRepository_.removeItem(
            buyerId,
            item.productId);
    }

    return true;
}

std::vector<Order> OrderService::getBuyerOrders(
    std::int64_t buyerId)
{
    if (buyerId <= 0)
    {
        return {};
    }

    return orderRepository_.findByBuyer(buyerId);
}

std::vector<Order> OrderService::getSellerOrders(
    std::int64_t sellerId)
{
    if (sellerId <= 0)
    {
        return {};
    }

    return orderRepository_.findBySeller(sellerId);
}

bool OrderService::updateSellerOrderStatus(
    std::int64_t sellerId,
    std::int64_t orderId,
    const std::string& status)
{
    if (sellerId <= 0 || orderId <= 0)
    {
        return false;
    }

    if (status != "CONFIRMED" &&
        status != "SHIPPED" &&
        status != "DELIVERED" &&
        status != "CANCELLED")
    {
        return false;
    }

    return orderRepository_.updateStatus(
        orderId,
        sellerId,
        status);
}

std::vector<Order> OrderService::getAllOrders(
    std::int64_t adminId)
{
    if (adminId <= 0)
    {
        return {};
    }

    return orderRepository_.findAll();
}

}
