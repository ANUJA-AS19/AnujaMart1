#include "CartController.h"

#include <json/json.h>
#include <cstdint>

namespace anuja::anujamart
{

void CartController::addItem(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = request->getJsonObject();

    if (!json ||
        !json->isMember("product_id") ||
        !json->isMember("quantity"))
    {
        Json::Value error;
        error["success"] = false;
        error["data"] = Json::nullValue;
        error["error"] =
            "product_id and quantity are required";

        callback(
            drogon::HttpResponse::newHttpJsonResponse(error));
        return;
    }

    const auto userId =
        request->attributes()->get<std::int64_t>("user_id");

    const auto productId =
        (*json)["product_id"].asInt64();

    const auto quantity =
        (*json)["quantity"].asInt64();

    const bool success =
        cartService_.addItem(
            userId,
            productId,
            quantity);

    Json::Value responseJson;
    responseJson["success"] = success;
    responseJson["data"] =
        success ? Json::Value("Cart item added")
                : Json::Value(Json::nullValue);
    responseJson["error"] =
        success ? Json::Value(Json::nullValue)
                : Json::Value("Unable to add cart item");

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson);

    response->setStatusCode(
        success
            ? drogon::k200OK
            : drogon::k400BadRequest);

    callback(response);
}


void CartController::getCart(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback)
{
    const auto userId =
        request->attributes()->get<std::int64_t>("user_id");

    const auto items =
        cartService_.getCart(userId);

    const auto totalCents =
        cartService_.getTotalCents(userId);

    Json::Value itemsJson(Json::arrayValue);

    for (const auto& item : items)
    {
        Json::Value itemJson;

        itemJson["id"] = item.id;
        itemJson["user_id"] = item.userId;
        itemJson["product_id"] = item.productId;
        itemJson["quantity"] = item.quantity;
        const auto product = productService_.getProduct(item.productId);
        if (product)
        {
            itemJson["name"] = product->name;
            itemJson["image_url"] = product->imageUrl;
            itemJson["price_cents"] = product->priceCents;
        }

        itemsJson.append(itemJson);
    }

    Json::Value data;
    data["items"] = itemsJson;
    data["total_cents"] = totalCents;

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"] = data;
    responseJson["error"] = Json::nullValue;

    callback(
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson));
}


void CartController::updateItem(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback,
    std::int64_t productId)
{
    auto json = request->getJsonObject();

    if (!json ||
        !json->isMember("quantity"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "quantity is required";

        callback(
            drogon::HttpResponse::newHttpJsonResponse(
                responseJson));

        return;
    }

    const auto userId =
        request->attributes()->get<std::int64_t>("user_id");

    const auto quantity =
        (*json)["quantity"].asInt64();

    const bool success =
        cartService_.updateItem(
            userId,
            productId,
            quantity);

    Json::Value responseJson;
    responseJson["success"] = success;
    responseJson["data"] =
        success ? Json::Value("Cart item updated")
                : Json::Value(Json::nullValue);
    responseJson["error"] =
        success ? Json::Value(Json::nullValue)
                : Json::Value("Unable to update cart item");

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson);

    response->setStatusCode(
        success
            ? drogon::k200OK
            : drogon::k400BadRequest);

    callback(response);
}


void CartController::removeItem(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback,
    std::int64_t productId)
{
    const auto userId =
        request->attributes()->get<std::int64_t>("user_id");

    const bool success =
        cartService_.removeItem(
            userId,
            productId);

    Json::Value responseJson;
    responseJson["success"] = success;
    responseJson["data"] =
        success ? Json::Value("Cart item removed")
                : Json::Value(Json::nullValue);
    responseJson["error"] =
        success ? Json::Value(Json::nullValue)
                : Json::Value("Unable to remove cart item");

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson);

    response->setStatusCode(
        success
            ? drogon::k200OK
            : drogon::k400BadRequest);

    callback(response);
}

}