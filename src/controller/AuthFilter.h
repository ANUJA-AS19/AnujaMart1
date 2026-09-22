#pragma once

#include "../repository/SessionRepository.h"

#include <drogon/HttpFilter.h>

namespace anuja::anujamart {

class AuthFilter : public drogon::HttpFilter<AuthFilter> {
public:
    AuthFilter() = default;

    void doFilter(
        const drogon::HttpRequestPtr& request,
        drogon::FilterCallback&& filterCallback,
        drogon::FilterChainCallback&& filterChainCallback) override;

private:
    SessionRepository sessionRepository_;
};

} // namespace anuja::anujamart
