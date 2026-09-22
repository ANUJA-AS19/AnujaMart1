import { CONFIG } from './config.js';
import { api, ApiError } from './api.js';
import {
  el, toast, formatMoney, parseMoneyToCents, centsToInput, formatDate, hashLink,
  productImage, priceTag, stars, setBusy, confirmDialog, statusPill, emptyState, table, safeUrl,
} from './ui.js';

const isBuyerOrGuest = (user) => !user || user.role === 'BUYER';

/* ================================================================== */
/* Shared helpers                                                      */
/* ================================================================== */

function pageHead(title, ...actions) {
  return el(
    'div',
    { class: 'page-head' },
    el('h1', {}, title),
    actions.length ? el('div', { class: 'page-actions' }, actions) : null,
  );
}

function field({ label, name, type = 'text', value = '', hint, ...attrs }) {
  const id = `f-${name}`;
  const input =
    type === 'textarea'
      ? el('textarea', { id, name, rows: 4, ...attrs, value })
      : el('input', { id, name, type, ...attrs, value });
  return el(
    'div',
    { class: 'field' },
    el('label', { for: id }, label),
    input,
    hint ? el('p', { class: 'hint' }, hint) : null,
    el('p', { class: 'field-error', 'data-error-for': name, role: 'alert' }),
  );
}

function selectField({ label, name, options, value = '' }) {
  const id = `f-${name}`;
  return el(
    'div',
    { class: 'field' },
    el('label', { for: id }, label),
    el('select', { id, name, value }, options.map((o) => el('option', { value: o.value }, o.label))),
    el('p', { class: 'field-error', 'data-error-for': name, role: 'alert' }),
  );
}

// Server field names -> our form field names
const FIELD_ALIASES = { price_cents: 'price', priceCents: 'price', stockQty: 'stock_qty', imageUrl: 'image_url' };

function clearFormErrors(form) {
  form.querySelectorAll('.field-error').forEach((n) => { n.textContent = ''; });
  const g = form.querySelector('.form-error');
  if (g) g.textContent = '';
}

function showFormErrors(form, err) {
  const general = form.querySelector('.form-error');
  const fields = err instanceof ApiError ? err.fields : {};
  let unplaced = false;
  let placed = false;
  for (const [name, msg] of Object.entries(fields)) {
    const key = FIELD_ALIASES[name] ?? name;
    const slot = form.querySelector(`[data-error-for="${CSS.escape(key)}"]`);
    if (slot) { slot.textContent = msg; placed = true; } else { unplaced = true; }
  }
  if (general) {
    general.textContent = err instanceof ApiError
      ? (placed && !unplaced ? 'Check the highlighted fields.' : err.message)
      : 'Something went wrong. Try again.';
  }
}

function bindForm(form, submitBtn, handler) {
  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    clearFormErrors(form);
    setBusy(submitBtn, true);
    try {
      await handler(new FormData(form));
    } catch (err) {
      showFormErrors(form, err);
    } finally {
      setBusy(submitBtn, false);
    }
  });
}

function tabsNav(items, active, onSelect) {
  return el(
    'div',
    { class: 'tabs', role: 'tablist' },
    items.map(([key, label]) =>
      el('button', {
        type: 'button', role: 'tab', class: 'tab', 'aria-selected': String(key === active),
        onclick: () => onSelect(key),
      }, label)),
  );
}

export const loading = () => el('p', { class: 'muted loading' }, 'Loading…');

export function notFound() {
  return emptyState('Page not found', 'That page does not exist.', '#/', 'Back to listings');
}

export function forbidden() {
  return emptyState('Not available for your account', 'Sign in with a different account type to open this page.', '#/', 'Back to listings');
}

export function errorPanel(err, retry) {
  return el(
    'div',
    { class: 'empty' },
    el('h2', {}, err?.status === 0 ? 'Server unreachable' : 'Could not load this page'),
    el('p', {}, err?.message ?? 'Unexpected error.'),
    el('button', { class: 'btn btn-primary', type: 'button', onclick: retry }, 'Try again'),
  );
}

