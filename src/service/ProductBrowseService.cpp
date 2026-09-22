#include "ProductBrowseService.h"

namespace anuja::anujamart
{

ProductBrowseService::ProductBrowseService(
    ProductBrowseRepository& repository)
    : repository_(repository)
{
}

std::vector<Product> ProductBrowseService::browseProducts(
    const std::string& search,
    const std::string& category)
{
    return repository_.searchProducts(search, category);
}

} // namespace anuja::anujamart
