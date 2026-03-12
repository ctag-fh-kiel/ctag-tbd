// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Shared Utilities
// Vanilla JS · Shoelace Web Components
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
//
// Licensed under the GNU Lesser General Public License (LGPL 3.0).
// https://www.gnu.org/licenses/lgpl-3.0.txt
//
// Part of the dadamachines additions to the CTAG TBD platform.
// See LICENSE in the repository root for full terms.
// ═══════════════════════════════════════════════════════════════
'use strict';

// ─── Constants ───────────────────────────────────────────────
const API_V2 = '/api/v2';

// sun-fill SVG not in our Shoelace bundle — register it as a data URI
// so the theme toggle doesn't trigger a network fetch
const SUN_FILL_SVG = '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" viewBox="0 0 16 16"><path d="M8 12a4 4 0 1 0 0-8 4 4 0 0 0 0 8M8 0a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 0m0 13a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 13m8-5a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2a.5.5 0 0 1 .5.5M3 8a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2A.5.5 0 0 1 3 8m10.657-5.657a.5.5 0 0 1 0 .707l-1.414 1.415a.5.5 0 1 1-.707-.708l1.414-1.414a.5.5 0 0 1 .707 0m-9.193 9.193a.5.5 0 0 1 0 .707L3.05 13.657a.5.5 0 0 1-.707-.707l1.414-1.414a.5.5 0 0 1 .707 0m9.193 2.121a.5.5 0 0 1-.707 0l-1.414-1.414a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707M3.757 4.464a.5.5 0 0 1-.707 0L1.636 3.05a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707"/></svg>';

// ═══════════════════════════════════════════════════════════════
//  HTML ESCAPING
// ═══════════════════════════════════════════════════════════════

function esc(s) {
  const d = document.createElement('div');
  d.textContent = s;
  return d.innerHTML;
}

// ═══════════════════════════════════════════════════════════════
//  FORMATTING
// ═══════════════════════════════════════════════════════════════

function formatBytes(bytes) {
  if (bytes < 1024) return `${bytes} B`;
  if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
  if (bytes < 1073741824) return `${(bytes / 1048576).toFixed(1)} MB`;
  return `${(bytes / 1073741824).toFixed(1)} GB`;
}

// ═══════════════════════════════════════════════════════════════
//  API CLIENT
// ═══════════════════════════════════════════════════════════════

/**
 * GET request to /api/v2/<path>
 * @param {string} path - e.g. '/plugins?action=list'
 * @returns {Promise<any>} parsed JSON response
 */
// Default timeout for API read requests (ms).
// ESP32 responses are fast (<200ms) but USB NCM can stall;
// 10s matches server-side recv/send_wait_timeout.
var API_TIMEOUT_MS = 10000;
// Longer timeout for mutation operations (loadPreset, savePreset, etc.)
// that trigger SD card I/O on the firmware side.
var API_MUTATION_TIMEOUT_MS = 20000;
// Extra-long timeout for plugin switches — WTOsc, WTOscDuo, Freakwaves, VctrSnt
// trigger ctagSampleRom SD card loading (all wavetable/sample data into PSRAM),
// which can take 15-30+ seconds. The firmware blocks the HTTP response until done.
var API_PLUGIN_SWITCH_TIMEOUT_MS = 45000;

// ─── Circuit Breaker ─────────────────────────────────────────
// Track consecutive API failures.  After _FAILURE_THRESHOLD in a row,
// trigger disconnect + drain the request queue to stop hammering a
// dead device.  Threshold is generous because the ESP32 httpd can
// be temporarily unresponsive during heavy plugin allocation
// (mutex contention with audio task, SD card I/O).
var _consecutiveFailures = 0;
var _FAILURE_THRESHOLD = 4;

async function apiFetch(path, timeoutMs, skipCircuitBreaker) {
  timeoutMs = timeoutMs || API_TIMEOUT_MS;
  try {
    const r = await fetch(`${API_V2}${path}`, {
      signal: AbortSignal.timeout(timeoutMs),
    });
    if (!r.ok) throw new Error(`API ${r.status}`);
    _consecutiveFailures = 0;
    var text = await r.text();
    if (!text || !text.trim()) return {};
    try { return JSON.parse(text); } catch(e) { return {}; }
  } catch(e) {
    // Don't count toward circuit breaker if caller opted out.
    // Plugin switch timeouts mean the device is busy (loading sample ROM),
    // NOT offline.  Only genuine network errors should trigger disconnect.
    if (!skipCircuitBreaker) {
      _consecutiveFailures++;
      if (_consecutiveFailures >= _FAILURE_THRESHOLD) {
        setDisconnected();
        apiQueue.drain();
      }
    }
    throw e;
  }
}

/**
 * POST request to /api/v2/<path> with optional JSON body.
 * If body is null/undefined, sends a POST with no body (for query-param-only mutations).
 * @param {string} path - e.g. '/device?action=setConfig'
 * @param {object|null} body - JSON-serializable data, or null for body-less POST
 * @returns {Promise<any>} parsed JSON response
 */
