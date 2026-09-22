#include "../src/service/AuthService.h"
#include <gtest/gtest.h>

using namespace anuja::anujamart;

TEST(AuthServiceTest, RejectsEmptyRegistrationFields) {
    UserRepository repository;
    AuthService auth(repository);
    EXPECT_FALSE(auth.registerUser("", "test@example.com", "Password123", UserRole::BUYER));
    EXPECT_FALSE(auth.registerUser("Test", "", "Password123", UserRole::BUYER));
    EXPECT_FALSE(auth.registerUser("Test", "test@example.com", "", UserRole::BUYER));
}

TEST(AuthServiceTest, RejectsEmptyLoginFields) {
    UserRepository repository;
    AuthService auth(repository);
    EXPECT_FALSE(auth.login("", "Password123").has_value());
    EXPECT_FALSE(auth.login("test@example.com", "").has_value());
}
