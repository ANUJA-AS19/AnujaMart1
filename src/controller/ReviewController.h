#pragma once

#include "../repository/ReviewRepository.h"
#include "../service/ReviewService.h"

#include <drogon/HttpController.h>

namespace anuja::anujamart
{

class ReviewController
    : public drogon::HttpController<ReviewController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ReviewController::createReview,
        "/api/v1/products/{product_id}/reviews",
        drogon::Post, "anuja::anujamart::AuthFilter");

    ADD_METHOD_TO(
        ReviewController::getProductReviews,
        "/api/v1/products/{product_id}/reviews",
        drogon::Get);

    METHOD_LIST_END

    void createReview(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback,
        std::int64_t productId);

    void getProductReviews(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback,
        std::int64_t productId);

private:
    ReviewRepository reviewRepository_;
    ReviewService reviewService_{reviewRepository_};
};

} // namespace anuja::anujamart
