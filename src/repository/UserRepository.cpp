#include "UserRepository.h"
#include "Database.h"

#include <pqxx/pqxx>
#include <iostream>

namespace anuja::anujamart
{

bool UserRepository::createUser(
    const std::string& name,
    const std::string& email,
    const std::string& passwordHash,
    UserRole role)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return false;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const std::string roleText =
            role == UserRole::BUYER ? "BUYER" :
            role == UserRole::SELLER ? "SELLER" :
            "ADMIN";

        transaction.exec(
            "INSERT INTO users "
            "(name, email, password_hash, role) "
            "VALUES ($1, $2, $3, $4)",
            pqxx::params{
                name,
                email,
                passwordHash,
                roleText
            });

        transaction.commit();

        return true;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Create user failed: "
                  << exception.what()
                  << std::endl;

        return false;
    }
}

std::optional<User> UserRepository::findByEmail(
    const std::string& email)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return std::nullopt;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "SELECT id, name, email, password_hash, role "
            "FROM users "
            "WHERE email = $1",
            pqxx::params{email});

        transaction.commit();

        if (result.empty())
        {
            return std::nullopt;
        }

        const auto& row = result[0];

        User user;

        user.id = row["id"].as<std::int64_t>();
        user.name = row["name"].c_str();
        user.email = row["email"].c_str();
        user.passwordHash = row["password_hash"].c_str();

        const std::string roleText = row["role"].c_str();

        if (roleText == "BUYER")
        {
            user.role = UserRole::BUYER;
        }
        else if (roleText == "SELLER")
        {
            user.role = UserRole::SELLER;
        }
        else
        {
            user.role = UserRole::ADMIN;
        }

        return user;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Find user failed: "
                  << exception.what()
                  << std::endl;

        return std::nullopt;
    }
}
 std::optional<User> UserRepository::findById(
    std::int64_t id)
{
    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return std::nullopt;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "SELECT id, name, email, password_hash, role "
            "FROM users WHERE id = $1",
            pqxx::params{id});

        transaction.commit();

        if (result.empty())
        {
            return std::nullopt;
        }

        const auto& row = result[0];

        User user;
        user.id = row["id"].as<std::int64_t>();
        user.name = row["name"].c_str();
        user.email = row["email"].c_str();
        user.passwordHash = row["password_hash"].c_str();

        const std::string role = row["role"].c_str();

        user.role = role == "BUYER"
            ? UserRole::BUYER
            : role == "SELLER"
                ? UserRole::SELLER
                : UserRole::ADMIN;

        return user;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Find user by ID failed: "
                  << exception.what() << std::endl;

        return std::nullopt;
    }
}

std::vector<User> UserRepository::findAll()
{
    std::vector<User> users;

    auto* db = Database::instance().connection();

    if (db == nullptr)
    {
        return users;
    }

    try
    {
        pqxx::transaction transaction(*db);

        const auto result = transaction.exec(
            "SELECT id, name, email, password_hash, role "
             "FROM users "
            "ORDER BY id ASC");

        for (const auto& row : result)
        {
            User user;

            user.id = row["id"].as<std::int64_t>();
            user.name = row["name"].c_str();
            user.email = row["email"].c_str();
            user.passwordHash = row["password_hash"].c_str();

            const std::string roleText = row["role"].c_str();

            if (roleText == "BUYER")
            {
                user.role = UserRole::BUYER;
            }
            else if (roleText == "SELLER")
            {
                user.role = UserRole::SELLER;
            }
            else
            {
                user.role = UserRole::ADMIN;
            }

            users.push_back(user);
        }

        transaction.commit();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Find all users failed: "
                  << exception.what()
                  << std::endl;

        users.clear();
    }

    return users;
}

} // namespace anuja::anujamart
