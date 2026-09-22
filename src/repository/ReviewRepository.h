#pragma once

#include "../model/Review.h"

#include <cstdint>
#include <vector>

namespace anuja::anujamart
{

class ReviewRepository
{
public:
    bool createReview(
        std::int64_t productId,
        std::int64_t userId,
        int rating,
        const std::string& comment);

    std::vector<Review> findByProduct(
        std::int64_t productId);
};

} // namespace anuja::anujamart
