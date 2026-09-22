import { useState } from "react";

function Orders() {
  const [orders, setOrders] = useState([]);
  const [message, setMessage] = useState("");

  const loadOrders = async () => {
    const token = localStorage.getItem("token");

    if (!token) {
      setMessage("Please login first.");
      return;
    }

    try {
      const response = await fetch(
        "http://localhost:8080/api/v1/orders",
        {
          headers: {
            Authorization: `Bearer ${token}`,
          },
        }
      );

      const result = await response.json();

      if (response.ok && result.success) {
        setOrders(result.data?.orders || result.data || []);
        setMessage("");
      } else {
        setMessage(result.error || "Unable to load orders");
      }
    } catch {
      setMessage("Cannot connect to backend");
    }
  };

  return (
    <section className="login">
      <h2>My Orders</h2>

      <button onClick={loadOrders}>View Orders</button>

      <p>{message}</p>

      {orders.map((order) => (
        <div className="product-card" key={order.id}>
          <h3>Order #{order.id}</h3>
          <p>Status: {order.status}</p>
          <p>
            Total: ₹{((order.total_cents || 0) / 100).toFixed(2)}
          </p>
        </div>
      ))}
    </section>
  );
}

export default Orders;