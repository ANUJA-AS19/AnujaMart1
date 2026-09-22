import { CONFIG } from './config.js';

/* ------------------------------------------------------------------ */
/* DOM builder.                                                        */
/* Strings are ALWAYS inserted as text nodes, never as HTML, so user-  */
/* supplied data (names, descriptions, reviews) cannot inject markup.  */
/* Never use innerHTML in this codebase.                               */
/* ------------------------------------------------------------------ */

function append(node, child) {
  if (child === null || child === undefined || child === false) return;
  if (Array.isArray(child)) {
    child.forEach((c) => append(node, c));
    return;
  }
  node.append(child instanceof Node ? child : document.createTextNode(String(child)));
}

export function el(tag, props, ...children) {
  const node = document.createElement(tag);
  const deferred = {};
  for (const [k, v] of Object.entries(props ?? {})) {
    if (v === null || v === undefined || v === false) continue;
    if (k === 'class') node.className = v;
    else if (k.startsWith('on') && typeof v === 'function') node.addEventListener(k.slice(2), v);
    else if (k === 'value' || k === 'checked') deferred[k] = v; // set after children (needed for <select>)
    else node.setAttribute(k, v === true ? '' : v);
  }
  children.forEach((c) => append(node, c));
  for (const [k, v] of Object.entries(deferred)) node[k] = v;
  return node;
}

export const clear = (node) => node.replaceChildren();

/* ------------------------------------------------------------------ */
/* Toasts, busy state, confirm dialog                                  */
/* ------------------------------------------------------------------ */

export function toast(message, kind = 'info') {
  const box = document.getElementById('toasts');
  const t = el('div', { class: `toast toast-${kind}`, role: kind === 'error' ? 'alert' : 'status' }, message);
  box.append(t);
  setTimeout(() => t.remove(), kind === 'error' ? 6000 : 3500);
}

export function setBusy(button, busy) {
  if (!button) return;
  button.disabled = busy;
  if (busy) button.setAttribute('aria-busy', 'true');
  else button.removeAttribute('aria-busy');
}

export function confirmDialog(message, confirmLabel = 'Confirm') {
  return new Promise((resolve) => {
    const dlg = el(
      'dialog',
      { class: 'dialog' },
      el(
        'form',
        { method: 'dialog' },
        el('p', { class: 'dialog-text' }, message),
        el(
          'div',
          { class: 'row end' },
          el('button', { class: 'btn btn-ghost', value: 'cancel' }, 'Cancel'),
          el('button', { class: 'btn btn-danger', value: 'ok' }, confirmLabel),
        ),
      ),
    );
    dlg.addEventListener('close', () => {
      resolve(dlg.returnValue === 'ok');
      dlg.remove();
    });
    document.body.append(dlg);
    dlg.showModal();
  });
}

/* ------------------------------------------------------------------ */
/* Money & dates (display only: the server owns the real arithmetic)   */
/* ------------------------------------------------------------------ */

let currencyFmt;
export function formatMoney(cents) {
  currencyFmt ??= new Intl.NumberFormat(CONFIG.locale, { style: 'currency', currency: CONFIG.currency });
  return currencyFmt.format(Number(cents) / 100);
}

/** "499", "499.5", "499.50" -> integer minor units. Returns null if invalid or <= 0. Avoids float maths. */
export function parseMoneyToCents(text) {
  const m = /^\s*(\d{1,10})(?:\.(\d{1,2}))?\s*$/.exec(text ?? '');
  if (!m) return null;
  const cents = Number(m[1]) * 100 + Number((m[2] ?? '').padEnd(2, '0'));
  return cents > 0 ? cents : null;
}

export function centsToInput(cents) {
  const c = Number(cents) || 0;
  return `${Math.floor(c / 100)}.${String(c % 100).padStart(2, '0')}`;
}

export function formatDate(value) {
  if (!value) return '';
  const d = new Date(value);
  if (Number.isNaN(d.getTime())) return String(value);
  return d.toLocaleDateString(CONFIG.locale, { year: 'numeric', month: 'short', day: 'numeric' });
}

export function hashLink(path, query = {}) {
  const qs = new URLSearchParams(Object.entries(query).filter(([, v]) => v !== '' && v != null));
  const s = qs.toString();
  return `#${path}${s ? `?${s}` : ''}`;
}

/* ------------------------------------------------------------------ */
/* Shared components                                                   */
/* ------------------------------------------------------------------ */

/** Only http(s) URLs are allowed as image sources. */
export function safeUrl(u) {
  if (!u) return null;
  try {
    const x = new URL(u, location.origin);
    return x.protocol === 'http:' || x.protocol === 'https:' ? x.href : null;
  } catch {
    return null;
  }
}

function placeholder(p) {
  let h = 0;
  for (const ch of String(p.name ?? '?')) h = (h * 31 + ch.charCodeAt(0)) % 360;
  const node = el('div', { class: 'ph', 'aria-hidden': 'true' }, (p.name ?? '?').charAt(0).toUpperCase());
  node.style.setProperty('--hue', String(h));
  return node;
}

export function productImage(p, alt = '') {
  const src = safeUrl(p.imageUrl);
  if (!src) return placeholder(p);
  const img = el('img', { src, alt, loading: 'lazy' });
  img.addEventListener('error', () => img.replaceWith(placeholder(p)));
  return img;
}

export const priceTag = (cents) => el('span', { class: 'price-tag' }, formatMoney(cents));

export function stars(rating) {
  const r = Math.max(0, Math.min(5, Math.round(Number(rating) || 0)));
  return el('span', { class: 'stars', role: 'img', 'aria-label': `${r} out of 5 stars` }, '★'.repeat(r) + '☆'.repeat(5 - r));
}

const titleCase = (s) => s.charAt(0) + s.slice(1).toLowerCase();
export const statusPill = (status) => el('span', { class: `pill pill-${status.toLowerCase()}` }, titleCase(status));

export function emptyState(title, text, href, cta) {
  return el(
    'div',
    { class: 'empty' },
    el('h2', {}, title),
    el('p', {}, text),
    href ? el('a', { class: 'btn btn-primary', href }, cta) : null,
  );
}

/** headers: string[]; rows: (Node|string)[][] */
export function table(headers, rows) {
  return el(
    'div',
    { class: 'table-wrap' },
    el(
      'table',
      {},
      el('thead', {}, el('tr', {}, headers.map((h) => el('th', { scope: 'col' }, h)))),
      el('tbody', {}, rows.map((r) => el('tr', {}, r.map((c) => el('td', {}, c))))),
    ),
  );
}
