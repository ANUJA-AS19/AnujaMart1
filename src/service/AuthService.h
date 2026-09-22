#pragma once

#include "../model/User.h"
#include "../repository/UserRepository.h"

#include <optional>
#include <string>

namespace anuja::anujamart
{

class AuthService
{
public:
    explicit AuthService(UserRepository& userRepository);

    bool registerUser(
        const std::string& name,
        const std::string& email,
        const std::string& password,
        UserRole role);

    std::optional<User> login(
        const std::string& email,
        const std::string& password);

private:
    UserRepository& userRepository_;
};

} // namespace anuja::anujamart