#pragma once

#include "../service/ProductService.h"

#include <drogon/HttpController.h>
#include <string>

namespace anuja::anujamart
{

class ProductController
    : public drogon::HttpController<ProductController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ProductController::createProduct,
        "/api/v1/products",
        drogon::Post, "anuja::anujamart::AuthFilter");

    ADD_METHOD_TO(
        ProductController::getSellerProducts,
        "/api/v1/products/seller/{1}",
        drogon::Get);

    ADD_METHOD_TO(
        ProductController::getProduct,
        "/api/v1/products/{1}",
        drogon::Get);

    ADD_METHOD_TO(
        ProductController::updateProduct,
        "/api/v1/products/{1}",
        drogon::Put, "anuja::anujamart::AuthFilter");

    ADD_METHOD_TO(
        ProductController::deleteProduct,
        "/api/v1/products/{1}",
        drogon::Delete, "anuja::anujamart::AuthFilter");

    METHOD_LIST_END

    void createProduct(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getSellerProducts(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::int64_t sellerId);

    void getProduct(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        const std::string& productIdText);

    void updateProduct(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::int64_t productId);

    void deleteProduct(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        const std::string& productIdText);

private:
    ProductRepository productRepository_;
    ProductService productService_{productRepository_};
};

} // namespace anuja::anujamart