async function addToCart(ctx, product, qty, btn) {
  const user = ctx.state.user;
  if (!user) {
    ctx.navigate('/login', { next: location.hash.slice(1) });
    return;
  }
  if (user.role !== 'BUYER') {
    toast('Only buyer accounts can use the cart.', 'error');
    return;
  }
  setBusy(btn, true);
  try {
    await api.addToCart(product.id, qty);
    toast(`Added ${product.name} to your cart`);
    await ctx.refreshCartCount();
  } catch (e) {
    toast(e.message, 'error');
  } finally {
    setBusy(btn, false);
  }
}

/* ================================================================== */
/* Browse / search                                                     */
/* ================================================================== */

function productCard(ctx, p) {
  const href = `#/product/${encodeURIComponent(p.id)}`;
  const soldOut = p.stockQty <= 0;
  let action = null;
  if (isBuyerOrGuest(ctx.state.user)) {
    const btn = el('button', { class: 'btn btn-primary btn-sm', type: 'button', disabled: soldOut }, soldOut ? 'Sold out' : 'Add to cart');
    btn.addEventListener('click', () => addToCart(ctx, p, 1, btn));
    action = btn;
  }
  return el(
    'article',
    { class: 'card' },
    el('a', { class: 'card-media', href, 'aria-label': p.name }, productImage(p)),
    el(
      'div',
      { class: 'card-body' },
      el('p', { class: 'card-cat' }, p.category),
      el('h3', { class: 'card-title' }, el('a', { href }, p.name)),
      p.sellerName ? el('p', { class: 'card-seller' }, `Sold by ${p.sellerName}`) : null,
      el('div', { class: 'card-foot' }, priceTag(p.priceCents), action),
    ),
  );
}

export async function browse(ctx) {
  const q = ctx.query.q ?? '';
  const category = ctx.query.category ?? '';
  const products = await api.products({ q, category });

  const chips = el(
    'nav',
    { class: 'chips', 'aria-label': 'Categories' },
    ['', ...CONFIG.categories].map((c) =>
      el('a', {
        class: `chip${c === category ? ' is-active' : ''}`,
        href: hashLink('/', { q, category: c }),
        'aria-current': c === category ? 'true' : null,
      }, c || 'All')),
  );

  const heading = q ? `Results for “${q}”` : category || 'All listings';
  const body = products.length
    ? el('div', { class: 'grid' }, products.map((p) => productCard(ctx, p)))
    : emptyState('Nothing matches yet', 'Try a different keyword or pick another category.', hashLink('/'), 'Clear filters');

  return el(
    'div',
    {},
    pageHead(heading),
    el('p', { class: 'muted result-count' }, `${products.length} ${products.length === 1 ? 'listing' : 'listings'}`),
    chips,
    body,
  );
}

/* ================================================================== */
/* Product detail + reviews                                            */
/* ================================================================== */