async function apiPostJSON(path, body, timeoutMs, skipCircuitBreaker) {
  timeoutMs = timeoutMs || API_TIMEOUT_MS;
  try {
    var opts = {
      method: 'POST',
      signal: AbortSignal.timeout(timeoutMs),
    };
    if (body != null) {
      opts.headers = { 'Content-Type': 'application/json' };
      opts.body = JSON.stringify(body);
    }
    const r = await fetch(`${API_V2}${path}`, opts);
    if (!r.ok) throw new Error(`API ${r.status}`);
    _consecutiveFailures = 0;
    var text = await r.text();
    if (!text || !text.trim()) return {};
    try { return JSON.parse(text); } catch(e) { return {}; }
  } catch(e) {
    if (!skipCircuitBreaker) {
      _consecutiveFailures++;
      if (_consecutiveFailures >= _FAILURE_THRESHOLD) {
        setDisconnected();
        apiQueue.drain();
      }
    }
    throw e;
  }
}

// ═══════════════════════════════════════════════════════════════
//  FETCH QUEUE — Serialize requests to avoid overwhelming ESP32
// ═══════════════════════════════════════════════════════════════

class FetchQueue {
  constructor() {
    this._queue = [];
    this._running = false;
    this._paused = false;
  }

  enqueue(fn) {
    if (this._paused) {
      return Promise.reject(new Error('Device offline \u2014 request cancelled'));
    }
    return new Promise((resolve, reject) => {
      this._queue.push({ fn, resolve, reject });
      this._process();
    });
  }

  async _process() {
    if (this._running) return;
    this._running = true;
    while (this._queue.length && !this._paused) {
      const { fn, resolve, reject } = this._queue.shift();
      try { resolve(await fn()); } catch (e) { reject(e); }
    }
    this._running = false;
  }

  /** Reject all pending items and pause the queue. */
  drain() {
    this._paused = true;
    while (this._queue.length) {
      var item = this._queue.shift();
      item.reject(new Error('Device offline \u2014 request cancelled'));
    }
  }

  /** Resume accepting new items after reconnection. */
  resume() {
    this._paused = false;
  }
}

// Shared queue for serializing parameter SET calls across views
const paramQueue = new FetchQueue();

// Global API queue — ALL API calls route through this to keep
// max 1 in-flight request at a time (ESP32 has only ~4 usable sockets).
const apiQueue = new FetchQueue();

/**
 * Queue-wrapped apiFetch — serializes all GET requests.
 * @param {string} path - e.g. '/plugins?action=list'
 * @returns {Promise<any>} parsed JSON response
 */
function queuedFetch(path, timeoutMs, skipCircuitBreaker) {
  return apiQueue.enqueue(function() { return apiFetch(path, timeoutMs, skipCircuitBreaker); });
}

/**
 * Queue-wrapped apiPostJSON — serializes all POST requests.
 * @param {string} path - e.g. '/device?action=setConfig'
 * @param {object|null} body
 * @param {number} [timeoutMs]
 * @param {boolean} [skipCircuitBreaker]
 * @returns {Promise<any>} parsed JSON response
 */
function queuedPost(path, body, timeoutMs, skipCircuitBreaker) {
  return apiQueue.enqueue(function() { return apiPostJSON(path, body, timeoutMs, skipCircuitBreaker); });
}

// ═══════════════════════════════════════════════════════════════
//  TOAST NOTIFICATIONS
// ═══════════════════════════════════════════════════════════════

var _toastBusy = false;
var _toastCount = 0;           // throttle: max toasts in flight
var _TOAST_MAX_PENDING = 5;    // discard beyond this to avoid DOM leak

function toast(message, variant, duration) {
  if (_toastBusy) return;          // prevent synchronous recursion
  if (_toastCount >= _TOAST_MAX_PENDING) return;  // throttle
  _toastBusy = true;
  _toastCount++;
  try {
    variant = variant || 'primary';
    duration = duration || 4000;
    var stack = document.getElementById('toast-stack');
    if (!stack) { _toastBusy = false; _toastCount--; return; }
    var alert = document.createElement('sl-alert');
    alert.variant = variant;
    alert.closable = true;
    alert.duration = duration;
    alert.innerHTML = '<sl-icon slot="icon" name="' + iconForVariant(variant) + '">' + '</sl-icon>' + esc(message);
    // Decrement counter when alert is removed from DOM (auto-hide or close)
    alert.addEventListener('sl-after-hide', function() { _toastCount = Math.max(0, _toastCount - 1); });
    stack.appendChild(alert);
    // Only call .toast() if sl-alert is already defined; skip silently otherwise
    if (customElements.get('sl-alert')) {
      try { alert.toast(); } catch(e) { console.warn('toast render failed:', e); }
    }
    // If sl-alert not defined yet, just leave it in DOM (don't queue .whenDefined
    // which causes async re-entrancy problems)
  } catch(e) {
    console.warn('toast() error:', e);
    _toastCount = Math.max(0, _toastCount - 1);
  } finally {
    _toastBusy = false;
  }
}

