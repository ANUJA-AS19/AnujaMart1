/**
 * EDIT THIS FILE FIRST.
 * Everything the frontend assumes about your backend lives here, so if one of
 * your Drogon routes differs from the guess below, change it in ONE place.
 *
 * Assumed response envelope (Section 11 of the spec):
 *   { "success": true,  "data": {...}, "error": null }
 *   { "success": false, "data": null,  "error": { "code": "...", "message": "...", "fields"?: {...} } }
 */
export const CONFIG = {
  brand: 'AnujaMart',        // rename to your own, e.g. 'AdityaMart'
  currency: 'USD',              // ISO code used to display price_cents
  locale: 'en-US',
  apiBase: '/api/v1',
  categories: ['Electronics', 'Fashion', 'Home', 'Books', 'Sports', 'Beauty', 'Toys', 'Other'],
};

/** Endpoint paths, relative to CONFIG.apiBase. */
export const EP = {
  // Auth / session
  register: '/auth/register',           // POST {name, email, password, role}
  login: '/auth/login',                 // POST {email, password}  -> user (or just sets the session)
  logout: '/auth/logout',               // POST  (NOT in the backend yet: the frontend just signs out locally)
  me: '/auth/me',                       // GET   (NOT in the backend yet: the frontend uses a saved copy instead)

  // Catalogue
  products: '/products',                // GET ?q=&category=   | POST (seller) create
  product: (id) => `/products/${id}`,   // GET | PUT (seller) | DELETE (seller)
  reviews: (id) => `/products/${id}/reviews`, // GET | POST {rating, comment}

  // Seller
  sellerProducts: (sellerId) => `/products/seller/${sellerId}`, // GET listings owned by this seller
  sellerOrders: '/seller/orders',       // GET incoming orders containing the seller's products
  sellerOrderStatus: '/seller/orders/status', // PUT {order_id, status}

  // Cart
  cart: '/cart',                        // GET -> {items:[...], total_cents}
  cartItems: '/cart/items',             // POST {product_id, quantity}
  cartItem: (id) => `/cart/items/${id}`,// PUT {quantity} | DELETE

  // Orders
  orders: '/orders',                    // GET buyer history
  checkout: '/orders/checkout',         // POST place order (mock payment)

  // Admin
  adminUsers: '/admin/users',           // GET
  adminOrders: '/admin/orders',         // GET
  adminProduct: (id) => `/admin/products/${id}`, // DELETE (moderate/remove)

  // Ops
  health: '/health',                    // GET {status, db}
  chatbot: '/api/v1/chatbot',                  // POST (used later for the chatbot widget)
};
