#pragma once

#include "../model/Product.h"
#include "../repository/ProductBrowseRepository.h"

#include <string>
#include <vector>

namespace anuja::anujamart
{

class ProductBrowseService
{
public:
    explicit ProductBrowseService(
        ProductBrowseRepository& repository);

    std::vector<Product> browseProducts(
        const std::string& search,
        const std::string& category);

private:
    ProductBrowseRepository& repository_;
};

} // namespace anuja::anujamart
