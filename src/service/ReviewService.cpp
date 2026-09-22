#include "ReviewService.h"

namespace anuja::anujamart
{

ReviewService::ReviewService(
    ReviewRepository& reviewRepository)
    : reviewRepository_(reviewRepository)
{
}

bool ReviewService::createReview(
    std::int64_t userId,
    std::int64_t productId,
    int rating,
    const std::string& comment)
{
    if (userId <= 0 ||
        productId <= 0 ||
        rating < 1 ||
        rating > 5)
    {
        return false;
    }

    return reviewRepository_.createReview(
        productId,
        userId,
        rating,
        comment);
}

std::vector<Review> ReviewService::getProductReviews(
    std::int64_t productId)
{
    if (productId <= 0)
        return {};

    return reviewRepository_.findByProduct(productId);
}

} // namespace anuja::anujamart
