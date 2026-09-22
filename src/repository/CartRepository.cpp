#include "CartRepository.h"

#include "Database.h"

#include <pqxx/pqxx>

namespace anuja::anujamart
{
bool CartRepository::addItem(
    std::int64_t userId,
    std::int64_t productId,
    std::int64_t quantity)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        transaction.exec(
            "INSERT INTO cart_items "
            "(user_id, product_id, quantity) "
            "VALUES ($1, $2, $3) "
            "ON CONFLICT (user_id, product_id) "
            "DO UPDATE SET quantity = cart_items.quantity + EXCLUDED.quantity",
            pqxx::params{userId, productId, quantity});

        transaction.commit();
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::vector<CartItem> CartRepository::findByUser(
    std::int64_t userId)
{
    std::vector<CartItem> items;

    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return items;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "SELECT id, user_id, product_id, quantity "
            "FROM cart_items "
            "WHERE user_id = $1 "
            "ORDER BY id",
            pqxx::params{userId});

        for (const auto& row : result)
        {
            CartItem item;

            item.id = row["id"].as<std::int64_t>();
            item.userId = row["user_id"].as<std::int64_t>();
            item.productId = row["product_id"].as<std::int64_t>();
            item.quantity = row["quantity"].as<std::int64_t>();

            items.push_back(item);
        }

        transaction.commit();
    }
    catch (const std::exception&)
    {
        items.clear();
    }

    return items;
}

bool CartRepository::updateItem(
    std::int64_t userId,
    std::int64_t productId,
    std::int64_t quantity)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "UPDATE cart_items "
            "SET quantity = $1 "
            "WHERE user_id = $2 "
            "AND product_id = $3",
            pqxx::params{quantity, userId, productId});

        transaction.commit();

        return result.affected_rows() > 0;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool CartRepository::removeItem(
    std::int64_t userId,
    std::int64_t productId)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "DELETE FROM cart_items "
            "WHERE user_id = $1 "
            "AND product_id = $2",
            pqxx::params{userId, productId});

        transaction.commit();

        return result.affected_rows() > 0;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::int64_t CartRepository::calculateTotal(
    std::int64_t userId)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return 0;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "SELECT COALESCE("
            "SUM(p.price_cents * c.quantity), 0) AS total "
            "FROM cart_items c "
            "JOIN products p ON p.id = c.product_id "
            "WHERE c.user_id = $1",
            pqxx::params{userId});

        transaction.commit();

        if (result.empty())
        {
            return 0;
        }

        return result[0]["total"].as<std::int64_t>();
    }
    catch (const std::exception&)
    {
        return 0;
    }
}

bool CartRepository::clearCart(
    std::int64_t userId)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        transaction.exec(
            "DELETE FROM cart_items "
            "WHERE user_id = $1",
            pqxx::params{userId});

        transaction.commit();
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}
}