function iconForVariant(v) {
  var map = {
    success: 'check2-circle',
    warning: 'exclamation-triangle',
    danger: 'exclamation-octagon',
    primary: 'info-circle',
    neutral: 'info-circle'
  };
  return map[v] || 'info-circle';
}

// ═══════════════════════════════════════════════════════════════
//  THEME MANAGEMENT
// ═══════════════════════════════════════════════════════════════

function setupThemeToggle(btnId) {
  var btn = document.getElementById(btnId || 'theme-toggle');
  if (!btn) return;
  var saved = localStorage.getItem('tbd-theme');
  if (saved === 'light') applyTheme('light');
  btn.addEventListener('click', function() {
    var isDark = document.documentElement.classList.contains('sl-theme-dark');
    applyTheme(isDark ? 'light' : 'dark');
  });
}

function applyTheme(theme) {
  var html = document.documentElement;
  var btn  = document.getElementById('theme-toggle');
  if (theme === 'light') {
    html.classList.remove('sl-theme-dark');
    html.classList.add('sl-theme-light');
    if (btn) btn.name = 'sun-fill';
  } else {
    html.classList.remove('sl-theme-light');
    html.classList.add('sl-theme-dark');
    if (btn) btn.name = 'moon-fill';
  }
  // If page loads only one theme <link> (e.g. index.html), swap its href
  var links = document.querySelectorAll('link[href*="/shoelace/themes/"]');
  if (links.length === 1) {
    links[0].href = '/shoelace/themes/' + theme + '.css?v=3';
  }
  // If both themes are pre-loaded (e.g. preset-macro-manager.html), class toggle suffices
  localStorage.setItem('tbd-theme', theme);
}

// ═══════════════════════════════════════════════════════════════
//  CONNECTION MONITOR
// ═══════════════════════════════════════════════════════════════

var connectionState = {
  status: 'connecting',   // 'connecting' | 'connected' | 'disconnected'
  retries: 0,
  maxRetries: 60,
  pollIntervalMs: 5000,   // WLED uses 5s for HTTP fallback reconnect
  _timer: null,
  _onConnect: null,
  _onDisconnect: null,
};

function startConnectionMonitor(onConnect, onDisconnect) {
  connectionState._onConnect = onConnect;
  connectionState._onDisconnect = onDisconnect;
}

function setConnected() {
  if (connectionState.status === 'connected') return;
  connectionState.status = 'connected';
  connectionState.retries = 0;
  _consecutiveFailures = 0;
  apiQueue.resume();
  updateConnectionUI();
  if (connectionState._onConnect) connectionState._onConnect();
}

function setDisconnected() {
  if (connectionState.status === 'disconnected') return;
  connectionState.status = 'disconnected';
  updateConnectionUI();
  if (connectionState._onDisconnect) connectionState._onDisconnect();
  scheduleReconnect();
}

function scheduleReconnect() {
  if (connectionState._timer) return;
  connectionState._timer = setInterval(async function() {
    if (connectionState.retries >= connectionState.maxRetries) {
      clearInterval(connectionState._timer);
      connectionState._timer = null;
      return;
    }
    // Skip poll if the API queue is busy (user-initiated request in-flight)
    if (apiQueue._running) return;
    connectionState.retries++;
    try {
      await apiFetch('/device?action=getIOCaps');
      clearInterval(connectionState._timer);
      connectionState._timer = null;
      setConnected();
    } catch (e) {
      // still disconnected
    }
  }, connectionState.pollIntervalMs);
}

function updateConnectionUI() {
  var el = document.getElementById('status-text');
  if (el) {
    if (connectionState.status === 'connected') {
      el.textContent = 'Connected';
      el.style.color = 'var(--sl-color-success-600)';
    } else if (connectionState.status === 'disconnected') {
      el.textContent = 'Offline';
      el.style.color = 'var(--sl-color-danger-600)';
    } else {
      el.textContent = 'Connecting\u2026';
      el.style.color = 'var(--sl-color-neutral-500)';
    }
  }
  // Update footer connection dot
  var dot = document.getElementById('footer-conn-dot');
  var txt = document.getElementById('footer-conn-text');
  if (dot) {
    dot.classList.toggle('offline', connectionState.status !== 'connected');
  }
  if (txt) {
    txt.textContent = connectionState.status === 'connected' ? 'Connected' :
                      connectionState.status === 'disconnected' ? 'Offline' : 'Connecting\u2026';
  }
  // Update header connection pill
  var pill = document.getElementById('conn-pill');
  var pillText = document.getElementById('conn-pill-text');
  if (pill) {
    pill.classList.toggle('offline', connectionState.status !== 'connected');
  }
  if (pillText) {
    pillText.textContent = connectionState.status === 'connected' ? 'Connected' :
                           connectionState.status === 'disconnected' ? 'Offline' : 'Connecting\u2026';
  }
}

// ═══════════════════════════════════════════════════════════════
//  DIALOG HELPERS
// ═══════════════════════════════════════════════════════════════

