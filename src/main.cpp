#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <drogon/drogon.h>
#include <pqxx/pqxx>

#include "plugin/MigrationService.h"
#include "repository/Database.h"
#include "controller/AuthFilter.h"

int main()
{
    const char* databaseUrl =
        std::getenv("ANUJAMART_DATABASE_URL");

    if (databaseUrl == nullptr ||
        std::string(databaseUrl).empty())
    {
        std::cerr
            << "ANUJAMART_DATABASE_URL environment variable "
               "is not set."
            << std::endl;

        return 1;
    }

    if (!anuja::anujamart::Database::instance().open(
            databaseUrl))
    {
        return 1;
    }

    anuja::anujamart::MigrationService migrationService;
    migrationService.runMigrations();

    // Find the frontend folder whether we run from the project
    // folder or from the build folder.
    std::string frontendRoot = "./public";
    if (!std::filesystem::exists("./public/index.html"))
    {
        frontendRoot = "../public";
    }
    drogon::app().setDocumentRoot(frontendRoot);
    drogon::app().enableSession(1800);

    drogon::app().registerHandler(
        "/api/v1/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(
               const drogon::HttpResponsePtr&)>&& callback)
        {
            Json::Value data;

            data["status"] = "UP";

            bool databaseUp = false;

            auto* db =
                anuja::anujamart::Database::instance()
                    .connection();

            if (db != nullptr)
            {
                try
                {
                    pqxx::transaction transaction(*db);
                    transaction.exec("SELECT 1");
                    transaction.commit();

                    databaseUp = true;
                }
                catch (const std::exception&)
                {
                    databaseUp = false;
                }
            }

            data["db"] =
                databaseUp ? "UP" : "DOWN";

            Json::Value responseJson;

            responseJson["success"] = databaseUp;
            responseJson["data"] = data;
            responseJson["error"] =
                databaseUp ? Json::nullValue
                           : Json::Value("Database unavailable");

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(
                    responseJson);

            response->setStatusCode(
                databaseUp
                    ? drogon::k200OK
                    : drogon::k503ServiceUnavailable);

            callback(response);
        });

    // Render (and most cloud hosts) assign the port to listen on
    // via the PORT environment variable, and require listening on
    // 0.0.0.0 rather than 127.0.0.1 so it's reachable externally.
    int port = 8080;
    if (const char* portEnv = std::getenv("PORT"))
    {
        port = std::atoi(portEnv);
    }

    drogon::app()
        .addListener("0.0.0.0", port)
        .run();

    anuja::anujamart::Database::instance().close();

    return 0;
}
