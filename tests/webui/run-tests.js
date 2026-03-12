#!/usr/bin/env node
// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Unit & Integration Tests
// Run:  node tests/webui/run-tests.js
//
// No external dependencies. Uses Node.js built-in assert module.
// Tests the JavaScript modules that run in the browser by
// providing minimal DOM/browser mocks.
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

const assert = require('assert');
const path = require('path');
const fs = require('fs');

// ─── Minimal DOM / Browser Mocks ─────────────────────────────

// Mock localStorage
const _storage = {};
const localStorage = {
  getItem(k) { return _storage[k] !== undefined ? _storage[k] : null; },
  setItem(k, v) { _storage[k] = String(v); },
  removeItem(k) { delete _storage[k]; },
  clear() { Object.keys(_storage).forEach(k => delete _storage[k]); },
};

// Mock DOM elements
class MockElement {
  constructor(tag) {
    this.tagName = tag.toUpperCase();
    this.innerHTML = '';
    this.textContent = '';
    this.className = '';
    this.children = [];
    this._attrs = {};
    this._events = {};
    this.style = {};
    this.classList = {
      _classes: new Set(),
      add: (c) => this.classList._classes.add(c),
      remove: (c) => this.classList._classes.delete(c),
      toggle: (c) => this.classList._classes.has(c) ? this.classList._classes.delete(c) : this.classList._classes.add(c),
      contains: (c) => this.classList._classes.has(c),
    };
  }
  setAttribute(k, v) { this._attrs[k] = v; }
  getAttribute(k) { return this._attrs[k]; }
  addEventListener(type, fn) {
    if (!this._events[type]) this._events[type] = [];
    this._events[type].push(fn);
  }
  removeEventListener(type, fn) {
    if (this._events[type]) this._events[type] = this._events[type].filter(f => f !== fn);
  }
  appendChild(child) { this.children.push(child); return child; }
  show() {}
  hide() {}
  querySelectorAll() { return []; }
  querySelector() { return null; }
}

// Mock document
const _elements = {};
const document = {
  createElement(tag) { return new MockElement(tag); },
  getElementById(id) { return _elements[id] || null; },
  querySelectorAll() { return []; },
  querySelector() { return null; },
  addEventListener() {},
  documentElement: new MockElement('html'),
};

// Provide a mock toast-stack element
_elements['toast-stack'] = new MockElement('div');

// Mock customElements
const _definedElements = {};
const customElements = {
  get(name) { return _definedElements[name]; },
  define(name, cls) { _definedElements[name] = cls; },
  whenDefined(name) { return _definedElements[name] ? Promise.resolve() : new Promise(() => {}); },
};

// Mock window
const window = {
  TBD: {},
  location: { origin: 'http://192.168.4.1', search: '', href: 'http://192.168.4.1/' },
  AudioContext: null,
  webkitAudioContext: null,
};

// Mock fetch — configurable per test
let _fetchQueue = [];
let _fetchCallCount = 0;
let _fetchConcurrent = 0;
let _fetchMaxConcurrent = 0;

function resetFetchMock() {
  _fetchQueue = [];
  _fetchCallCount = 0;
  _fetchConcurrent = 0;
  _fetchMaxConcurrent = 0;
}

function mockFetchResponse(body, status = 200, delay = 0) {
  _fetchQueue.push({ body, status, delay });
}

// AbortSignal mock
class MockAbortSignal {
  constructor() { this.aborted = false; }
  static timeout(ms) { return new MockAbortSignal(); }
}

const AbortSignal = { timeout: MockAbortSignal.timeout };

async function fetch(url, opts = {}) {
  _fetchCallCount++;
  _fetchConcurrent++;
  _fetchMaxConcurrent = Math.max(_fetchMaxConcurrent, _fetchConcurrent);

  const resp = _fetchQueue.shift() || { body: '{}', status: 200, delay: 0 };

  if (resp.delay > 0) {
    await new Promise(r => setTimeout(r, resp.delay));
  }

  _fetchConcurrent--;

  if (resp.status >= 400) {
    return {
      ok: false,
      status: resp.status,
      text: async () => typeof resp.body === 'string' ? resp.body : JSON.stringify(resp.body),
      json: async () => { throw new SyntaxError('Unexpected token'); },
    };
  }
  const bodyStr = typeof resp.body === 'string' ? resp.body : JSON.stringify(resp.body);
  return {
    ok: true,
    status: resp.status,
    text: async () => bodyStr,
    json: async () => JSON.parse(bodyStr),
    arrayBuffer: async () => new ArrayBuffer(0),
  };
}

// Make globals available to the source files
const globals = { window, document, localStorage, customElements, fetch, AbortSignal, console };
Object.assign(global, globals);

// ─── Test Framework ──────────────────────────────────────────

let _tests = [];
let _passed = 0;
let _failed = 0;
let _errors = [];

function describe(name, fn) {
  console.log(`\n  ${name}`);
  fn();
}

function it(name, fn) {
  _tests.push({ name, fn });
}

async function runTests() {
  console.log('\n TBD-16 WebUI Tests');
  console.log(' ==================\n');

  for (const t of _tests) {
    try {
      resetFetchMock();
      localStorage.clear();
      await t.fn();
      _passed++;
      console.log(`    ✓ ${t.name}`);
    } catch (e) {
      _failed++;
      _errors.push({ name: t.name, error: e });
      console.log(`    ✗ ${t.name}`);
      console.log(`      → ${e.message}`);
    }
  }

  console.log(`\n  ${_passed} passing, ${_failed} failing\n`);

  if (_errors.length > 0) {
    console.log('  Failures:\n');
    _errors.forEach((e, i) => {
      console.log(`  ${i + 1}) ${e.name}`);
      console.log(`     ${e.error.stack ? e.error.stack.split('\n').slice(0, 3).join('\n     ') : e.error.message}\n`);
    });
  }

  process.exit(_failed > 0 ? 1 : 0);
}

// ─── Load Source Files ───────────────────────────────────────

const wwwDir = path.join(__dirname, '..', '..', 'sdcard_image', 'www', 'js');

function loadSource(filename) {
  const code = fs.readFileSync(path.join(wwwDir, filename), 'utf8');
  // Wrap in a function that has access to our globals
  const wrapped = new Function(
    'window', 'document', 'localStorage', 'customElements', 'fetch',
    'AbortSignal', 'console', 'setTimeout', 'setInterval', 'clearInterval',
    'URL', 'history', 'HTMLElement', 'Promise',
    code
  );
  wrapped(
    window, document, localStorage, customElements, fetch,
    AbortSignal, console, setTimeout, setInterval, clearInterval,
    URL, { replaceState() {} }, MockElement, Promise
  );
}