export async function productDetail(ctx) {
  const id = ctx.params.id;
  const [product, reviews] = await Promise.all([api.product(id), api.reviews(id).catch(() => [])]);
  const user = ctx.state.user;
  const inStock = product.stockQty > 0;

  const avg = reviews.length ? reviews.reduce((s, r) => s + r.rating, 0) / reviews.length : null;

  // Purchase controls
  let purchase = null;
  if (isBuyerOrGuest(user)) {
    const qty = el('input', { id: 'qty', type: 'number', min: 1, max: Math.max(product.stockQty, 1), value: 1 });
    const btn = el('button', { class: 'btn btn-primary', type: 'button', disabled: !inStock }, inStock ? 'Add to cart' : 'Sold out');
    btn.addEventListener('click', () => {
      const n = Math.max(1, Math.min(Number(qty.value) || 1, product.stockQty));
      addToCart(ctx, product, n, btn);
    });
    purchase = el('div', { class: 'buy-row' }, inStock ? el('label', { for: 'qty' }, 'Quantity') : null, inStock ? qty : null, btn);
  }

  // Moderation / ownership
  const owner = [];
  if (user?.role === 'ADMIN') {
    const rm = el('button', { class: 'btn btn-danger', type: 'button' }, 'Remove listing');
    rm.addEventListener('click', async () => {
      if (!(await confirmDialog(`Remove “${product.name}” from the marketplace?`, 'Remove listing'))) return;
      try {
        await api.adminDeleteProduct(product.id);
        toast('Listing removed');
        ctx.navigate('/');
      } catch (e) {
        toast(e.message, 'error');
      }
    });
    owner.push(rm);
  }
  if (user?.role === 'SELLER' && String(user.id) === String(product.sellerId)) {
    owner.push(el('a', { class: 'btn btn-ghost', href: '#/seller' }, 'Manage in dashboard'));
  }

  // Reviews
  const reviewsSection = el('section', { class: 'reviews', id: 'reviews', 'aria-labelledby': 'reviews-h' });
  reviewsSection.append(
    el('h2', { id: 'reviews-h' }, 'Reviews'),
    avg !== null
      ? el('p', { class: 'avg' }, stars(avg), ` ${avg.toFixed(1)} from ${reviews.length} ${reviews.length === 1 ? 'review' : 'reviews'}`)
      : el('p', { class: 'muted' }, 'No reviews yet.'),
  );

  if (user?.role === 'BUYER') {
    const form = el('form', { class: 'form review-form', novalidate: true });
    const submit = el('button', { class: 'btn btn-primary', type: 'submit' }, 'Post review');
    form.append(
      el('h3', {}, 'Review this product'),
      el('p', { class: 'hint' }, 'You can review a product once your order for it is delivered.'),
      selectField({
        label: 'Rating', name: 'rating', value: '5',
        options: [5, 4, 3, 2, 1].map((n) => ({ value: String(n), label: `${n} ${n === 1 ? 'star' : 'stars'}` })),
      }),
      field({ label: 'Comment', name: 'comment', type: 'textarea', maxlength: 1000 }),
      el('p', { class: 'form-error', role: 'alert' }),
      submit,
    );
    bindForm(form, submit, async (fd) => {
      await api.addReview(product.id, Number(fd.get('rating')), String(fd.get('comment') ?? '').trim());
      toast('Review posted');
      ctx.rerender();
    });
    reviewsSection.append(form);
  }

  reviewsSection.append(
    el(
      'ul',
      { class: 'review-list' },
      reviews.map((r) =>
        el('li', { class: 'review' },
          el('div', { class: 'review-head' }, stars(r.rating), el('strong', {}, r.userName), el('span', { class: 'muted' }, formatDate(r.createdAt))),
          r.comment ? el('p', {}, r.comment) : null)),
    ),
  );

  if (ctx.query.review) setTimeout(() => reviewsSection.scrollIntoView({ behavior: 'smooth' }), 0);

  return el(
    'div',
    {},
    el('a', { class: 'back', href: '#/' }, 'Back to listings'),
    el(
      'div',
      { class: 'detail' },
      el('div', { class: 'detail-media' }, productImage(product, product.name)),
      el(
        'div',
        { class: 'detail-info' },
        el('p', { class: 'card-cat' }, product.category),
        el('h1', {}, product.name),
        product.sellerName ? el('p', { class: 'card-seller' }, `Sold by ${product.sellerName}`) : null,
        el('div', { class: 'detail-price' }, priceTag(product.priceCents)),
        el('p', { class: inStock ? 'stock ok' : 'stock bad' }, inStock ? `${product.stockQty} in stock` : 'Out of stock'),
        el('p', { class: 'description' }, product.description),
        purchase,
        owner.length ? el('div', { class: 'row' }, owner) : null,
      ),
    ),
    reviewsSection,
  );
}

/* ================================================================== */
/* Auth                                                                */
/* ================================================================== */

function landingFor(user, next) {
  if (user.role === 'SELLER') return '/seller';
  if (user.role === 'ADMIN') return '/admin';
  return next && next.startsWith('/') ? next : '/';
}

