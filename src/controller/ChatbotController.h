#pragma once

#include "../service/ProductBrowseService.h"

#include <drogon/HttpController.h>

namespace anuja::anujamart
{

class ChatbotController
    : public drogon::HttpController<ChatbotController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ChatbotController::chat,
        "/api/v1/chatbot",
        drogon::Post);

    METHOD_LIST_END

    void chat(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    ProductBrowseRepository productBrowseRepository_;
    ProductBrowseService productBrowseService_{
        productBrowseRepository_};
};

}
