import { useState } from "react";

function Register() {
  const [form, setForm] = useState({
    name: "",
    email: "",
    password: "",
  });

  const [message, setMessage] = useState("");

  const handleRegister = async (event) => {
    event.preventDefault();
    setMessage("Registering...");

    try {
      const response = await fetch(
        "http://localhost:8080/api/v1/auth/register",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify(form),
        }
      );

      const result = await response.json();
      setMessage(
        response.ok && result.success
          ? "Registration successful!"
          : result.error || "Registration failed"
      );
    } catch {
      setMessage("Cannot connect to backend");
    }
  };

  return (
    <section className="login">
      <h2>Create Account</h2>

      <form onSubmit={handleRegister}>
        <input
          placeholder="Name"
          value={form.name}
          onChange={(e) => setForm({ ...form, name: e.target.value })}
          required
        />

        <input
          type="email"
          placeholder="Email"
          value={form.email}
          onChange={(e) => setForm({ ...form, email: e.target.value })}
          required
        />

        <input
          type="password"
          placeholder="Password"
          value={form.password}
          onChange={(e) => setForm({ ...form, password: e.target.value })}
          required
        />

        <button type="submit">Register</button>
      </form>

      <p>{message}</p>
    </section>
  );
}

export default Register;