export async function login(ctx) {
  if (ctx.state.user) {
    ctx.navigate(landingFor(ctx.state.user));
    return el('div');
  }
  const form = el('form', { class: 'form auth-form', novalidate: true });
  const submit = el('button', { class: 'btn btn-primary btn-block', type: 'submit' }, 'Sign in');
  form.append(
    field({ label: 'Email', name: 'email', type: 'email', autocomplete: 'email', required: true }),
    field({ label: 'Password', name: 'password', type: 'password', autocomplete: 'current-password', required: true }),
    el('p', { class: 'form-error', role: 'alert' }),
    submit,
  );
  bindForm(form, submit, async (fd) => {
    const errors = {};
    if (!fd.get('email')) errors.email = 'Enter your email.';
    if (!fd.get('password')) errors.password = 'Enter your password.';
    if (Object.keys(errors).length) throw new ApiError(400, 'VALIDATION_ERROR', 'Check the highlighted fields.', errors);
    const user = await api.login(String(fd.get('email')).trim(), String(fd.get('password')));
    if (!user) throw new ApiError(401, 'UNAUTHENTICATED', 'Sign-in failed. Try again.');
    await ctx.setUser(user);
    toast(`Welcome back, ${user.name}`);
    ctx.navigate(landingFor(user, ctx.query.next));
  });
  return el(
    'div',
    { class: 'auth' },
    el('h1', {}, 'Sign in'),
    form,
    el('p', { class: 'muted' }, 'New here? ', el('a', { href: '#/register' }, 'Create an account')),
  );
}

export async function register(ctx) {
  if (ctx.state.user) {
    ctx.navigate(landingFor(ctx.state.user));
    return el('div');
  }
  const form = el('form', { class: 'form auth-form', novalidate: true });
  const submit = el('button', { class: 'btn btn-primary btn-block', type: 'submit' }, 'Create account');
  form.append(
    field({ label: 'Full name', name: 'name', autocomplete: 'name', required: true, maxlength: 100 }),
    field({ label: 'Email', name: 'email', type: 'email', autocomplete: 'email', required: true }),
    field({ label: 'Password', name: 'password', type: 'password', autocomplete: 'new-password', required: true, hint: 'At least 8 characters.' }),
    selectField({
      label: 'I want to', name: 'role', value: 'BUYER',
      options: [{ value: 'BUYER', label: 'Buy products' }, { value: 'SELLER', label: 'Sell products' }],
    }),
    el('p', { class: 'form-error', role: 'alert' }),
    submit,
  );
  bindForm(form, submit, async (fd) => {
    const name = String(fd.get('name')).trim();
    const email = String(fd.get('email')).trim();
    const password = String(fd.get('password'));
    const errors = {};
    if (!name) errors.name = 'Enter your name.';
    if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email)) errors.email = 'Enter a valid email address.';
    if (password.length < 8) errors.password = 'Use at least 8 characters.';
    if (Object.keys(errors).length) throw new ApiError(400, 'VALIDATION_ERROR', 'Check the highlighted fields.', errors);

    await api.register(name, email, password, String(fd.get('role')));
    toast('Account created');
    try {
      const user = await api.login(email, password);
      await ctx.setUser(user);
      ctx.navigate(landingFor(user));
    } catch {
      ctx.navigate('/login');
    }
  });
  return el(
    'div',
    { class: 'auth' },
    el('h1', {}, 'Create your account'),
    form,
    el('p', { class: 'muted' }, 'Already registered? ', el('a', { href: '#/login' }, 'Sign in')),
  );
}

/* ================================================================== */
/* Cart & checkout                                                     */
/* ================================================================== */

