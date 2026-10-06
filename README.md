# AnujaMart

AnujaMart is a native C++20 online marketplace application developed as a capstone project.

## Features

- Buyer and Seller registration and login
- Seller product listing management
- Product browsing, search and category filtering
- Shopping cart with quantity updates and removal
- Mock-payment checkout
- Buyer order history
- Seller order management and status updates
- Product reviews and star ratings
- Admin user, order and listing management
- AI-assisted chatbot for product and marketplace questions

## Technology Stack

- C++20
- Drogon
- PostgreSQL
- libpqxx
- nlohmann/json
- libsodium
- CMake
- vcpkg
- HTML, CSS and JavaScript
- Docker
- Render

## Project Structure

- `src/controller/` - HTTP controllers and API endpoints
- `src/service/` - Business logic
- `src/repository/` - Database operations
- `src/models/` - Data models
- `src/plugin/` - Database migration and initialization
- `public/` - Frontend files
- `test/` - Automated tests

## Environment Variables

Create a `.env` file using `.env.example` as a reference.

Required variables:

```text
ANUJAMART_DATABASE_URL=your_database_url
ANUJAMART_ADMIN_PASSWORD=your_admin_password
GEMINI_API_KEY=your_api_key

## Deployment

The application is containerized with Docker and deployed on Render.

Live application:

https://anujamart1.onrender.com/

## Security

- Database queries use parameterized SQL.
- Passwords are stored using secure password hashing.
- Authentication and authorization are handled by the backend.
- Secrets are provided through environment variables.
- `.env` files are excluded from Git.
- Local database and build files are excluded from Git.

## Testing

The project includes automated backend tests and manual testing of the major marketplace workflows, including authentication, product management, browsing, cart, checkout, orders, reviews, admin functions and chatbot functionality.

## Project Status

Core marketplace features and the main Phase 3 features have been implemented and tested on the deployed application.
