#pragma once

#include <memory>
#include <string>

#include <pqxx/pqxx>

namespace anuja::anujamart
{
class Database
{
public:
    static Database& instance();

    bool open(const std::string& connectionString);

    pqxx::connection* connection();

    void close();

private:
    Database() = default;
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    std::unique_ptr<pqxx::connection> connection_;
};
}