export async function cart(ctx) {
  const c = await api.cart();
  if (!c.items.length) {
    return emptyState('Your cart is empty', 'Add something from the listings and it will show up here.', '#/', 'Browse listings');
  }

  async function run(fn) {
    try {
      await fn();
      await ctx.refreshCartCount();
      ctx.rerender();
    } catch (e) {
      toast(e.message, 'error');
    }
  }

  const rows = c.items.map((item) => {
    const minus = el('button', { class: 'btn btn-ghost btn-icon', type: 'button', 'aria-label': `Decrease quantity of ${item.name}` }, '−');
    const plus = el('button', { class: 'btn btn-ghost btn-icon', type: 'button', 'aria-label': `Increase quantity of ${item.name}` }, '+');
    const remove = el('button', { class: 'btn btn-link', type: 'button' }, 'Remove');
    minus.addEventListener('click', () => run(() => (item.quantity <= 1 ? api.removeCartItem(item.id) : api.updateCartItem(item.id, item.quantity - 1))));
    plus.addEventListener('click', () => run(() => api.updateCartItem(item.id, item.quantity + 1)));
    remove.addEventListener('click', () => run(() => api.removeCartItem(item.id)));
    const atMax = item.quantity >= item.stockQty;
    if (atMax) plus.disabled = true;
    return el(
      'li',
      { class: 'cart-row' },
      el('a', { class: 'cart-thumb', href: `#/product/${encodeURIComponent(item.productId)}`, 'aria-label': item.name }, productImage({ name: item.name, imageUrl: item.imageUrl })),
      el('div', { class: 'cart-info' },
        el('a', { class: 'cart-name', href: `#/product/${encodeURIComponent(item.productId)}` }, item.name),
        el('p', { class: 'muted' }, `${formatMoney(item.unitPriceCents)} each`),
        remove),
      el('div', { class: 'qty', role: 'group', 'aria-label': `Quantity for ${item.name}` }, minus, el('span', { class: 'qty-n' }, String(item.quantity)), plus),
      el('div', { class: 'cart-line' }, formatMoney(item.lineTotalCents)),
    );
  });

  return el(
    'div',
    {},
    pageHead('Your cart'),
    el(
      'div',
      { class: 'cart' },
      el('ul', { class: 'cart-list' }, rows),
      el('aside', { class: 'summary', 'aria-label': 'Order summary' },
        el('h2', {}, 'Order total'),
        el('p', { class: 'summary-total' }, formatMoney(c.totalCents)),
        el('p', { class: 'muted' }, 'Final amount is confirmed by the server when you place the order.'),
        el('a', { class: 'btn btn-primary btn-block', href: '#/checkout' }, 'Go to checkout')),
    ),
  );
}

export async function checkout(ctx) {
  const c = await api.cart();
  if (!c.items.length) {
    return emptyState('Nothing to check out', 'Your cart is empty.', '#/', 'Browse listings');
  }
  const pay = el('button', { class: 'btn btn-accent btn-block', type: 'button' }, `Confirm mock payment of ${formatMoney(c.totalCents)}`);
  const msg = el('p', { class: 'form-error', role: 'alert' });
  const panel = el('div', { class: 'summary summary-wide' });

  pay.addEventListener('click', async () => {
    setBusy(pay, true);
    msg.textContent = '';
    try {
      const order = await api.placeOrder();
      await ctx.refreshCartCount();
      panel.replaceChildren(
        el('h2', {}, 'Order placed'),
        el('p', {}, order?.id ? `Your order #${order.id} is confirmed.` : 'Your order is confirmed.'),
        el('a', { class: 'btn btn-primary', href: '#/orders' }, 'View my orders'),
      );
    } catch (e) {
      msg.textContent = e.message;
      setBusy(pay, false);
    }
  });

  panel.append(
    el('h2', {}, 'Payment'),
    el('p', {}, 'This is a demo store. No card details are collected and nothing is charged.'),
    msg,
    pay,
  );

  return el(
    'div',
    {},
    pageHead('Checkout'),
    el('ul', { class: 'order-lines' },
      c.items.map((i) => el('li', {}, el('span', {}, `${i.name} × ${i.quantity}`), el('span', {}, formatMoney(i.lineTotalCents))))),
    el('p', { class: 'summary-total' }, `Total ${formatMoney(c.totalCents)}`),
    panel,
  );
}

