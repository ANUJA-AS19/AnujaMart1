import { CONFIG } from './config.js';
import { api, setUnauthorizedHandler, getAuthToken, setAuthToken } from './api.js';
import { el, toast, hashLink } from './ui.js';
import * as V from './views.js';

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */

const state = { user: null };
const USER_CACHE_KEY = 'ynm.user'; // UX hint only. The server session is the authority.

const routes = [
  { path: '/', view: V.browse },
  { path: '/product/:id', view: V.productDetail },
  { path: '/login', view: V.login },
  { path: '/register', view: V.register },
  { path: '/cart', view: V.cart, roles: ['BUYER'] },
  { path: '/checkout', view: V.checkout, roles: ['BUYER'] },
  { path: '/orders', view: V.orders, roles: ['BUYER'] },
  { path: '/seller', view: V.seller, roles: ['SELLER'] },
  { path: '/admin', view: V.admin, roles: ['ADMIN'] },
];

/* ------------------------------------------------------------------ */
/* Routing                                                             */
/* ------------------------------------------------------------------ */

function parseHash() {
  const raw = location.hash.replace(/^#/, '') || '/';
  const i = raw.indexOf('?');
  const path = (i === -1 ? raw : raw.slice(0, i)) || '/';
  const query = Object.fromEntries(new URLSearchParams(i === -1 ? '' : raw.slice(i + 1)));
  return { path, query };
}

function matchRoute(path) {
  for (const route of routes) {
    const names = [];
    const pattern = route.path.replace(/:([A-Za-z]+)/g, (_, n) => { names.push(n); return '([^/]+)'; });
    const m = new RegExp(`^${pattern}$`).exec(path);
    if (m) {
      const params = Object.fromEntries(names.map((n, idx) => [n, decodeURIComponent(m[idx + 1])]));
      return { route, params };
    }
  }
  return null;
}

/** navigate('/login', {next: '/cart'}) or navigate('/product/3?review=1') */
function navigate(to, query) {
  const target = query ? hashLink(to, query) : `#${to}`;
  if (location.hash === target) render(false);
  else location.hash = target;
}

let renderToken = 0;

async function render(isNavigation = true) {
  const token = ++renderToken;
  const main = document.getElementById('main');
  const { path, query } = parseHash();
  renderHeader(path);

  const found = matchRoute(path);
  if (!found) {
    main.replaceChildren(V.notFound());
    return;
  }
  const { route, params } = found;

  if (route.roles) {
    if (!state.user) {
      navigate('/login', { next: path });
      return;
    }
    if (!route.roles.includes(state.user.role)) {
      main.replaceChildren(V.forbidden());
      return;
    }
  }

  if (isNavigation) main.replaceChildren(V.loading());
  try {
    const node = await route.view({ ...ctx, params, query });
    if (token !== renderToken) return; // a newer navigation superseded this one
    main.replaceChildren(node);
  } catch (err) {
    if (token !== renderToken) return;
    main.replaceChildren(V.errorPanel(err, () => render(true)));
  }
  if (isNavigation && token === renderToken) {
    window.scrollTo(0, 0);
    main.focus({ preventScroll: true });
  }
}

/* ------------------------------------------------------------------ */
/* Session + cart badge                                                */
/* ------------------------------------------------------------------ */

async function setUser(user) {
  state.user = user;
  if (!user) setAuthToken(null);
  try {
    if (user) sessionStorage.setItem(USER_CACHE_KEY, JSON.stringify(user));
    else sessionStorage.removeItem(USER_CACHE_KEY);
  } catch { /* storage unavailable */ }
  renderHeader(parseHash().path);
  await refreshCartCount();
}

async function refreshCartCount() {
  const badge = document.getElementById('cart-count');
  if (!badge) return;
  if (state.user?.role !== 'BUYER') {
    badge.hidden = true;
    return;
  }
  try {
    const c = await api.cart();
    const n = c.items.reduce((s, i) => s + i.quantity, 0);
    badge.textContent = String(n);
    badge.hidden = n === 0;
  } catch {
    badge.hidden = true;
  }
}

async function logout() {
  try {
    await api.logout();
  } catch { /* clear local state even if the call fails */ }
  await setUser(null);
  toast('Signed out');
  navigate('/');
}

setUnauthorizedHandler(() => {
  if (state.user) {
    setUser(null);
    toast('Your session expired. Sign in again.', 'error');
    navigate('/login');
  }
});

const ctx = { state, navigate, setUser, refreshCartCount, rerender: () => render(false) };

/* ------------------------------------------------------------------ */
/* Header / footer                                                     */
/* ------------------------------------------------------------------ */

function renderHeader(currentPath) {
  const u = state.user;
  const link = (href, label, ...extra) =>
    el('a', { href, class: 'nav-link', 'aria-current': currentPath === href.slice(1) ? 'page' : null }, label, ...extra);

  const links = [link('#/', 'Browse')];
  if (u?.role === 'BUYER') {
    links.push(link('#/cart', 'Cart ', el('span', { id: 'cart-count', class: 'badge', hidden: true }, '0')));
    links.push(link('#/orders', 'Orders'));
  }
  if (u?.role === 'SELLER') links.push(link('#/seller', 'Dashboard'));
  if (u?.role === 'ADMIN') links.push(link('#/admin', 'Admin'));

  const account = u
    ? [el('span', { class: 'who' }, u.name), el('button', { class: 'btn btn-ghost-light btn-sm', type: 'button', onclick: logout }, 'Sign out')]
    : [link('#/login', 'Sign in'), el('a', { class: 'btn btn-accent btn-sm', href: '#/register' }, 'Create account')];

  const search = el(
    'form',
    {
      class: 'search', role: 'search',
      onsubmit: (e) => {
        e.preventDefault();
        navigate('/', { q: String(new FormData(e.target).get('q') ?? '').trim() });
      },
    },
    el('label', { class: 'sr-only', for: 'site-q' }, 'Search products'),
    el('input', { id: 'site-q', name: 'q', type: 'search', placeholder: 'Search products', value: currentPath === '/' ? (parseHash().query.q ?? '') : '' }),
    el('button', { class: 'btn btn-accent', type: 'submit' }, 'Search'),
  );

  document.getElementById('site-header').replaceChildren(
    el('div', { class: 'container header-inner' },
      el('a', { class: 'brand', href: '#/' }, CONFIG.brand),
      search,
      el('nav', { class: 'nav', 'aria-label': 'Main' }, links, el('span', { class: 'nav-account' }, account))),
  );
  if (u?.role === 'BUYER') refreshCartCount();
}

async function renderFooter() {
  const status = el('span', { class: 'health' }, 'Checking server…');
  document.getElementById('site-footer').replaceChildren(
    el('div', { class: 'container footer-inner' }, el('p', {}, `© ${new Date().getFullYear()} ${CONFIG.brand}`), status),
  );
  try {
    const h = await api.health();
    status.textContent = `Server ${h?.status ?? 'UP'}, database ${h?.db ?? 'UP'}`;
    status.classList.add('ok');
  } catch {
    status.textContent = 'Server unreachable';
    status.classList.add('bad');
  }
}

/* ------------------------------------------------------------------ */
/* Boot                                                                */
/* ------------------------------------------------------------------ */

async function init() {
  document.title = CONFIG.brand;
  // The backend has no /auth/me route, so restore the saved user, but only while we still hold a token.
  // If the token has expired, the first protected request returns 401 and we sign out cleanly.
  state.user = null;
  if (getAuthToken()) {
    try { state.user = JSON.parse(sessionStorage.getItem(USER_CACHE_KEY)); } catch { /* ignore */ }
  }
  renderFooter();
  window.addEventListener('hashchange', () => render(true));
  render(true);
}

init();