/**
 * Wrap a Shoelace sl-dialog in a Promise for await-friendly confirmation.
 * @param {string} dialogId - DOM id of the sl-dialog
 * @param {string} okBtnId - DOM id of the confirm button
 * @param {string} cancelBtnId - DOM id of the cancel button
 * @returns {Promise<boolean>} true if confirmed, false if cancelled
 */
function confirmDialog(dialogId, okBtnId, cancelBtnId) {
  return new Promise(function(resolve) {
    var dlg = document.getElementById(dialogId);
    var ok  = document.getElementById(okBtnId);
    var cancel = document.getElementById(cancelBtnId);
    if (!dlg) { resolve(false); return; }

    function cleanup() {
      ok.removeEventListener('click', onOk);
      if (cancel) cancel.removeEventListener('click', onCancel);
      dlg.removeEventListener('sl-request-close', onCancel);
    }
    function onOk() { cleanup(); dlg.hide(); resolve(true); }
    function onCancel() { cleanup(); dlg.hide(); resolve(false); }

    ok.addEventListener('click', onOk);
    if (cancel) cancel.addEventListener('click', onCancel);
    dlg.addEventListener('sl-request-close', onCancel);
    dlg.show();
  });
}

// ═══════════════════════════════════════════════════════════════
//  LOADING OVERLAY — visual feedback during heavy operations
// ═══════════════════════════════════════════════════════════════

function showLoading(message) {
  var overlay = document.getElementById('loading-overlay');
  var text = document.getElementById('loading-text');
  if (overlay) {
    if (text) text.textContent = message || 'Loading\u2026';
    overlay.classList.remove('hidden');
  }
}

function hideLoading() {
  var overlay = document.getElementById('loading-overlay');
  if (overlay) overlay.classList.add('hidden');
}

// ═══════════════════════════════════════════════════════════════
//  EXPORTS — attach to window for non-module scripts
// ═══════════════════════════════════════════════════════════════

// ─── Control Mode ──────────────────────────────────────────
var _waLoaded = false;

function isControlMode() {
  return localStorage.getItem('tbd-control-mode') === '1';
}

function setControlMode(on) {
  localStorage.setItem('tbd-control-mode', on ? '1' : '0');
  if (on && !_waLoaded) loadWebAudioControls();
}

function loadWebAudioControls() {
  // webaudio-controls.js is now included in the app-bundle.js
  // so it's always available — no dynamic loading needed
  _waLoaded = true;
  return Promise.resolve();
}

// ═══════════════════════════════════════════════════════════════
//  SVG KNOB RENDERER — matching webaudio-controls style
// ═══════════════════════════════════════════════════════════════

/**
 * Render an SVG rotary knob matching the webaudio-controls style.
 *
 * opts.value   – current value (default 0)
 * opts.min     – minimum value (default 0)
 * opts.max     – maximum value (default 127)
 * opts.size    – pixel diameter (default 52)
 * opts.color   – 'normal' (dark charcoal) | 'macro' (orange/gold)
 *
 * A "macro knob" controls 2+ DSP parameters via the mapping formula.
 */
function renderKnobSVG(opts) {
  var value = opts.value || 0;
  var min = opts.min || 0;
  var max = opts.max || 127;
  var size = opts.size || 52;
  var color = opts.color || 'normal';

  var pct = max > min ? ((value - min) / (max - min)) : 0;
  pct = Math.max(0, Math.min(1, pct));

  var cx = size / 2;
  var cy = size / 2;
  var r = (size / 2) - 2;

  // Rotation: 270° sweep from -135° to +135° (bottom-left to bottom-right)
  var angle = -135 + pct * 270;
  var rad = angle * Math.PI / 180;

  // Indicator line: from ~40% radius to ~88% radius
  var x1 = cx + r * 0.40 * Math.sin(rad);
  var y1 = cy - r * 0.40 * Math.cos(rad);
  var x2 = cx + r * 0.88 * Math.sin(rad);
  var y2 = cy - r * 0.88 * Math.cos(rad);

  // Color schemes — matching webaudio-controls colors attribute
  // colors = "indicator ; outerFill ; centerFill"
  var indicator, outerFill, centerFill, outerStroke;
  if (color === 'macro') {
    // Orange/gold for macro knobs (controls 2+ DSP params)
    indicator  = '#fef3c7';  // warm light yellow
    outerFill  = '#92400e';  // amber-800
    centerFill = '#b45309';  // amber-700
    outerStroke = '#78350f'; // amber-900
  } else if (color === 'mix') {
    // Light/silver knobs for the inverted Mix page
    indicator  = '#334155';  // dark slate indicator
    outerFill  = '#cbd5e1';  // slate-300
    centerFill = '#e2e8f0';  // slate-200
    outerStroke = '#94a3b8'; // slate-400
  } else {
    // Dark charcoal for normal knobs (1:1 mapping)
    indicator  = '#ccc';
    outerFill  = '#484848';
    centerFill = '#525252';
    outerStroke = '#3a3a3a';
  }

  // Use unique gradient IDs to avoid conflicts when multiple knobs are rendered
  var uid = 'k' + Math.random().toString(36).substr(2, 5);

  var svg = '';
  svg += '<svg width="' + size + '" height="' + size + '" viewBox="0 0 ' + size + ' ' + size + '" class="knob-svg">';

  // Definitions for gradients
  svg += '<defs>';
  // Radial gradient: center lighter, edge darker
  svg += '<radialGradient id="' + uid + 'g" cx="50%" cy="50%">';
  svg += '<stop offset="0%" stop-color="' + centerFill + '"/>';
  svg += '<stop offset="100%" stop-color="' + outerFill + '"/>';
  svg += '</radialGradient>';
  // Subtle bottom shadow
  svg += '<linearGradient id="' + uid + 's" x1="0" y1="0" x2="0" y2="1">';
  svg += '<stop offset="0%" stop-color="#000" stop-opacity="0"/>';
  svg += '<stop offset="100%" stop-color="#000" stop-opacity="0.15"/>';
  svg += '</linearGradient>';
  svg += '</defs>';

  // Outer shadow halo
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + r + '" fill="' + outerFill + '" opacity="0.2"/>';

  // Main knob body with gradient
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + (r - 1) + '" fill="url(#' + uid + 'g)"/>';

  // Bottom shadow overlay
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + (r - 1) + '" fill="url(#' + uid + 's)"/>';

  // Edge ring
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + (r - 1) + '" fill="none" stroke="' + outerStroke + '" stroke-width="0.5"/>';

  // Indicator tick line
  svg += '<line x1="' + x1.toFixed(1) + '" y1="' + y1.toFixed(1) + '" x2="' + x2.toFixed(1) + '" y2="' + y2.toFixed(1) + '" ';
  svg += 'stroke="' + indicator + '" stroke-width="2.5" stroke-linecap="butt"/>';

  svg += '</svg>';
  return svg;
}

