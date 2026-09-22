#pragma once

#include <cstdint>
#include <string>

namespace anuja::anujamart
{
struct SellerOrder
{
    std::int64_t orderId = 0;
    std::int64_t buyerId = 0;
    std::int64_t productId = 0;
    std::int64_t quantity = 0;
    std::int64_t unitPriceCents = 0;
    std::string status;
};
}