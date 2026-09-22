#pragma once

#include "../model/Review.h"
#include "../repository/ReviewRepository.h"

#include <cstdint>
#include <vector>

namespace anuja::anujamart
{

class ReviewService
{
public:
    explicit ReviewService(
        ReviewRepository& reviewRepository);

    bool createReview(
        std::int64_t userId,
        std::int64_t productId,
        int rating,
        const std::string& comment);

    std::vector<Review> getProductReviews(
        std::int64_t productId);

private:
    ReviewRepository& reviewRepository_;
};

} // namespace anuja::anujamart
