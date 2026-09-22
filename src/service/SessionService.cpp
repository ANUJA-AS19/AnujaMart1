#include "SessionService.h"

#include <sodium.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace anuja::anujamart {

SessionService::SessionService(SessionRepository& repository)
    : repository_(repository) {
}

std::string SessionService::generateToken() {
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize secure random generator");
    }

    unsigned char buffer[32];
    randombytes_buf(buffer, sizeof(buffer));

    std::ostringstream token;

    for (unsigned char byte : buffer) {
        token << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(byte);
    }

    return token.str();
}

std::optional<std::string> SessionService::createSession(std::int64_t userId) {
    try {
        const std::string token = generateToken();

        if (!repository_.createSession(token, userId)) {
            return std::nullopt;
        }

        return token;
    } catch (const std::exception& exception) {
        throw std::runtime_error(
            std::string("Session creation failed: ") + exception.what());
    }
}

std::optional<Session> SessionService::validateSession(
    const std::string& token) {
    return repository_.findValidSession(token);
}

bool SessionService::updateActivity(const std::string& token) {
    return repository_.updateActivity(token);
}

bool SessionService::logout(const std::string& token) {
    return repository_.deleteSession(token);
}

} // namespace anuja::anujamart
