#pragma once

#include "../model/User.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace anuja::anujamart
{

class UserRepository
{
public:
    bool createUser(
        const std::string& name,
        const std::string& email,
        const std::string& passwordHash,
        UserRole role);

    std::optional<User> findByEmail(
        const std::string& email);
 
 std::optional<User> findById(
    std::int64_t id);
    std::vector<User> findAll();
};

} // namespace anuja::anujamart
 
