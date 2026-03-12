#!/usr/bin/env node
/**
 * TBD-16 WebUI — Development Server
 *
 * Serves the WebUI from sdcard_image/www/ and provides mock API endpoints
 * that mirror the real ESP32 RestServer.cpp/MacroAPI.cpp/SampleAPI.cpp.
 * Uses real data from sdcard_image/data/ (synthdefinitions, macrodefinitions,
 * macrosoundpresets, sp/ plugin configs).
 *
 * Mock API Endpoints (v2 — matching RestServer.cpp):
 *   GET  /api/v2/device?action=getIOCaps              → IO capabilities
 *   GET  /api/v2/device?action=getConfig               → system config
 *   POST /api/v2/device?action=setConfig               → save config
 *   GET  /api/v2/device?action=getFavorites             → all favorites
 *   POST /api/v2/device?action=storeFavorite&id=N       → store favorite
 *   POST /api/v2/device?action=recallFavorite&id=N      → recall favorite
 *   POST /api/v2/device?action=reboot                   → reboot (no-op)
 *   GET  /api/v2/plugins?action=list                    → list of sound processors
 *   GET  /api/v2/plugins?action=getActive&ch=N          → active plugin for channel
 *   POST /api/v2/plugins?action=setActive&ch=N&id=X     → set active plugin
 *   GET  /api/v2/plugins?action=getParams&ch=N          → plugin parameters
 *   POST /api/v2/plugins?action=setParam&ch=N&id=X&key=K&val=V → set a parameter
 *   GET  /api/v2/plugins?action=getPresets&ch=N         → preset list for channel
 *   POST /api/v2/plugins?action=loadPreset&ch=N&number=N → load preset
 *   POST /api/v2/plugins?action=savePreset&ch=N&number=N&name=X → save preset
 *   GET  /api/v2/plugins?action=getPresetData&id=X      → full preset JSON
 *   POST /api/v2/plugins?action=setPresetData&id=X      → write preset JSON
 *   GET  /api/v2/samples                               → file list + configfiles scan
 *   GET  /api/v2/samples?getconfig=<path>               → serve JSON from data/
 *   POST /api/v2/samples?action=uploadconfig&path=<p>   → save JSON to data/
 *   POST /api/v2/samples?action=manage                  → delete/rename files
 *   POST /api/v2/samples?action=reload                  → reload sample rom
 *   GET  /api/v2/macros                                 → current track state
 *   GET  /api/v2/macros?action=getall                   → bulk macro data
 *   POST /api/v2/macros?action=update_track             → set track machine/macro/params
 *   POST /api/v2/macros?action=reload                   → reload macros from disk
 *
 * Usage:
 *   cd sdcard_image/www
 *   node tools/dev-server.js [port]
 *
 * Default port: 3001
 * Open http://localhost:3001/ for index.html (Plugin & Sample Manager)
 * Open http://localhost:3001/preset-macro-manager.html for Preset & Macro Manager
 */

const http = require('http');
const fs   = require('fs');
const path = require('path');
const url  = require('url');

const PORT = parseInt(process.argv[2], 10) || 3001;
const REPO_ROOT = path.resolve(__dirname, '..', '..', '..');
const WEBROOT = path.resolve(__dirname, '..');
const DATA_DIR = path.resolve(REPO_ROOT, 'sdcard_image', 'data');
const SAMPLE_ROOT = path.resolve(REPO_ROOT, 'sample_rom', 'tbdsamples');

// ───────── MIME types ─────────
const MIME = {
  '.html': 'text/html; charset=utf-8',
  '.css':  'text/css; charset=utf-8',
  '.js':   'application/javascript; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.svg':  'image/svg+xml',
  '.png':  'image/png',
  '.jpg':  'image/jpeg',
  '.ico':  'image/x-icon',
  '.woff': 'font/woff',
  '.woff2':'font/woff2',
  '.wav':  'audio/wav',
};

// ═══════════════════════════════════════════════════════════════
//  IN-MEMORY STATE (mock firmware state)
// ═══════════════════════════════════════════════════════════════

let synthDefs = null;     // Loaded from synthdefinitions.json
let pluginList = [];      // Loaded from synthdefinitions.json machines
const trackState = [];    // 19 tracks: { index, name, type, machine, macro, parameters[] }
const channelState = {};  // Channel 0/1: { activePlugin, params, presets }
let systemConfig = {};    // Loaded from spm-config.jsn
const favorites = {};     // slot# -> data

