import { useState } from "react";

function Checkout() {
  const [message, setMessage] = useState("");

  const checkout = async () => {
    const token = localStorage.getItem("token");

    if (!token) {
      setMessage("Please login first.");
      return;
    }

    try {
      const response = await fetch(
        "http://localhost:8080/api/v1/orders/checkout",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${token}`,
          },
          body: JSON.stringify({}),
        }
      );

      const result = await response.json();

      setMessage(
        response.ok && result.success
          ? "Order placed successfully!"
          : result.error || "Checkout failed"
      );
    } catch {
      setMessage("Cannot connect to backend");
    }
  };

  return (
    <section className="login">
      <h2>Checkout</h2>
      <button onClick={checkout}>Place Order</button>
      <p>{message}</p>
    </section>
  );
}

export default Checkout;