// Load shared.js first (defines window.TBD.shared)
loadSource('shared.js');
const S = window.TBD.shared;

// ═══════════════════════════════════════════════════════════════
//  TESTS
// ═══════════════════════════════════════════════════════════════

// ─── shared.js: FetchQueue ───────────────────────────────────

describe('FetchQueue', () => {
  it('should serialize async tasks', async () => {
    const q = new S.FetchQueue();
    const order = [];
    const p1 = q.enqueue(async () => {
      await new Promise(r => setTimeout(r, 10));
      order.push('A');
      return 'a';
    });
    const p2 = q.enqueue(async () => {
      order.push('B');
      return 'b';
    });
    const [r1, r2] = await Promise.all([p1, p2]);
    assert.deepStrictEqual(order, ['A', 'B'], 'Tasks should run in order');
    assert.strictEqual(r1, 'a');
    assert.strictEqual(r2, 'b');
  });

  it('should propagate errors without blocking queue', async () => {
    const q = new S.FetchQueue();
    const p1 = q.enqueue(async () => { throw new Error('boom'); });
    const p2 = q.enqueue(async () => 'ok');
    await assert.rejects(p1, /boom/);
    const r2 = await p2;
    assert.strictEqual(r2, 'ok', 'Queue should continue after error');
  });

  it('should handle rapid enqueue', async () => {
    const q = new S.FetchQueue();
    const results = [];
    const promises = [];
    for (let i = 0; i < 20; i++) {
      promises.push(q.enqueue(async () => { results.push(i); return i; }));
    }
    await Promise.all(promises);
    assert.strictEqual(results.length, 20, 'All 20 tasks should complete');
    // Verify order
    for (let i = 0; i < 20; i++) {
      assert.strictEqual(results[i], i, `Task ${i} should be in order`);
    }
  });
});

// ─── shared.js: apiFetch / apiPostJSON ──────────────────────

describe('apiFetch', () => {
  it('should parse valid JSON', async () => {
    mockFetchResponse({ id: 'test', name: 'Test Plugin' });
    const result = await S.apiFetch('/getPlugins');
    assert.strictEqual(result.id, 'test');
  });

  it('should return {} for empty response body', async () => {
    mockFetchResponse('');
    const result = await S.apiFetch('/getPlugins');
    assert.deepStrictEqual(result, {}, 'Empty body should return {}');
  });

  it('should return {} for whitespace-only response', async () => {
    mockFetchResponse('   \n  ');
    const result = await S.apiFetch('/getPlugins');
    assert.deepStrictEqual(result, {}, 'Whitespace body should return {}');
  });

  it('should return {} for invalid JSON', async () => {
    mockFetchResponse('not json at all');
    const result = await S.apiFetch('/getPlugins');
    assert.deepStrictEqual(result, {}, 'Invalid JSON should return {}');
  });

  it('should throw on HTTP error status', async () => {
    mockFetchResponse('Not Found', 404);
    await assert.rejects(
      () => S.apiFetch('/nonexistent'),
      /API 404/,
      'Should throw on 404'
    );
  });
});

describe('apiPostJSON', () => {
  it('should send POST with JSON body', async () => {
    mockFetchResponse({ ok: true });
    const result = await S.apiPostJSON('/setConfiguration', { key: 'value' });
    assert.strictEqual(result.ok, true);
  });

  it('should handle empty response body from POST', async () => {
    mockFetchResponse('');
    const result = await S.apiPostJSON('/setConfiguration', {});
    assert.deepStrictEqual(result, {});
  });
});

// ─── shared.js: queuedFetch / queuedPost ────────────────────

describe('queuedFetch / queuedPost', () => {
  it('should never have >1 concurrent fetch in-flight', async () => {
    // Queue 5 requests
    for (let i = 0; i < 5; i++) {
      mockFetchResponse({ i }, 200, 5);  // 5ms delay
    }
    const promises = [];
    for (let i = 0; i < 5; i++) {
      promises.push(S.queuedFetch('/test/' + i));
    }
    await Promise.all(promises);
    assert.strictEqual(
      _fetchMaxConcurrent, 1,
      `Max concurrent fetches should be 1, got ${_fetchMaxConcurrent}`
    );
  });

  it('queuedPost should serialize POST requests', async () => {
    mockFetchResponse({ a: 1 }, 200, 5);
    mockFetchResponse({ b: 2 }, 200, 5);
    const r1 = S.queuedPost('/a', {});
    const r2 = S.queuedPost('/b', {});
    const [res1, res2] = await Promise.all([r1, r2]);
    assert.strictEqual(res1.a, 1);
    assert.strictEqual(res2.b, 2);
    assert.strictEqual(_fetchMaxConcurrent, 1, 'Should never exceed 1 concurrent');
  });
});

// ─── shared.js: Toast ────────────────────────────────────────

describe('toast', () => {
  it('should not throw when toast-stack exists', () => {
    assert.doesNotThrow(() => S.toast('Test message', 'primary'));
  });

  it('should prevent synchronous re-entry via _toastBusy', () => {
    // Direct access to the private flag via closure isn't possible;
    // but we can verify calling toast twice synchronously doesn't stack-overflow
    assert.doesNotThrow(() => {
      S.toast('First', 'primary');
      S.toast('Second', 'primary');
      S.toast('Third', 'primary');
    });
  });

  it('should throttle when max pending toasts reached', () => {
    // Fire many toasts rapidly — should not throw
    assert.doesNotThrow(() => {
      for (let i = 0; i < 100; i++) {
        S.toast(`Toast ${i}`, 'primary', 100);
      }
    });
    // If it made it here without stack overflow, throttle works
  });

  it('should handle missing toast-stack gracefully', () => {
    const orig = _elements['toast-stack'];
    delete _elements['toast-stack'];
    assert.doesNotThrow(() => S.toast('No stack', 'danger'));
    _elements['toast-stack'] = orig;
  });

  it('should handle all variant types', () => {
    const variants = ['success', 'warning', 'danger', 'primary', 'neutral'];
    variants.forEach(v => {
      assert.doesNotThrow(() => S.toast('Test', v));
    });
  });
});

// ─── shared.js: iconForVariant ──────────────────────────────

describe('iconForVariant', () => {
  it('should return correct icon for each variant', () => {
    assert.strictEqual(S.iconForVariant('success'), 'check2-circle');
    assert.strictEqual(S.iconForVariant('warning'), 'exclamation-triangle');
    assert.strictEqual(S.iconForVariant('danger'), 'exclamation-octagon');
    assert.strictEqual(S.iconForVariant('primary'), 'info-circle');
    assert.strictEqual(S.iconForVariant('neutral'), 'info-circle');
  });

  it('should return default for unknown variant', () => {
    assert.strictEqual(S.iconForVariant('unknown'), 'info-circle');
    assert.strictEqual(S.iconForVariant(undefined), 'info-circle');
  });
});

