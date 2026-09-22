#include "ReviewController.h"

#include <json/json.h>

namespace anuja::anujamart
{

void ReviewController::createReview(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback,
    std::int64_t productId)
{
    Json::Value responseJson;

    if (productId <= 0)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] = "Invalid product_id";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
                responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const auto json = request->getJsonObject();

    if (!json)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] = "JSON body is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
                responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }


    if (!json->isMember("rating") ||
        !(*json)["rating"].isInt())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "rating is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
                responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const std::int64_t userId = request->attributes()->get<std::int64_t>("user_id");

    const int rating =
        (*json)["rating"].asInt();

    std::string comment;

    if (json->isMember("comment"))
    {
        if (!(*json)["comment"].isString())
        {
            responseJson["success"] = false;
            responseJson["data"] = Json::nullValue;
            responseJson["error"] =
                "comment must be a string";

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(
                    responseJson);

            response->setStatusCode(
                drogon::k400BadRequest);

            callback(response);
            return;
        }

        comment = (*json)["comment"].asString();
    }

    const bool created =
        reviewService_.createReview(
            userId,
            productId,
            rating,
            comment);

    if (!created)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Unable to create review";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
                responseJson);

        response->setStatusCode(
            drogon::k400BadRequest);

        callback(response);
        return;
    }

    responseJson["success"] = true;
    responseJson["data"]["product_id"] =
        Json::Int64(productId);
    responseJson["data"]["user_id"] =
        Json::Int64(userId);
    responseJson["data"]["rating"] = rating;
    responseJson["data"]["comment"] = comment;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson);

    response->setStatusCode(drogon::k201Created);
    callback(response);
}


void ReviewController::getProductReviews(
    const drogon::HttpRequestPtr&,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback,
    std::int64_t productId)
{
    Json::Value responseJson;

    if (productId <= 0)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] = "Invalid product_id";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
                responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const auto reviews =
        reviewService_.getProductReviews(productId);

    Json::Value reviewsJson(Json::arrayValue);

    int totalRating = 0;

    for (const auto& review : reviews)
    {
        Json::Value item;

        item["id"] = Json::Int64(review.id);
        item["product_id"] =
            Json::Int64(review.productId);
        item["user_id"] =
            Json::Int64(review.userId);
        item["rating"] = review.rating;
        item["comment"] = review.comment;
        item["created_at"] = review.createdAt;

        reviewsJson.append(item);

        totalRating += review.rating;
    }

    double averageRating = 0.0;

    if (!reviews.empty())
    {
        averageRating =
            static_cast<double>(totalRating) /
            static_cast<double>(reviews.size());
    }

    responseJson["success"] = true;
    responseJson["data"]["product_id"] =
        Json::Int64(productId);
    responseJson["data"]["reviews"] = reviewsJson;
    responseJson["data"]["review_count"] =
        static_cast<Json::UInt64>(reviews.size());
    responseJson["data"]["average_rating"] =
        averageRating;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}

} // namespace anuja::anujamart
