#include "ChatbotController.h"

#include <json/json.h>
#include <algorithm>
#include <cctype>
#include <string>

namespace anuja::anujamart
{

void ChatbotController::chat(
    const drogon::HttpRequestPtr& request,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;

    auto json = request->getJsonObject();

    if (!json || !json->isMember("message") ||
        !(*json)["message"].isString())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] =
            "A string message is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    std::string message =
        (*json)["message"].asString();

    std::transform(
        message.begin(),
        message.end(),
        message.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(
                std::tolower(character));
        });

    std::string reply;

    if (message.find("hello") != std::string::npos ||
        message.find("hi") != std::string::npos)
    {
        reply =
            "Hello! Welcome to AnujaMart. "
            "How can I help you today?";
    }
    else if (message.find("product") != std::string::npos ||
             message.find("catalog") != std::string::npos)
    {
        reply =
            "You can browse products, search by category, "
            "and view product details through the AnujaMart API.";
    }
    else if (message.find("price") != std::string::npos)
    {
        reply =
            "Product prices are displayed in cents through "
            "the product API.";
    }
    else if (message.find("order") != std::string::npos)
    {
        reply =
            "You can place an order using checkout and "
            "view your order history through the order API.";
    }
    else if (message.find("cart") != std::string::npos)
    {
        reply =
            "You can add products to your cart, update quantities, "
            "and remove cart items.";
    }
    else if (message.find("payment") != std::string::npos)
    {
        reply =
            "AnujaMart currently supports mock payment processing "
            "during checkout.";
    }
    else
    {
        reply =
            "I can help with products, prices, cart, orders, "
            "and payments. Please ask a related question.";
    }

    responseJson["success"] = true;
    responseJson["data"]["message"] = message;
    responseJson["data"]["reply"] = reply;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}

}
