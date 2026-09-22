#pragma once

#include "../model/Product.h"

#include <string>
#include <vector>

namespace anuja::anujamart
{
class ProductBrowseRepository
{
public:
    std::vector<Product> searchProducts(
        const std::string& searchTerm,
        const std::string& category);
};
}