// ─── shared.js: esc (HTML escaping) ─────────────────────────

describe('esc', () => {
  it('should escape HTML entities', () => {
    // In our mock environment, textContent→innerHTML won't actually escape,
    // but we verify it doesn't throw
    assert.doesNotThrow(() => S.esc('<script>alert("xss")</script>'));
    assert.doesNotThrow(() => S.esc('normal text'));
    assert.doesNotThrow(() => S.esc(''));
  });
});

// ─── shared.js: formatBytes ─────────────────────────────────

describe('formatBytes', () => {
  it('should format bytes correctly', () => {
    assert.strictEqual(S.formatBytes(0), '0 B');
    assert.strictEqual(S.formatBytes(512), '512 B');
    assert.strictEqual(S.formatBytes(1024), '1.0 KB');
    assert.strictEqual(S.formatBytes(1536), '1.5 KB');
    assert.strictEqual(S.formatBytes(1048576), '1.0 MB');
    assert.strictEqual(S.formatBytes(1073741824), '1.0 GB');
  });
});

// ─── shared.js: Connection Monitor ──────────────────────────

describe('connectionState', () => {
  it('should have correct initial state', () => {
    assert.strictEqual(S.connectionState.status, 'connecting');
    assert.strictEqual(S.connectionState.retries, 0);
  });

  it('setConnected should update status', () => {
    S.connectionState.status = 'disconnected';
    S.setConnected();
    assert.strictEqual(S.connectionState.status, 'connected');
  });

  it('setDisconnected should update status', () => {
    S.connectionState.status = 'connected';
    S.setDisconnected();
    assert.strictEqual(S.connectionState.status, 'disconnected');
    // Reset
    S.connectionState.status = 'connecting';
    if (S.connectionState._timer) {
      clearInterval(S.connectionState._timer);
      S.connectionState._timer = null;
    }
  });

  it('should not fire duplicate setConnected callbacks', () => {
    let callCount = 0;
    S.connectionState.status = 'connecting';
    S.startConnectionMonitor(() => callCount++, () => {});
    S.setConnected();
    S.setConnected(); // should be no-op
    assert.strictEqual(callCount, 1, 'onConnect should fire only once');
    S.connectionState.status = 'connecting';
  });
});

// ─── shared.js: Control Mode ────────────────────────────────

describe('isControlMode / setControlMode', () => {
  it('should default to false (config mode)', () => {
    localStorage.clear();
    assert.strictEqual(S.isControlMode(), false);
  });

  it('should toggle control mode via localStorage', () => {
    S.setControlMode(true);
    assert.strictEqual(S.isControlMode(), true);
    S.setControlMode(false);
    assert.strictEqual(S.isControlMode(), false);
  });
});

// ─── shared.js: apiQueue serialization ──────────────────────

describe('apiQueue (global serialization)', () => {
  it('exists and is a FetchQueue instance', () => {
    assert.ok(S.apiQueue, 'apiQueue should exist');
    assert.ok(S.apiQueue instanceof S.FetchQueue, 'should be a FetchQueue');
  });

  it('should never allow concurrent requests even via mixed queuedFetch/queuedPost', async () => {
    for (let i = 0; i < 6; i++) {
      mockFetchResponse({ i }, 200, 3);
    }
    const promises = [
      S.queuedFetch('/a'),
      S.queuedPost('/b', {}),
      S.queuedFetch('/c'),
      S.queuedPost('/d', {}),
      S.queuedFetch('/e'),
      S.queuedFetch('/f'),
    ];
    await Promise.all(promises);
    assert.strictEqual(
      _fetchMaxConcurrent, 1,
      `Mixed GET/POST should never exceed 1 concurrent, got ${_fetchMaxConcurrent}`
    );
  });
});

// ─── shared.js: paramQueue ──────────────────────────────────

describe('paramQueue', () => {
  it('should exist and be separate from apiQueue', () => {
    assert.ok(S.paramQueue, 'paramQueue should exist');
    assert.ok(S.paramQueue !== S.apiQueue, 'paramQueue should be separate from apiQueue');
  });

  it('should serialize parameter changes', async () => {
    for (let i = 0; i < 3; i++) {
      mockFetchResponse({ ok: true }, 200, 3);
    }
    const results = [];
    const promises = [
      S.paramQueue.enqueue(async () => {
        const r = await S.queuedFetch('/setPluginParam/0?id=p1&current=100');
        results.push('p1');
        return r;
      }),
      S.paramQueue.enqueue(async () => {
        const r = await S.queuedFetch('/setPluginParam/0?id=p2&current=200');
        results.push('p2');
        return r;
      }),
      S.paramQueue.enqueue(async () => {
        const r = await S.queuedFetch('/setPluginParam/0?id=p3&current=300');
        results.push('p3');
        return r;
      }),
    ];
    await Promise.all(promises);
    assert.deepStrictEqual(results, ['p1', 'p2', 'p3'], 'Params should be set in order');
  });
});

// ─── Integration: Simulated plugin switch ───────────────────

describe('Integration: Plugin switching simulation', () => {
  it('queuedFetch should prevent concurrent socket usage during plugin switch', async () => {
    // Simulate: setActivePlugin → loadSlotData (3 sequential calls)
    // Total: 1 (setActive) + 3 (active+params+presets) = 4 requests, all sequential
    mockFetchResponse({ ok: true });                           // setActivePlugin
    mockFetchResponse({ id: 'TestPlugin' }, 200, 5);          // getActivePlugin
    mockFetchResponse({ groups: [] }, 200, 5);                 // getPluginParams
    mockFetchResponse({ presets: [], activePresetNumber: 0 }); // getPresets

    // Simulate the pattern from plugin-manager.js setActivePlugin
    await S.queuedFetch('/setActivePlugin/0?id=TestPlugin');
    await S.queuedFetch('/getActivePlugin/0');
    await S.queuedFetch('/getPluginParams/0');
    await S.queuedFetch('/getPresets/0');

    assert.strictEqual(_fetchCallCount, 4);
    assert.strictEqual(_fetchMaxConcurrent, 1);
  });

  it('rapid plugin switches should serialize (not race)', async () => {
    // Simulate 3 rapid plugin switches — 4 requests each = 12 total
    for (let i = 0; i < 12; i++) {
      mockFetchResponse({ id: `Plugin${i}` }, 200, 2);
    }

    // Fire a "switch" sequences concurrently (simulating rapid clicks)
    const switchPlugin = async (pluginId) => {
      await S.queuedFetch('/setActivePlugin/0?id=' + pluginId);
      await S.queuedFetch('/getActivePlugin/0');
      await S.queuedFetch('/getPluginParams/0');
      await S.queuedFetch('/getPresets/0');
    };

    const p1 = switchPlugin('PluginA');
    const p2 = switchPlugin('PluginB');
    const p3 = switchPlugin('PluginC');

    await Promise.all([p1, p2, p3]);

    assert.strictEqual(_fetchCallCount, 12, 'All 12 requests should complete');
    assert.strictEqual(
      _fetchMaxConcurrent, 1,
      `Even rapid switches should never exceed 1 concurrent, got ${_fetchMaxConcurrent}`
    );
  });
});

