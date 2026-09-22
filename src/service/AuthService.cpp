#include "AuthService.h"

#include <sodium.h>

namespace anuja::anujamart
{

AuthService::AuthService(UserRepository& userRepository)
    : userRepository_(userRepository)
{
}

bool AuthService::registerUser(
    const std::string& name,
    const std::string& email,
    const std::string& password,
    UserRole role)
{
    if (name.empty() || email.empty() || password.empty())
    {
        return false;
    }

    if (sodium_init() < 0)
    {
        return false;
    }

    char passwordHash[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(
            passwordHash,
            password.c_str(),
            password.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        return false;
    }

    return userRepository_.createUser(
        name,
        email,
        passwordHash,
        role);
}

std::optional<User> AuthService::login(
    const std::string& email,
    const std::string& password)
{
    if (email.empty() || password.empty())
    {
        return std::nullopt;
    }

    if (sodium_init() < 0)
    {
        return std::nullopt;
    }

    auto user = userRepository_.findByEmail(email);

    if (!user.has_value())
    {
        return std::nullopt;
    }

    if (crypto_pwhash_str_verify(
            user->passwordHash.c_str(),
            password.c_str(),
            password.size()) != 0)
    {
        return std::nullopt;
    }

    return user;
}

} // namespace anuja::anujamart