/**
 * Analyze a macro definition's mappings to determine which virtual knob
 * indices are "macro" (control 2+ DSP parameters).
 * Returns: { paramIdx: [{ ctrl, start, mul, div }, ...], ... }
 */
function analyzeMappings(def) {
  var result = {};
  if (!def || !def.mapping) return result;

  def.mapping.forEach(function(m) {
    if (!m.add) return;
    m.add.forEach(function(a) {
      if (!result[a.src]) result[a.src] = [];
      result[a.src].push({ ctrl: m.ctrl, start: m.start || 0, mul: a.mul, div: a.div });
    });
  });
  return result;
}

/**
 * Check if a virtual parameter is a "macro knob" (controls 2+ DSP params).
 */
function isMacroKnob(mappingAnalysis, paramIdx) {
  var entries = mappingAnalysis[paramIdx];
  return entries && entries.length >= 2;
}

/**
/**
 * Apply a response curve to a 0-127 value.
 * Must match the C++ applyCurve() in MacroTranslator.cpp exactly.
 */
function applyCurve(val, curveType) {
  if (!curveType || curveType === 'linear') return val;
  if (val <= 0) return 0;
  if (val >= 127) return 127;

  switch (curveType) {
    case 'log':
      if (val <= 16) return val * 4;
      if (val <= 64) return 64 + Math.round((val - 16) * 36 / 48);
      return 100 + Math.round((val - 64) * 27 / 63);

    case 'exp':
      return Math.round(val * val / 127);

    default:
      return val;
  }
}

/**
 * Compute the real CC output values for a given knob value.
 * Returns an array of { ctrl, name, value, pct } for each mapping target.
 *   ctrl  — CC number
 *   name  — human-readable DSP param name
 *   value — computed output (0-127)
 *   pct   — percentage of 127 (for bar display)
 */
function computeMappingOutputs(def, paramIdx, knobValue) {
  if (!def || !def.mapping) return [];
  var results = [];
  def.mapping.forEach(function(m) {
    if (!m.add) return;
    m.add.forEach(function(a) {
      if (a.src !== paramIdx) return;
      var curved = applyCurve(knobValue, a.curve);
      var val = (m.start || 0) + Math.round(curved * a.mul / a.div);
      val = Math.max(0, Math.min(127, val));
      results.push({
        ctrl: m.ctrl,
        name: resolveCCName(def.machine, m.ctrl),
        value: val,
        pct: Math.round(val / 127 * 100)
      });
    });
  });
  return results;
}

/**
 * Resolve a CC number to the human-readable parameter name for a given machine.
 * Returns the parameter name (e.g. "Freq") or "CC <n>" if not found.
 */
function resolveCCName(machineId, ctrl) {
  var info = getMachineInfo(machineId);
  if (!info || !info.parameters) return 'CC ' + ctrl;
  var param = info.parameters.find(function(p) { return p.ctrl === ctrl; });
  return param ? param.name : 'CC ' + ctrl;
}

// ═══════════════════════════════════════════════════════════════
//  SHARED DATA STORE — both Performer and Designer use this
// ═══════════════════════════════════════════════════════════════

