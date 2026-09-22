#pragma once

#include <cstdint>

namespace anuja::anujamart
{

struct CartItem
{
    std::int64_t id = 0;
    std::int64_t userId = 0;
    std::int64_t productId = 0;
    std::int64_t quantity = 0;
};

}