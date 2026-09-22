#pragma once

#include <cstdint>
#include <string>

namespace anuja::anujamart
{

enum class UserRole
{
    BUYER,
    SELLER,
    ADMIN
};

struct User
{
    std::int64_t id = 0;
    std::string name;
    std::string email;
    std::string passwordHash;
    UserRole role = UserRole::BUYER;
};

} // namespace anuja::anujamart