var sharedData = {
  synthDefs: null,
  tracks: [],
  machines: [],
  macroDefs: [],
  soundPresets: [],
  activeTrack: -1,
  loaded: false,
};

var _trackChangeCallbacks = [];

/**
 * Load all data from the device (synthdefs, macrodefs, soundpresets, tracks).
 * Called once at boot; both views read from sharedData.
 *
 * Uses TWO sequential requests to stay within ESP32 HTTP socket limits:
 *   1. GET /api/v2/samples?getconfig=synthdefinitions.json  → synth defs
 *   2. GET /api/v2/macros?action=getall                     → bulk macro data
 *
 * The "getall" endpoint returns { macroDefs, soundPresets, tracks } in a
 * single response, replacing the previous 60+ individual file-fetches.
 */
function loadSharedData() {
  showLoading('Loading tracks & definitions…');
  return fetch('/api/v2/samples?getconfig=synthdefinitions.json').then(function(r) {
    if (!r.ok) throw new Error('HTTP ' + r.status);
    return r.json();
  }).then(function(synthDefs) {
    sharedData.synthDefs = synthDefs;
    sharedData.tracks = synthDefs.tracks || [];
    sharedData.machines = synthDefs.machines || [];

    return fetch('/api/v2/macros?action=getall').then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    });
  }).then(function(macroData) {
    sharedData.macroDefs = macroData.macroDefs || [];
    sharedData.soundPresets = macroData.soundPresets || [];
    // Merge firmware track state into rich synthDefs tracks (don't overwrite!)
    if (macroData.tracks && Array.isArray(macroData.tracks)) {
      macroData.tracks.forEach(function(fwTrack) {
        var existing = sharedData.tracks.find(function(t) { return t.index === fwTrack.index; });
        if (existing) {
          if (fwTrack.machine) existing.currentMachine = fwTrack.machine;
          if (fwTrack.macro) existing.currentMacro = fwTrack.macro;
        }
      });
    }
    sharedData.loaded = true;
    setConnected();
    hideLoading();
    console.log('[Shared] Loaded:', sharedData.tracks.length, 'tracks,',
                sharedData.machines.length, 'machines,',
                sharedData.macroDefs.length, 'macro defs,',
                sharedData.soundPresets.length, 'sound presets');
    return sharedData;
  }).catch(function(err) {
    hideLoading();
    console.error('[Shared] Load error:', err);
    toast('Failed to load data: ' + err.message, 'danger', 4000);
    throw err;
  });
}

/**
 * Reload macro definitions and sound presets from device (after save/delete).
 * Single request via the bulk macroapi endpoint.
 */
function reloadMacroData() {
  return fetch('/api/v2/macros?action=getall').then(function(r) {
    return r.ok ? r.json() : { macroDefs: [], soundPresets: [] };
  }).then(function(macroData) {
    sharedData.macroDefs = macroData.macroDefs || [];
    sharedData.soundPresets = macroData.soundPresets || [];
    // Merge firmware track state into rich synthDefs tracks (don't overwrite!)
    if (macroData.tracks && Array.isArray(macroData.tracks)) {
      macroData.tracks.forEach(function(fwTrack) {
        var existing = sharedData.tracks.find(function(t) { return t.index === fwTrack.index; });
        if (existing) {
          if (fwTrack.machine) existing.currentMachine = fwTrack.machine;
          if (fwTrack.macro) existing.currentMacro = fwTrack.macro;
        }
      });
    }
    return sharedData;
  });
}

/**
 * Tell firmware to reload macros from disk (after saving/deleting definitions).
 * Disables processing, calls RefreshMacros(), re-enables processing.
 */
function reloadFirmwareMacros() {
  return fetch('/api/v2/macros?action=reload', { method: 'POST' })
    .then(function(r) { return r.ok ? r.json() : null; })
    .catch(function(err) {
      console.warn('[Shared] Firmware macro reload failed:', err);
    });
}

/**
 * Register a callback for track changes.
 * Callback receives (trackIndex, track).
 */
function onTrackChange(callback) {
  _trackChangeCallbacks.push(callback);
}

/**
 * Select a track. Updates shared state and notifies all listeners.
 */
function selectSharedTrack(idx) {
  var track = sharedData.tracks.find(function(t) { return t.index === idx; });
  if (!track) return;

  sharedData.activeTrack = idx;

  // Update track strip visuals
  document.querySelectorAll('.track-strip').forEach(function(s) {
    s.classList.toggle('active', parseInt(s.getAttribute('data-track'), 10) === idx);
  });

  // Notify all registered listeners
  _trackChangeCallbacks.forEach(function(cb) {
    try { cb(idx, track); } catch(e) { console.error('Track change callback error:', e); }
  });
}

/**
 * Get machine info by id from shared data.
 */
function getMachineInfo(machineId) {
  return sharedData.machines.find(function(m) { return m.id === machineId; }) || null;
}

/**
 * Get available (non-empty) machines for a track.
 */
function getTrackMachines(track) {
  return (track.machines || []).filter(function(m) {
    return m !== 'nodrum' && m !== 'nosynth' && m !== 'nofx';
  });
}

