#include "ProductService.h"

namespace anuja::anujamart
{

ProductService::ProductService(
    ProductRepository& productRepository)
    : productRepository_(productRepository)
{
}

bool ProductService::createProduct(
    std::int64_t sellerId,
    const std::string& name,
    const std::string& description,
    std::int64_t priceCents,
    std::int64_t stockQty,
    const std::string& category,
    const std::string& imageUrl)
{
    if (sellerId <= 0 ||
        name.empty() ||
        description.empty() ||
        category.empty() ||
        priceCents < 0 ||
        stockQty < 0)
    {
        return false;
    }

    return productRepository_.createProduct(
        sellerId,
        name,
        description,
        priceCents,
        stockQty,
        category,
        imageUrl);
}

std::vector<Product> ProductService::getSellerProducts(
    std::int64_t sellerId)
{
    if (sellerId <= 0)
        return {};

    return productRepository_.findBySeller(sellerId);
}

std::optional<Product> ProductService::getProduct(
    std::int64_t productId)
{
    if (productId <= 0)
        return std::nullopt;

    return productRepository_.findById(productId);
}

bool ProductService::updateProduct(
    std::int64_t productId,
    std::int64_t sellerId,
    const std::string& name,
    const std::string& description,
    std::int64_t priceCents,
    std::int64_t stockQty,
    const std::string& category,
    const std::string& imageUrl)
{
    if (productId <= 0 ||
        sellerId <= 0 ||
        name.empty() ||
        description.empty() ||
        category.empty() ||
        priceCents < 0 ||
        stockQty < 0)
    {
        return false;
    }

    return productRepository_.updateProduct(
        productId,
        sellerId,
        name,
        description,
        priceCents,
        stockQty,
        category,
        imageUrl);
}

bool ProductService::deleteProduct(
    std::int64_t productId,
    std::int64_t sellerId)
{
    if (productId <= 0 || sellerId <= 0)
        return false;

    return productRepository_.deleteProduct(
        productId,
        sellerId);
}

bool ProductService::adminDeleteProduct(
    std::int64_t adminId,
    std::int64_t productId)
{
    if (adminId <= 0 || productId <= 0)
        return false;

    return productRepository_.adminDeleteProduct(
        productId);
}

} // namespace anuja::anujamart
