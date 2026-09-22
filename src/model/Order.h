#pragma once

#include <cstdint>
#include <string>

namespace anuja::anujamart
{
struct Order
{
    std::int64_t id = 0;
    std::int64_t buyerId = 0;
    std::string status;
    std::int64_t totalAmountCents = 0;
};
} // namespace anuja::anujamart