/**
 * Render the shared track overview strip.
 */
function renderTrackOverview() {
  var container = document.getElementById('track-overview');
  if (!container) return;

  var html = '';
  sharedData.tracks.forEach(function(track) {
    var classes = 'track-strip';
    if (track.index >= 16 && track.index <= 17) classes += ' track-fx';
    if (track.index === 18) classes += ' track-master';
    if (track.index === sharedData.activeTrack) classes += ' active';

    var avail = getTrackMachines(track);
    var defaultMachine = avail.length > 0 ? avail[0] : '—';

    html += '<div class="' + classes + '" data-track="' + track.index + '">';
    html += '<span class="track-num">' + String(track.index + 1).padStart(2, '0') + '</span>';
    html += '<span class="track-name">' + esc(track.name) + '</span>';
    html += '</div>';
  });

  container.innerHTML = html;
}

/**
 * Set up click events on the shared track overview strip.
 */
function setupTrackOverviewEvents() {
  var container = document.getElementById('track-overview');
  if (!container) return;

  container.addEventListener('click', function(e) {
    var strip = e.target.closest('.track-strip');
    if (!strip) return;
    var trackIdx = parseInt(strip.getAttribute('data-track'), 10);
    selectSharedTrack(trackIdx);
  });
}

// ═══════════════════════════════════════════════════════════════
// Shared Knob Group Renderer
// Renders macro definition knob groups identically for both
// the Presets view (interactive) and the Macros view (preview).
//
// Parameters:
//   def         — macro definition object (groups, mapping, machine)
//   paramValues — array of current knob values (by param.idx)
//                 if null, uses param.def defaults
//   options     — { knobSize: 64 }
//
// Returns an HTML string (no container div — caller wraps).
// ═══════════════════════════════════════════════════════════════
function renderKnobGroups(def, paramValues, options) {
  if (!def || !def.groups) return '';
  var opts = options || {};
  var knobSize = opts.knobSize || 64;
  var mappingInfo = analyzeMappings(def);
  var html = '';
  var hasParams = false;
  var visiblePageNum = 0;  // count only groups that have parameters

  def.groups.forEach(function(group, gi) {
    if (!group.parameters || group.parameters.length === 0) return;
    hasParams = true;
    visiblePageNum++;
    var isMixGroup = (group.name === 'Mix');

    html += '<div class="macro-group' + (isMixGroup ? ' is-mix' : '') + '" data-group="' + gi + '">';

    // Group header — Page N / Name
    html += '<div class="macro-group-header">';
    html += '<sl-icon name="chevron-down" class="macro-group-chevron"></sl-icon>';
    html += '<span class="macro-group-page-label">Page ' + visiblePageNum + '</span>';
    html += '<span class="macro-group-name">' + esc(group.name || '') + '</span>';
    html += '</div>';

    // Grid of knobs (4 columns)
    html += '<div class="macro-group-body">';
    group.parameters.forEach(function(param) {
      var value = paramValues && paramValues[param.idx] !== undefined
        ? paramValues[param.idx]
        : (param.def || 0);
      var min = param.min || 0;
      var max = param.max || 127;
      var isMacro = isMacroKnob(mappingInfo, param.idx);
      var knobColor = isMixGroup ? 'mix' : (isMacro ? 'macro' : 'normal');
      var cellClass = 'macro-knob-cell' + (isMacro ? ' is-macro' : '') + (isMixGroup ? ' is-mix' : '');

      // Name ABOVE → Knob → Value BELOW
      html += '<div class="' + cellClass + '" data-param-idx="' + param.idx + '">';
      html += '<span class="macro-knob-label">' + esc(param.name || ('P' + param.idx)) + '</span>';
      html += '<div class="macro-knob" ';
      html += 'data-value="' + value + '" data-min="' + min + '" data-max="' + max + '" data-idx="' + param.idx + '" data-color="' + knobColor + '">';
      html += renderKnobSVG({ value: value, min: min, max: max, color: knobColor, size: knobSize });
      html += '</div>';
      html += '<span class="macro-knob-value' + (isMacro ? ' is-macro' : '') + '">' + value + '</span>';

      // Target panel with range bars, value dots, display hints, badges
      var targets = mappingInfo[param.idx] || [];
      if (targets.length > 0) {
        var outputs = computeMappingOutputs(def, param.idx, value);
        html += '<div class="knob-target-panel' + (isMacro ? ' is-macro' : '') + '" data-knob-idx="' + param.idx + '">';
        if (isMacro) {
          html += '<div class="knob-target-badge">MACRO</div>';
        }
        outputs.forEach(function(o) {
          var mapping = def.mapping.find(function(mm) { return mm.ctrl === o.ctrl; });
          var rangeLow = 0, rangeHigh = 127;
          var sourceCurve = '';
          if (mapping && mapping.add) {
            if (mapping.add.length === 1) {
              rangeLow = mapping.start || 0;
              var a = mapping.add[0];
              rangeHigh = rangeLow + Math.round(127 * (a.mul || 1) / (a.div || 1));
              rangeHigh = Math.min(127, rangeHigh);
              sourceCurve = a.curve || '';
            } else {
              rangeLow = mapping.start || 0;
              rangeHigh = rangeLow;
              mapping.add.forEach(function(a) {
                rangeHigh += Math.round(127 * (a.mul || 1) / (a.div || 1));
                if (a.src === param.idx) sourceCurve = a.curve || '';
              });
              rangeHigh = Math.min(127, rangeHigh);
            }
          }
          var rangeLowPct = rangeLow / 127 * 100;
          var rangeWidthPct = (rangeHigh - rangeLow) / 127 * 100;
          var valuePct = o.value / 127 * 100;

          html += '<div class="knob-target-row" data-ctrl="' + o.ctrl + '">';
          html += '<span class="knob-target-name">' + esc(o.name) + '</span>';
          html += '<span class="knob-target-bar">';
          html += '<span class="knob-target-range" style="left:' + rangeLowPct + '%;width:' + rangeWidthPct + '%"></span>';
          html += '<span class="knob-target-dot" style="left:' + valuePct + '%"></span>';
          html += '</span>';

          // Display hint formatting
          var targetDH = window.TBD && window.TBD.displayHints;
          var targetFmt = String(o.value);
          if (targetDH && def.machine) {
            var targetParamId = def.machine + '_' + esc(o.name).replace(/[- ]/g, '_');
            var targetHint = targetDH.resolveHint(targetParamId, o.name);
            if (targetHint) {
              var physVal = targetDH.rawToDisplay(o.value, 0, 127, targetHint);
              targetFmt = targetDH.formatDisplayValue(physVal, targetHint);
            }
          }
          html += '<span class="knob-target-val">' + targetFmt + '</span>';
          // 14-bit badge
          if (mapping && mapping.bits === 14) {
            html += '<span class="knob-target-14bit">14-bit</span>';
          }
          html += '</div>';

          // Curve badge
          if (sourceCurve && sourceCurve !== 'linear') {
            html += '<span class="curve-badge">' + esc(sourceCurve) + '</span>';
          }
        });
        html += '</div>';
      }

      html += '</div>'; // .macro-knob-cell
    });
    html += '</div>'; // .macro-group-body
    html += '</div>'; // .macro-group
  });

  if (!hasParams) {
    html += '<div class="empty-state" style="padding:2rem;">';
    html += '<sl-icon name="sliders" style="font-size:2rem;"></sl-icon>';
    html += '<h3>No Parameters Defined</h3>';
    html += '<p>Add parameters in the Macro Builder to see knobs here.</p>';
    html += '</div>';
  }

  return html;
}