function loadSynthDefinitions() {
  const p = path.join(DATA_DIR, 'synthdefinitions.json');
  try {
    synthDefs = JSON.parse(fs.readFileSync(p, 'utf8'));
  } catch (e) {
    console.warn('Could not load synthdefinitions.json:', e.message);
    synthDefs = { tracks: [], machines: [] };
  }

  // Build plugin list from machines
  pluginList = (synthDefs.machines || []).map(m => ({
    id: m.id || m.name,
    name: m.name || m.id,
    isStereo: m.isStereo || false,
    hint: m.hint || '',
  }));

  // Initialize 19 track slots (0-15 instruments, 16-18 FX)
  for (let i = 0; i < 19; i++) {
    const trackDef = synthDefs.tracks.find(t => t.index === i);
    trackState[i] = {
      index: i,
      name: trackDef ? trackDef.name : 'Track ' + (i + 1),
      type: trackDef ? trackDef.type : 'drum',
      machine: '',
      macro: '',
      parameters: [],
    };
  }

  // Initialize channels
  for (const ch of [0, 1]) {
    channelState[ch] = channelState[ch] || {
      activePlugin: pluginList.length > 0 ? pluginList[0].id : 'TBD03',
      params: [],
      activePreset: 0,
    };
  }
}

function loadSystemConfig() {
  try {
    systemConfig = JSON.parse(
      fs.readFileSync(path.join(DATA_DIR, 'spm-config.jsn'), 'utf8')
    );
  } catch (e) {
    systemConfig = { activeProcessors: ['TBD03', 'TBD03'] };
  }
}

loadSynthDefinitions();
loadSystemConfig();

// ═══════════════════════════════════════════════════════════════
//  DATA DIRECTORY HELPERS
// ═══════════════════════════════════════════════════════════════

/** Recursively scan for .json/.jsn files under a directory, returning
 *  entries with { name, path (relative folder), size, mtime } —
 *  matching the real SampleAPI::scan_json_files format. */
function scanJsonFiles(baseDir, relDir) {
  const results = [];
  const dirPath = relDir ? path.join(baseDir, relDir) : baseDir;
  let entries;
  try { entries = fs.readdirSync(dirPath, { withFileTypes: true }); }
  catch (e) { return results; }

  for (const ent of entries) {
    if (ent.name.startsWith('.')) continue;
    const relPath = relDir ? relDir + '/' + ent.name : ent.name;
    const fullPath = path.join(baseDir, relPath);

    if (ent.isDirectory()) {
      results.push(...scanJsonFiles(baseDir, relPath));
    } else if (ent.isFile()) {
      const ext = path.extname(ent.name).toLowerCase();
      if (ext === '.json' || ext === '.jsn') {
        const stat = fs.statSync(fullPath);
        results.push({
          name: ent.name,
          path: relDir || '',
          size: stat.size,
          mtime: Math.floor(stat.mtimeMs),
        });
      }
    }
  }
  return results;
}

/** Scan WAV files under sample_rom */
function scanWavFiles(baseDir, relDir) {
  const results = [];
  const dirPath = relDir ? path.join(baseDir, relDir) : baseDir;
  let entries;
  try { entries = fs.readdirSync(dirPath, { withFileTypes: true }); }
  catch (e) { return results; }

  for (const ent of entries) {
    if (ent.name.startsWith('.')) continue;
    const relPath = relDir ? relDir + '/' + ent.name : ent.name;
    const fullPath = path.join(baseDir, relPath);

    if (ent.isDirectory()) {
      results.push(...scanWavFiles(baseDir, relPath));
    } else if (ent.isFile()) {
      const ext = path.extname(ent.name).toLowerCase();
      if (ext === '.wav') {
        const stat = fs.statSync(fullPath);
        results.push({
          name: ent.name,
          path: relDir || '',
          size: stat.size,
        });
      }
    }
  }
  return results;
}

