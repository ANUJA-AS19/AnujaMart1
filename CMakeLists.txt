cmake_minimum_required(VERSION 3.25)

project(AnujaMart
     VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)

add_executable(anujamart
    src/main.cpp

    src/plugin/MigrationService.cpp

    src/repository/Database.cpp
    src/repository/UserRepository.cpp
    src/repository/ProductRepository.cpp
    src/repository/ProductBrowseRepository.cpp
    src/repository/CartRepository.cpp
    src/repository/OrderRepository.cpp
    src/repository/SessionRepository.cpp 

    src/service/AuthService.cpp
    src/service/UserService.cpp
    src/service/ProductService.cpp
    src/service/ProductBrowseService.cpp
    src/service/CartService.cpp
    src/service/OrderService.cpp
    src/service/SessionService.cpp 

    src/controller/AuthController.cpp
    src/controller/AuthFilter.cpp
    src/controller/AdminController.cpp
    src/controller/ReviewController.cpp
    src/service/ReviewService.cpp
    src/repository/ReviewRepository.cpp
    src/controller/ProductController.cpp
    src/controller/ProductBrowseController.cpp
    src/controller/CartController.cpp
    src/controller/OrderController.cpp
    src/controller/ChatbotController.cpp
)

target_link_libraries(anujamart
    PRIVATE
    Drogon::Drogon
    nlohmann_json::nlohmann_json
    libpqxx::pqxx
    unofficial-sodium::sodium
)

include(CTest)
enable_testing()
find_package(GTest CONFIG REQUIRED)

add_executable(anujamart_tests test/AuthServiceTest.cpp)
target_link_libraries(anujamart_tests PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(anujamart_tests)

target_link_libraries(anujamart_tests PRIVATE
    libpqxx::pqxx
    unofficial-sodium::sodium
)

target_sources(anujamart_tests PRIVATE
    src/service/AuthService.cpp
    src/repository/UserRepository.cpp
    src/repository/Database.cpp
)
