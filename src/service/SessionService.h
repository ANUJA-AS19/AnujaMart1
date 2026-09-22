#pragma once

#include "../repository/SessionRepository.h"

#include <cstdint>
#include <optional>
#include <string>

namespace anuja::anujamart {

class SessionService {
public:
    explicit SessionService(SessionRepository& repository);

    std::optional<std::string> createSession(std::int64_t userId);
    std::optional<Session> validateSession(const std::string& token);
    bool updateActivity(const std::string& token);
    bool logout(const std::string& token);

private:
    SessionRepository& repository_;

    std::string generateToken();
};

} // namespace anuja::anujamart
