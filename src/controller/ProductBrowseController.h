#pragma once

#include "../service/ProductBrowseService.h"

#include <drogon/HttpController.h>

namespace anuja::anujamart
{

class ProductBrowseController
    : public drogon::HttpController<ProductBrowseController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ProductBrowseController::browseProducts,
        "/api/v1/products/search",
        drogon::Get);
ADD_METHOD_TO(
    ProductBrowseController::browseProducts,
    "/api/v1/products",
    drogon::Get);

    METHOD_LIST_END

    void browseProducts(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    ProductBrowseRepository productBrowseRepository_;
    ProductBrowseService productBrowseService_{
        productBrowseRepository_};
};

} // namespace anuja::anujamart