// ─── Active Tab State ────────────────────────────────────────
var _activeTab = 'presets';
function setActiveTab(tab) { _activeTab = tab; }
function getActiveTab() { return _activeTab; }

window.TBD = window.TBD || {};
window.TBD.shared = {
  API_V2: API_V2,
  API_TIMEOUT_MS: API_TIMEOUT_MS,
  API_MUTATION_TIMEOUT_MS: API_MUTATION_TIMEOUT_MS,
  API_PLUGIN_SWITCH_TIMEOUT_MS: API_PLUGIN_SWITCH_TIMEOUT_MS,
  esc: esc,
  formatBytes: formatBytes,
  apiFetch: apiFetch,
  apiPostJSON: apiPostJSON,
  queuedFetch: queuedFetch,
  queuedPost: queuedPost,
  FetchQueue: FetchQueue,
  paramQueue: paramQueue,
  apiQueue: apiQueue,
  toast: toast,
  iconForVariant: iconForVariant,
  setupThemeToggle: setupThemeToggle,
  applyTheme: applyTheme,
  connectionState: connectionState,
  startConnectionMonitor: startConnectionMonitor,
  setConnected: setConnected,
  setDisconnected: setDisconnected,
  updateConnectionUI: updateConnectionUI,
  confirmDialog: confirmDialog,
  isControlMode: isControlMode,
  setControlMode: setControlMode,
  loadWebAudioControls: loadWebAudioControls,
  showLoading: showLoading,
  hideLoading: hideLoading,
  // SVG knob renderer + mapping analysis
  renderKnobSVG: renderKnobSVG,
  analyzeMappings: analyzeMappings,
  isMacroKnob: isMacroKnob,
  resolveCCName: resolveCCName,
  computeMappingOutputs: computeMappingOutputs,
  renderKnobGroups: renderKnobGroups,
  // Shared data & track management
  data: sharedData,
  loadSharedData: loadSharedData,
  reloadMacroData: reloadMacroData,
  reloadFirmwareMacros: reloadFirmwareMacros,
  onTrackChange: onTrackChange,
  selectTrack: selectSharedTrack,
  getMachineInfo: getMachineInfo,
  getTrackMachines: getTrackMachines,
  renderTrackOverview: renderTrackOverview,
  setupTrackOverviewEvents: setupTrackOverviewEvents,
  setActiveTab: setActiveTab,
  getActiveTab: getActiveTab,
};
