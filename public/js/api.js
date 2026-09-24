import { CONFIG, EP } from './config.js';

/* ------------------------------------------------------------------ */
/* Errors                                                              */
/* ------------------------------------------------------------------ */

export class ApiError extends Error {
  constructor(status, code, message, fields) {
    super(message);
    this.name = 'ApiError';
    this.status = status;
    this.code = code;
    this.fields = normalizeFields(fields);
  }
}

/** Accepts {email: "msg"} or [{field: "email", message: "msg"}] and returns {email: "msg"}. */
function normalizeFields(f) {
  const out = {};
  if (!f) return out;
  if (Array.isArray(f)) {
    for (const item of f) {
      const name = item?.field ?? item?.name;
      if (name) out[name] = item.message ?? item.error ?? 'Invalid value';
    }
  } else if (typeof f === 'object') {
    for (const [k, v] of Object.entries(f)) {
      out[k] = typeof v === 'string' ? v : (v?.message ?? 'Invalid value');
    }
  }
  return out;
}

/* ---- Bearer token (the backend returns a token from /auth/login) ---- */
const TOKEN_KEY = 'ynm.token';
let authToken = null;
try { authToken = sessionStorage.getItem(TOKEN_KEY); } catch { /* storage unavailable */ }

export const getAuthToken = () => authToken;
export function setAuthToken(token) {
  authToken = token || null;
  try {
    if (token) sessionStorage.setItem(TOKEN_KEY, token);
    else sessionStorage.removeItem(TOKEN_KEY);
  } catch { /* storage unavailable */ }
}

let onUnauthorized = null;
/** main.js registers a callback that runs when a 401 arrives for a logged-in user. */
export function setUnauthorizedHandler(fn) {
  onUnauthorized = fn;
}

/* ------------------------------------------------------------------ */
/* Core request                                                        */
/* ------------------------------------------------------------------ */

async function request(method, path, body, query) {
  let url = CONFIG.apiBase + path;
  if (query) {
    const qs = new URLSearchParams();
    for (const [k, v] of Object.entries(query)) {
      if (v !== undefined && v !== null && v !== '') qs.set(k, v);
    }
    const s = qs.toString();
    if (s) url += `?${s}`;
  }

  const opts = {
    method,
    credentials: 'same-origin', // sends the Drogon session cookie
    headers: { Accept: 'application/json' },
  };
  if (authToken) opts.headers.Authorization = `Bearer ${authToken}`;
  if (body !== undefined) {
    opts.headers['Content-Type'] = 'application/json';
    opts.body = JSON.stringify(body);
  }

  let res;
  try {
    res = await fetch(url, opts);
  } catch {
    throw new ApiError(0, 'NETWORK_ERROR', 'Cannot reach the server. Check that the backend is running.');
  }

  let payload = null;
  try {
    payload = await res.json();
  } catch {
    /* empty or non-JSON body */
  }

  if (!res.ok || (payload && payload.success === false)) {
    const raw = payload?.error;
    const err = typeof raw === 'string' ? { message: raw } : (raw ?? {});
    const apiErr = new ApiError(
      res.status,
      err.code ?? `HTTP_${res.status}`,
      err.message ?? payload?.message ?? `Request failed (${res.status}).`,
      err.fields ?? err.details ?? null,
    );
    if (res.status === 401 && onUnauthorized) onUnauthorized(apiErr);
    throw apiErr;
  }

  if (payload && typeof payload === 'object' && 'data' in payload) return payload.data;
  return payload; // e.g. /health may return a bare {status, db}
}

/* ------------------------------------------------------------------ */
/* Normalizers: tolerate snake_case or camelCase from the backend      */
/* ------------------------------------------------------------------ */

const pick = (o, ...keys) => {
  for (const k of keys) {
    if (o && o[k] !== undefined && o[k] !== null) return o[k];
  }
  return undefined;
};

