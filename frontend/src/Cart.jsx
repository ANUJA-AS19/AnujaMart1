import { useState } from "react";

function Cart() {
  const [message, setMessage] = useState("");

  const loadCart = async () => {
    const token = localStorage.getItem("token");

    if (!token) {
      setMessage("Please login first.");
      return;
    }

    const response = await fetch("http://localhost:8080/api/v1/cart", {
      headers: {
        Authorization: `Bearer ${token}`,
      },
    });

    const result = await response.json();

    if (response.ok && result.success) {
      setMessage(
        `Cart total: ₹${(result.data.total_cents / 100).toFixed(2)}`
      );
    } else {
      setMessage(result.error || "Unable to load cart");
    }
  };

  return (
    <section className="login">
      <h2>Shopping Cart</h2>
      <button onClick={loadCart}>View Cart</button>
      <p>{message}</p>
    </section>
  );
}

export default Cart;