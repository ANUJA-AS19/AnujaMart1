#include "OrderController.h"

#include <json/json.h>
#include <cstdint>

namespace anuja::anujamart
{

void OrderController::checkout(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback)
{
    const auto buyerId =
        request->attributes()->get<std::int64_t>("user_id");

    std::int64_t orderId = 0;
    std::int64_t totalAmountCents = 0;

    const bool success =
        orderService_.checkout(
            buyerId,
            orderId,
            totalAmountCents);

    Json::Value responseJson;
    responseJson["success"] = success;

    if (success)
    {
        Json::Value data;

        data["order_id"] = orderId;
        data["total_amount_cents"] =
            totalAmountCents;
        data["payment_status"] = "PAID";
        data["message"] =
            "Mock payment successful";

        responseJson["data"] = data;
        responseJson["error"] = Json::nullValue;
    }
    else
    {
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Unable to checkout cart";
    }

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson);

    response->setStatusCode(
        success
            ? drogon::k200OK
            : drogon::k400BadRequest);

    callback(response);
}


void OrderController::getBuyerOrders(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback)
{
    const auto buyerId =
        request->attributes()->get<std::int64_t>("user_id");


    const auto orders =
        orderService_.getBuyerOrders(buyerId);

    Json::Value ordersJson(
        Json::arrayValue);

    for (const auto& order : orders)
    {
        Json::Value orderJson;

        orderJson["id"] = order.id;
        orderJson["buyer_id"] = order.buyerId;
        orderJson["status"] = order.status;
        orderJson["total_amount_cents"] =
            order.totalAmountCents;

        ordersJson.append(orderJson);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"] = ordersJson;
    responseJson["error"] = Json::nullValue;

    callback(
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson));
}


void OrderController::getSellerOrders(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback)
{
    const auto sellerId =
        request->attributes()->get<std::int64_t>("user_id");

    const auto user = userRepository_.findById(sellerId);
    if (!user.has_value() || (user->role != UserRole::SELLER && user->role != UserRole::ADMIN)) {
        Json::Value e; e["success"] = false; e["data"] = Json::nullValue; e["error"] = "Seller or admin role required"; auto r = drogon::HttpResponse::newHttpJsonResponse(e); r->setStatusCode(drogon::k403Forbidden); callback(r); return;
    }

    const auto orders =
        orderService_.getSellerOrders(sellerId);

    Json::Value ordersJson(
        Json::arrayValue);

    for (const auto& order : orders)
    {
        Json::Value orderJson;

        orderJson["id"] = order.id;
        orderJson["buyer_id"] = order.buyerId;
        orderJson["status"] = order.status;
        orderJson["total_amount_cents"] =
            order.totalAmountCents;

        ordersJson.append(orderJson);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"] = ordersJson;
    responseJson["error"] = Json::nullValue;

    callback(
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson));
}


void OrderController::updateSellerOrderStatus(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback)
{
    const auto orderIdString =
        request->getParameter("order_id");

    const auto status =
        request->getParameter("status");

    Json::Value responseJson;

    if (orderIdString.empty() ||
        status.empty())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "order_id and status are required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
                responseJson);

        response->setStatusCode(
            drogon::k400BadRequest);

        callback(response);
        return;
    }

    const auto orderId =
        std::stoll(orderIdString);

    const auto sellerId =
        request->attributes()->get<std::int64_t>("user_id");

    const auto user = userRepository_.findById(sellerId);
    if (!user.has_value() || (user->role != UserRole::SELLER && user->role != UserRole::ADMIN)) {
        Json::Value e; e["success"] = false; e["data"] = Json::nullValue; e["error"] = "Seller or admin role required"; auto r = drogon::HttpResponse::newHttpJsonResponse(e); r->setStatusCode(drogon::k403Forbidden); callback(r); return;
    }

    const bool success =
        orderService_.updateSellerOrderStatus(
            sellerId,
            orderId,
            status);

    responseJson["success"] = success;

    if (success)
    {
        responseJson["data"]["message"] =
            "Order status updated successfully";

        responseJson["data"]["order_id"] = Json::Int64(
            orderId);

        responseJson["data"]["status"] =
            status;

        responseJson["error"] = Json::nullValue;
    }
    else
    {
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Unable to update order status";
    }

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