#pragma once

#include "../model/User.h"
#include "../repository/UserRepository.h"

#include <cstdint>
#include <vector>

namespace anuja::anujamart
{

class UserService
{
public:
    explicit UserService(UserRepository& userRepository);

    std::vector<User> getAllUsers(
        std::int64_t adminId);

private:
    UserRepository& userRepository_;
};

} // namespace anuja::anujamart
