#include "SessionRepository.h"
#include "Database.h"

#include <iostream>
#include <pqxx/pqxx>

namespace anuja::anujamart
{

bool SessionRepository::createSession(
    const std::string& token,
    std::int64_t userId,
    int expiryMinutes)
{
    auto* db = Database::instance().connection();

    if (db == nullptr || token.empty())
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        transaction.exec(
            "INSERT INTO sessions "
            "(session_token, user_id, expires_at) "
            "VALUES ($1, $2, CURRENT_TIMESTAMP + "
            "($3 * INTERVAL '1 minute'))",
            pqxx::params{
                token,
                userId,
                expiryMinutes
            });

        transaction.commit();

        return true;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Create session failed: "
                  << exception.what()
                  << std::endl;

        return false;
    }
}

std::optional<Session>
SessionRepository::findValidSession(
    const std::string& token)
{
    auto* db = Database::instance().connection();

    if (db == nullptr || token.empty())
    {
        return std::nullopt;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "SELECT user_id "
            "FROM sessions "
            "WHERE session_token = $1 "
            "AND expires_at > CURRENT_TIMESTAMP "
            "AND last_activity_at > "
            "CURRENT_TIMESTAMP - INTERVAL '30 minutes'",
            pqxx::params{token});

        transaction.commit();

        if (result.empty())
        {
            return std::nullopt;
        }

        Session session;
        session.token = token;
        session.userId =
            result[0]["user_id"].as<std::int64_t>();

        return session;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Find session failed: "
                  << exception.what()
                  << std::endl;

        return std::nullopt;
    }
}

bool SessionRepository::updateActivity(
    const std::string& token)
{
    auto* db = Database::instance().connection();

    if (db == nullptr || token.empty())
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "UPDATE sessions "
            "SET last_activity_at = CURRENT_TIMESTAMP "
            "WHERE session_token = $1 "
            "AND expires_at > CURRENT_TIMESTAMP",
            pqxx::params{token});

        transaction.commit();

        return result.affected_rows() > 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Update session activity failed: "
                  << exception.what()
                  << std::endl;

        return false;
    }
}

bool SessionRepository::deleteSession(
    const std::string& token)
{
    auto* db = Database::instance().connection();

    if (db == nullptr || token.empty())
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "DELETE FROM sessions "
            "WHERE session_token = $1",
            pqxx::params{token});

        transaction.commit();

        return result.affected_rows() > 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Delete session failed: "
                  << exception.what()
                  << std::endl;

        return false;
    }
}

} // namespace anuja::anujamart
