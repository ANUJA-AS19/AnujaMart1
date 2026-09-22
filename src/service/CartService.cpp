#include "CartService.h"

namespace anuja::anujamart
{

CartService::CartService(
    CartRepository& repository)
    : repository_(repository)
{
}

bool CartService::addItem(
    std::int64_t userId,
    std::int64_t productId,
    std::int64_t quantity)
{
    if (userId <= 0 ||
        productId <= 0 ||
        quantity <= 0)
    {
        return false;
    }

    return repository_.addItem(
        userId,
        productId,
        quantity);
}

bool CartService::updateItem(
    std::int64_t userId,
    std::int64_t productId,
    std::int64_t quantity)
{
    if (userId <= 0 ||
        productId <= 0 ||
        quantity <= 0)
    {
        return false;
    }

    return repository_.updateItem(
        userId,
        productId,
        quantity);
}

bool CartService::removeItem(
    std::int64_t userId,
    std::int64_t productId)
{
    if (userId <= 0 ||
        productId <= 0)
    {
        return false;
    }

    return repository_.removeItem(
        userId,
        productId);
}

std::vector<CartItem>
CartService::getCart(
    std::int64_t userId)
{
    if (userId <= 0)
    {
        return {};
    }

    return repository_.findByUser(userId);
}

std::int64_t
CartService::getTotalCents(
    std::int64_t userId)
{
    if (userId <= 0)
    {
        return 0;
    }

    return repository_.calculateTotal(userId);
}

}