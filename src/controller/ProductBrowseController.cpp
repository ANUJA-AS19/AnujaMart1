#include "ProductBrowseController.h"

#include <json/json.h>

namespace anuja::anujamart
{

void ProductBrowseController::browseProducts(
    const drogon::HttpRequestPtr& request,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    const std::string search =
        request->getParameter("search");

    const std::string category =
        request->getParameter("category");

    const auto products =
        productBrowseService_.browseProducts(
            search,
            category);

    Json::Value productsJson(Json::arrayValue);

    for (const auto& product : products)
    {
        Json::Value item;

        item["id"] = Json::Int64(product.id);
        item["seller_id"] =
            Json::Int64(product.sellerId);
        item["name"] = product.name;
        item["description"] = product.description;
        item["price_cents"] =
            Json::Int64(product.priceCents);
        item["stock_qty"] =
            Json::Int64(product.stockQty);
        item["category"] = product.category;
        item["image_url"] = product.imageUrl;

        productsJson.append(item);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["products"] = productsJson;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(
            responseJson);

    callback(response);
}

} // namespace anuja::anujamart
