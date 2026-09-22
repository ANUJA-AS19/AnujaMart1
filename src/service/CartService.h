#pragma once

#include "../model/CartItem.h"
#include "../repository/CartRepository.h"

#include <cstdint>
#include <vector>

namespace anuja::anujamart
{

class CartService
{
public:
    explicit CartService(
        CartRepository& repository);

    bool addItem(
        std::int64_t userId,
        std::int64_t productId,
        std::int64_t quantity);

    bool updateItem(
        std::int64_t userId,
        std::int64_t productId,
        std::int64_t quantity);

    bool removeItem(
        std::int64_t userId,
        std::int64_t productId);

    std::vector<CartItem> getCart(
        std::int64_t userId);

    std::int64_t getTotalCents(
        std::int64_t userId);

private:
    CartRepository& repository_;
};

}