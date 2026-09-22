#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace anuja::anujamart
{

struct Session
{
    std::string token;
    std::int64_t userId;
};

class SessionRepository
{
public:
    bool createSession(
        const std::string& token,
        std::int64_t userId,
        int expiryMinutes = 30);

    std::optional<Session> findValidSession(
        const std::string& token);

    bool updateActivity(
        const std::string& token);

    bool deleteSession(
        const std::string& token);
};

} // namespace anuja::anujamart