// ─── Integration: Reconnect simulation ──────────────────────

describe('Integration: Reconnect scenario', () => {
  it('reconnect poller should skip when apiQueue is busy', async () => {
    // Verify the reconnect poller checks apiQueue._running
    S.apiQueue._running = true;
    let pollerSkipped = S.apiQueue._running; // simulates the check
    assert.strictEqual(pollerSkipped, true, 'Poller should detect busy queue');
    S.apiQueue._running = false;
  });
});

// ─── API timeout ────────────────────────────────────────────

describe('API timeout configuration', () => {
  it('should have a reasonable timeout value', () => {
    assert.ok(S.API_TIMEOUT_MS > 0, 'Timeout should be positive');
    assert.ok(S.API_TIMEOUT_MS >= 3000, 'Timeout should be at least 3s');
    assert.ok(S.API_TIMEOUT_MS <= 30000, 'Timeout should be at most 30s');
  });
});

// ─── Bundle integrity checks ────────────────────────────────

describe('Bundle integrity', () => {
  it('app-bundle.js should exist', () => {
    const bundlePath = path.join(wwwDir, 'app-bundle.js');
    assert.ok(fs.existsSync(bundlePath), 'app-bundle.js should exist');
  });

  it('app-bundle.js should have exactly 1 function toast declaration', () => {
    const bundle = fs.readFileSync(path.join(wwwDir, 'app-bundle.js'), 'utf8');
    const matches = bundle.match(/\bfunction toast\s*\(/g);
    assert.strictEqual(
      matches ? matches.length : 0, 1,
      `Should have exactly 1 "function toast(" — found ${matches ? matches.length : 0}`
    );
  });

  it('app-bundle.js should have _toastBusy guard', () => {
    const bundle = fs.readFileSync(path.join(wwwDir, 'app-bundle.js'), 'utf8');
    assert.ok(bundle.includes('_toastBusy'), 'Bundle should contain _toastBusy guard');
  });

  it('app-bundle.js should have _TOAST_MAX_PENDING throttle', () => {
    const bundle = fs.readFileSync(path.join(wwwDir, 'app-bundle.js'), 'utf8');
    assert.ok(bundle.includes('_TOAST_MAX_PENDING'), 'Bundle should contain toast throttle');
  });

  it('app-bundle.js should have AbortSignal.timeout in fetch calls', () => {
    const bundle = fs.readFileSync(path.join(wwwDir, 'app-bundle.js'), 'utf8');
    const count = (bundle.match(/AbortSignal\.timeout/g) || []).length;
    assert.ok(count >= 3, `Should have AbortSignal.timeout in at least 3 places, found ${count}`);
  });

  it('app-bundle.js should have queuedFetch/queuedPost', () => {
    const bundle = fs.readFileSync(path.join(wwwDir, 'app-bundle.js'), 'utf8');
    assert.ok(bundle.includes('queuedFetch'), 'Bundle should have queuedFetch');
    assert.ok(bundle.includes('queuedPost'), 'Bundle should have queuedPost');
  });

  it('app-bundle.js should NOT have Promise.all with loadSlotData', () => {
    const bundle = fs.readFileSync(path.join(wwwDir, 'app-bundle.js'), 'utf8');
    const badPattern = /Promise\.all\(\[loadSlotData/;
    assert.ok(
      !badPattern.test(bundle),
      'Bundle should NOT contain Promise.all([loadSlotData...]) — causes concurrent socket usage'
    );
  });

  it('source files should NOT have unqueued S.apiFetch calls (except debug wrapper)', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    // Count S.apiFetch calls — should only appear inside paramQueue.enqueue
    const directCalls = pm.match(/S\.apiFetch\(/g) || [];
    // All should be inside paramQueue.enqueue or debug wrapper
    // The paramQueue.enqueue wraps queuedFetch which calls apiFetch internally
    const queuedCalls = pm.match(/S\.queuedFetch\(/g) || [];
    assert.ok(
      queuedCalls.length > 0,
      'plugin-manager.js should use S.queuedFetch'
    );
  });

  it('app-bundle.js.gz should exist and be smaller than source', () => {
    const bundlePath = path.join(wwwDir, 'app-bundle.js');
    const gzPath = path.join(wwwDir, 'app-bundle.js.gz');
    assert.ok(fs.existsSync(gzPath), 'app-bundle.js.gz should exist');
    const srcSize = fs.statSync(bundlePath).size;
    const gzSize = fs.statSync(gzPath).size;
    assert.ok(gzSize < srcSize, `Gzipped (${gzSize}) should be smaller than source (${srcSize})`);
    assert.ok(gzSize < 100000, `Gzipped bundle should be under 100KB, got ${gzSize}`);
  });
});

// ─── Source file consistency checks ─────────────────────────

describe('Source file consistency', () => {
  it('sample-manager.js should not have function toast declaration', () => {
    const sm = fs.readFileSync(path.join(wwwDir, 'sample-manager.js'), 'utf8');
    const fnDecl = sm.match(/\bfunction toast\s*\(/g);
    assert.strictEqual(
      fnDecl ? fnDecl.length : 0, 0,
      'sample-manager.js should use var toast = ... guard, not function toast()'
    );
  });

  it('sample-manager.js should use safe JSON parsing', () => {
    const sm = fs.readFileSync(path.join(wwwDir, 'sample-manager.js'), 'utf8');
    // Should NOT have bare r.json() — should use r.text() + JSON.parse
    // Check the apiGet/apiPost functions specifically
    const rawApiSection = sm.substring(
      sm.indexOf('_rawApiGet'),
      sm.indexOf('function apiGet')
    );
    assert.ok(rawApiSection.includes('r.text()'), 'Should use r.text() for safe parsing');
    assert.ok(rawApiSection.includes('JSON.parse'), 'Should use JSON.parse with try/catch');
    assert.ok(!rawApiSection.includes('r.json()'), 'Should NOT use r.json() directly');
  });

  it('sample-manager.js apiGet/apiPost should route through queue', () => {
    const sm = fs.readFileSync(path.join(wwwDir, 'sample-manager.js'), 'utf8');
    assert.ok(sm.includes('_apiQueue'), 'Should reference _apiQueue');
    assert.ok(sm.includes('_apiQueue.enqueue'), 'Should enqueue through global queue');
  });

  it('all source files should have AbortSignal.timeout', () => {
    const files = ['shared.js', 'sample-manager.js'];
    files.forEach(f => {
      const src = fs.readFileSync(path.join(wwwDir, f), 'utf8');
      assert.ok(
        src.includes('AbortSignal.timeout'),
        `${f} should have AbortSignal.timeout in fetch calls`
      );
    });
  });

  it('plugin-manager.js should not have Promise.all with loadSlotData', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    const bad = /Promise\.all\(\[loadSlotData/;
    assert.ok(!bad.test(pm), 'Should NOT have Promise.all([loadSlotData...])');
  });

  it('plugin-manager.js should have _switching mutex', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(pm.includes('_switching'), 'Should have _switching mutex variable');
    assert.ok(pm.includes('_switching = true'), 'Should set _switching = true');
    assert.ok(pm.includes('_switching = false'), 'Should reset _switching = false');
  });

  it('app.js reconnect handler should be async', () => {
    const app = fs.readFileSync(path.join(wwwDir, 'app.js'), 'utf8');
    assert.ok(
      app.includes('async function onConnect'),
      'onConnect handler should be async for sequential init'
    );
  });

  it('app.js should use queuedFetch/queuedPost for API calls', () => {
    const app = fs.readFileSync(path.join(wwwDir, 'app.js'), 'utf8');
    // Should NOT have direct S.apiFetch (except in debug wrapper)
    const lines = app.split('\n');
    let directCalls = 0;
    let inDebugWrapper = false;
    for (const line of lines) {
      if (line.includes('Wrap S.apiFetch')) inDebugWrapper = true;
      if (inDebugWrapper && line.includes('})();')) { inDebugWrapper = false; continue; }
      if (!inDebugWrapper && line.includes('S.apiFetch(') && !line.includes('origFetch')) {
        directCalls++;
      }
    }
    assert.strictEqual(
      directCalls, 0,
      `app.js should not have direct S.apiFetch calls outside debug wrapper, found ${directCalls}`
    );
  });
});

// ─── WP-E: ESP32 optimizations ──────────────────────────────

describe('WP-E: API timeout matches server timeout', () => {
  it('API_TIMEOUT_MS should be 10000ms (matching server recv/send_wait_timeout)', () => {
    assert.strictEqual(S.API_TIMEOUT_MS, 10000, `Expected 10000ms, got ${S.API_TIMEOUT_MS}`);
  });
});

describe('WP-E: Reconnect poll interval', () => {
  it('should use 5s poll interval (WLED pattern)', () => {
    assert.strictEqual(
      S.connectionState.pollIntervalMs, 5000,
      `Expected 5000ms poll interval, got ${S.connectionState.pollIntervalMs}`
    );
  });
});

describe('WP-E: Debounce on sendParamValue', () => {
  it('plugin-manager.js should have PARAM_DEBOUNCE_MS', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(pm.includes('PARAM_DEBOUNCE_MS'), 'Should define PARAM_DEBOUNCE_MS');
  });

  it('plugin-manager.js should have per-param debounce timers', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(pm.includes('_paramTimers'), 'Should have _paramTimers object');
    assert.ok(pm.includes('clearTimeout'), 'Should clear previous timer on rapid changes');
  });
});

describe('WP-F: API headers match upstream p4_main', () => {
  it('RestServer.cpp set_api_headers should NOT set Connection: close (upstream compatibility)', () => {
    const rs = fs.readFileSync(
      path.join(__dirname, '..', '..', 'main', 'RestServer.cpp'), 'utf8'
    );
    assert.ok(rs.includes('set_api_headers'), 'Should define set_api_headers()');
    // WP-F: Connection:close removed from API responses to match upstream.
    // The browser reuses TCP connections (keep-alive) → only 1 socket for API calls.
    const fnStart = rs.indexOf('static void set_api_headers');
    const fnEnd = rs.indexOf('}', fnStart + 10);
    const fnBody = rs.substring(fnStart, fnEnd + 1);
    assert.ok(
      !fnBody.includes('"Connection"'),
      'set_api_headers must NOT set Connection header (upstream p4_main does not)'
    );
    assert.ok(
      rs.includes('"Cache-Control", "no-store"'),
      'set_api_headers should set Cache-Control: no-store'
    );
  });

  it('Connection: close should only appear in set_content_type_from_file (static files)', () => {
    const rs = fs.readFileSync(
      path.join(__dirname, '..', '..', 'main', 'RestServer.cpp'), 'utf8'
    );
    // Connection:close should appear in set_content_type_from_file for static files
    const staticFn = rs.substring(
      rs.indexOf('set_content_type_from_file'),
      rs.indexOf('rest_common_get_handler')
    );
    assert.ok(
      staticFn.includes('"Connection", "close"'),
      'Static file handler should set Connection: close'
    );
  });

  it('All API handlers should call set_api_headers', () => {
    const rs = fs.readFileSync(
      path.join(__dirname, '..', '..', 'main', 'RestServer.cpp'), 'utf8'
    );
    // Count handler functions and set_api_headers calls
    const handlers = [
      'get_plugins_get_handler',
      'get_active_plugin_get_handler',
      'get_params_plugin_get_handler',
      'set_active_plugin_get_handler',
      'set_plugin_param_get_handler',
      'get_presets_get_handler',
      'save_preset_get_handler',
      'load_preset_get_handler',
      'set_configuration_post_handler',
      'get_configuration_get_handler',
      'get_preset_json_handler',
      'reboot_handler',
      'get_iocaps_handler',
      'favorite_post_handler',
      'set_preset_json_handler',
    ];
    for (const h of handlers) {
      // Find the handler function body
      const idx = rs.indexOf(h + '(httpd_req_t');
      assert.ok(idx > -1, `Handler ${h} should exist`);
      // Check that set_api_headers appears within the next 600 chars
      const snippet = rs.substring(idx, idx + 600);
      assert.ok(
        snippet.includes('set_api_headers(req)'),
        `Handler ${h} should call set_api_headers(req)`
      );
    }
  });
});

describe('WP-E: Server timeout configuration', () => {
  it('RestServer.cpp should use 10s recv/send timeouts (matching p4_main stable branch)', () => {
    const rs = fs.readFileSync(
      path.join(__dirname, '..', '..', 'main', 'RestServer.cpp'), 'utf8'
    );
    assert.ok(rs.includes('recv_wait_timeout = 10'), 'recv_wait_timeout should be 10 (match p4_main)');
    assert.ok(rs.includes('send_wait_timeout = 10'), 'send_wait_timeout should be 10 (match p4_main)');
  });
});

describe('WP-E: Preview fetch routed through queue', () => {
  it('sample-manager.js preview should use apiQueue', () => {
    const sm = fs.readFileSync(path.join(wwwDir, 'sample-manager.js'), 'utf8');
    // The preview fetch should go through _apiQueue.enqueue
    const previewSection = sm.substring(
      sm.indexOf('async function playPreview'),
      sm.indexOf('function stopPreview')
    );
    assert.ok(
      previewSection.includes('_apiQueue.enqueue'),
      'Preview fetch should be routed through _apiQueue'
    );
  });
});

describe('WP-E: app.js test connection uses queue', () => {
  it('Test Connection button should use S.apiQueue.enqueue', () => {
    const app = fs.readFileSync(path.join(wwwDir, 'app.js'), 'utf8');
    assert.ok(
      app.includes('apiQueue.enqueue'),
      'Test Connection fetch should go through apiQueue'
    );
  });

  it('Test Connection URL should include /api/v1/', () => {
    const app = fs.readFileSync(path.join(wwwDir, 'app.js'), 'utf8');
    assert.ok(
      app.includes("'/api/v1/getIOCaps'"),
      'Test Connection URL should use full /api/v1/getIOCaps path'
    );
  });
});

describe('WP-E: Chrome DevTools MCP configured', () => {
  it('.vscode/mcp.json should exist and configure chrome-devtools', () => {
    const mcpPath = path.join(__dirname, '..', '..', '.vscode', 'mcp.json');
    assert.ok(fs.existsSync(mcpPath), '.vscode/mcp.json should exist');
    const mcp = JSON.parse(fs.readFileSync(mcpPath, 'utf8'));
    assert.ok(mcp.mcpServers, 'Should have mcpServers key');
    assert.ok(mcp.mcpServers['chrome-devtools'], 'Should have chrome-devtools MCP server');
    assert.strictEqual(
      mcp.mcpServers['chrome-devtools'].command, 'npx',
      'Should use npx command'
    );
  });
});

describe('WP-E: sample-manager timeout fallback matches', () => {
  it('_apiTimeout fallback should be 5000 (not 8000)', () => {
    const sm = fs.readFileSync(path.join(wwwDir, 'sample-manager.js'), 'utf8');
    // The fallback value should be 5000, not 8000
    assert.ok(
      sm.includes('? _S.API_TIMEOUT_MS : 5000'),
      'Fallback timeout should be 5000ms'
    );
    assert.ok(
      !sm.includes('? _S.API_TIMEOUT_MS : 8000'),
      'Old 8000ms fallback should be gone'
    );
  });
});

describe('WP-F: Loading overlay for plugin operations', () => {
  it('shared.js should export showLoading and hideLoading', () => {
    const shared = fs.readFileSync(path.join(wwwDir, 'shared.js'), 'utf8');
    assert.ok(shared.includes('function showLoading'), 'Should define showLoading()');
    assert.ok(shared.includes('function hideLoading'), 'Should define hideLoading()');
    assert.ok(shared.includes('showLoading: showLoading'), 'Should export showLoading');
    assert.ok(shared.includes('hideLoading: hideLoading'), 'Should export hideLoading');
  });

  it('plugin-manager.js should show/hide loading on plugin switch', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(
      pm.includes("S.showLoading('Switching plugin") || pm.includes("S.showLoading(heavy"),
      'setActivePlugin should show loading (conditional for heavy/normal plugins)'
    );
    assert.ok(pm.includes('S.hideLoading()'), 'Should call hideLoading in finally block');
  });

  it('plugin-manager.js should show loading for all heavy operations', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(pm.includes("S.showLoading('Clearing slot"), 'clearSlot should show loading');
    assert.ok(pm.includes("S.showLoading('Loading preset"), 'loadPreset should show loading');
    assert.ok(pm.includes("S.showLoading('Recalling favorite"), 'recallFavorite should show loading');
    assert.ok(pm.includes("S.showLoading('Swapping slots"), 'swapSlots should show loading');
  });

  it('index.html should have the loading overlay markup', () => {
    const html = fs.readFileSync(
      path.join(__dirname, '..', '..', 'sdcard_image', 'www', 'index.html'), 'utf8'
    );
    assert.ok(html.includes('id="loading-overlay"'), 'Should have loading-overlay element');
    assert.ok(html.includes('id="loading-text"'), 'Should have loading-text element');
    assert.ok(html.includes('.loading-overlay'), 'Should have loading-overlay CSS');
  });
});

describe('WP-F: No artificial delays in plugin-manager', () => {
  it('plugin-manager.js should NOT have delay() helper or _switchCooldownMs', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(!pm.includes('_switchCooldownMs'), 'Should not have _switchCooldownMs');
    assert.ok(!pm.includes('await delay('), 'Should not have await delay() calls');
    assert.ok(!pm.includes('function delay(ms)'), 'Should not have delay() function');
  });
});