/* ================================================================== */
/* Buyer orders                                                        */
/* ================================================================== */

export async function orders() {
  const list = await api.orders();
  if (!list.length) {
    return emptyState('No orders yet', 'When you place an order it will appear here.', '#/', 'Browse listings');
  }
  const sorted = [...list].sort((a, b) => Number(b.id) - Number(a.id));
  return el(
    'div',
    {},
    pageHead('Your orders'),
    el('div', { class: 'stack' },
      sorted.map((o) =>
        el('section', { class: 'order', 'aria-label': `Order ${o.id}` },
          el('header', { class: 'order-head' },
            el('div', {}, el('strong', {}, `Order #${o.id}`), el('span', { class: 'muted' }, ` ${formatDate(o.createdAt)}`)),
            statusPill(o.status),
            el('strong', {}, formatMoney(o.totalCents))),
          el('ul', { class: 'order-lines' },
            o.items.map((it) =>
              el('li', {},
                el('span', {},
                  el('a', { href: `#/product/${encodeURIComponent(it.productId)}` }, it.name),
                  ` × ${it.quantity}`),
                el('span', { class: 'order-line-right' },
                  formatMoney(it.lineTotalCents),
                  o.status === 'DELIVERED'
                    ? el('a', { class: 'btn btn-ghost btn-sm', href: hashLink(`/product/${it.productId}`, { review: 1 }) }, 'Write a review')
                    : null)))))),
    ),
  );
}

/* ================================================================== */
/* Seller dashboard                                                    */
/* ================================================================== */

const NEXT_STATUS = { PENDING: 'CONFIRMED', CONFIRMED: 'SHIPPED', SHIPPED: 'DELIVERED' };

function openListingDialog(existing, onSaved, sellerId) {
  const isEdit = Boolean(existing);
  const form = el('form', { class: 'form', novalidate: true });
  const save = el('button', { class: 'btn btn-primary', type: 'submit' }, isEdit ? 'Save changes' : 'Create listing');
  const cancel = el('button', { class: 'btn btn-ghost', type: 'button' }, 'Cancel');
  form.append(
    el('h2', {}, isEdit ? 'Edit listing' : 'New listing'),
    field({ label: 'Name', name: 'name', required: true, maxlength: 120, value: existing?.name ?? '' }),
    field({ label: 'Description', name: 'description', type: 'textarea', value: existing?.description ?? '' }),
    el('div', { class: 'grid-2' },
      field({ label: `Price (${CONFIG.currency})`, name: 'price', inputmode: 'decimal', value: existing ? centsToInput(existing.priceCents) : '', hint: 'For example 499.00' }),
      field({ label: 'Stock', name: 'stock_qty', type: 'number', min: 0, step: 1, value: existing?.stockQty ?? 1 })),
    selectField({
      label: 'Category', name: 'category', value: existing?.category || CONFIG.categories[0],
      options: CONFIG.categories.map((c) => ({ value: c, label: c })),
    }),
    field({ label: 'Image URL', name: 'image_url', placeholder: 'https://…', value: existing?.imageUrl ?? '', hint: 'Optional. Must start with http:// or https://' }),
    el('p', { class: 'form-error', role: 'alert' }),
    el('div', { class: 'row end' }, cancel, save),
  );
  const dlg = el('dialog', { class: 'dialog dialog-wide' }, form);
  dlg.addEventListener('close', () => dlg.remove());
  cancel.addEventListener('click', () => dlg.close());

  bindForm(form, save, async (fd) => {
    const name = String(fd.get('name')).trim();
    const price = parseMoneyToCents(fd.get('price'));
    const stock = Number(fd.get('stock_qty'));
    const imageUrl = String(fd.get('image_url') ?? '').trim();
    const errors = {};
    if (!name) errors.name = 'Enter a name.';
    if (price === null) errors.price = 'Enter a price above 0, like 499.00.';
    if (!Number.isInteger(stock) || stock < 0) errors.stock_qty = 'Enter a whole number, 0 or more.';
    if (imageUrl && !safeUrl(imageUrl)) errors.image_url = 'Use a link that starts with http:// or https://';
    if (Object.keys(errors).length) throw new ApiError(400, 'VALIDATION_ERROR', 'Check the highlighted fields.', errors);

    const input = {
      name,
      description: String(fd.get('description') ?? '').trim(),
      priceCents: price,
      stockQty: stock,
      category: String(fd.get('category')),
      imageUrl,
    };
    if (isEdit) await api.updateProduct(existing.id, input, sellerId);
    else await api.createProduct(input, sellerId);
    toast(isEdit ? 'Listing saved' : 'Listing created');
    dlg.close();
    onSaved();
  });

  document.body.append(dlg);
  dlg.showModal();
}