/** Scan directories */
function scanDirectories(baseDir, relDir) {
  const results = [];
  const dirPath = relDir ? path.join(baseDir, relDir) : baseDir;
  let entries;
  try { entries = fs.readdirSync(dirPath, { withFileTypes: true }); }
  catch (e) { return results; }

  for (const ent of entries) {
    if (ent.name.startsWith('.')) continue;
    if (ent.isDirectory()) {
      const relPath = relDir ? relDir + '/' + ent.name : ent.name;
      results.push({ path: relPath });
      results.push(...scanDirectories(baseDir, relPath));
    }
  }
  return results;
}

/** Generate mock plugin params for a given plugin */
function generateMockParams(pluginId) {
  // Try to load the real sp/ config file if it exists
  const spPath = path.join(DATA_DIR, 'sp', `mui-${pluginId}.jsn`);
  try {
    const data = JSON.parse(fs.readFileSync(spPath, 'utf8'));
    if (data.params) return data;
  } catch (e) { /* fall through to mock */ }

  // Generate mock params
  const params = [];
  for (let i = 0; i < 8; i++) {
    params.push({
      id: `param_${i}`,
      name: `Param ${i}`,
      type: 'int',
      current: 64,
      min: 0,
      max: 127,
    });
  }
  return { id: pluginId, params };
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/device?action=getIOCaps
// ═══════════════════════════════════════════════════════════════

function handleGetIOCaps(req, res) {
  // Mock IOCaps matching the real device shape
  const triggers = [];
  const cvs = [];
  for (const prefix of ['A', 'B', 'C', 'D']) {
    triggers.push(`${prefix}_NOTE`, `${prefix}_VELO`, `${prefix}_P_PROG`, `${prefix}_P_AT`);
    cvs.push(`${prefix}_NOTE`, `${prefix}_VELO`, `${prefix}_P_BANK`, `${prefix}_P_SBNK`,
             `${prefix}_P_PRG`, `${prefix}_P_PB`, `${prefix}_P_PB_LG`, `${prefix}_P_AT`,
             `${prefix}_P_MW_1`, `${prefix}_P_BC_2`);
  }
  sendJson(res, 200, {
    HWV: 'DADA',
    FWV: '0.0.0-dev',
    p: 'dada',
    t: triggers,
    cv: cvs,
  });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=list
// ═══════════════════════════════════════════════════════════════

function handleGetPlugins(req, res) {
  sendJson(res, 200, pluginList);
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=getActive&ch=N
// ═══════════════════════════════════════════════════════════════

function handleGetActivePlugin(req, res, channel) {
  const cs = channelState[channel] || channelState[0];
  sendJson(res, 200, { id: cs.activePlugin });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=setActive&ch=N&id=X
// ═══════════════════════════════════════════════════════════════

function handleSetActivePlugin(req, res, channel) {
  const parsed = url.parse(req.url, true);
  const pluginId = parsed.query.id;
  if (!pluginId) return sendJson(res, 400, { error: 'Missing id' });
  const cs = channelState[channel] || channelState[0];
  cs.activePlugin = pluginId;
  cs.params = generateMockParams(pluginId).params || [];
  console.log(`[plugin] Channel ${channel} active plugin → ${pluginId}`);
  sendJson(res, 200, { ok: true });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=getParams&ch=N
// ═══════════════════════════════════════════════════════════════

function handleGetPluginParams(req, res, channel) {
  const cs = channelState[channel] || channelState[0];
  const data = generateMockParams(cs.activePlugin);
  sendJson(res, 200, data);
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=setParam&ch=N&id=X&key=K&val=V
// ═══════════════════════════════════════════════════════════════

function handleSetPluginParam(req, res, channel) {
  const parsed = url.parse(req.url, true);
  const id = parsed.query.id;
  const key = parsed.query.key || 'current';
  const value = parsed.query.val;
  console.log(`[param] Ch${channel} ${key} ${id} = ${value}`);
  sendJson(res, 200, { ok: true });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=getPresets&ch=N
// ═══════════════════════════════════════════════════════════════

function handleGetPresets(req, res, channel) {
  sendJson(res, 200, {
    activePresetNumber: 0,
    presets: [{ name: 'Default', number: 0 }],
  });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=loadPreset&ch=N&number=N
// ═══════════════════════════════════════════════════════════════

function handleLoadPreset(req, res, channel) {
  const parsed = url.parse(req.url, true);
  console.log(`[preset] Load preset ${parsed.query.number} on channel ${channel}`);
  sendJson(res, 200, { ok: true });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=savePreset&ch=N&number=N&name=X
// ═══════════════════════════════════════════════════════════════

function handleSavePreset(req, res, channel) {
  const parsed = url.parse(req.url, true);
  console.log(`[preset] Save preset "${parsed.query.name}" #${parsed.query.number} on channel ${channel}`);
  sendJson(res, 200, { ok: true });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=getPresetData&id=X
// ═══════════════════════════════════════════════════════════════

function handleGetPresetData(req, res, pluginId) {
  const data = generateMockParams(pluginId);
  sendJson(res, 200, data);
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/plugins?action=setPresetData&id=X
// ═══════════════════════════════════════════════════════════════

function handleSetPresetData(req, res, pluginId) {
  readJsonBody(req, (err, body) => {
    if (err) return sendJson(res, 400, { error: 'Invalid JSON' });
    console.log(`[preset] Set preset data for ${pluginId}`);
    sendJson(res, 200, { ok: true });
  });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/device?action=getConfig
// ═══════════════════════════════════════════════════════════════

function handleGetConfiguration(req, res) {
  sendJson(res, 200, systemConfig);
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/device?action=setConfig
// ═══════════════════════════════════════════════════════════════

function handleSetConfiguration(req, res) {
  readJsonBody(req, (err, body) => {
    if (err) return sendJson(res, 400, { error: 'Invalid JSON' });
    Object.assign(systemConfig, body);
    console.log('[config] Configuration updated');
    sendJson(res, 200, { ok: true });
  });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/device?action=getFavorites|storeFavorite|recallFavorite
// ═══════════════════════════════════════════════════════════════

function handleFavoriteGet(req, res, action) {
  if (action === 'getFavorites') {
    return sendJson(res, 200, Object.values(favorites));
  }
  sendJson(res, 400, { error: 'Unknown device GET action: ' + action });
}

function handleFavoritePost(req, res, action, query) {
  if (action === 'storeFavorite') {
    const idx = parseInt(query.id, 10);
    readJsonBody(req, (err, body) => {
      if (err) return sendJson(res, 400, { error: 'Invalid JSON' });
      favorites[idx] = body;
      console.log(`[fav] Stored favorite ${idx}`);
      sendJson(res, 200, { ok: true });
    });
    return;
  }

  if (action === 'recallFavorite') {
    const idx = parseInt(query.id, 10);
    console.log(`[fav] Recalled favorite ${idx}`);
    return sendJson(res, 200, favorites[idx] || { ok: true });
  }

  sendJson(res, 200, { ok: true });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/samples
// ═══════════════════════════════════════════════════════════════

function handleSamplesGet(req, res) {
  const parsed = url.parse(req.url, true);
  const q = parsed.query;

  // Get a config file: ?getconfig=synthdefinitions.json
  if (q.getconfig) {
    const configPath = path.join(DATA_DIR, q.getconfig);
    // Security: must stay inside DATA_DIR
    if (!configPath.startsWith(DATA_DIR)) {
      return sendJson(res, 403, { error: 'Forbidden' });
    }
    try {
      const raw = fs.readFileSync(configPath, 'utf8');
      res.writeHead(200, {
        'Content-Type': 'application/json; charset=utf-8',
        'Access-Control-Allow-Origin': '*',
        'Cache-Control': 'no-cache',
      });
      return res.end(raw);
    } catch (e) {
      return sendJson(res, 404, { error: 'Not found: ' + q.getconfig });
    }
  }

  // Preview WAV: ?preview=path/name.wav
  if (q.preview) {
    const wavPath = path.join(SAMPLE_ROOT, q.preview);
    if (!wavPath.startsWith(SAMPLE_ROOT)) {
      return sendJson(res, 403, { error: 'Forbidden' });
    }
    try {
      const stat = fs.statSync(wavPath);
      res.writeHead(200, {
        'Content-Type': 'audio/wav',
        'Content-Length': stat.size,
        'Access-Control-Allow-Origin': '*',
      });
      return fs.createReadStream(wavPath).pipe(res);
    } catch (e) {
      return sendJson(res, 404, { error: 'Not found' });
    }
  }

  // Default: full response matching real SampleAPI::samples_get_handler
  // Scans /sdcard/data/ for config files, /sdcard/tbdsamples/ for WAV files
  const configfiles = scanJsonFiles(DATA_DIR, '');
  const files = scanWavFiles(SAMPLE_ROOT, '');
  const directories = scanDirectories(SAMPLE_ROOT, '');

  // Load kits from sample_rom.jsn if available
  let kits = {};
  try {
    const sampleRom = JSON.parse(
      fs.readFileSync(path.join(DATA_DIR, 'sample_rom.jsn'), 'utf8')
    );
    kits = {
      smp_banks: sampleRom.smp_banks || [],
      smp_bank_names: sampleRom.smp_bank_names || [],
      smp_bank_tags: sampleRom.smp_bank_tags || [],
      active_smp_bank: sampleRom.active_smp_bank || 0,
    };
  } catch (e) {
    kits = { smp_banks: [], smp_bank_names: [], smp_bank_tags: [], active_smp_bank: 0 };
  }

  return sendJson(res, 200, {
    files,
    directories,
    configfiles,
    kits,
    capacity: { total: 32000000000, used: 1000000000 },
    active_kit_entries: [],
  });
}

function handleSamplesPost(req, res) {
  const parsed = url.parse(req.url, true);
  const action = parsed.query.action || '';

  if (action === 'uploadconfig') {
    const filePath = parsed.query.path;
    if (!filePath) {
      return sendJson(res, 400, { error: 'Missing path parameter' });
    }
    const fullPath = path.join(DATA_DIR, filePath);
    if (!fullPath.startsWith(DATA_DIR)) {
      return sendJson(res, 403, { error: 'Forbidden' });
    }
    readBody(req, (err, body) => {
      if (err) return sendJson(res, 400, { error: 'Bad request' });
      const dir = path.dirname(fullPath);
      fs.mkdirSync(dir, { recursive: true });
      fs.writeFileSync(fullPath, body);
      console.log(`[upload] Wrote ${filePath} (${body.length} bytes)`);
      sendJson(res, 200, { ok: true });
    });
    return;
  }

  if (action === 'manage') {
    readJsonBody(req, (err, body) => {
      if (err) return sendJson(res, 400, { error: 'Invalid JSON' });
      if (body.action === 'deleteconfig') {
        const fullPath = path.join(DATA_DIR, body.path);
        if (fullPath.startsWith(DATA_DIR) && fs.existsSync(fullPath)) {
          fs.unlinkSync(fullPath);
          console.log(`[delete] Removed ${body.path}`);
        }
      } else if (body.action === 'rename') {
        console.log(`[manage] rename ${body.from} → ${body.to}`);
      } else if (body.action === 'delete') {
        console.log(`[manage] delete ${body.path}`);
      } else if (body.action === 'saveKit' || body.action === 'createKit' || body.action === 'deleteKit') {
        console.log(`[manage] ${body.action}`);
      } else if (body.action === 'createFolder' || body.action === 'renameFolder' || body.action === 'deleteFolder') {
        console.log(`[manage] ${body.action} ${body.path || ''}`);
      }
      sendJson(res, 200, { ok: true });
    });
    return;
  }

  if (action === 'upload') {
    // WAV file upload — accept and discard in dev mode
    readBody(req, (err, body) => {
      console.log(`[upload] WAV upload to ${parsed.query.path}/${parsed.query.filename} (${body ? body.length : 0} bytes) — dev mode, not saved`);
      sendJson(res, 200, { ok: true });
    });
    return;
  }

  if (action === 'reload') {
    console.log('[samples] Reload PSRAM sample rom requested');
    sendJson(res, 200, { ok: true });
    return;
  }

  sendJson(res, 400, { error: 'Unknown action: ' + action });
}

// ═══════════════════════════════════════════════════════════════
//  API: /api/v2/macros
// ═══════════════════════════════════════════════════════════════

function handleMacroApiGet(req, res) {
  const parsed = url.parse(req.url, true);
  const action = parsed.query.action || '';

  if (action === 'getall') {
    // Bulk endpoint — return macroDefs, soundPresets, and tracks
    const macroDefs = scanJsonFiles(DATA_DIR, 'macrodefinitions').map(f => {
      try {
        return JSON.parse(fs.readFileSync(path.join(DATA_DIR, f.path ? f.path + '/' + f.name : f.name), 'utf8'));
      } catch (e) { return null; }
    }).filter(Boolean);
    const soundPresets = scanJsonFiles(DATA_DIR, 'macrosoundpresets').map(f => {
      try {
        return JSON.parse(fs.readFileSync(path.join(DATA_DIR, f.path ? f.path + '/' + f.name : f.name), 'utf8'));
      } catch (e) { return null; }
    }).filter(Boolean);
    const tracks = trackState.slice(0, 19).map(t => ({
      index: t.index, name: t.name, type: t.type,
      machine: t.machine, macro: t.macro,
    }));
    return sendJson(res, 200, { macroDefs, soundPresets, tracks });
  }

  // Default: return tracks
  const tracks = trackState.slice(0, 19).map(t => ({
    index: t.index,
    name: t.name,
    type: t.type,
    machine: t.machine,
    macro: t.macro,
  }));
  sendJson(res, 200, { tracks });
}

function handleMacroApiPost(req, res) {
  const parsed = url.parse(req.url, true);
  const action = parsed.query.action || '';

  if (action === 'update_track' || action === 'set_track_parameters') {
    readJsonBody(req, (err, body) => {
      if (err) return sendJson(res, 400, { error: 'Invalid JSON' });

      const trackIdx = body.track;
      if (trackIdx === undefined || trackIdx < 0 || trackIdx >= 19) {
        return sendJson(res, 400, { error: 'Invalid track index' });
      }

      const ts = trackState[trackIdx];
      if (body.machine !== undefined) {
        ts.machine = body.machine;
        console.log(`[macro] Track ${trackIdx} machine → ${body.machine}`);
      }
      if (body.macro !== undefined) {
        ts.macro = body.macro;
        console.log(`[macro] Track ${trackIdx} macro → ${body.macro}`);
      }
      if (body.parameters !== undefined && Array.isArray(body.parameters)) {
        ts.parameters = body.parameters;
        const abbreviated = body.parameters.length > 4
          ? body.parameters.slice(0, 4).join(', ') + '...'
          : body.parameters.join(', ');
        console.log(`[macro] Track ${trackIdx} params → [${abbreviated}]`);
      }

      sendJson(res, 200, { ok: true });
    });
    return;
  }

  if (action === 'reload') {
    console.log('[macroapi] Reload macros from disk');
    loadSynthDefinitions();
    sendJson(res, 200, { ok: true });
    return;
  }

  sendJson(res, 400, { error: 'Unknown action: ' + action });
}

// ═══════════════════════════════════════════════════════════════
//  STATIC FILE SERVING
// ═══════════════════════════════════════════════════════════════

function serveStatic(req, res) {
  let reqPath = url.parse(req.url).pathname;
  if (reqPath === '/') reqPath = '/index.html';

  const filePath = path.join(WEBROOT, path.normalize(reqPath));
  if (!filePath.startsWith(WEBROOT)) {
    res.writeHead(403);
    return res.end('Forbidden');
  }

  fs.stat(filePath, (err, stat) => {
    if (err || !stat.isFile()) {
      res.writeHead(404, { 'Content-Type': 'text/plain' });
      return res.end('Not found: ' + reqPath);
    }

    const ext = path.extname(filePath).toLowerCase();
    const mime = MIME[ext] || 'application/octet-stream';

    res.writeHead(200, {
      'Content-Type': mime,
      'Content-Length': stat.size,
      'Cache-Control': 'no-cache',
      'Access-Control-Allow-Origin': '*',
    });

    fs.createReadStream(filePath).pipe(res);
  });
}

// ═══════════════════════════════════════════════════════════════
//  UTILITY
// ═══════════════════════════════════════════════════════════════

function sendJson(res, code, obj) {
  const body = JSON.stringify(obj);
  res.writeHead(code, {
    'Content-Type': 'application/json; charset=utf-8',
    'Content-Length': Buffer.byteLength(body),
    'Access-Control-Allow-Origin': '*',
    'Cache-Control': 'no-cache',
  });
  res.end(body);
}

function readBody(req, cb) {
  let chunks = [];
  req.on('data', c => chunks.push(c));
  req.on('end', () => cb(null, Buffer.concat(chunks).toString()));
}

function readJsonBody(req, cb) {
  readBody(req, (err, body) => {
    if (err) return cb(err);
    try {
      cb(null, JSON.parse(body));
    } catch (e) {
      cb(e);
    }
  });
}

// ═══════════════════════════════════════════════════════════════
//  ROUTER
// ═══════════════════════════════════════════════════════════════

const server = http.createServer((req, res) => {
  const parsed = url.parse(req.url, true);
  const p = parsed.pathname;

  // Logging
  const ts = new Date().toISOString().slice(11, 19);
  if (!p.includes('favicon')) {
    console.log(`  ${ts} ${req.method} ${p}${parsed.search || ''}`);
  }

  // CORS preflight
  if (req.method === 'OPTIONS') {
    res.writeHead(204, {
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
      'Access-Control-Allow-Headers': 'Content-Type',
    });
    return res.end();
  }

  // ── API Routes (v2 — action-based dispatch) ──

  // /api/v2/plugins
  if (p === '/api/v2/plugins' && req.method === 'GET') {
    const action = parsed.query.action || '';
    const ch = parseInt(parsed.query.ch || '0', 10);
    if (action === 'list') return handleGetPlugins(req, res);
    if (action === 'getActive') return handleGetActivePlugin(req, res, ch);
    if (action === 'getParams') return handleGetPluginParams(req, res, ch);
    if (action === 'getPresets') return handleGetPresets(req, res, ch);
    if (action === 'getPresetData') return handleGetPresetData(req, res, parsed.query.id);
    return sendJson(res, 400, { error: 'Unknown plugins GET action: ' + action });
  }
  if (p === '/api/v2/plugins' && req.method === 'POST') {
    const action = parsed.query.action || '';
    const ch = parseInt(parsed.query.ch || '0', 10);
    if (action === 'setActive') return handleSetActivePlugin(req, res, ch);
    if (action === 'setParam') return handleSetPluginParam(req, res, ch);
    if (action === 'loadPreset') return handleLoadPreset(req, res, ch);
    if (action === 'savePreset') return handleSavePreset(req, res, ch);
    if (action === 'setPresetData') return handleSetPresetData(req, res, parsed.query.id);
    return sendJson(res, 400, { error: 'Unknown plugins POST action: ' + action });
  }

  // /api/v2/device
  if (p === '/api/v2/device' && req.method === 'GET') {
    const action = parsed.query.action || '';
    if (action === 'getIOCaps') return handleGetIOCaps(req, res);
    if (action === 'getConfig') return handleGetConfiguration(req, res);
    if (action === 'getFavorites') return handleFavoriteGet(req, res, action);
    return sendJson(res, 400, { error: 'Unknown device GET action: ' + action });
  }
  if (p === '/api/v2/device' && req.method === 'POST') {
    const action = parsed.query.action || '';
    if (action === 'setConfig') return handleSetConfiguration(req, res);
    if (action === 'reboot') {
      console.log('[reboot] Reboot requested — ignored in dev mode');
      return sendJson(res, 200, { ok: true });
    }
    if (action === 'storeFavorite' || action === 'recallFavorite')
      return handleFavoritePost(req, res, action, parsed.query);
    return sendJson(res, 400, { error: 'Unknown device POST action: ' + action });
  }

  // /api/v2/samples
  if (p === '/api/v2/samples' && req.method === 'GET')
    return handleSamplesGet(req, res);
  if (p === '/api/v2/samples' && req.method === 'POST')
    return handleSamplesPost(req, res);

  // /api/v2/macros
  if (p === '/api/v2/macros' && req.method === 'GET')
    return handleMacroApiGet(req, res);
  if (p === '/api/v2/macros' && req.method === 'POST')
    return handleMacroApiPost(req, res);

  // Static files (catch-all — matches real RestServer.cpp behavior)
  serveStatic(req, res);
});

server.listen(PORT, () => {
  console.log(`\n  TBD-16 WebUI Dev Server`);
  console.log(`  ──────────────────────────────────────`);
  console.log(`  Static files : ${WEBROOT}`);
  console.log(`  Data dir     : ${DATA_DIR}`);
  console.log(`  Sample dir   : ${SAMPLE_ROOT}`);
  console.log(`  Listening    : http://localhost:${PORT}`);
  console.log();
  console.log(`  Pages:`);
  console.log(`    http://localhost:${PORT}/                          → Plugin & Sample Manager`);
  console.log(`    http://localhost:${PORT}/preset-macro-manager.html → Preset & Macro Manager`);
  console.log();
});
