#pragma once
#include<cstdint>
#include<string>
namespace anuja::anujamart
{
struct Product{
std::int64_t id=0;
std::int64_t sellerId=0;
std::string name;
std::string description;
std::int64_t priceCents=0;
std::int64_t stockQty=0;
std::string category;
std::string imageUrl;
};
}//namespace anuja:anujamart