export async function seller(ctx) {
  const user = ctx.state.user;
  const data = { listings: [], orders: [], ordersError: null };
  let tab = ctx.query.tab === 'orders' ? 'orders' : 'listings';

  const root = el('div');

  async function reload() {
    data.listings = await api.sellerProducts(user.id);
    data.ordersError = null;
    try {
      data.orders = await api.sellerOrders();
    } catch (e) {
      data.orders = [];
      data.ordersError = e;
    }
    draw();
  }

  // Only count line items that belong to this seller
  const myItems = (o) => o.items.filter((i) => i.sellerId == null || String(i.sellerId) === String(user.id));

  function stats() {
    const live = data.orders.filter((o) => o.status !== 'CANCELLED');
    const revenue = live.reduce((s, o) => s + myItems(o).reduce((t, i) => t + i.lineTotalCents, 0), 0);
    const pending = data.orders.filter((o) => o.status === 'PENDING').length;
    const stat = (label, value) => el('div', { class: 'stat' }, el('dd', {}, value), el('dt', {}, label));
    return el('dl', { class: 'stats' },
      stat('Active listings', String(data.listings.length)),
      stat('Orders to fulfil', String(pending)),
      stat('Revenue', formatMoney(revenue)));
  }

  function listingsPanel() {
    if (!data.listings.length) {
      return el('div', { class: 'empty' }, el('h2', {}, 'No listings yet'), el('p', {}, 'Create your first listing so buyers can find it.'));
    }
    return table(
      ['Product', 'Category', 'Price', 'Stock', ''],
      data.listings.map((p) => {
        const edit = el('button', { class: 'btn btn-ghost btn-sm', type: 'button' }, 'Edit');
        const del = el('button', { class: 'btn btn-link danger', type: 'button' }, 'Delete');
        edit.addEventListener('click', () => openListingDialog(p, reload, user.id));
        del.addEventListener('click', async () => {
          if (!(await confirmDialog(`Delete “${p.name}”? This cannot be undone.`, 'Delete listing'))) return;
          try {
            await api.deleteProduct(p.id, user.id);
            toast('Listing deleted');
            await reload();
          } catch (e) {
            toast(e.message, 'error');
          }
        });
        return [
          el('a', { href: `#/product/${encodeURIComponent(p.id)}` }, p.name),
          p.category,
          formatMoney(p.priceCents),
          p.stockQty <= 0 ? el('span', { class: 'stock bad' }, 'Sold out') : String(p.stockQty),
          el('div', { class: 'row end' }, edit, del),
        ];
      }),
    );
  }

  function ordersPanel() {
    if (data.ordersError) {
      return el('div', { class: 'empty' }, el('h2', {}, 'Could not load orders'), el('p', {}, data.ordersError.message));
    }
    if (!data.orders.length) {
      return el('div', { class: 'empty' }, el('h2', {}, 'No orders yet'), el('p', {}, 'Orders for your products will show up here.'));
    }
    return table(
      ['Order', 'Buyer', 'Items', 'Total', 'Status', ''],
      data.orders.map((o) => {
        const mine = myItems(o);
        const next = NEXT_STATUS[o.status];
        let action = '';
        if (next) {
          const b = el('button', { class: 'btn btn-ghost btn-sm', type: 'button' }, `Mark ${next.toLowerCase()}`);
          b.addEventListener('click', async () => {
            setBusy(b, true);
            try {
              await api.setOrderStatus(o.id, next);
              toast(`Order #${o.id} marked ${next.toLowerCase()}`);
              await reload();
            } catch (e) {
              toast(e.message, 'error');
              setBusy(b, false);
            }
          });
          action = b;
        }
        return [
          `#${o.id} ${formatDate(o.createdAt)}`,
          o.buyerName ?? `User ${o.buyerId ?? ''}`,
          el('ul', { class: 'plain' }, mine.map((i) => el('li', {}, `${i.name} × ${i.quantity}`))),
          formatMoney(mine.reduce((t, i) => t + i.lineTotalCents, 0)),
          statusPill(o.status),
          action,
        ];
      }),
    );
  }

  function draw() {
    const newBtn = el('button', { class: 'btn btn-primary', type: 'button' }, 'New listing');
    newBtn.addEventListener('click', () => openListingDialog(null, reload, user.id));
    root.replaceChildren(
      pageHead('Seller dashboard', newBtn),
      stats(),
      tabsNav([['listings', 'My listings'], ['orders', 'Incoming orders']], tab, (k) => { tab = k; draw(); }),
      el('div', { class: 'panel', role: 'tabpanel' }, tab === 'listings' ? listingsPanel() : ordersPanel()),
    );
  }

  await reload();
  return root;
}

