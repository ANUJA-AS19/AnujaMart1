#include "Database.h"

#include <iostream>

namespace anuja::anujamart
{
Database& Database::instance()
{
    static Database database;
    return database;
}

Database::~Database()
{
    close();
}

bool Database::open(const std::string& connectionString)
{
    try
    {
        connection_ =
            std::make_unique<pqxx::connection>(connectionString);

        if (!connection_->is_open())
        {
            connection_.reset();
            return false;
        }

        std::cout << "PostgreSQL connection established."
                  << std::endl;

        return true;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "PostgreSQL connection failed: "
            << exception.what()
            << std::endl;

        connection_.reset();
        return false;
    }
}

pqxx::connection* Database::connection()
{
    return connection_.get();
}

void Database::close()
{
    if (connection_)
    {
        connection_->close();
        connection_.reset();
    }
}
}