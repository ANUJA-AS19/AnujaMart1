#include "MigrationService.h"

#include "../repository/Database.h"

#include <iostream>
#include <pqxx/pqxx>

namespace anuja::anujamart
{

void MigrationService::runMigrations()
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        std::cerr
            << "Migration failed: database connection is not available."
            << std::endl;

        return;
    }

    try
    {
        pqxx::transaction transaction(*db);

        transaction.exec(R"SQL(
            CREATE TABLE IF NOT EXISTS schema_migrations (
                version INTEGER PRIMARY KEY,
                applied_at TIMESTAMPTZ NOT NULL
                    DEFAULT CURRENT_TIMESTAMP
            );
        )SQL");

        const auto result = transaction.exec(
            "SELECT COUNT(*) "
            "FROM schema_migrations "
            "WHERE version = 1;");

        const bool migrationExists =
            result[0][0].as<int>() > 0;

        if (!migrationExists)
        {
            transaction.exec(R"SQL(
                CREATE TABLE IF NOT EXISTS users (
                    id BIGSERIAL PRIMARY KEY,
                    name VARCHAR(100) NOT NULL,
                    email VARCHAR(255) NOT NULL UNIQUE,
                    password_hash TEXT NOT NULL,
                    role VARCHAR(10) NOT NULL
                        CHECK (role IN ('BUYER', 'SELLER', 'ADMIN')),
                    created_at TIMESTAMPTZ NOT NULL
                        DEFAULT CURRENT_TIMESTAMP
                );

                CREATE TABLE IF NOT EXISTS products (
                    id BIGSERIAL PRIMARY KEY,
                    seller_id BIGINT NOT NULL,
                    name VARCHAR(200) NOT NULL,
                    description TEXT NOT NULL DEFAULT '',
                    price_cents BIGINT NOT NULL
                        CHECK (price_cents >= 0),
                    stock_qty BIGINT NOT NULL
                        CHECK (stock_qty >= 0),
                    category VARCHAR(100) NOT NULL,
                    image_url TEXT NOT NULL DEFAULT '',
                    is_active BOOLEAN NOT NULL DEFAULT TRUE,
                    created_at TIMESTAMPTZ NOT NULL
                        DEFAULT CURRENT_TIMESTAMP,
                    FOREIGN KEY (seller_id)
                        REFERENCES users(id)
                );

                CREATE TABLE IF NOT EXISTS orders (
                    id BIGSERIAL PRIMARY KEY,
                    buyer_id BIGINT NOT NULL,
                    status VARCHAR(20) NOT NULL
                        CHECK (
                            status IN (
                                'PENDING',
                                'CONFIRMED',
                                'SHIPPED',
                                'DELIVERED',
                                'CANCELLED'
                            )
                        ),
                    total_amount_cents BIGINT NOT NULL
                        CHECK (total_amount_cents >= 0),
                    created_at TIMESTAMPTZ NOT NULL
                        DEFAULT CURRENT_TIMESTAMP,
                    FOREIGN KEY (buyer_id)
                        REFERENCES users(id)
                );

                CREATE TABLE IF NOT EXISTS order_items (
                    id BIGSERIAL PRIMARY KEY,
                    order_id BIGINT NOT NULL,
                    product_id BIGINT NOT NULL,
                    quantity BIGINT NOT NULL
                        CHECK (quantity > 0),
                    unit_price_cents BIGINT NOT NULL
                        CHECK (unit_price_cents >= 0),
                    FOREIGN KEY (order_id)
                        REFERENCES orders(id)
                        ON DELETE CASCADE,
                    FOREIGN KEY (product_id)
                        REFERENCES products(id)
                );

                CREATE TABLE IF NOT EXISTS cart_items (
                    id BIGSERIAL PRIMARY KEY,
                    user_id BIGINT NOT NULL,
                    product_id BIGINT NOT NULL,
                    quantity BIGINT NOT NULL
                        CHECK (quantity > 0),
                    FOREIGN KEY (user_id)
                        REFERENCES users(id)
                        ON DELETE CASCADE,
                    FOREIGN KEY (product_id)
                        REFERENCES products(id)
                        ON DELETE CASCADE,
                    UNIQUE (user_id, product_id)
                );

                CREATE TABLE IF NOT EXISTS reviews (
                    id BIGSERIAL PRIMARY KEY,
                    product_id BIGINT NOT NULL,
                    user_id BIGINT NOT NULL,
                    rating INTEGER NOT NULL
                        CHECK (rating BETWEEN 1 AND 5),
                    comment TEXT NOT NULL DEFAULT '',
                    created_at TIMESTAMPTZ NOT NULL
                        DEFAULT CURRENT_TIMESTAMP,
                    FOREIGN KEY (product_id)
                        REFERENCES products(id)
                        ON DELETE CASCADE,
                    FOREIGN KEY (user_id)
                        REFERENCES users(id)
                        ON DELETE CASCADE
                );

                CREATE TABLE IF NOT EXISTS sessions (
                    id BIGSERIAL PRIMARY KEY,
                    session_token VARCHAR(128) NOT NULL UNIQUE,
                    user_id BIGINT NOT NULL,
                    created_at TIMESTAMPTZ NOT NULL
                        DEFAULT CURRENT_TIMESTAMP,
                    last_activity_at TIMESTAMPTZ NOT NULL
                        DEFAULT CURRENT_TIMESTAMP,
                    expires_at TIMESTAMPTZ NOT NULL,
                    FOREIGN KEY (user_id)
                        REFERENCES users(id)
                        ON DELETE CASCADE
                );

                CREATE INDEX IF NOT EXISTS idx_products_seller
                    ON products(seller_id);

                CREATE INDEX IF NOT EXISTS idx_orders_buyer
                    ON orders(buyer_id);

                CREATE INDEX IF NOT EXISTS idx_order_items_order
                    ON order_items(order_id);

                CREATE INDEX IF NOT EXISTS idx_order_items_product
                    ON order_items(product_id);

                CREATE INDEX IF NOT EXISTS idx_cart_items_user
                    ON cart_items(user_id);

                CREATE INDEX IF NOT EXISTS idx_cart_items_product
                    ON cart_items(product_id);

                CREATE INDEX IF NOT EXISTS idx_reviews_product
                    ON reviews(product_id);

                CREATE INDEX IF NOT EXISTS idx_reviews_user
                    ON reviews(user_id);

                CREATE INDEX IF NOT EXISTS idx_sessions_token
                    ON sessions(session_token);

                CREATE INDEX IF NOT EXISTS idx_sessions_user
                    ON sessions(user_id);

                INSERT INTO schema_migrations (version)
                VALUES (1);
            )SQL");

            transaction.commit();

            std::cout
                << "PostgreSQL migration V1 completed successfully."
                << std::endl;
        }
        else
        {
            transaction.exec(
                "ALTER TABLE products "
                "ADD COLUMN IF NOT EXISTS "
                "is_active BOOLEAN NOT NULL DEFAULT TRUE;");

            transaction.exec(R"SQL(
                CREATE TABLE IF NOT EXISTS sessions (
                    id BIGSERIAL PRIMARY KEY,
                    session_token VARCHAR(128) NOT NULL UNIQUE,
                    user_id BIGINT NOT NULL,
                    created_at TIMESTAMPTZ NOT NULL
                        DEFAULT CURRENT_TIMESTAMP,
                    last_activity_at TIMESTAMPTZ NOT NULL
                        DEFAULT CURRENT_TIMESTAMP,
                    expires_at TIMESTAMPTZ NOT NULL,
                    FOREIGN KEY (user_id)
                        REFERENCES users(id)
                        ON DELETE CASCADE
                );

                CREATE INDEX IF NOT EXISTS idx_sessions_token
                    ON sessions(session_token);

                CREATE INDEX IF NOT EXISTS idx_sessions_user
                    ON sessions(user_id);
            )SQL");

            transaction.commit();

            std::cout
                << "PostgreSQL database is already up to date."
                << std::endl;
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "PostgreSQL migration failed: "
            << exception.what()
            << std::endl;
    }
}

} // namespace anuja::anujamart