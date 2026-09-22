#include "AdminController.h"

#include <json/json.h>

namespace anuja::anujamart
{

void AdminController::getAllUsers(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback)
{
    const std::string adminIdText =
        std::to_string(request->attributes()->get<std::int64_t>("user_id"));

    if (adminIdText.empty())
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "admin_id is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    std::int64_t adminId = 0;

    try
    {
        adminId = std::stoll(adminIdText);
    }
    catch (...)
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "admin_id must be a valid number";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const auto users = userService_.getAllUsers(adminId);

    if (users.empty())
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "Access denied. Admin role required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k403Forbidden);
        callback(response);
        return;
    }

    Json::Value usersJson(Json::arrayValue);

    for (const auto& user : users)
    {
        Json::Value item;

        item["id"] = Json::Int64(user.id);
        item["name"] = user.name;
        item["email"] = user.email;

        if (user.role == UserRole::BUYER)
            item["role"] = "BUYER";
        else if (user.role == UserRole::SELLER)
            item["role"] = "SELLER";
        else
            item["role"] = "ADMIN";

        usersJson.append(item);
    }

    Json::Value json;
    json["success"] = true;
    json["data"]["users"] = usersJson;
    json["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(json);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}


void AdminController::getAllOrders(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback)
{
    const std::string adminIdText =
        std::to_string(request->attributes()->get<std::int64_t>("user_id"));

    if (adminIdText.empty())
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "admin_id is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    std::int64_t adminId = 0;

    try
    {
        adminId = std::stoll(adminIdText);
    }
    catch (...)
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "admin_id must be a valid number";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const auto users = userService_.getAllUsers(adminId);

    if (users.empty())
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "Access denied. Admin role required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k403Forbidden);
        callback(response);
        return;
    }

    const auto orders = orderService_.getAllOrders(adminId);

    Json::Value ordersJson(Json::arrayValue);

    for (const auto& order : orders)
    {
        Json::Value item;

        item["id"] = Json::Int64(order.id);
        item["buyer_id"] = Json::Int64(order.buyerId);
        item["status"] = order.status;
        item["total_amount_cents"] =
            Json::Int64(order.totalAmountCents);

        ordersJson.append(item);
    }

    Json::Value json;
    json["success"] = true;
    json["data"]["orders"] = ordersJson;
    json["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(json);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}


void AdminController::deleteProduct(
    const drogon::HttpRequestPtr& request,
    std::function<void(
        const drogon::HttpResponsePtr&)>&& callback,
    std::int64_t productId)
{
    const std::string adminIdText =
        std::to_string(request->attributes()->get<std::int64_t>("user_id"));

    if (adminIdText.empty())
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "admin_id is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    std::int64_t adminId = 0;

    try
    {
        adminId = std::stoll(adminIdText);
    }
    catch (...)
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "admin_id must be a valid number";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (productId <= 0)
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "product_id must be a valid number";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const auto users = userService_.getAllUsers(adminId);

    if (users.empty())
    {
        Json::Value json;
        json["success"] = false;
        json["data"] = Json::nullValue;
        json["error"] = "Access denied. Admin role required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(json);

        response->setStatusCode(drogon::k403Forbidden);
        callback(response);
        return;
    }

    const bool deleted =
        productService_.adminDeleteProduct(
            adminId,
            productId);

    Json::Value json;
    json["success"] = deleted;
    json["data"] = Json::nullValue;

    if (deleted)
    {
        json["error"] = Json::nullValue;
    }
    else
    {
        json["error"] = "Product not found or already deleted";
    }

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(json);

    response->setStatusCode(
        deleted ? drogon::k200OK : drogon::k404NotFound);

    callback(response);
}

} // namespace anuja::anujamart
