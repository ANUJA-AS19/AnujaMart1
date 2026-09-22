#include "AuthController.h"

#include <drogon/drogon.h>

namespace anuja::anujamart
{

void AuthController::registerUser(
    const drogon::HttpRequestPtr& request,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = request->getJsonObject();

    if (!json ||
        !json->isMember("name") ||
        !json->isMember("email") ||
        !json->isMember("password"))
    {
        Json::Value error;
        error["message"] = "name, email and password are required";

        auto response = drogon::HttpResponse::newHttpJsonResponse(error);
        response->setStatusCode(drogon::k400BadRequest);

        callback(response);
        return;
    }

    const std::string name = (*json)["name"].asString();
    const std::string email = (*json)["email"].asString();
    const std::string password = (*json)["password"].asString();

    // Public registration always creates a BUYER.
    // Seller and admin accounts must be created through authorized administration.
       UserRole role = UserRole::BUYER;
   if (json->isMember("role") &&
       (*json)["role"].asString() == "SELLER")
   {
       role = UserRole::SELLER;
   }

    const bool created = authService_.registerUser(
        name,
        email,
        password,
        role);

    Json::Value responseJson;

    if (!created)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] = "Registration failed";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    responseJson["success"] = true;
    responseJson["data"]["message"] = "User registered successfully";
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k201Created);
    callback(response);
}

void AuthController::login(
    const drogon::HttpRequestPtr& request,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = request->getJsonObject();

    if (!json ||
        !json->isMember("email") ||
        !json->isMember("password"))
    {
        Json::Value error;
        error["message"] = "email and password are required";

        auto response = drogon::HttpResponse::newHttpJsonResponse(error);
        response->setStatusCode(drogon::k400BadRequest);

        callback(response);
        return;
    }

    const std::string email = (*json)["email"].asString();
    const std::string password = (*json)["password"].asString();

    auto user = authService_.login(email, password);

    Json::Value responseJson;

    if (!user.has_value())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"] = "Invalid email or password";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    auto sessionToken = sessionService_.createSession(user->id);

    if (!sessionToken.has_value()) { callback(drogon::HttpResponse::newHttpResponse()); return; }

    responseJson["success"] = true;
    responseJson["data"]["token"] = *sessionToken;
    responseJson["data"]["id"] =
        static_cast<Json::Int64>(user->id);
    responseJson["data"]["name"] = user->name;
    responseJson["data"]["email"] = user->email;

    if (user->role == UserRole::SELLER)
    {
        responseJson["data"]["role"] = "SELLER";
    }
    else if (user->role == UserRole::ADMIN)
    {
        responseJson["data"]["role"] = "ADMIN";
    }
    else
    {
        responseJson["data"]["role"] = "BUYER";
    }

    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    callback(response);
}

} // namespace anuja::anujamart