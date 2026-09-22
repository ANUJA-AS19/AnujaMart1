
#pragma once

#include "../model/CartItem.h"

#include <cstdint>
#include <vector>

namespace anuja::anujamart
{
class CartRepository
{
public:
    bool addItem(
        std::int64_t userId,
        std::int64_t productId,
        std::int64_t quantity);

    std::vector<CartItem> findByUser(
        std::int64_t userId);

    bool updateItem(
        std::int64_t userId,
        std::int64_t productId,
        std::int64_t quantity);

    bool removeItem(
        std::int64_t userId,
        std::int64_t productId);

    std::int64_t calculateTotal(
        std::int64_t userId);

    bool clearCart(
        std::int64_t userId);
};
}