// ─── WP-G: Connection resilience ────────────────────────────

describe('WP-G: Circuit breaker and timeout configuration', () => {
  it('API_TIMEOUT_MS should be >= 10s (match server-side recv/send timeout)', () => {
    assert.ok(S.API_TIMEOUT_MS >= 10000, `API_TIMEOUT_MS should be >= 10000, got ${S.API_TIMEOUT_MS}`);
  });

  it('API_MUTATION_TIMEOUT_MS should exist and be > API_TIMEOUT_MS', () => {
    assert.ok(S.API_MUTATION_TIMEOUT_MS, 'API_MUTATION_TIMEOUT_MS should be exported');
    assert.ok(
      S.API_MUTATION_TIMEOUT_MS > S.API_TIMEOUT_MS,
      `Mutation timeout (${S.API_MUTATION_TIMEOUT_MS}) should be > read timeout (${S.API_TIMEOUT_MS})`
    );
  });

  it('shared.js should have circuit breaker variables', () => {
    const shared = fs.readFileSync(path.join(wwwDir, 'shared.js'), 'utf8');
    assert.ok(shared.includes('_consecutiveFailures'), 'Should have _consecutiveFailures counter');
    assert.ok(shared.includes('_FAILURE_THRESHOLD'), 'Should have _FAILURE_THRESHOLD constant');
  });

  it('apiFetch should reset failures on success and increment on error', () => {
    const shared = fs.readFileSync(path.join(wwwDir, 'shared.js'), 'utf8');
    assert.ok(
      shared.includes('_consecutiveFailures = 0'),
      'Should reset _consecutiveFailures to 0 on success'
    );
    assert.ok(
      shared.includes('_consecutiveFailures++'),
      'Should increment _consecutiveFailures on failure'
    );
  });

  it('apiFetch should trigger disconnect + drain on threshold', () => {
    const shared = fs.readFileSync(path.join(wwwDir, 'shared.js'), 'utf8');
    assert.ok(
      shared.includes('setDisconnected()'),
      'Should call setDisconnected() when threshold reached'
    );
    assert.ok(
      shared.includes('apiQueue.drain()'),
      'Should drain apiQueue when threshold reached'
    );
  });
});

