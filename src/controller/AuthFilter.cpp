#include "AuthFilter.h"

#include <drogon/drogon.h>

namespace anuja::anujamart {

void AuthFilter::doFilter(
    const drogon::HttpRequestPtr& request,
    drogon::FilterCallback&& filterCallback,
    drogon::FilterChainCallback&& filterChainCallback) {

    const std::string authorization =
        request->getHeader("Authorization");

    const std::string prefix = "Bearer ";

    if (authorization.rfind(prefix, 0) != 0) {
        Json::Value error;
        error["success"] = false;
        error["data"] = Json::nullValue;
        error["error"] = "Authentication required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(error);

        response->setStatusCode(drogon::k401Unauthorized);
        filterCallback(response);
        return;
    }

    const std::string token =
        authorization.substr(prefix.size());

    auto session =
        sessionRepository_.findValidSession(token);

    if (!session.has_value()) {
        Json::Value error;
        error["success"] = false;
        error["data"] = Json::nullValue;
        error["error"] = "Invalid or expired session";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(error);

        response->setStatusCode(drogon::k401Unauthorized);
        filterCallback(response);
        return;
    }

    sessionRepository_.updateActivity(token);

    request->attributes()->insert(
        "user_id",
        session->userId);

    filterChainCallback();
}

} // namespace anuja::anujamart
