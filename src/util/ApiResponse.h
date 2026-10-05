#pragma once

#include <json/json.h>
#include <string>

namespace anuja::anujamart
{

inline Json::Value makeError(
    const std::string& code,
    const std::string& message)
{
    Json::Value error;
    error["code"] = code;
    error["message"] = message;
    return error;
}

}
