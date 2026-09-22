#include "ReviewRepository.h"

#include "Database.h"

#include <pqxx/pqxx>

namespace anuja::anujamart
{

bool ReviewRepository::createReview(
    std::int64_t productId,
    std::int64_t userId,
    int rating,
    const std::string& comment)
{
    auto* db = Database::instance().connection();

    if (db == nullptr ||
        productId <= 0 ||
        userId <= 0 ||
        rating < 1 ||
        rating > 5)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec_params(
            "INSERT INTO reviews "
            "(product_id, user_id, rating, comment) "
            "VALUES ($1, $2, $3, $4) "
            "RETURNING id",
            productId,
            userId,
            rating,
            comment);

        transaction.commit();

        return !result.empty();
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::vector<Review> ReviewRepository::findByProduct(
    std::int64_t productId)
{
    std::vector<Review> reviews;

    auto* db = Database::instance().connection();

    if (db == nullptr || productId <= 0)
        return reviews;

    try
    {
        pqxx::read_transaction transaction(*db);

        const auto result = transaction.exec_params(
            "SELECT id, product_id, user_id, rating, "
            "comment, created_at "
            "FROM reviews "
            "WHERE product_id = $1 "
            "ORDER BY created_at DESC",
            productId);

        for (const auto& row : result)
        {
            Review review;

            review.id = row["id"].as<std::int64_t>();
            review.productId =
                row["product_id"].as<std::int64_t>();
            review.userId =
                row["user_id"].as<std::int64_t>();
            review.rating =
                row["rating"].as<int>();
            review.comment =
                row["comment"].as<std::string>();
            review.createdAt =
                row["created_at"].as<std::string>();

            reviews.push_back(review);
        }
    }
    catch (const std::exception&)
    {
        return {};
    }

    return reviews;
}

} // namespace anuja::anujamart