export function toList(d) {
  if (Array.isArray(d)) return d;
  if (!d || typeof d !== 'object') return [];
  return d.items ?? d.content ?? d.products ?? d.orders ?? d.users ?? d.reviews ?? d.results ?? [];
}

export function normUser(u) {
  return {
    id: pick(u, 'id'),
    name: pick(u, 'name') ?? '',
    email: pick(u, 'email') ?? '',
    role: String(pick(u, 'role') ?? '').toUpperCase(),
    createdAt: pick(u, 'created_at', 'createdAt'),
  };
}

export function normProduct(p) {
  return {
    id: pick(p, 'id'),
    sellerId: pick(p, 'seller_id', 'sellerId'),
    sellerName: pick(p, 'seller_name', 'sellerName') ?? p?.seller?.name,
    name: pick(p, 'name') ?? '',
    description: pick(p, 'description') ?? '',
    priceCents: Number(pick(p, 'price_cents', 'priceCents') ?? 0),
    stockQty: Number(pick(p, 'stock_qty', 'stockQty', 'stock') ?? 0),
    category: pick(p, 'category') ?? '',
    imageUrl: pick(p, 'image_url', 'imageUrl'),
    createdAt: pick(p, 'created_at', 'createdAt'),
  };
}

export function normCartItem(i) {
  const p = i?.product ?? {};
  const unitPriceCents = Number(
    pick(i, 'unit_price_cents', 'price_cents', 'unitPriceCents', 'priceCents') ??
      pick(p, 'price_cents', 'priceCents') ?? 0,
  );
  const quantity = Number(pick(i, 'quantity', 'qty') ?? 1);
  return {
    id: pick(i, 'id'),
    productId: pick(i, 'product_id', 'productId') ?? p.id,
    name: pick(i, 'name', 'product_name', 'productName') ?? p.name ?? '',
    imageUrl: pick(i, 'image_url', 'imageUrl') ?? pick(p, 'image_url', 'imageUrl'),
    stockQty: Number(pick(i, 'stock_qty', 'stockQty') ?? pick(p, 'stock_qty', 'stockQty') ?? Infinity),
    unitPriceCents,
    quantity,
    lineTotalCents: Number(pick(i, 'line_total_cents', 'lineTotalCents') ?? unitPriceCents * quantity),
  };
}

export function normCart(d) {
  const raw = Array.isArray(d) ? d : (d?.items ?? d?.cart_items ?? []);
  const items = raw.map(normCartItem);
  const totalCents = Number(
    pick(d, 'total_cents', 'totalCents') ?? items.reduce((s, i) => s + i.lineTotalCents, 0),
  );
  return { items, totalCents };
}

export function normOrder(o) {
  const rawItems = pick(o, 'items', 'order_items', 'orderItems') ?? [];
  return {
    id: pick(o, 'id'),
    buyerId: pick(o, 'buyer_id', 'buyerId'),
    buyerName: pick(o, 'buyer_name', 'buyerName') ?? o?.buyer?.name,
    status: String(pick(o, 'status') ?? 'PENDING').toUpperCase(),
    totalCents: Number(pick(o, 'total_amount_cents', 'total_cents', 'totalAmountCents', 'totalCents') ?? 0),
    createdAt: pick(o, 'created_at', 'createdAt'),
    items: rawItems.map((it) => {
      const unitPriceCents = Number(pick(it, 'unit_price_cents', 'unitPriceCents') ?? 0);
      const quantity = Number(pick(it, 'quantity', 'qty') ?? 1);
      return {
        productId: pick(it, 'product_id', 'productId'),
        sellerId: pick(it, 'seller_id', 'sellerId'),
        name: pick(it, 'name', 'product_name', 'productName') ?? it?.product?.name ?? `Product #${pick(it, 'product_id', 'productId')}`,
        unitPriceCents,
        quantity,
        lineTotalCents: unitPriceCents * quantity,
      };
    }),
  };
}