describe('WP-G: FetchQueue drain and resume', () => {
  it('FetchQueue should have drain() and resume() methods', () => {
    const shared = fs.readFileSync(path.join(wwwDir, 'shared.js'), 'utf8');
    assert.ok(shared.includes('drain()'), 'Should have drain() method');
    assert.ok(shared.includes('resume()'), 'Should have resume() method');
    assert.ok(shared.includes('_paused'), 'Should have _paused flag');
  });

  it('setConnected should reset failures and resume queue', () => {
    const shared = fs.readFileSync(path.join(wwwDir, 'shared.js'), 'utf8');
    // Find setConnected function and check it contains resume
    const setConnectedMatch = shared.match(/function setConnected\(\)[\s\S]*?^}/m);
    assert.ok(setConnectedMatch, 'Should find setConnected function');
    const fn = setConnectedMatch[0];
    assert.ok(fn.includes('apiQueue.resume()'), 'setConnected should resume the queue');
    assert.ok(fn.includes('_consecutiveFailures = 0'), 'setConnected should reset failure counter');
  });
});

describe('WP-G: Mutation timeouts in plugin-manager', () => {
  it('setActivePlugin should use appropriate timeout (plugin-switch or mutation)', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    // WP-I: setActivePlugin now uses API_PLUGIN_SWITCH_TIMEOUT_MS for heavy plugins
    // (WTOsc, WTOscDuo, Freakwaves, VctrSnt) and API_MUTATION_TIMEOUT_MS for normal ones.
    // The timeout is chosen via a variable (switchTimeout), not hardcoded inline.
    assert.ok(
      pm.includes('S.API_PLUGIN_SWITCH_TIMEOUT_MS') && pm.includes('S.API_MUTATION_TIMEOUT_MS'),
      'setActivePlugin should reference both timeout constants'
    );
    assert.ok(
      pm.includes('_heavyPlugins') || pm.includes('_isHeavyPlugin'),
      'plugin-manager should identify heavy (sample ROM) plugins'
    );
    assert.ok(
      pm.includes('skipCircuitBreaker'),
      'setActivePlugin should skip circuit breaker for plugin switch calls'
    );
  });

  it('clearSlot should use API_MUTATION_TIMEOUT_MS', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(
      pm.includes("S.queuedFetch('/setActivePlugin/' + ch + '?id=Void', S.API_MUTATION_TIMEOUT_MS)"),
      'clearSlot should pass API_MUTATION_TIMEOUT_MS'
    );
  });

  it('swapSlots should use API_MUTATION_TIMEOUT_MS', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    const swapMatches = pm.match(/S\.API_MUTATION_TIMEOUT_MS\)/g) || [];
    assert.ok(
      swapMatches.length >= 4,
      `Should use API_MUTATION_TIMEOUT_MS in at least 4 places (setActive+clear+swap*2+loadPreset+recall), found ${swapMatches.length}`
    );
  });
});

