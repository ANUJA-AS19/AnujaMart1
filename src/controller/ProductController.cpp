#include "ProductController.h"

#include <json/json.h>
#include <stdexcept>

namespace anuja::anujamart
{

bool hasValidProductTypes(const Json::Value& json)
{
    const bool validSellerId =
        json["seller_id"].isInt64() ||
        json["seller_id"].isInt();

    const bool validPrice =
        json["price_cents"].isInt64() ||
        json["price_cents"].isInt();

    const bool validStock =
        json["stock_qty"].isInt64() ||
        json["stock_qty"].isInt();

    const bool validImage =
        !json.isMember("image_url") ||
        json["image_url"].isString();

    return validSellerId &&
           json["name"].isString() &&
           json["description"].isString() &&
           validPrice &&
           validStock &&
           json["category"].isString() &&
           validImage;
}


void ProductController::createProduct(
    const drogon::HttpRequestPtr& request,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = request->getJsonObject();

    if (!json)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] = "Invalid JSON body";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!json->isMember("seller_id") ||
        !json->isMember("name") ||
        !json->isMember("description") ||
        !json->isMember("price_cents") ||
        !json->isMember("stock_qty") ||
        !json->isMember("category"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "seller_id, name, description, price_cents, "
            "stock_qty and category are required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!hasValidProductTypes(*json))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Invalid data type for product fields";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const std::int64_t sellerId =
        (*json)["seller_id"].asInt64();

    const std::string name =
        (*json)["name"].asString();

    const std::string description =
        (*json)["description"].asString();

    const std::int64_t priceCents =
        (*json)["price_cents"].asInt64();

    const std::int64_t stockQty =
        (*json)["stock_qty"].asInt64();

    const std::string category =
        (*json)["category"].asString();

    std::string imageUrl;

    if (json->isMember("image_url"))
    {
        imageUrl = (*json)["image_url"].asString();
    }

    const bool created =
        productService_.createProduct(
            sellerId,
            name,
            description,
            priceCents,
            stockQty,
            category,
            imageUrl);

    Json::Value responseJson;
    responseJson["success"] = created;

    if (created)
    {
        responseJson["data"]["message"] =
            "Product created successfully";

        responseJson["error"] = Json::nullValue;
    }
    else
    {
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Failed to create product";
    }

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(
        created
            ? drogon::k201Created
            : drogon::k400BadRequest);

    callback(response);
}


void ProductController::getSellerProducts(
    const drogon::HttpRequestPtr&,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    std::int64_t sellerId)
{
    const auto products =
        productService_.getSellerProducts(sellerId);

    Json::Value productsJson(Json::arrayValue);

    for (const auto& product : products)
    {
        Json::Value item;

        item["id"] = Json::Int64(product.id);
        item["seller_id"] = Json::Int64(product.sellerId);
        item["name"] = product.name;
        item["description"] = product.description;
        item["price_cents"] = Json::Int64(product.priceCents);
        item["stock_qty"] = Json::Int64(product.stockQty);
        item["category"] = product.category;
        item["image_url"] = product.imageUrl;

        productsJson.append(item);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["products"] = productsJson;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    callback(response);
}


void ProductController::getProduct(
    const drogon::HttpRequestPtr&,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    const std::string& productIdText)
{
    std::int64_t productId = 0;

    try
    {
        std::size_t position = 0;
        productId = std::stoll(productIdText, &position);

        if (position != productIdText.size() || productId <= 0)
            throw std::invalid_argument("Invalid product ID");
    }
    catch (...)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] = "product_id must be a valid number";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }
    const auto product =
        productService_.getProduct(productId);

    if (!product)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Product not found";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k404NotFound);
        callback(response);
        return;
    }

    Json::Value data;

    data["id"] = Json::Int64(product->id);
    data["seller_id"] = Json::Int64(product->sellerId);
    data["name"] = product->name;
    data["description"] = product->description;
    data["price_cents"] =
        Json::Int64(product->priceCents);
    data["stock_qty"] =
        Json::Int64(product->stockQty);
    data["category"] = product->category;
    data["image_url"] = product->imageUrl;

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"] = data;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    callback(response);
}


void ProductController::updateProduct(
    const drogon::HttpRequestPtr& request,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    std::int64_t productId)
{
    auto json = request->getJsonObject();

    if (!json)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Invalid JSON body";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!json->isMember("seller_id") ||
        !json->isMember("name") ||
        !json->isMember("description") ||
        !json->isMember("price_cents") ||
        !json->isMember("stock_qty") ||
        !json->isMember("category"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "seller_id, name, description, price_cents, "
            "stock_qty and category are required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!hasValidProductTypes(*json))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Invalid data type for product fields";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const std::int64_t sellerId =
        (*json)["seller_id"].asInt64();

    const std::string name =
        (*json)["name"].asString();

    const std::string description =
        (*json)["description"].asString();

    const std::int64_t priceCents =
        (*json)["price_cents"].asInt64();

    const std::int64_t stockQty =
        (*json)["stock_qty"].asInt64();

    const std::string category =
        (*json)["category"].asString();

    std::string imageUrl;

    if (json->isMember("image_url"))
    {
        imageUrl = (*json)["image_url"].asString();
    }

    const bool updated =
        productService_.updateProduct(
            productId,
            sellerId,
            name,
            description,
            priceCents,
            stockQty,
            category,
            imageUrl);

    Json::Value responseJson;
    responseJson["success"] = updated;

    if (updated)
    {
        responseJson["data"]["message"] =
            "Product updated successfully";

        responseJson["error"] = Json::nullValue;
    }
    else
    {
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Product not found or seller does not own it";
    }

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(
        updated
            ? drogon::k200OK
            : drogon::k404NotFound);

    callback(response);
}


void ProductController::deleteProduct(
    const drogon::HttpRequestPtr& request,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    const std::string& productIdText)
{
    std::int64_t productId = 0;

    try
    {
        std::size_t position = 0;
        productId = std::stoll(productIdText, &position);

        if (position != productIdText.size() || productId <= 0)
        {
            throw std::invalid_argument("Invalid product ID");
        }
    }
    catch (...)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] = "product_id must be a valid number";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const std::string sellerIdText =
        request->getParameter("seller_id");

    if (sellerIdText.empty())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "seller_id is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    std::int64_t sellerId = 0;

    try
    {
        sellerId = std::stoll(sellerIdText);
    }
    catch (...)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "seller_id must be a valid number";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const bool deleted =
        productService_.deleteProduct(
            productId,
            sellerId);

    Json::Value responseJson;
    responseJson["success"] = deleted;

    if (deleted)
    {
        responseJson["data"]["message"] =
            "Product deleted successfully";

        responseJson["error"] = Json::nullValue;
    }
    else
    {
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "Product not found or seller does not own it";
    }

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(
        deleted
            ? drogon::k200OK
            : drogon::k404NotFound);

    callback(response);
}

} // namespace anuja::anujamart