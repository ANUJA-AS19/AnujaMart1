import { useState } from "react";

function Login() {
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [message, setMessage] = useState("");

  const handleLogin = async (event) => {
    event.preventDefault();
    setMessage("Logging in...");

    try {
      const response = await fetch(
        "http://localhost:8080/api/v1/auth/login",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify({ email, password }),
        }
      );

      const result = await response.json();

      if (response.ok && result.success) {
        localStorage.setItem("token", result.data.token);
        setMessage("Login successful!");
      } else {
        setMessage(result.error || "Login failed");
      }
    } catch (error) {
      setMessage("Cannot connect to backend");
    }
  };

  return (
    <section id="login" className="login">
      <h2>Login to AnujaMart</h2>

      <form onSubmit={handleLogin}>
        <input
          type="email"
          placeholder="Email"
          value={email}
          onChange={(event) => setEmail(event.target.value)}
          required
        />

        <input
          type="password"
          placeholder="Password"
          value={password}
          onChange={(event) => setPassword(event.target.value)}
          required
        />

        <button type="submit">Login</button>
      </form>

      <p>{message}</p>
    </section>
  );
}

export default Login;