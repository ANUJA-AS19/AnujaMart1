#pragma once

#include "../repository/CartRepository.h"
#include "../service/CartService.h"

#include <drogon/HttpController.h>

namespace anuja::anujamart
{

class CartController
    : public drogon::HttpController<CartController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        CartController::addItem,
        "/api/v1/cart/items",
        drogon::Post, "anuja::anujamart::AuthFilter");

    ADD_METHOD_TO(
        CartController::getCart,
        "/api/v1/cart",
        drogon::Get, "anuja::anujamart::AuthFilter");

    ADD_METHOD_TO(
        CartController::updateItem,
        "/api/v1/cart/items/{1}",
        drogon::Put, "anuja::anujamart::AuthFilter");

    ADD_METHOD_TO(
        CartController::removeItem,
        "/api/v1/cart/items/{1}",
        drogon::Delete, "anuja::anujamart::AuthFilter");

    METHOD_LIST_END

    void addItem(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback);

    void getCart(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback);

    void updateItem(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback,
        std::int64_t productId);

    void removeItem(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback,
        std::int64_t productId);

private:
    CartRepository cartRepository_;

    CartService cartService_{
        cartRepository_};
};

}