export function normReview(r) {
  return {
    id: pick(r, 'id'),
    userName: pick(r, 'user_name', 'userName', 'reviewer') ?? r?.user?.name ?? 'Buyer',
    rating: Number(pick(r, 'rating') ?? 0),
    comment: pick(r, 'comment') ?? '',
    createdAt: pick(r, 'created_at', 'createdAt'),
  };
}

/** JS-side product input -> wire format (snake_case, integer cents). */
function toWireProduct(i, sellerId) {
  const wire = {
    seller_id: sellerId, // required by the backend
    name: i.name,
    description: i.description,
    price_cents: i.priceCents,
    stock_qty: i.stockQty,
    category: i.category,
  };
  if (i.imageUrl) wire.image_url = i.imageUrl; // omit when empty (the backend rejects null)
  return wire;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

export const api = {
  health: () => request('GET', EP.health),

  // --- chatbot
  chatbot: (message) => request('POST', EP.chatbot, { message }),

  // --- auth
  async me() {
    const d = await request('GET', EP.me);
    const u = d?.user ?? d;
    return u && (u.id !== undefined || u.email) ? normUser(u) : null;
  },
  async login(email, password) {
    const d = await request('POST', EP.login, { email, password });
    if (d?.token) setAuthToken(d.token);
    const u = d?.user ?? d;
    if (u && u.role) return normUser(u);
    try {
      const me = await api.me(); // login only set the cookie; fetch the user separately
      if (me) return me;
    } catch { /* fall through */ }
    throw new ApiError(500, 'NO_USER_DATA', 'Signed in, but the server did not send your account details (id, name, role).');
  },
  register: (name, email, password, role) =>
    request('POST', EP.register, { name, email, password, role }),
  async logout() {
    try {
      await request('POST', EP.logout);
    } finally {
      setAuthToken(null); // always forget the token locally
    }
  },

  // --- catalogue
  async products(filters = {}) {
    return toList(await request('GET', EP.products, undefined, filters)).map(normProduct);
  },
  async product(id) {
    return normProduct(await request('GET', EP.product(id)));
  },
  async reviews(productId) {
    return toList(await request('GET', EP.reviews(productId))).map(normReview);
  },
  addReview: (productId, rating, comment) =>
    request('POST', EP.reviews(productId), { rating, comment }),

  // --- seller
  async sellerProducts(sellerId) {
    return toList(await request('GET', EP.sellerProducts(sellerId))).map(normProduct);
  },
  async sellerOrders() {
    return toList(await request('GET', EP.sellerOrders)).map(normOrder);
  },
  createProduct: (input, sellerId) => request('POST', EP.products, toWireProduct(input, sellerId)),
  updateProduct: (id, input, sellerId) => request('PUT', EP.product(id), toWireProduct(input, sellerId)),
  deleteProduct: (id, sellerId) => request('DELETE', EP.product(id), undefined, { seller_id: sellerId }),
  setOrderStatus: (orderId, status) =>
    request('PUT', EP.sellerOrderStatus, { order_id: orderId, status }),

  // --- cart
  async cart() {
    return normCart(await request('GET', EP.cart));
  },
  addToCart: (productId, quantity = 1) =>
    request('POST', EP.cartItems, { product_id: productId, quantity }),
  updateCartItem: (itemId, quantity) => request('PUT', EP.cartItem(itemId), { quantity }),
  removeCartItem: (itemId) => request('DELETE', EP.cartItem(itemId)),

  // --- orders
  async orders() {
    return toList(await request('GET', EP.orders)).map(normOrder);
  },
  async placeOrder() {
    // Mock payment: no card data is ever collected or sent.
    const d = await request('POST', EP.checkout, { payment_method: 'MOCK' });
    return d ? normOrder(d.order ?? d) : null;
  },

  // --- admin
  async adminUsers() {
    return toList(await request('GET', EP.adminUsers)).map(normUser);
  },
  async adminOrders() {
    return toList(await request('GET', EP.adminOrders)).map(normOrder);
  },
  adminDeleteProduct: (id) => request('DELETE', EP.adminProduct(id)),
};
