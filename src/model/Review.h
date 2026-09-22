#pragma once

#include <cstdint>
#include <string>

namespace anuja::anujamart
{

struct Review
{
    std::int64_t id = 0;
    std::int64_t productId = 0;
    std::int64_t userId = 0;
    int rating = 0;
    std::string comment;
    std::string createdAt;
};

} // namespace anuja::anujamart