/* ================================================================== */
/* Admin                                                               */
/* ================================================================== */

export async function admin(ctx) {
  const root = el('div');
  const panel = el('div', { class: 'panel', role: 'tabpanel' });
  let tab = ['users', 'orders', 'listings'].includes(ctx.query.tab) ? ctx.query.tab : 'users';

  const loaders = {
    async users() {
      const users = await api.adminUsers();
      return users.length
        ? table(['ID', 'Name', 'Email', 'Role', 'Joined'], users.map((u) => [String(u.id), u.name, u.email, u.role, formatDate(u.createdAt)]))
        : emptyState('No users', 'Nobody has registered yet.');
    },
    async orders() {
      const list = await api.adminOrders();
      return list.length
        ? table(['Order', 'Buyer', 'Status', 'Total', 'Placed'],
            list.map((o) => [`#${o.id}`, o.buyerName ?? `User ${o.buyerId ?? ''}`, statusPill(o.status), formatMoney(o.totalCents), formatDate(o.createdAt)]))
        : emptyState('No orders', 'No orders have been placed yet.');
    },
    async listings() {
      const list = await api.products({});
      if (!list.length) return emptyState('No listings', 'Sellers have not published anything yet.');
      return table(['Product', 'Seller', 'Price', 'Stock', ''],
        list.map((p) => {
          const rm = el('button', { class: 'btn btn-link danger', type: 'button' }, 'Remove');
          rm.addEventListener('click', async () => {
            if (!(await confirmDialog(`Remove “${p.name}” from the marketplace?`, 'Remove listing'))) return;
            try {
              await api.adminDeleteProduct(p.id);
              toast('Listing removed');
              load();
            } catch (e) {
              toast(e.message, 'error');
            }
          });
          return [el('a', { href: `#/product/${encodeURIComponent(p.id)}` }, p.name), p.sellerName ?? String(p.sellerId ?? ''), formatMoney(p.priceCents), String(p.stockQty), rm];
        }));
    },
  };

  async function load() {
    panel.replaceChildren(loading());
    try {
      panel.replaceChildren(await loaders[tab]());
    } catch (e) {
      panel.replaceChildren(errorPanel(e, load));
    }
  }

  function draw() {
    root.replaceChildren(
      pageHead('Admin'),
      tabsNav([['users', 'Users'], ['orders', 'Orders'], ['listings', 'Listings']], tab, (k) => { tab = k; draw(); }),
      panel,
    );
    load();
  }

  draw();
  return root;
}
