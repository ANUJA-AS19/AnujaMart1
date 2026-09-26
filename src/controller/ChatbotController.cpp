#include "ChatbotController.h"

#include <json/json.h>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
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
        responseJson["error"] = "A string message is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    std::string message = (*json)["message"].asString();

    std::string searchText = message;

    std::transform(
        searchText.begin(),
        searchText.end(),
        searchText.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        });

    std::string reply;

    /*
     * Try to search the real product database.
     *
     * We remove common chatbot words so questions such as:
     * "What is the price of laptop?"
     * can search for "laptop".
     */
    std::string searchTerm = searchText;

    const std::vector<std::string> wordsToRemove = {
        "what", "is", "the", "price", "of", "for",
        "how", "much", "does", "cost", "costs",
        "available", "availability", "do", "you",
        "have", "any", "product", "products",
        "tell", "me", "about", "please", "can",
        "i", "buy", "in", "stock", "there",
        "are", "there", "a", "an", "some"
    };

    for (const auto& word : wordsToRemove)
    {
        std::string::size_type position = 0;

        while ((position = searchTerm.find(word, position))
               != std::string::npos)
        {
            searchTerm.erase(position, word.length());
        }
    }

    // Remove punctuation.
    searchTerm.erase(
        std::remove_if(
            searchTerm.begin(),
            searchTerm.end(),
            [](unsigned char character)
            {
                return std::ispunct(character);
            }),
        searchTerm.end());

    // Remove extra spaces.
    std::string cleanedSearch;
    bool previousWasSpace = false;

    for (char character : searchTerm)
    {
        if (std::isspace(
                static_cast<unsigned char>(character)))
        {
            if (!previousWasSpace)
            {
                cleanedSearch += ' ';
                previousWasSpace = true;
            }
        }
        else
        {
            cleanedSearch += character;
            previousWasSpace = false;
        }
    }

    searchTerm = cleanedSearch;

    while (!searchTerm.empty() && searchTerm.front() == ' ')
    {
        searchTerm.erase(searchTerm.begin());
    }

    while (!searchTerm.empty() && searchTerm.back() == ' ')
    {
        searchTerm.pop_back();
    }

    auto products =
        productBrowseService_.browseProducts(searchTerm, "");

    if (!products.empty())
    {
        std::ostringstream messageStream;

        if (products.size() == 1)
        {
            const auto& product = products.front();

            double price =
                static_cast<double>(product.priceCents) / 100.0;

            messageStream
                << product.name
                << " costs ₹"
                << std::fixed
                << std::setprecision(2)
                << price
                << ". ";

            if (product.stockQty > 0)
            {
                messageStream
                    << "It is available with "
                    << product.stockQty
                    << " item(s) in stock.";
            }
            else
            {
                messageStream
                    << "It is currently out of stock.";
            }

            if (!product.description.empty())
            {
                messageStream
                    << " "
                    << product.description;
            }

            reply = messageStream.str();
        }
        else
        {
            messageStream
                << "I found these products in AnujaMart:\n";

            for (const auto& product : products)
            {
                double price =
                    static_cast<double>(product.priceCents) / 100.0;

                messageStream
                    << "\n• "
                    << product.name
                    << " - ₹"
                    << std::fixed
                    << std::setprecision(2)
                    << price;

                if (product.stockQty > 0)
                {
                    messageStream
                        << " ("
                        << product.stockQty
                        << " in stock)";
                }
                else
                {
                    messageStream
                        << " (out of stock)";
                }
            }

            reply = messageStream.str();
        }
    }
    else if (searchText.find("hello") != std::string::npos ||
             searchText.find("hi") != std::string::npos)
    {
        reply =
            "Hello! Welcome to AnujaMart. "
            "You can ask me about products, prices, "
            "availability, cart, orders, and payments.";
    }
    else if (searchText.find("cart") != std::string::npos)
    {
        reply =
            "You can add products to your cart, update quantities, "
            "and remove cart items.";
    }
    else if (searchText.find("order") != std::string::npos)
    {
        reply =
            "You can place an order using checkout and "
            "view your order history through AnujaMart.";
    }
    else if (searchText.find("payment") != std::string::npos)
    {
        reply =
            "AnujaMart currently supports mock payment processing "
            "during checkout.";
    }
    else
    {
        reply =
            "I can help you with AnujaMart products, prices, "
            "availability, cart, orders, and payments. "
            "Try asking: 'What is the price of laptop?'";
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