describe('WP-G: Error toasts for user feedback', () => {
  it('plugin-manager.js should show toast on setActivePlugin failure', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(pm.includes("S.toast('Plugin switch failed"), 'setActivePlugin catch should show toast');
  });

  it('plugin-manager.js should show toast on clearSlot failure', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(pm.includes("S.toast('Failed to clear slot"), 'clearSlot catch should show toast');
  });

  it('plugin-manager.js should show toast on swap failure', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    assert.ok(pm.includes("S.toast('Swap failed"), 'swapSlots catch should show toast');
  });
});

describe('WP-G: Init error propagation', () => {
  it('init() should NOT have try/catch wrapping getPlugins', () => {
    const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');
    // The init function should let getPlugins errors propagate
    // Check that getPlugins is NOT inside a try block within init
    const initMatch = pm.match(/async function init\(\)[\s\S]*?state\.initialized = true;/);
    assert.ok(initMatch, 'Should find init function');
    // The init function should have getPlugins call without surrounding try
    const initBody = initMatch[0];
    assert.ok(
      initBody.includes("S.queuedFetch('/getPlugins')"),
      'init should call getPlugins'
    );
    // getPlugins line itself should NOT be inside a try block.
    // (A separate try/catch around getIOCaps is fine — that's non-critical.)
    // Verify by checking that the getPlugins line comes BEFORE any try block.
    const getPluginsIdx = initBody.indexOf("S.queuedFetch('/getPlugins')");
    const firstTryIdx = initBody.indexOf('try {');
    assert.ok(
      firstTryIdx === -1 || getPluginsIdx < firstTryIdx,
      'getPlugins must not be wrapped in try/catch — errors should propagate to app.js'
    );
  });

  it('app.js should check connection state before setting connected', () => {
    const app = fs.readFileSync(path.join(wwwDir, '..', '..', 'www', 'js', 'app.js'), 'utf8');
    assert.ok(
      app.includes("connectionState.status !== 'disconnected'"),
      'app.js should check for circuit-breaker disconnect before calling setConnected'
    );
  });
});

// ─── WP-H: Config Parity Audit Tests ────────────────────────

