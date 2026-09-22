import { useEffect, useState } from "react";
import "./App.css";
import Login from "./Login";
import Register from "./Register";
import Cart from "./Cart";
import Checkout from "./Checkout";

function App() {
  const [products, setProducts] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetch("http://localhost:8080/api/v1/products")
      .then((response) => response.json())
      .then((result) => {
        setProducts(result.data?.products || []);
        setLoading(false);
      })
      .catch((error) => {
        console.error("Product loading error:", error);
        setLoading(false);
      });
  }, []);

  const addToCart = async (productId) => {
    const token = localStorage.getItem("token");

    if (!token) {
      alert("Please login first.");
      return;
    }

    try {
      const response = await fetch(
        "http://localhost:8080/api/v1/cart/items",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${token}`,
          },
          body: JSON.stringify({
            product_id: productId,
            quantity: 1,
          }),
        }
      );

      const result = await response.json();

      if (response.ok && result.success) {
        alert("Product added to cart!");
      } else {
        alert(result.error || "Unable to add product");
      }
    } catch (error) {
      console.error("Cart error:", error);
      alert("Cannot connect to backend");
    }
  };

  return (
    <div className="app">
      <header className="navbar">
        <h1>🛒 AnujaMart</h1>

        <nav>
          <a href="/">Home</a>
          <a href="#products">Products</a>
          <a href="#login">Login</a>
          <a href="#register">Register</a>
          <a href="#cart">🛍️ Cart</a>
          <a href="#checkout">Checkout</a>
        </nav>
      </header>

      <section className="hero">
        <h2>Welcome to AnujaMart</h2>
        <p>Shop smart. Shop easily.</p>

        <a href="#products">
          <button>Explore Products</button>
        </a>
      </section>

      <section id="products" className="products">
        <h2>Our Products</h2>

        {loading ? (
          <p>Loading products...</p>
        ) : products.length === 0 ? (
          <p>No products available.</p>
        ) : (
          <div className="product-grid">
            {products.map((product) => (
              <div className="product-card" key={product.id}>
                <h3>{product.name}</h3>
                <p>{product.description}</p>

                <strong>
                  ₹{((product.price_cents || 0) / 100).toFixed(2)}
                </strong>

                <button onClick={() => addToCart(product.id)}>
                  Add to Cart
                </button>
              </div>
            ))}
          </div>
        )}
      </section>

      <section id="login">
        <Login />
      </section>

      <section id="register">
        <Register />
      </section>

      <section id="cart">
        <Cart />
      </section>

      <section id="checkout">
        <Checkout />
      </section>

      <footer>
        <p>© 2026 AnujaMart. All rights reserved.</p>
      </footer>
    </div>
  );
}

export default App;