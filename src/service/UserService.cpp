#include "UserService.h"

namespace anuja::anujamart
{

UserService::UserService(
    UserRepository& userRepository)
    : userRepository_(userRepository)
{
}

std::vector<User> UserService::getAllUsers(
    std::int64_t adminId)
{
    if (adminId <= 0)
    {
        return {};
    }

    const auto admin =
        userRepository_.findById(adminId);

    if (!admin.has_value() ||
        admin->role != UserRole::ADMIN)
    {
        return {};
    }

    return userRepository_.findAll();
}

} // namespace anuja::anujamart