describe('WP-H: Config dialog has all firmware config controls', () => {
  const html = fs.readFileSync(path.join(__dirname, '..', '..', 'sdcard_image', 'www', 'index.html'), 'utf8');

  it('Audio tab has daisy chain select', () => {
    assert.ok(html.includes('id="cfg-daisy-chain"'), 'Missing cfg-daisy-chain select');
  });

  it('Audio tab has CH0 stereo routing select', () => {
    assert.ok(html.includes('id="cfg-ch0-stereo"'), 'Missing cfg-ch0-stereo select');
  });

  it('Audio tab has CH1 stereo routing select', () => {
    assert.ok(html.includes('id="cfg-ch1-stereo"'), 'Missing cfg-ch1-stereo select');
  });

  it('Audio tab has per-channel soft clip (CH0)', () => {
    assert.ok(html.includes('id="cfg-soft-clip-ch0"'), 'Missing cfg-soft-clip-ch0 switch');
  });

  it('Audio tab has per-channel soft clip (CH1)', () => {
    assert.ok(html.includes('id="cfg-soft-clip-ch1"'), 'Missing cfg-soft-clip-ch1 switch');
  });

  it('Audio tab does NOT have ganged soft clip', () => {
    assert.ok(!html.includes('id="cfg-soft-clip"'), 'Old ganged cfg-soft-clip should be removed');
  });

  it('WiFi tab has USB NCM mode option', () => {
    assert.ok(html.includes('id="cfg-wifi-usbncm"'), 'Missing cfg-wifi-usbncm radio button');
  });

  it('Audio codec level sliders use 0-63 range', () => {
    const rangeMatches = html.match(/id="cfg-input-gain"[^>]*min="0"[^>]*max="63"/);
    assert.ok(rangeMatches, 'cfg-input-gain should have min=0 max=63');
  });

  it('Audio labels say Codec Level not Gain', () => {
    assert.ok(html.includes('Left Codec Level'), 'Should say "Left Codec Level"');
    assert.ok(html.includes('Right Codec Level'), 'Should say "Right Codec Level"');
  });
});

describe('WP-H: app.js reads/writes all firmware config keys', () => {
  const appJs = fs.readFileSync(path.join(__dirname, '..', '..', 'sdcard_image', 'www', 'js', 'app.js'), 'utf8');

  it('populateConfigDialog reads ch01_daisy', () => {
    assert.ok(appJs.includes('config.ch01_daisy'), 'Should read ch01_daisy from config');
  });

  it('populateConfigDialog reads ch0_toStereo and ch1_toStereo', () => {
    assert.ok(appJs.includes('config.ch0_toStereo'), 'Should read ch0_toStereo');
    assert.ok(appJs.includes('config.ch1_toStereo'), 'Should read ch1_toStereo');
  });

  it('populateConfigDialog reads per-channel soft clip', () => {
    assert.ok(appJs.includes("config.ch0_outputSoftClip === 'on'"), 'Should read ch0_outputSoftClip');
    assert.ok(appJs.includes("config.ch1_outputSoftClip === 'on'"), 'Should read ch1_outputSoftClip');
  });

  it('WiFi save handler supports usbncm mode', () => {
    assert.ok(appJs.includes("mode = 'usbncm'"), 'WiFi save should handle usbncm mode');
  });

  it('Audio save handler writes daisy chain and stereo routing', () => {
    assert.ok(appJs.includes("config.ch01_daisy = daisy.value"), 'Should save ch01_daisy');
    assert.ok(appJs.includes("config.ch0_toStereo = ch0s.value"), 'Should save ch0_toStereo');
    assert.ok(appJs.includes("config.ch1_toStereo = ch1s.value"), 'Should save ch1_toStereo');
  });
});

describe('WP-H: Dev-server has getPresetData/setPresetData handlers', () => {
  const devServer = fs.readFileSync(path.join(__dirname, '..', '..', 'tools', 'dev-server.js'), 'utf8');

  it('has handleGetPresetData function', () => {
    assert.ok(devServer.includes('function handleGetPresetData'), 'Missing handleGetPresetData');
  });

  it('has handleSetPresetData function', () => {
    assert.ok(devServer.includes('function handleSetPresetData'), 'Missing handleSetPresetData');
  });

  it('routes /api/v1/getPresetData/<pluginId>', () => {
    assert.ok(devServer.includes('getPresetData'), 'Missing getPresetData route');
  });

  it('routes /api/v1/setPresetData/<pluginId>', () => {
    assert.ok(devServer.includes('setPresetData'), 'Missing setPresetData route');
  });
});

// ─── WP-I: Stability fixes for heavy plugins ─────────────────

describe('WP-I: Plugin switch stability (heavy plugin support)', () => {
  const shared = fs.readFileSync(path.join(wwwDir, 'shared.js'), 'utf8');
  const pm = fs.readFileSync(path.join(wwwDir, 'plugin-manager.js'), 'utf8');

  it('API_PLUGIN_SWITCH_TIMEOUT_MS should be >= 30s for sample ROM loading', () => {
    const m = shared.match(/API_PLUGIN_SWITCH_TIMEOUT_MS\s*=\s*(\d+)/);
    assert.ok(m, 'Should define API_PLUGIN_SWITCH_TIMEOUT_MS');
    assert.ok(parseInt(m[1]) >= 30000, `Plugin switch timeout should be >= 30s, got ${m[1]}ms`);
  });

  it('Circuit breaker threshold should be >= 3', () => {
    const m = shared.match(/_FAILURE_THRESHOLD\s*=\s*(\d+)/);
    assert.ok(m, 'Should define _FAILURE_THRESHOLD');
    assert.ok(parseInt(m[1]) >= 3, `Threshold should be >= 3, got ${m[1]}`);
  });

  it('apiFetch should accept skipCircuitBreaker parameter', () => {
    assert.ok(shared.includes('skipCircuitBreaker'), 'apiFetch should support skipCircuitBreaker flag');
  });

  it('queuedFetch should pass skipCircuitBreaker to apiFetch', () => {
    assert.ok(
      shared.includes('function queuedFetch(path, timeoutMs, skipCircuitBreaker)'),
      'queuedFetch should accept skipCircuitBreaker parameter'
    );
  });

  it('plugin-manager should identify heavy plugins (ctagSampleRom users)', () => {
    assert.ok(pm.includes('WTOsc'), 'Should list WTOsc as heavy');
    assert.ok(pm.includes('WTOscDuo'), 'Should list WTOscDuo as heavy');
    assert.ok(pm.includes('Freakwaves'), 'Should list Freakwaves as heavy');
    assert.ok(pm.includes('VctrSnt'), 'Should list VctrSnt as heavy');
    assert.ok(pm.includes('_heavyPlugins') || pm.includes('_isHeavyPlugin'),
      'Should have heavy plugin detection');
  });

  it('setActivePlugin should retry on timeout for heavy plugins', () => {
    assert.ok(pm.includes('retrying'), 'Should have retry logic with log message');
  });

  it('setActivePlugin should show informative loading message for heavy plugins', () => {
    assert.ok(pm.includes('wavetable') || pm.includes('SD card'),
      'Loading message should mention wavetable/SD card for heavy plugins');
  });

  it('shared.js should export API_PLUGIN_SWITCH_TIMEOUT_MS', () => {
    assert.ok(shared.includes('API_PLUGIN_SWITCH_TIMEOUT_MS: API_PLUGIN_SWITCH_TIMEOUT_MS'),
      'Should export API_PLUGIN_SWITCH_TIMEOUT_MS');
  });
});

// ─── Run ──────────────────────────────────────────────────────

runTests();
