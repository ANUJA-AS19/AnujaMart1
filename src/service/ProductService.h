#pragma once

#include "../model/Product.h"
#include "../repository/ProductRepository.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace anuja::anujamart
{

class ProductService
{
public:
    explicit ProductService(
        ProductRepository& productRepository);

    bool createProduct(
        std::int64_t sellerId,
        const std::string& name,
        const std::string& description,
        std::int64_t priceCents,
        std::int64_t stockQty,
        const std::string& category,
        const std::string& imageUrl);

    std::vector<Product> getSellerProducts(
        std::int64_t sellerId);

    std::optional<Product> getProduct(
        std::int64_t productId);

    bool updateProduct(
        std::int64_t productId,
        std::int64_t sellerId,
        const std::string& name,
        const std::string& description,
        std::int64_t priceCents,
        std::int64_t stockQty,
        const std::string& category,
        const std::string& imageUrl);

    bool deleteProduct(
        std::int64_t productId,
        std::int64_t sellerId);

    bool adminDeleteProduct(
        std::int64_t adminId,
        std::int64_t productId);

private:
    ProductRepository& productRepository_;
};

} // namespace anuja::anujamart
