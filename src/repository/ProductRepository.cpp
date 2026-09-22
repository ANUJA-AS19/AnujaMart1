#include "ProductRepository.h"
#include "Database.h"

#include <pqxx/pqxx>
#include <iostream>

namespace anuja::anujamart
{

bool ProductRepository::createProduct(
    std::int64_t sellerId,
    const std::string& name,
    const std::string& description,
    std::int64_t priceCents,
    std::int64_t stockQty,
    const std::string& category,
    const std::string& imageUrl)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        transaction.exec_params(
            "INSERT INTO products "
            "(seller_id, name, description, price_cents, "
            "stock_qty, category, image_url) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7)",
            sellerId,
            name,
            description,
            priceCents,
            stockQty,
            category,
            imageUrl);

        transaction.commit();
        return true;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Create product failed: "
                  << exception.what()
                  << std::endl;

        return false;
    }
}

std::vector<Product> ProductRepository::findBySeller(
    std::int64_t sellerId)
{
    std::vector<Product> products;

    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return products;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec_params(
            "SELECT id, seller_id, name, description, "
            "price_cents, stock_qty, category, image_url "
            "FROM products "
            "WHERE seller_id = $1 AND is_active = TRUE "
            "ORDER BY id",
            sellerId);

        for (const auto& row : result)
        {
            Product product;

            product.id = row["id"].as<std::int64_t>();
            product.sellerId =
                row["seller_id"].as<std::int64_t>();
            product.name =
                row["name"].as<std::string>();
            product.description =
                row["description"].as<std::string>();
            product.priceCents =
                row["price_cents"].as<std::int64_t>();
            product.stockQty =
                row["stock_qty"].as<std::int64_t>();
            product.category =
                row["category"].as<std::string>();
            product.imageUrl =
                row["image_url"].as<std::string>();

            products.push_back(product);
        }

        transaction.commit();
    }
    catch (const std::exception&)
    {
        products.clear();
    }

    return products;
}

std::optional<Product> ProductRepository::findById(
    std::int64_t productId)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return std::nullopt;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec_params(
            "SELECT id, seller_id, name, description, "
            "price_cents, stock_qty, category, image_url "
            "FROM products "
            "WHERE id = $1 AND is_active = TRUE",
            productId);

        if (result.empty())
        {
            transaction.commit();
            return std::nullopt;
        }

        const auto& row = result[0];

        Product product;

        product.id = row["id"].as<std::int64_t>();
        product.sellerId =
            row["seller_id"].as<std::int64_t>();
        product.name =
            row["name"].as<std::string>();
        product.description =
            row["description"].as<std::string>();
        product.priceCents =
            row["price_cents"].as<std::int64_t>();
        product.stockQty =
            row["stock_qty"].as<std::int64_t>();
        product.category =
            row["category"].as<std::string>();
        product.imageUrl =
            row["image_url"].as<std::string>();

        transaction.commit();

        return product;
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

bool ProductRepository::updateProduct(
    std::int64_t productId,
    std::int64_t sellerId,
    const std::string& name,
    const std::string& description,
    std::int64_t priceCents,
    std::int64_t stockQty,
    const std::string& category,
    const std::string& imageUrl)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec_params(
            "UPDATE products "
            "SET name = $1, "
            "description = $2, "
            "price_cents = $3, "
            "stock_qty = $4, "
            "category = $5, "
            "image_url = $6 "
            "WHERE id = $7 "
            "AND seller_id = $8 "
            "AND is_active = TRUE",
            name,
            description,
            priceCents,
            stockQty,
            category,
            imageUrl,
            productId,
            sellerId);

        transaction.commit();

        return result.affected_rows() == 1;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ProductRepository::deleteProduct(
    std::int64_t productId,
    std::int64_t sellerId)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec_params(
            "UPDATE products "
            "SET is_active = FALSE "
            "WHERE id = $1 "
            "AND seller_id = $2 "
            "AND is_active = TRUE",
            productId,
            sellerId);

        std::cerr << "Deleting productId=" << productId
                  << ", sellerId=" << sellerId
                  << ", affectedRows="
                  << result.affected_rows()
                  << std::endl;

        transaction.commit();

        return result.affected_rows() == 1;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ProductRepository::adminDeleteProduct(
    std::int64_t productId)
{
    auto* db = Database::instance().connection();

    if (db == nullptr || productId <= 0)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec_params(
            "UPDATE products "
            "SET is_active = FALSE "
            "WHERE id = $1 "
            "AND is_active = TRUE",
            productId);

        transaction.commit();

        return result.affected_rows() == 1;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ProductRepository::decreaseStock(
    std::int64_t productId,
    std::int64_t quantity)
{
    auto* db = Database::instance().connection();

    if (db == nullptr ||
        productId <= 0 ||
        quantity <= 0)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec_params(
            "UPDATE products "
            "SET stock_qty = stock_qty - $1 "
            "WHERE id = $2 "
            "AND is_active = TRUE "
            "AND stock_qty >= $1",
            quantity,
            productId);

        transaction.commit();

        return result.affected_rows() == 1;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

}
