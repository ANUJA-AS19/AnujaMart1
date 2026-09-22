#include "ProductBrowseRepository.h"

#include "Database.h"

#include <iostream>
#include <pqxx/pqxx>

namespace anuja::anujamart
{
std::vector<Product> ProductBrowseRepository::searchProducts(
    const std::string& search,
    const std::string& category)
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

        const auto result = transaction.exec(
            "SELECT id, seller_id, name, description, "
            "price_cents, stock_qty, category, image_url "
            "FROM products "
            "WHERE is_active = TRUE AND "
            "($1 = '' OR "
            "LOWER(name) LIKE LOWER('%' || $1 || '%') OR "
            "LOWER(description) LIKE LOWER('%' || $1 || '%')) "
            "AND "
            "($2 = '' OR LOWER(category) = LOWER($2)) "
            "ORDER BY id",
            pqxx::params{search, category});

        for (const auto& row : result)
        {
            Product product;

            product.id =
                row["id"].as<std::int64_t>();

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
    catch (const std::exception& exception)
    {
        std::cerr
            << "Product search failed: "
            << exception.what()
            << std::endl;
    }

    return products;
}
}