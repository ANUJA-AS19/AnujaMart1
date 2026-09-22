#include "OrderRepository.h"
#include "Database.h"

#include <pqxx/pqxx>

namespace anuja::anujamart
{

bool OrderRepository::createOrder(
    std::int64_t buyerId,
    std::int64_t totalAmountCents,
    std::int64_t& orderId)
{
    try
    {
        pqxx::connection* connection =
            Database::instance().connection();

        pqxx::work transaction(*connection);

        const auto result = transaction.exec_params(
            R"(
                INSERT INTO orders (
                    buyer_id,
                    status,
                    total_amount_cents
                )
                VALUES ($1, 'CONFIRMED', $2)
                RETURNING id
            )",
            buyerId,
            totalAmountCents);

        if (result.empty())
        {
            return false;
        }

        orderId =
            result[0][0].as<std::int64_t>();

        transaction.commit();
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool OrderRepository::addOrderItem(
    std::int64_t orderId,
    std::int64_t productId,
    std::int64_t quantity,
    std::int64_t unitPriceCents)
{
    try
    {
        pqxx::connection* connection =
            Database::instance().connection();

        pqxx::work transaction(*connection);

        transaction.exec_params(
            R"(
                INSERT INTO order_items (
                    order_id,
                    product_id,
                    quantity,
                    unit_price_cents
                )
                VALUES ($1, $2, $3, $4)
            )",
            orderId,
            productId,
            quantity,
            unitPriceCents);

        transaction.commit();
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::vector<Order> OrderRepository::findByBuyer(
    std::int64_t buyerId)
{
    std::vector<Order> orders;

    try
    {
        pqxx::connection* connection =
            Database::instance().connection();

        pqxx::work transaction(*connection);

        const auto result = transaction.exec_params(
            R"(
                SELECT
                    id,
                    buyer_id,
                    status,
                    total_amount_cents
                FROM orders
                WHERE buyer_id = $1
                ORDER BY id DESC
            )",
            buyerId);

        for (const auto& row : result)
        {
            Order order;

            order.id =
                row["id"].as<std::int64_t>();

            order.buyerId =
                row["buyer_id"].as<std::int64_t>();

            order.status =
                row["status"].as<std::string>();

            order.totalAmountCents =
                row["total_amount_cents"].as<std::int64_t>();

            orders.push_back(order);
        }

        transaction.commit();
    }
    catch (const std::exception&)
    {
        return {};
    }

    return orders;
}

std::vector<Order> OrderRepository::findBySeller(
    std::int64_t sellerId)
{
    std::vector<Order> orders;

    try
    {
        pqxx::connection* connection =
            Database::instance().connection();

        pqxx::work transaction(*connection);

        const auto result = transaction.exec_params(
            R"(
                SELECT DISTINCT
                    o.id,
                    o.buyer_id,
                    o.status,
                    o.total_amount_cents
                FROM orders o
                INNER JOIN order_items oi
                    ON oi.order_id = o.id
                INNER JOIN products p
                    ON p.id = oi.product_id
                WHERE p.seller_id = $1
                ORDER BY o.id DESC
            )",
            sellerId);

        for (const auto& row : result)
        {
            Order order;

            order.id =
                row["id"].as<std::int64_t>();

            order.buyerId =
                row["buyer_id"].as<std::int64_t>();

            order.status =
                row["status"].as<std::string>();

            order.totalAmountCents =
                row["total_amount_cents"].as<std::int64_t>();

            orders.push_back(order);
        }

        transaction.commit();
    }
    catch (const std::exception&)
    {
        return {};
    }

    return orders;
}

bool OrderRepository::updateStatus(
    std::int64_t orderId,
    std::int64_t sellerId,
    const std::string& status)
{
    try
    {
        pqxx::connection* connection =
            Database::instance().connection();

        pqxx::work transaction(*connection);

        const auto result = transaction.exec_params(
            R"(
                UPDATE orders
                SET status = $1
                WHERE id = $2
                  AND EXISTS (
                      SELECT 1
                      FROM order_items oi
                      INNER JOIN products p
                          ON p.id = oi.product_id
                      WHERE oi.order_id = orders.id
                        AND p.seller_id = $3
                  )
                RETURNING id
            )",
            status,
            orderId,
            sellerId);

        if (result.empty())
        {
            return false;
        }

        transaction.commit();
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::vector<Order> OrderRepository::findAll()
{
    std::vector<Order> orders;

    try
    {
        pqxx::connection* connection =
            Database::instance().connection();

        pqxx::work transaction(*connection);

        const auto result = transaction.exec(
            R"(
                SELECT
                    id,
                    buyer_id,
                    status,
                    total_amount_cents
                FROM orders
                ORDER BY id DESC
            )");

        for (const auto& row : result)
        {
            Order order;

            order.id =
                row["id"].as<std::int64_t>();

            order.buyerId =
                row["buyer_id"].as<std::int64_t>();

            order.status =
                row["status"].as<std::string>();

            order.totalAmountCents =
                row["total_amount_cents"].as<std::int64_t>();

            orders.push_back(order);
        }

        transaction.commit();
    }
    catch (const std::exception&)
    {
        return {};
    }

    return orders;
}

}
