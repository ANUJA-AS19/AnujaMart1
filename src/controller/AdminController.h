#pragma once

#include "../repository/CartRepository.h"
#include "../repository/OrderRepository.h"
#include "../repository/ProductRepository.h"
#include "../service/OrderService.h"
#include "../service/ProductService.h"
#include "../service/UserService.h"

#include <drogon/HttpController.h>

namespace anuja::anujamart
{

class AdminController
    : public drogon::HttpController<AdminController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        AdminController::getAllUsers,
        "/api/v1/admin/users",
        drogon::Get, "anuja::anujamart::AuthFilter");

    ADD_METHOD_TO(
        AdminController::getAllOrders,
        "/api/v1/admin/orders",
        drogon::Get, "anuja::anujamart::AuthFilter");

    ADD_METHOD_TO(
        AdminController::deleteProduct,
        "/api/v1/admin/products/{product_id}",
        drogon::Delete, "anuja::anujamart::AuthFilter");

    METHOD_LIST_END

    void getAllUsers(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback);

    void getAllOrders(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback);

    void deleteProduct(
        const drogon::HttpRequestPtr& request,
        std::function<void(
            const drogon::HttpResponsePtr&)>&& callback,
        std::int64_t productId);

private:
    UserRepository userRepository_;
    UserService userService_{userRepository_};

    OrderRepository orderRepository_;
    CartRepository cartRepository_;
    ProductRepository productRepository_;

    OrderService orderService_{
        orderRepository_,
        cartRepository_,
        productRepository_};

    ProductService productService_{
        productRepository_};
};

} // namespace anuja::anujamart
