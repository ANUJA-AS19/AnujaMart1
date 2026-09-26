#include "ChatbotController.h"

#include <json/json.h>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

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

    // Convert the user's message to lowercase.
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
     * ------------------------------------------------------------
     * 1. GREETING
     * ------------------------------------------------------------
     */
    if (searchText == "hi" ||
        searchText == "hello" ||
        searchText == "hey" ||
        searchText == "hi there" ||
        searchText == "hello there")
    {
        reply =
            "Hello! Welcome to AnujaMart. "
            "You can ask me about products, prices, "
            "availability, cart, orders, and payments.";
    }

    /*
     * ------------------------------------------------------------
     * 2. HELP / WHAT CAN YOU DO?
     * ------------------------------------------------------------
     */
    else if (searchText.find("what can you do") != std::string::npos ||
             searchText.find("how can you help") != std::string::npos ||
             searchText.find("what do you do") != std::string::npos ||
             searchText == "help")
    {
        reply =
            "I can help you with AnujaMart products, prices, "
            "availability, product details, cart, orders, and payments. "
            "For example, you can ask 'What is the price of laptop?'";
    }

    /*
     * ------------------------------------------------------------
     * 3. CART
     * ------------------------------------------------------------
     */
    else if (searchText.find("cart") != std::string::npos)
    {
        reply =
            "You can add products to your cart, update quantities, "
            "and remove cart items.";
    }

    /*
     * ------------------------------------------------------------
     * 4. ORDERS
     * ------------------------------------------------------------
     */
    else if (searchText.find("order") != std::string::npos)
    {
        reply =
            "You can place an order using checkout and "
            "view your order history through AnujaMart.";
    }

    /*
     * ------------------------------------------------------------
     * 5. PAYMENT
     * ------------------------------------------------------------
     */
    else if (searchText.find("payment") != std::string::npos)
    {
        reply =
            "AnujaMart currently supports mock payment processing "
            "during checkout.";
    }

    /*
     * ------------------------------------------------------------
     * 6. PRODUCT SEARCH
     *
     * Convert the question into a product search term.
     * Example:
     * "What is the price of laptop?"
     * becomes:
     * "laptop"
     * ------------------------------------------------------------
     */

    else
    {
        std::string searchTerm = searchText;

        // Remove punctuation first.
        searchTerm.erase(
            std::remove_if(
                searchTerm.begin(),
                searchTerm.end(),
                [](unsigned char character)
                {
                    return std::ispunct(character);
                }),
            searchTerm.end());

        // Split into individual words.
        std::istringstream input(searchTerm);
        std::vector<std::string> words;
        std::string word;

        while (input >> word)
        {
            words.push_back(word);
        }

        // Words that describe the question rather than the product.
        const std::vector<std::string> wordsToRemove = {
            "what", "is", "the", "price", "of", "for",
            "how", "much", "does", "do", "you", "have",
            "any", "product", "products", "tell", "me",
            "about", "please", "can", "i", "buy",
            "in", "stock", "available", "availability",
            "there", "are", "a", "an", "some",
            "cost", "costs", "show", "give", "details",
            "information", "information"
        };

        for (const auto& currentWord : words)
        {
            bool removeWord = false;

            for (const auto& unwantedWord : wordsToRemove)
            {
                if (currentWord == unwantedWord)
                {
                    removeWord = true;
                    break;
                }
            }

            if (!removeWord)
            {
                if (!searchTerm.empty())
                {
                    // Nothing needed here.
                }
            }
        }

        // Build the actual product search term.
        std::ostringstream cleanedSearchStream;

        for (const auto& currentWord : words)
        {
            bool removeWord = false;

            for (const auto& unwantedWord : wordsToRemove)
            {
                if (currentWord == unwantedWord)
                {
                    removeWord = true;
                    break;
                }
            }

            if (!removeWord)
            {
                if (cleanedSearchStream.tellp() > 0)
                {
                    cleanedSearchStream << ' ';
                }

                cleanedSearchStream << currentWord;
            }
        }

        searchTerm = cleanedSearchStream.str();

        // Search the real AnujaMart product database.
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
                    << "I found these products in AnujaMart:";

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
        else
        {
            reply =
                "I could not find that product in AnujaMart. "
                "Please check the product name and try again.";
        }
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
