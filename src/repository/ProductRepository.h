#pragma once

#include "../model/Product.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace anuja::anujamart
{

class ProductRepository
{
public:
    bool createProduct(
        std::int64_t sellerId,
        const std::string& name,
        const std::string& description,
        std::int64_t priceCents,
        std::int64_t stockQty,
        const std::string& category,
        const std::string& imageUrl);

    std::vector<Product> findBySeller(
        std::int64_t sellerId);

    std::optional<Product> findById(
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
    std::int64_t productId);
bool decreaseStock(
    std::int64_t productId,
    std::int64_t quantity);
};

} // namespace anuja::anujamart
