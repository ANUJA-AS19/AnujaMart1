#pragma once

#include "../service/AuthService.h"
#include "../service/SessionService.h"

#include <drogon/HttpController.h>

namespace anuja::anujamart
{

class AuthController : public drogon::HttpController<AuthController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        AuthController::registerUser,
        "/api/v1/auth/register",
        drogon::Post);

    ADD_METHOD_TO(
        AuthController::login,
        "/api/v1/auth/login",
        drogon::Post);

    METHOD_LIST_END

    void registerUser(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void login(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    UserRepository userRepository_;
    AuthService authService_{userRepository_};

    SessionRepository sessionRepository_;
    SessionService sessionService_{sessionRepository_};
};

} // namespace anuja::anujamart
