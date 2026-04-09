// ═══════════════════════════════════════════════════════════════
// TBD-16 Sample Manager  —  Client Application
// Vanilla JS  ·  Shoelace Web Components  ·  Sortable.js
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

// ─── Shared Utilities (from shared.js when available) ────────
var _S = (window.TBD && window.TBD.shared) ? window.TBD.shared : null;

// ─── Configuration ───────────────────────────────────────────
const API_BASE      = '/api/v2/samples';
const SAMPLE_RATE   = 44100;
const MAX_FILENAME  = 32;
const SLICES_PER_BANK = 32;
const NUM_BANKS     = 8;
const PSRAM_MAX     = 29_360_128;           // ~28 MB
const UPLOAD_SOFT_LIMIT = 10 * 1024 * 1024; // 10 MB
const USER_FOLDER   = 'user';               // writable user area on SD card

const DEFAULT_BANKS = [
  { name: 'KICK',     color: '#4CAF50' },
  { name: 'SNARE',    color: '#2196F3' },
  { name: 'HIHAT CL', color: '#FFC107' },
  { name: 'HIHAT OP', color: '#FF9800' },
  { name: 'CLAP',     color: '#E91E63' },
  { name: 'RIM',      color: '#9C27B0' },
  { name: 'PERC',     color: '#00BCD4' },
  { name: 'OTHER',    color: '#607D8B' },
];

// Extra color palette for banks beyond the defaults
const BANK_COLORS = [
  '#4CAF50', '#2196F3', '#FFC107', '#FF9800',
  '#E91E63', '#9C27B0', '#00BCD4', '#607D8B',
  '#8BC34A', '#03A9F4', '#FFEB3B', '#FF5722',
  '#673AB7', '#009688', '#795548', '#F44336',
];

// ─── Application State ──────────────────────────────────────
const state = {
  files: [],            // [{ name, path, size }] from API
  folders: [],          // unique folder paths

  kits: {               // from sample_rom.json via API
    smp_banks: [],
    smp_bank_names: [],
    smp_bank_tags: [],
    smp_bank_meta: [],
    active_smp_bank: 0,
  },

  kitEntries: [],       // flat Kit descriptor array (may have nulls)
  banks: DEFAULT_BANKS.map(b => ({ ...b, collapsed: false })),

  capacity: { psram_max_bytes: PSRAM_MAX, active_bank_bytes: 0 },

  viewMode: 'banked',   // 'banked' | 'flat'
  targetFolder: USER_FOLDER,
  targetBank: -1,

  // Pool navigation — current folder being viewed ("" = root)
  poolPath: '',
  poolSort: { key: 'name', dir: 'asc' },  // key: 'name'|'dur'|'smp', dir: 'asc'|'desc'

  uploadQueue: [],
  uploading: false,

  audioCtx: null,
  currentSource: null,

  _renameCtx: null,
  _deleteCtx: null,
  _pickerBank: -1,
  _pickerSelected: new Set(),

  // Multi-select for batch operations
  selectedFiles: new Set(),   // Set of 'path/name' keys
  selectionMode: false,

  sortableInstances: [],
  initializing: true,
  dirty: false,
};

let uploadIdCounter = 0;

// ═══════════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════════

function formatDuration(nsamples) {
  const s = nsamples / SAMPLE_RATE;
  return s < 1 ? `${(s * 1000) | 0}ms` : `${s.toFixed(2)}s`;
}

function formatBytes(bytes) {
  if (bytes < 1024) return `${bytes} B`;
  if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
  if (bytes < 1073741824) return `${(bytes / 1048576).toFixed(1)} MB`;
  return `${(bytes / 1073741824).toFixed(1)} GB`;
}

function fileDuration(size) {
  return formatDuration(Math.max(0, (size - 44) / 2));
}

function nsamples(size) { return Math.max(0, (size - 44) / 2); }

function formatSamples(size) {
  const n = nsamples(size);
  if (n >= 1_000_000) return (n / 1_000_000).toFixed(1) + 'M';
  if (n >= 1_000) return (n / 1_000).toFixed(1) + 'k';
  return String(Math.round(n));
}

function formatDate(ms) {
  if (!ms) return '';
  // FAT32 on ESP32 without RTC returns epoch 1980-01-01. Dates before
  // 2020 are meaningless — hide them rather than showing "01/01/80".
  const d = new Date(ms);
  if (d.getFullYear() < 2020) return '';
  const mo = String(d.getMonth() + 1).padStart(2, '0');
  const da = String(d.getDate()).padStart(2, '0');
  const yr = String(d.getFullYear()).slice(-2);
  const hh = String(d.getHours()).padStart(2, '0');
  const mm = String(d.getMinutes()).padStart(2, '0');
  return `${mo}/${da}/${yr} ${hh}:${mm}`;
}

function esc(s) {
  const d = document.createElement('div');
  d.textContent = s;
  return d.innerHTML;
}

/** Check if a path is inside the user-writable folder */
function isUserWritable(path) {
  if (!path) return false;
  return path === USER_FOLDER || path.startsWith(USER_FOLDER + '/');
}

/** Check if a path is a CHILD of the user folder (not the folder itself) */
function isUserWritableChild(path) {
  if (!path) return false;
  return path.startsWith(USER_FOLDER + '/');
}

/** Check if we're currently browsing inside the user-writable folder */
function isInUserFolder() {
  return isUserWritable(state.poolPath);
}

// ═══════════════════════════════════════════════════════════════
//  WAV ENCODING & CONVERSION
// ═══════════════════════════════════════════════════════════════

function writeString(view, off, str) {
  for (let i = 0; i < str.length; i++) view.setUint8(off + i, str.charCodeAt(i));
}

function encodeWAV(samples, sr = SAMPLE_RATE) {
  const bps = 16, nCh = 1;
  const blockAlign = nCh * (bps / 8);
  const dataBytes = samples.length * (bps / 8);
  const total = 44 + dataBytes;
  const buf = new ArrayBuffer(total);
  const v = new DataView(buf);

  writeString(v, 0, 'RIFF');
  v.setUint32(4, total - 8, true);
  writeString(v, 8, 'WAVE');
  writeString(v, 12, 'fmt ');
  v.setUint32(16, 16, true);
  v.setUint16(20, 1, true);
  v.setUint16(22, nCh, true);
  v.setUint32(24, sr, true);
  v.setUint32(28, sr * blockAlign, true);
  v.setUint16(32, blockAlign, true);
  v.setUint16(34, bps, true);
  writeString(v, 36, 'data');
  v.setUint32(40, dataBytes, true);

  let off = 44;
  for (let i = 0; i < samples.length; i++) {
    const s = Math.max(-1, Math.min(1, samples[i]));
    v.setInt16(off, s < 0 ? s * 0x8000 : s * 0x7FFF, true);
    off += 2;
  }
  return new Blob([buf], { type: 'audio/wav' });
}

function sanitizeFilename(name, maxLen = MAX_FILENAME) {
  let stem = name.replace(/\.[^.]+$/, '');
  stem = stem.normalize('NFKD').replace(/[^\x00-\x7F]/g, '');
  stem = stem.replace(/[\s\-]+/g, '_');
  stem = stem.replace(/[^A-Za-z0-9_]/g, '');
  stem = stem.replace(/_+/g, '_').replace(/^_|_$/g, '');
  if (!stem) stem = 'SAMPLE';
  if (stem.length > maxLen) {
    const half = Math.floor((maxLen - 1) / 2);
    stem = stem.slice(0, half) + '_' + stem.slice(-(maxLen - 1 - half));
  }
  return stem;
}

async function convertToWAV(file) {
  if (!state.audioCtx) {
    state.audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  }
  const arrayBuf = await file.arrayBuffer();
  const audioBuf = await state.audioCtx.decodeAudioData(arrayBuf);
  const numFrames = Math.ceil(audioBuf.duration * SAMPLE_RATE);
  const offCtx = new OfflineAudioContext(1, numFrames, SAMPLE_RATE);
  const src = offCtx.createBufferSource();
  src.buffer = audioBuf;
  src.connect(offCtx.destination);
  src.start(0);
  const rendered = await offCtx.startRendering();
  const mono = rendered.getChannelData(0);
  const blob = encodeWAV(mono, SAMPLE_RATE);
  return {
    blob,
    nsamples: mono.length,
    duration: mono.length / SAMPLE_RATE,
    byteLength: blob.size,
  };
}

// ═══════════════════════════════════════════════════════════════
//  API CLIENT — routes through global queue + safe JSON parsing
// ═══════════════════════════════════════════════════════════════

var _apiTimeout = (_S && _S.API_TIMEOUT_MS) ? _S.API_TIMEOUT_MS : 5000;
var _apiQueue   = (_S && _S.apiQueue)       ? _S.apiQueue       : null;
// Longer timeout for sample list fetch — scanning hundreds of files on SD card
// can take 15-30s on FAT32 with slow I/O.
var _sampleListTimeout = 45000;

async function _rawApiGet(queryString, timeoutMs) {
  const r = await fetch(`${API_BASE}${queryString}`, {
    signal: AbortSignal.timeout(timeoutMs || _apiTimeout),
  });
  if (!r.ok) throw new Error(`API ${r.status}`);
  var text = await r.text();
  if (!text || !text.trim()) return {};
  try { return JSON.parse(text); } catch(e) { return {}; }
}

async function _rawApiPost(queryString, body) {
  const r = await fetch(`${API_BASE}${queryString}`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
    signal: AbortSignal.timeout(_apiTimeout),
  });
  if (!r.ok) throw new Error(`API ${r.status}`);
  var text = await r.text();
  if (!text || !text.trim()) return {};
  try { return JSON.parse(text); } catch(e) { return {}; }
}

function apiGet(queryString, timeoutMs) {
  if (_apiQueue) return _apiQueue.enqueue(function() { return _rawApiGet(queryString, timeoutMs); });
  return _rawApiGet(queryString, timeoutMs);
}

function apiPost(queryString, body) {
  if (_apiQueue) return _apiQueue.enqueue(function() { return _rawApiPost(queryString, body); });
  return _rawApiPost(queryString, body);
}

async function fetchSampleList() {
  const kitIdx = state.kits.active_smp_bank || 0;
  const d = await apiGet(`?kit=${kitIdx}`, _sampleListTimeout);
  state.files    = d.files || [];
  state.kits     = d.kits  || state.kits;
  state.kitEntries = d.active_kit_entries || [];
  state.capacity = d.capacity || state.capacity;

  // Load bank metadata
  const meta = state.kits.smp_bank_meta;
  if (meta && meta[state.kits.active_smp_bank] && meta[state.kits.active_smp_bank].banks) {
    const banksMeta = meta[state.kits.active_smp_bank].banks;
    // Rebuild banks array from metadata
    state.banks = banksMeta.map((b, i) => ({
      name: b.name || (DEFAULT_BANKS[i] ? DEFAULT_BANKS[i].name : `BANK ${i + 1}`),
      color: b.color || (DEFAULT_BANKS[i] ? DEFAULT_BANKS[i].color : '#607D8B'),
      collapsed: state.banks[i] ? state.banks[i].collapsed : false,
    }));
  }

  // Derive unique folder list (merge file-derived + firmware-reported directories)
  const fset = new Set(state.files.map(f => f.path));
  // Also include directories reported by firmware (catches empty folders)
  if (d.directories) {
    for (const dir of d.directories) fset.add(dir);
  }
  state.folders = [...fset].filter(Boolean).sort();

  // Mark kit entries whose sample file is missing from the pool
  markMissingKitEntries();
}

/**
 * Mark each kit entry with _missing = true if its sample file
 * cannot be found in state.files (the current sample pool).
 */
function markMissingKitEntries() {
  // Build a Set of "path/filename" keys from available files
  const available = new Set();
  for (const f of state.files) {
    const key = f.path ? `${f.path}/${f.name}` : f.name;
    available.add(key);
  }
  for (let i = 0; i < state.kitEntries.length; i++) {
    const e = state.kitEntries[i];
    if (!e) continue;
    const key = e.path ? `${e.path}/${e.filename}` : e.filename;
    e._missing = !available.has(key);
  }
}

/**
 * Find all kit entries that reference a given file.
 * Returns array of { bankIdx, slotIdx, entry } for display in warnings.
 */
function findKitReferencesForFile(path, filename) {
  const refs = [];
  if (!state.kitEntries) return refs;
  const targetKey = path ? `${path}/${filename}` : filename;
  for (let i = 0; i < state.kitEntries.length; i++) {
    const e = state.kitEntries[i];
    if (!e) continue;
    const eKey = e.path ? `${e.path}/${e.filename}` : e.filename;
    if (eKey === targetKey) {
      const bankIdx = Math.floor(i / SLICES_PER_BANK);
      const slotIdx = i - bankIdx * SLICES_PER_BANK;
      refs.push({ bankIdx, slotIdx, entry: e });
    }
  }
  return refs;
}

function uploadSample(blob, path, filename, onProgress) {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    const url = `${API_BASE}?action=upload&path=${encodeURIComponent(path)}&filename=${encodeURIComponent(filename)}`;
    xhr.open('POST', url);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');
    xhr.upload.onprogress = e => {
      if (e.lengthComputable && onProgress) onProgress(Math.round(e.loaded / e.total * 100));
    };
    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 300) {
        try { resolve(JSON.parse(xhr.responseText)); }
        catch { resolve({ ok: true }); }
      } else { reject(new Error(`Upload ${xhr.status}`)); }
    };
    xhr.onerror = () => reject(new Error('Upload network error'));
    xhr.send(blob);
  });
}

async function renameSample(path, oldName, newName) {
  return apiPost('?action=manage', { action: 'rename', path, oldName, newName });
}

async function deleteSample(path, filename) {
  return apiPost('?action=manage', { action: 'delete', path, filename });
}

async function saveKitDescriptor(entries, banksMeta) {
  return apiPost('?action=manage', {
    action: 'saveKit',
    bankIndex: state.kits.active_smp_bank,
    entries,
    banksMeta,
  });
}

async function reloadPSRAM() {
  return apiPost('?action=reload', {});
}

async function createKitOnDevice(name, entries) {
  return apiPost('?action=manage', { action: 'createKit', name, entries });
}

async function createFolderOnDevice(folderPath) {
  return apiPost('?action=manage', { action: 'createFolder', path: folderPath });
}

async function deleteKitOnDevice(kitIndex) {
  return apiPost('?action=manage', { action: 'deleteKit', kitIndex });
}

async function renameFolderOnDevice(oldPath, newPath) {
  return apiPost('?action=manage', { action: 'renameFolder', oldPath, newPath });
}

async function deleteFolderOnDevice(folderPath) {
  return apiPost('?action=manage', { action: 'deleteFolder', path: folderPath });
}

// ═══════════════════════════════════════════════════════════════
//  KIT SAFETY — Check if folder operations affect any kit
// ═══════════════════════════════════════════════════════════════

/**
 * Find all kit entries that reference files within a given folder path.
 * Returns an array of { kitIndex, entryIndex, entry } objects.
 */
function findKitReferencesInFolder(folderPath) {
  const refs = [];
  if (!state.kitEntries) return refs;
  for (let i = 0; i < state.kitEntries.length; i++) {
    const e = state.kitEntries[i];
    if (!e || !e.path) continue;
    // Match exact folder or subfolder
    if (e.path === folderPath || e.path.startsWith(folderPath + '/')) {
      refs.push({ kitIndex: state.kits.active_smp_bank, entryIndex: i, entry: e });
    }
  }
  return refs;
}

/**
 * Update kit entries in-memory after a folder rename.
 * Changes all path references from oldPath to newPath.
 * Returns the number of entries updated.
 */
function updateKitPathsAfterRename(oldPath, newPath) {
  let count = 0;
  if (!state.kitEntries) return count;
  for (let i = 0; i < state.kitEntries.length; i++) {
    const e = state.kitEntries[i];
    if (!e || !e.path) continue;
    if (e.path === oldPath) {
      e.path = newPath;
      count++;
    } else if (e.path.startsWith(oldPath + '/')) {
      e.path = newPath + e.path.substring(oldPath.length);
      count++;
    }
  }
  return count;
}

// ═══════════════════════════════════════════════════════════════
//  UPLOAD QUEUE
// ═══════════════════════════════════════════════════════════════

function addToUploadQueue(fileList, targetPath, targetBank) {
  for (const file of fileList) {
    let name = sanitizeFilename(file.name);

    // De-duplicate
    let counter = 2;
    const taken = n =>
      state.files.some(f => f.name === n && f.path === targetPath) ||
      state.uploadQueue.some(q => q.sanitizedName === n && q.targetPath === targetPath && q.status !== 'error');
    while (taken(name)) {
      const suffix = '_' + counter++;
      const base = sanitizeFilename(file.name, MAX_FILENAME - suffix.length);
      name = base + suffix;
    }

    state.uploadQueue.push({
      id: ++uploadIdCounter,
      file,
      originalName: file.name,
      sanitizedName: name,
      targetPath,
      targetBank,
      status: 'queued',
      progress: 0,
      error: null,
    });
  }

  renderTransferBar();
  processUploadQueue();
}

async function processUploadQueue() {
  if (state.uploading) return;
  state.uploading = true;

  while (true) {
    const item = state.uploadQueue.find(q => q.status === 'queued');
    if (!item) break;

    try {
      item.status = 'converting';
      renderTransferBar();

      const result = await convertToWAV(item.file);

      if (result.byteLength > UPLOAD_SOFT_LIMIT) {
        toast(`${item.originalName}: large file (${formatBytes(result.byteLength)})`, 'warning');
      }

      item.status = 'uploading';
      item.byteLength = result.byteLength;
      renderTransferBar();

      await uploadSample(result.blob, item.targetPath, item.sanitizedName, pct => {
        item.progress = pct;
        renderTransferBar();
      });

      item.status = 'done';
      item.nsamples = result.nsamples;
      item.duration = result.duration;

      if (item.targetBank >= 0) {
        addEntryToBank(item.targetBank, item.sanitizedName, item.targetPath, result.nsamples);
      }

      // No per-file toast — the transfer panel provides sufficient feedback
    } catch (e) {
      item.status = 'error';
      item.error = e.message;
      toast(`Failed: ${item.originalName} — ${e.message}`, 'danger');
    }
    renderTransferBar();
  }

  state.uploading = false;

  // Refresh after batch
  try {
    await fetchSampleList();
    renderPoolContent();
    renderKitEditor();
    updateCapacityBar();
    updateStorageBar();
  } catch (e) {
    console.error('Refresh after upload failed:', e);
  }
}

// ═══════════════════════════════════════════════════════════════
//  AUDIO PREVIEW
// ═══════════════════════════════════════════════════════════════

async function playPreview(path, filename) {
  stopPreview();
  if (!state.audioCtx) {
    state.audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  }
  try {
    const url = `${API_BASE}?preview=${encodeURIComponent(path + '/' + filename)}`;
    const r = await (_apiQueue ? _apiQueue.enqueue(function() {
      return fetch(url, { signal: AbortSignal.timeout(_apiTimeout) });
    }) : fetch(url, { signal: AbortSignal.timeout(_apiTimeout) }));
    if (!r.ok) throw new Error(`Preview ${r.status}`);
    const ab = await r.arrayBuffer();
    const buf = await state.audioCtx.decodeAudioData(ab);
    const src = state.audioCtx.createBufferSource();
    src.buffer = buf;
    src.connect(state.audioCtx.destination);
    src.onended = () => { state.currentSource = null; };
    src.start(0);
    state.currentSource = src;
  } catch (e) {
    console.error('Preview error:', e);
    toast('Preview failed', 'danger');
  }
}

function stopPreview() {
  if (state.currentSource) {
    try { state.currentSource.stop(); } catch { /* ignore */ }
    state.currentSource = null;
  }
}

// ═══════════════════════════════════════════════════════════════
//  KIT / BANK LOGIC
// ═══════════════════════════════════════════════════════════════

function getBankEntries(bankIdx) {
  const start = bankIdx * SLICES_PER_BANK;
  const end   = start + SLICES_PER_BANK;
  const out = [];
  for (let i = start; i < end; i++) {
    if (i < state.kitEntries.length && state.kitEntries[i]) {
      out.push({ ...state.kitEntries[i], _idx: i });
    }
  }
  return out;
}

function addEntryToBank(bankIdx, filename, path, nsamp) {
  const start = bankIdx * SLICES_PER_BANK;
  const entries = getBankEntries(bankIdx);
  if (entries.length >= SLICES_PER_BANK) {
    toast(`Bank ${state.banks[bankIdx].name} is full`, 'warning');
    return false;
  }
  const pos = start + entries.length;
  while (state.kitEntries.length <= pos) state.kitEntries.push(null);
  state.kitEntries[pos] = { filename, path, nsamples: nsamp, sname: '' };
  return true;
}

function removeEntryFromBank(absIdx) {
  if (absIdx >= 0 && absIdx < state.kitEntries.length) {
    state.kitEntries[absIdx] = null;
    compactBank(Math.floor(absIdx / SLICES_PER_BANK));
  }
}

function compactBank(bankIdx) {
  const start = bankIdx * SLICES_PER_BANK;
  const entries = [];
  for (let i = start; i < start + SLICES_PER_BANK && i < state.kitEntries.length; i++) {
    if (state.kitEntries[i]) entries.push(state.kitEntries[i]);
  }
  while (state.kitEntries.length < start + SLICES_PER_BANK) state.kitEntries.push(null);
  for (let i = 0; i < SLICES_PER_BANK; i++) {
    state.kitEntries[start + i] = i < entries.length ? entries[i] : null;
  }
}

function reorderBankSlots(bankIdx, oldSlot, newSlot) {
  const start = bankIdx * SLICES_PER_BANK;
  const entries = getBankEntries(bankIdx).map(e => {
    const { _idx, ...rest } = e; return rest;
  });
  const [moved] = entries.splice(oldSlot, 1);
  entries.splice(newSlot, 0, moved);
  while (state.kitEntries.length < start + SLICES_PER_BANK) state.kitEntries.push(null);
  for (let i = 0; i < SLICES_PER_BANK; i++) {
    state.kitEntries[start + i] = i < entries.length ? entries[i] : null;
  }
}

function getKitForSave() {
  // Preserve null entries so bank slot positions are maintained.
  // Without this, a dense array would put all samples in bank 0 on reload.
  const maxIdx = state.banks.length * SLICES_PER_BANK;
  const result = [];
  for (let i = 0; i < maxIdx; i++) {
    if (i < state.kitEntries.length && state.kitEntries[i]) {
      result.push(state.kitEntries[i]);
    } else {
      result.push(null);
    }
  }
  // Trim trailing nulls for storage efficiency
  while (result.length > 0 && result[result.length - 1] === null) {
    result.pop();
  }
  return result;
}

function getBanksMeta() {
  return state.banks.map(b => ({ name: b.name, color: b.color }));
}

function markDirty() {
  state.dirty = true;
  updateSaveButton();
}

function updateSaveButton() {
  const btn = document.getElementById('save-kit-btn');
  if (!btn) return;
  if (state.dirty) {
    btn.variant = 'warning';
    btn.innerHTML = '<sl-icon slot="prefix" name="floppy"></sl-icon>Save *';
  } else {
    btn.variant = 'default';
    btn.innerHTML = '<sl-icon slot="prefix" name="floppy"></sl-icon>Save';
  }
}

async function saveKit() {
  try {
    await saveKitDescriptor(getKitForSave(), getBanksMeta());
    state.dirty = false;
    updateSaveButton();
    toast('Kit saved to SD card. Changes take effect on next power-up.', 'success', 5000);
  } catch (e) {
    toast(`Save failed: ${e.message}`, 'danger');
  }
}



function calculateUsedBytes() {
  let total = 0;
  for (const e of state.kitEntries) {
    if (e && e.nsamples) total += e.nsamples * 2;
  }
  return total;
}

// ─── Bank Management ────────────────────────────────────────

function addBank(name) {
  if (state.banks.length >= 32) {
    toast('Maximum 32 banks reached', 'warning');
    return;
  }
  const idx = state.banks.length;
  const color = BANK_COLORS[idx % BANK_COLORS.length];
  const bankName = name || getNextUniqueBankName();
  state.banks.push({ name: bankName, color, collapsed: false });
  // Extend kitEntries to cover the new bank's slots
  while (state.kitEntries.length < (idx + 1) * SLICES_PER_BANK) {
    state.kitEntries.push(null);
  }
  markDirty();
  renderKitEditor();
  toast(`Bank "${state.banks[idx].name}" added`, 'success');
}

function deleteBank(bankIdx) {
  if (state.banks.length <= 1) {
    toast('Cannot delete the last bank', 'warning');
    return;
  }
  const name = state.banks[bankIdx].name;
  const entries = getBankEntries(bankIdx);
  // Always show confirmation dialog
  openDeleteBankDialog(bankIdx, name, entries.length);
}

function performDeleteBank(bankIdx, name) {
  // Remove the bank's slots from kitEntries
  const start = bankIdx * SLICES_PER_BANK;
  state.kitEntries.splice(start, SLICES_PER_BANK);
  // Remove the bank from state
  state.banks.splice(bankIdx, 1);
  markDirty();
  renderKitEditor();
  toast(`Bank "${name}" deleted`, 'success');
}

/** Generate a unique sequential bank name that doesn't conflict with existing banks */
function getNextUniqueBankName() {
  const existingNames = new Set(state.banks.map(b => b.name));
  let num = state.banks.length + 1;
  while (existingNames.has(`BANK ${num}`)) { num++; }
  return `BANK ${num}`;
}

// ═══════════════════════════════════════════════════════════════
//  TOAST NOTIFICATIONS
//  When bundled with shared.js, toast() and iconForVariant() are
//  already defined with recursion guard (_toastBusy). We use var
//  assignment (not function declaration) so we DON'T overwrite
//  shared.js's guarded version during hoisting.
// ═══════════════════════════════════════════════════════════════

/* eslint-disable no-func-assign */
var iconForVariant = (typeof iconForVariant === 'function') ? iconForVariant : function(v) {
  if (_S && _S.iconForVariant) return _S.iconForVariant(v);
  return { success: 'check2-circle', warning: 'exclamation-triangle',
           danger: 'exclamation-octagon', primary: 'info-circle',
           neutral: 'info-circle' }[v] || 'info-circle';
};

var toast = (typeof toast === 'function') ? toast : function(message, variant, duration) {
  variant = variant || 'primary';
  duration = duration || 4000;
  if (_S && _S.toast) { _S.toast(message, variant, duration); return; }
  var stack = document.getElementById('toast-stack');
  if (!stack) return;
  var alert = document.createElement('sl-alert');
  alert.variant = variant;
  alert.closable = true;
  alert.duration = duration;
  alert.innerHTML = '<sl-icon slot="icon" name="' + iconForVariant(variant) + '"></sl-icon>' + esc(message);
  stack.appendChild(alert);
  customElements.whenDefined('sl-alert').then(function() {
    try { alert.toast(); } catch(e) { console.warn('toast failed:', e); }
  }).catch(function() {});
};

// ═══════════════════════════════════════════════════════════════
//  UI — PSRAM Capacity Bar (Kit Editor)
// ═══════════════════════════════════════════════════════════════

function updateCapacityBar() {
  const used = calculateUsedBytes();
  const max  = state.capacity.psram_max_bytes || PSRAM_MAX;
  const pct  = Math.min(100, Math.round(used / max * 100));
  const fill = document.getElementById('capacity-fill');
  const lbl  = document.getElementById('capacity-label');
  if (fill) {
    fill.style.width = `${pct}%`;
    fill.classList.remove('warn', 'danger');
    if (pct > 90)      fill.classList.add('danger');
    else if (pct > 70) fill.classList.add('warn');
  }
  if (lbl) {
    const usedMB = (used / 1048576).toFixed(1);
    const maxMB  = (max  / 1048576).toFixed(0);
    lbl.textContent = `${usedMB} MB / ${maxMB} MB`;
  }
}

// ═══════════════════════════════════════════════════════════════
//  UI — SD Card Storage Bar (Header)
// ═══════════════════════════════════════════════════════════════

function updateStorageBar() {
  const totalFiles = state.files.length;

  // Update pool file count
  const countEl = document.getElementById('pool-file-count');
  if (countEl) countEl.textContent = `${totalFiles} files`;

  // Update header storage display
  const textEl = document.getElementById('storage-text');
  const fillEl = document.getElementById('storage-bar-fill');
  if (!textEl || !fillEl) return;

  // Use real SD card info from firmware API if available
  const sd = state.capacity;
  const sdTotal = sd.sd_total_bytes || 0;
  const sdFree  = sd.sd_free_bytes  || 0;

  if (sdTotal > 0) {
    const sdUsed  = sdTotal - sdFree;
    const usedPct = Math.min(100, (sdUsed / sdTotal) * 100);
    const freePct = Math.round(100 - usedPct);
    textEl.textContent = `${freePct}% Free / ${formatBytes(sdTotal)}`;
    fillEl.style.width = `${usedPct}%`;
    fillEl.classList.remove('warn', 'danger');
    if (freePct < 10) fillEl.classList.add('danger');
    else if (freePct < 25) fillEl.classList.add('warn');
  } else {
    // Fallback: show file count only, no bar
    const totalBytes = state.files.reduce((sum, f) => sum + (f.size || 0), 0);
    textEl.textContent = `${formatBytes(totalBytes)} used`;
    fillEl.style.width = '0%';
  }
}

// ═══════════════════════════════════════════════════════════════
//  UI — Transfer Bar (bottom, Elektron-style)
// ═══════════════════════════════════════════════════════════════

function renderTransferBar() {
  const bar = document.getElementById('transfer-bar');
  const body = document.getElementById('transfer-body');
  const badge = document.getElementById('transfer-badge');
  const statusEl = document.getElementById('transfer-status');
  const actionsEl = document.getElementById('transfer-actions');

  const all = state.uploadQueue;
  const pending = all.filter(q => q.status === 'queued' || q.status === 'converting' || q.status === 'uploading');
  const done = all.filter(q => q.status === 'done');
  const errors = all.filter(q => q.status === 'error');

  // Badge — show count of pending transfers
  if (pending.length > 0) {
    badge.textContent = pending.length;
    badge.classList.remove('hidden');
    // Auto-expand when there are active transfers
    bar.classList.remove('collapsed');
    bar.classList.add('expanded');
  } else {
    badge.classList.add('hidden');
  }

  // Status text
  if (pending.length > 0) {
    const current = all.find(q => q.status === 'uploading' || q.status === 'converting');
    statusEl.textContent = current
      ? `${current.status === 'converting' ? 'Converting' : 'Uploading'} ${current.sanitizedName}…`
      : `${pending.length} queued`;
  } else if (all.length > 0) {
    statusEl.textContent = `${done.length} done` + (errors.length > 0 ? `, ${errors.length} failed` : '');
  } else {
    statusEl.textContent = '';
  }

  // Clear buttons in header (shown when there are items and nothing pending)
  if (all.length > 0 && pending.length === 0) {
    actionsEl.innerHTML =
      '<button data-act="clear-finished">Clear finished</button>' +
      '<button data-act="clear-all">Clear all</button>';
  } else if (all.length > 0) {
    // During transfers, only allow clear all
    actionsEl.innerHTML = '<button data-act="clear-all">Clear all</button>';
  } else {
    actionsEl.innerHTML = '';
  }

  // Empty state
  if (all.length === 0) {
    body.innerHTML = '<div class="transfer-empty">No transfers yet.</div>';
    return;
  }

  // Table — numbered rows, newest first
  const rows = [...all].reverse().map((item, i) => {
    const num = all.length - i;
    const statusMap = {
      queued: '<span style="color:var(--sl-color-neutral-500);">Waiting</span>',
      converting: '<span style="color:var(--sl-color-primary-600);">Converting…</span>',
      uploading: '<span style="color:var(--sl-color-primary-600);">Uploading</span>',
      done: '<span style="color:var(--sl-color-success-600);">Done</span>',
      error: '<span style="color:var(--sl-color-danger-600);">Failed</span>',
    };
    const statusHTML = statusMap[item.status] || item.status;
    const size = item.byteLength ? formatBytes(item.byteLength) : '—';
    const location = item.targetPath ? '/' + item.targetPath : '';
    const rowClass = item.status === 'done' ? ' class="transfer-row-done"'
      : item.status === 'error' ? ' class="transfer-row-error"' : '';

    let progressHTML = '';
    if (item.status === 'uploading') {
      progressHTML = `<sl-progress-bar value="${item.progress}"></sl-progress-bar>`;
    } else if (item.status === 'converting') {
      progressHTML = '<sl-progress-bar indeterminate></sl-progress-bar>';
    } else if (item.status === 'done') {
      progressHTML = '<sl-progress-bar value="100"></sl-progress-bar>';
    }

    return `<tr${rowClass}>
      <td class="col-num">${num}</td>
      <td class="col-file" title="${esc(item.originalName)}">${esc(item.sanitizedName)}</td>
      <td class="col-size">${size}</td>
      <td class="col-progress">${progressHTML}</td>
      <td class="col-status">${statusHTML}</td>
      <td class="col-location" title="${esc(location)}">${esc(location)}</td>
    </tr>`;
  }).join('');

  body.innerHTML = `<table class="transfer-table">
    <thead><tr>
      <th>#</th><th>NAME</th><th class="col-r">SIZE</th><th>PROGRESS</th><th>STATUS</th><th>LOCATION</th>
    </tr></thead>
    <tbody>${rows}</tbody>
  </table>`;
}

function clearTransferLog(mode = 'finished') {
  if (mode === 'all') {
    state.uploadQueue = [];
  } else {
    // Only clear completed/error items, keep pending
    state.uploadQueue = state.uploadQueue.filter(
      q => q.status === 'queued' || q.status === 'converting' || q.status === 'uploading'
    );
  }
  renderTransferBar();
}

// ═══════════════════════════════════════════════════════════════
//  MULTI-SELECT & BATCH DELETE
// ═══════════════════════════════════════════════════════════════

function toggleSelectionMode() {
  // Can only use selection mode in user-writable folders
  if (!state.selectionMode && !isInUserFolder()) {
    toast('Select is only available in the user folder', 'warning');
    return;
  }
  state.selectionMode = !state.selectionMode;
  if (!state.selectionMode) {
    state.selectedFiles.clear();
  }
  renderPoolContent();
}

function updateSelectionToolbar() {
  const bar = document.getElementById('selection-toolbar');
  if (!bar) return;
  const writable = isInUserFolder();
  if (state.selectionMode && writable) {
    const count = state.selectedFiles.size;
    bar.classList.add('active');
    bar.innerHTML = `
      <span class="sel-label">${count} selected</span>
      <button class="sel-btn" data-act="select-all">Select All</button>
      <button class="sel-btn" data-act="select-none">Deselect</button>
      <div style="flex:1;"></div>
      <button class="sel-btn ${count > 0 ? 'sel-btn-danger' : ''}" data-act="delete-selected" ${count === 0 ? 'disabled' : ''}>Delete ${count > 0 ? count + ' Files' : 'Selected'}</button>
      <button class="sel-btn" data-act="cancel-select">Cancel</button>`;
  } else {
    bar.classList.remove('active');
    bar.innerHTML = '';
  }
}

function setupSelectionToolbar() {
  const bar = document.getElementById('selection-toolbar');
  if (!bar) return;
  bar.addEventListener('click', e => {
    const btn = e.target.closest('[data-act]');
    if (!btn) return;
    const act = btn.dataset.act;
    if (act === 'select-all') {
      // Select all writable files in current folder
      const items = getPoolItems().filter(i => i.type === 'file' && isUserWritable(i.path));
      for (const item of items) {
        const stem = item.name.replace(/\.wav$/i, '');
        state.selectedFiles.add(`${item.path}/${stem}`);
      }
      renderPoolContent();
    } else if (act === 'select-none') {
      state.selectedFiles.clear();
      renderPoolContent();
    } else if (act === 'delete-selected') {
      openBatchDeleteDialog();
    } else if (act === 'cancel-select') {
      state.selectionMode = false;
      state.selectedFiles.clear();
      renderPoolContent();
    }
  });

  // Handle checkbox changes via delegation on pool content
  document.getElementById('pool-content').addEventListener('change', e => {
    if (e.target.matches('.pool-select-cb')) {
      const key = e.target.dataset.key;
      if (e.target.checked) {
        state.selectedFiles.add(key);
      } else {
        state.selectedFiles.delete(key);
      }
      updateSelectionToolbar();
    }
  });
}

function openBatchDeleteDialog() {
  const count = state.selectedFiles.size;
  if (count === 0) return;
  const dlg = document.getElementById('batch-delete-dialog');
  document.getElementById('batch-delete-msg').textContent =
    `Delete ${count} selected file(s)? This cannot be undone.`;
  dlg.show();
}

function setupBatchDeleteDialog() {
  const dlg = document.getElementById('batch-delete-dialog');
  const ok  = document.getElementById('batch-delete-ok');
  const can = document.getElementById('batch-delete-cancel');

  ok.addEventListener('click', async () => {
    dlg.hide();
    const keys = [...state.selectedFiles];
    let deleted = 0, failed = 0;
    for (const key of keys) {
      const lastSlash = key.lastIndexOf('/');
      const path = key.substring(0, lastSlash);
      const name = key.substring(lastSlash + 1);
      try {
        await deleteSample(path, name);
        deleted++;
      } catch {
        failed++;
      }
    }
    state.selectedFiles.clear();
    state.selectionMode = false;
    toast(`Deleted ${deleted} file(s)${failed > 0 ? `, ${failed} failed` : ''}`,
          failed > 0 ? 'warning' : 'success');
    await fetchSampleList();
    renderPoolContent();
    renderKitEditor();
    updateCapacityBar();
    updateStorageBar();
  });
  can.addEventListener('click', () => dlg.hide());
}

function setupTransferBar() {
  document.getElementById('transfer-bar-header').addEventListener('click', e => {
    // Don't toggle when clicking clear buttons
    if (e.target.closest('.transfer-bar-actions')) return;
    const bar = document.getElementById('transfer-bar');
    bar.classList.toggle('collapsed');
    bar.classList.toggle('expanded');
  });
  // Delegate click for the clear buttons (in header)
  document.getElementById('transfer-actions').addEventListener('click', e => {
    e.stopPropagation();
    const btn = e.target.closest('[data-act]');
    if (!btn) return;
    if (btn.dataset.act === 'clear-all') clearTransferLog('all');
    else if (btn.dataset.act === 'clear-finished') clearTransferLog('finished');
  });
}

// ═══════════════════════════════════════════════════════════════
//  UI — Pool Content (Folder Navigation)
// ═══════════════════════════════════════════════════════════════

/**
 * Get items to display for the current poolPath.
 * At root: show folders + root-level files.
 * Inside a folder: show files in that folder.
 */
function getPoolItems() {
  const currentPath = state.poolPath;
  let folders = [];
  let files = [];

  // Pre-compute folder sizes and file counts recursively under each folder path
  const folderSizeMap = {};
  const folderCountMap = {};
  for (const f of state.files) {
    if (!f.path) continue;
    const parts = f.path.split('/').filter(Boolean);
    let accum = '';
    for (const seg of parts) {
      accum = accum ? accum + '/' + seg : seg;
      folderSizeMap[accum] = (folderSizeMap[accum] || 0) + (f.size || 0);
      folderCountMap[accum] = (folderCountMap[accum] || 0) + 1;
    }
  }

  if (currentPath === '') {
    // Root level: show unique top-level folders + root files
    const topFolders = new Set();
    const rootFiles = [];
    for (const f of state.files) {
      const parts = f.path.split('/').filter(Boolean);
      if (parts.length > 0) {
        topFolders.add(parts[0]);
      }
      if (!f.path || f.path === '/' || f.path === '.') {
        rootFiles.push(f);
      }
    }
    // Also include top-level entries from state.folders (catches empty folders)
    for (const fp of state.folders) {
      const top = fp.split('/').filter(Boolean)[0];
      if (top) topFolders.add(top);
    }
    folders = [...topFolders].sort().map(name => ({
      type: 'folder', name, path: name, size: folderSizeMap[name] || 0, fileCount: folderCountMap[name] || 0,
    }));
    files = rootFiles.map(f => ({ type: 'file', ...f }));
  } else {
    // Inside a folder: show sub-folders and files at this level
    const prefix = currentPath;
    const subFolders = new Set();

    for (const f of state.files) {
      if (f.path === prefix) {
        files.push({ type: 'file', ...f });
      } else if (f.path.startsWith(prefix + '/')) {
        const rest = f.path.substring(prefix.length + 1);
        const nextSeg = rest.split('/')[0];
        if (nextSeg) subFolders.add(nextSeg);
      }
    }
    // Also include sub-folder entries from state.folders (catches empty sub-folders)
    for (const fp of state.folders) {
      if (fp.startsWith(prefix + '/')) {
        const rest = fp.substring(prefix.length + 1);
        const nextSeg = rest.split('/')[0];
        if (nextSeg) subFolders.add(nextSeg);
      }
    }
    folders = [...subFolders].sort().map(name => ({
      type: 'folder', name, path: prefix + '/' + name, size: folderSizeMap[prefix + '/' + name] || 0, fileCount: folderCountMap[prefix + '/' + name] || 0,
    }));
  }

  // Sort files
  const { key, dir } = state.poolSort;
  const mul = dir === 'asc' ? 1 : -1;
  files.sort((a, b) => {
    if (key === 'name') return mul * a.name.localeCompare(b.name);
    if (key === 'dur')  return mul * ((a.size || 0) - (b.size || 0));
    if (key === 'smp')  return mul * ((a.size || 0) - (b.size || 0));
    return 0;
  });

  return [...folders, ...files];
}

function renderBreadcrumb() {
  const el = document.getElementById('pool-breadcrumb');
  const homeHtml = '<sl-icon name="hdd" class="pool-breadcrumb-home" data-nav=""></sl-icon><span class="pool-breadcrumb-home-label" data-nav="">TBD-16 SD Card</span>';
  if (state.poolPath === '') {
    el.innerHTML = homeHtml;
    return;
  }

  const parts = state.poolPath.split('/').filter(Boolean);
  let html = homeHtml;
  for (let i = 0; i < parts.length; i++) {
    const pathSoFar = parts.slice(0, i + 1).join('/');
    html += '<span class="pool-breadcrumb-sep">/</span>';
    if (i < parts.length - 1) {
      html += `<span class="pool-breadcrumb-seg" data-nav="${esc(pathSoFar)}">${esc(parts[i])}</span>`;
    } else {
      html += `<span class="pool-breadcrumb-current">${esc(parts[i])}</span>`;
    }
  }
  el.innerHTML = html;
}

function renderPoolContent() {
  const wrap = document.getElementById('pool-content');
  const items = getPoolItems();

  renderBreadcrumb();
  updateSelectionToolbar();

  if (items.length === 0) {
    wrap.innerHTML = '<div style="color:var(--sl-color-neutral-400);font-size:0.8rem;padding:1rem;text-align:center;">Empty folder.</div>';
    updateStorageBar();
    return;
  }

  const inUserArea = isInUserFolder();

  wrap.innerHTML = items.map(item => {
    if (item.type === 'folder') {
      // Show rename/delete only for sub-folders inside user folder, NOT for the user root itself
      const folderEditable = isUserWritableChild(item.path);
      const folderActions = folderEditable
        ? `<sl-icon-button name="pencil" label="Rename folder" class="action-hover" data-act="rename-folder" data-folder-path="${esc(item.path)}" data-folder-name="${esc(item.name)}" onclick="event.stopPropagation();"></sl-icon-button>
           <sl-icon-button name="trash3" label="Delete folder" class="action-hover" data-act="delete-folder" data-folder-path="${esc(item.path)}" data-folder-name="${esc(item.name)}" onclick="event.stopPropagation();"></sl-icon-button>`
        : '';
      const countLabel = item.fileCount ? `${item.fileCount} sample${item.fileCount !== 1 ? 's' : ''}` : '';
      return `<div class="sample-row folder-row" data-nav="${esc(item.path)}" data-folder-path="${esc(item.path)}" draggable="true">
        <sl-icon name="folder2-open" class="sample-row-icon"></sl-icon>
        <span class="sample-row-name">${esc(item.name)}</span>
        <span class="sample-row-dur" style="opacity:0.5;font-size:0.72rem;">${countLabel}</span>
        <span class="sample-row-smp" style="opacity:0.6">${item.size ? formatBytes(item.size) : ''}</span>
        <div class="sample-row-actions">
          ${folderActions}
          <sl-icon-button name="chevron-right" label="Open" style="font-size:0.85rem;color:var(--sl-color-neutral-400);"></sl-icon-button>
        </div>
      </div>`;
    }

    const stem = item.name.replace(/\.wav$/i, '');
    const durStr = item.size ? fileDuration(item.size) : '';
    const fileWritable = isUserWritable(item.path);
    const fileKey = `${item.path}/${stem}`;
    const isSelected = state.selectedFiles.has(fileKey);

    // Checkbox for multi-select (only in user-writable areas)
    const checkboxHTML = (state.selectionMode && fileWritable)
      ? `<input type="checkbox" class="pool-select-cb" data-key="${esc(fileKey)}" ${isSelected ? 'checked' : ''} onclick="event.stopPropagation();">`
      : '';

    const editActions = fileWritable
      ? `<sl-icon-button name="pencil"     label="Rename" class="action-hover" data-act="rename"   data-path="${esc(item.path)}" data-name="${esc(stem)}"></sl-icon-button>
         <sl-icon-button name="trash3"     label="Delete" class="action-hover" data-act="delete"   data-path="${esc(item.path)}" data-name="${esc(stem)}"></sl-icon-button>`
      : '';
    return `<div class="sample-row file-row pool-file${isSelected ? ' selected' : ''}"
        data-path="${esc(item.path)}" data-name="${esc(stem)}" data-size="${item.size}"
        draggable="true">
      ${checkboxHTML}
      <sl-icon name="music-note-beamed" class="sample-row-icon"></sl-icon>
      <span class="sample-row-name" title="${esc(stem)}">${esc(stem)}</span>
      <span class="sample-row-dur">${durStr}</span>
      <span class="sample-row-smp">${item.size ? formatBytes(item.size) : ''}</span>
      <div class="sample-row-actions">
        <sl-icon-button name="play-fill"  label="Preview" data-act="preview"  data-path="${esc(item.path)}" data-name="${esc(stem)}"></sl-icon-button>
        ${editActions}
      </div>
    </div>`;
  }).join('');

  updateSortArrows();
  updateStorageBar();
}

function navigatePool(path) {
  state.poolPath = path;
  renderPoolContent();
  updateDropZoneTarget();
}

function updateSortArrows() {
  const header = document.getElementById('pool-col-header');
  if (!header) return;
  header.querySelectorAll('.col-sortable').forEach(el => {
    const arrow = el.querySelector('.sort-arrow');
    const key = el.dataset.sort;
    if (key === state.poolSort.key) {
      arrow.textContent = state.poolSort.dir === 'asc' ? '\u25B2' : '\u25BC';
    } else {
      arrow.textContent = '';
    }
  });
}

function setupColumnSort() {
  const header = document.getElementById('pool-col-header');
  if (!header) return;
  header.addEventListener('click', e => {
    const col = e.target.closest('.col-sortable');
    if (!col) return;
    const key = col.dataset.sort;
    if (state.poolSort.key === key) {
      state.poolSort.dir = state.poolSort.dir === 'asc' ? 'desc' : 'asc';
    } else {
      state.poolSort.key = key;
      state.poolSort.dir = 'asc';
    }
    renderPoolContent();
  });
}

// ═══════════════════════════════════════════════════════════════
//  UI — Kit Selector
// ═══════════════════════════════════════════════════════════════

function renderKitSelector() {
  const sel = document.getElementById('kit-select');
  const names = state.kits.smp_bank_names || [];
  const activeIdx = state.kits.active_smp_bank || 0;
  if (names.length === 0) {
    sel.innerHTML = '<sl-option value="0">Default</sl-option>';
    customElements.whenDefined('sl-select').then(() => {
      requestAnimationFrame(() => { sel.value = '0'; });
    });
    return;
  }
  sel.innerHTML = names.map((n, i) =>
    `<sl-option value="${i}">${esc(n)}${i === activeIdx ? ' \u25CF' : ''}</sl-option>`
  ).join('');
  const kitVal = String(activeIdx);
  customElements.whenDefined('sl-select').then(() => {
    requestAnimationFrame(() => { sel.value = kitVal; });
  });
  // Update PSRAM active label
  const label = document.getElementById('psram-active-label');
  if (label) {
    const activeName = names[activeIdx] || 'Default';
    label.textContent = `PSRAM: ${activeName}`;
    label.title = `Kit "${activeName}" is loaded in PSRAM`;
  }
}

// ═══════════════════════════════════════════════════════════════
//  UI — Kit Editor (Right Panel)
// ═══════════════════════════════════════════════════════════════

function renderKitEditor() {
  renderKitSelector();
  const container = document.getElementById('bank-container');
  if (state.viewMode === 'banked') {
    renderBankedView(container);
  } else {
    renderFlatView(container);
  }
  setupBankSortables();
  updateCapacityBar();
}

function renderBankedView(container) {
  let html = '';
  for (let i = 0; i < state.banks.length; i++) {
    const bank = state.banks[i];
    const entries = getBankEntries(i);
    const cls = bank.collapsed ? ' collapsed' : '';
    const displayIdx = String(i + 1).padStart(2, '0');
    html += `<div class="bank-card${cls}" data-bank="${i}">
      <div class="bank-header" data-act="toggle-bank" data-bank="${i}"
           style="background:color-mix(in srgb, ${bank.color} 12%, transparent);border-left:3px solid ${bank.color};">
        <div class="bank-header-left">
          <sl-icon name="chevron-down" class="bank-chevron"></sl-icon>
          <span class="bank-idx">${displayIdx}</span>
          <input class="bank-name-input" value="${esc(bank.name)}"
                 data-act="edit-bank-name" data-bank="${i}" spellcheck="false"
                 onclick="event.stopPropagation();">
          <span class="bank-count">${entries.length}/${SLICES_PER_BANK}</span>
        </div>
        <div class="bank-header-right">
          <sl-icon-button name="trash3" label="Delete bank" class="bank-delete-btn"
                         data-act="delete-bank" data-bank="${i}"></sl-icon-button>
        </div>
      </div>
      <div class="bank-body" data-bank="${i}" data-drop="bank">
        ${entries.length === 0
          ? '<div class="bank-empty">Empty — drop samples here</div>'
          : renderBankSlots(entries, i)}
        <sl-button class="bank-add-btn" variant="text" size="small"
                   data-act="add-sample" data-bank="${i}">
          <sl-icon slot="prefix" name="plus-lg"></sl-icon>Add sample
        </sl-button>
      </div>
    </div>`;
  }
  html += `<button class="add-bank-btn" data-act="add-bank">
    <sl-icon name="plus-circle"></sl-icon> Add Bank
  </button>`;
  container.innerHTML = html;
}

function renderFlatView(container) {
  const allEntries = [];
  for (let i = 0; i < state.kitEntries.length; i++) {
    if (state.kitEntries[i]) allEntries.push({ ...state.kitEntries[i], _idx: i });
  }
  if (allEntries.length === 0) {
    container.innerHTML = '<div class="bank-empty" style="margin-top:1rem;">No samples in this Kit.</div>';
    return;
  }
  let html = '<div class="bank-body" data-bank="0" data-drop="bank">';
  html += allEntries.map(e => {
    const size = e.nsamples ? formatBytes(e.nsamples * 2 + 44) : '?';
    const isMissing = e._missing;
    const missingCls = isMissing ? ' bank-slot-missing' : '';
    const missingIcon = isMissing ? '<sl-icon name="exclamation-triangle" class="bank-slot-missing-icon" title="Sample not found — drag a replacement here"></sl-icon>' : '';
    const previewBtn = isMissing
      ? '<sl-icon-button name="play-fill" label="Preview" disabled style="opacity:0.3;"></sl-icon-button>'
      : `<sl-icon-button name="play-fill" label="Preview" data-act="preview-kit" data-index="${e._idx}"></sl-icon-button>`;
    // Compute bank index & name for this entry
    const bankIdx = Math.floor(e._idx / SLICES_PER_BANK);
    const bank = state.banks[bankIdx];
    const bankName = bank ? bank.name : 'Bank ' + (bankIdx + 1);
    const bankColor = bank ? bank.color : '#888';
    const bankNum = String(bankIdx + 1).padStart(2, '0');
    const bankBadge = `<span class="flat-bank-badge" style="background:color-mix(in srgb, ${bankColor} 15%, transparent);color:${bankColor};border:1px solid color-mix(in srgb, ${bankColor} 30%, transparent);">${esc(bankNum + ' ' + bankName)}</span>`;
    return `<div class="bank-slot${missingCls}" data-index="${e._idx}" data-bank="0" title="${isMissing ? 'Missing sample — drag a file from the pool to replace' : ''}">
      <span class="drag-handle">⫶</span>
      <sl-icon name="music-note-beamed" class="sample-row-icon"></sl-icon>
      <span class="bank-slot-name" title="${esc(e.filename)}">${missingIcon}${esc(e.sname || e.filename)}</span>
      ${bankBadge}
      <span class="bank-slot-dur">${size}</span>
      <div class="bank-slot-actions">
        ${previewBtn}
        <sl-icon-button name="pencil"    label="Edit" class="action-hover" data-act="edit-sname"   data-index="${e._idx}"></sl-icon-button>
        <sl-icon-button name="x-lg"      label="Remove" class="action-hover" data-act="remove-slot"  data-index="${e._idx}"></sl-icon-button>
      </div>
    </div>`;
  }).join('');
  html += '</div>';
  container.innerHTML = html;
}

function renderBankSlots(entries, bankIdx) {
  return entries.map((e, slotIdx) => {
    const size = e.nsamples ? formatBytes(e.nsamples * 2 + 44) : '?';
    const absIdx = bankIdx * SLICES_PER_BANK + slotIdx;
    const isMissing = e._missing;
    const missingCls = isMissing ? ' bank-slot-missing' : '';
    const missingIcon = isMissing ? '<sl-icon name="exclamation-triangle" class="bank-slot-missing-icon" title="Sample not found — drag a replacement here"></sl-icon>' : '';
    const previewBtn = isMissing
      ? '<sl-icon-button name="play-fill" label="Preview" disabled style="opacity:0.3;"></sl-icon-button>'
      : `<sl-icon-button name="play-fill" label="Preview" data-act="preview-kit" data-index="${absIdx}"></sl-icon-button>`;
    return `<div class="bank-slot${missingCls}" data-index="${absIdx}" data-bank="${bankIdx}" title="${isMissing ? 'Missing sample — drag a file from the pool to replace' : ''}">
      <span class="drag-handle">⫶</span>
      <sl-icon name="music-note-beamed" class="sample-row-icon"></sl-icon>
      <span class="bank-slot-name" title="${esc(e.filename)}">${missingIcon}${esc(e.sname || e.filename)}</span>
      <span class="bank-slot-dur">${size}</span>
      <div class="bank-slot-actions">
        ${previewBtn}
        <sl-icon-button name="pencil"    label="Edit" class="action-hover" data-act="edit-sname"   data-index="${absIdx}"></sl-icon-button>
        <sl-icon-button name="x-lg"      label="Remove" class="action-hover" data-act="remove-slot"  data-index="${absIdx}"></sl-icon-button>
      </div>
    </div>`;
  }).join('');
}

// ═══════════════════════════════════════════════════════════════
//  SORTABLE.JS
// ═══════════════════════════════════════════════════════════════

function setupBankSortables() {
  for (const s of state.sortableInstances) { try { s.destroy(); } catch {} }
  state.sortableInstances = [];

  const bodies = document.querySelectorAll('.bank-body');
  for (const body of bodies) {
    const bankIdx = parseInt(body.dataset.bank, 10);
    if (typeof Sortable === 'undefined') break;
    const inst = Sortable.create(body, {
      handle: '.drag-handle',
      animation: 150,
      ghostClass: 'sortable-ghost',
      chosenClass: 'sortable-chosen',
      draggable: '.bank-slot',
      onEnd(evt) {
        if (evt.oldIndex !== evt.newIndex) {
          reorderBankSlots(bankIdx, evt.oldIndex, evt.newIndex);
          markDirty();
          renderKitEditor();
        }
      },
    });
    state.sortableInstances.push(inst);
  }
}

// ═══════════════════════════════════════════════════════════════
//  DRAG & DROP: Pool → Bank
// ═══════════════════════════════════════════════════════════════

function setupPoolDragEvents() {
  const poolPanel = document.getElementById('pool-panel');

  poolPanel.addEventListener('dragstart', e => {
    // Handle folder drag
    const folderRow = e.target.closest('.folder-row[data-folder-path]');
    if (folderRow) {
      const folderPath = folderRow.dataset.folderPath;
      const filesInFolder = state.files.filter(f => f.path === folderPath || f.path.startsWith(folderPath + '/'));
      if (filesInFolder.length === 0) return;
      const samples = filesInFolder.map(f => ({
        name: f.name.replace(/\.wav$/i, ''),
        path: f.path,
        size: f.size || 0,
      }));
      e.dataTransfer.setData('application/x-tbd-samples', JSON.stringify(samples));
      e.dataTransfer.effectAllowed = 'copy';
      return;
    }

    const row = e.target.closest('.pool-file');
    if (!row) return;

    // Multi-select drag: if in selection mode and dragged item is selected, drag all selected
    const draggedKey = `${row.dataset.path}/${row.dataset.name}`;
    if (state.selectionMode && state.selectedFiles.size > 0 && state.selectedFiles.has(draggedKey)) {
      const samples = [];
      for (const key of state.selectedFiles) {
        const lastSlash = key.lastIndexOf('/');
        const path = key.substring(0, lastSlash);
        const name = key.substring(lastSlash + 1);
        const file = state.files.find(f => f.path === path && f.name === name);
        samples.push({ name, path, size: file ? file.size : 0 });
      }
      e.dataTransfer.setData('application/x-tbd-samples', JSON.stringify(samples));
      e.dataTransfer.effectAllowed = 'copy';
      return;
    }

    // Single file drag
    e.dataTransfer.setData('application/x-tbd-sample', JSON.stringify({
      name: row.dataset.name,
      path: row.dataset.path,
      size: parseInt(row.dataset.size, 10),
    }));
    e.dataTransfer.effectAllowed = 'copy';
  });

  const kitPanel = document.getElementById('kit-panel');
  kitPanel.addEventListener('dragover', e => {
    // Allow drop on individual bank slots (replace) or bank body (add)
    const slot = e.target.closest('.bank-slot');
    const body = e.target.closest('[data-drop="bank"]');
    if (!slot && !body) return;
    e.preventDefault();
    e.dataTransfer.dropEffect = 'copy';
    // Only highlight slot for single-sample drops
    const isMulti = e.dataTransfer.types.includes('application/x-tbd-samples');
    if (slot && !isMulti) {
      slot.classList.add('drop-target-slot');
    } else if (body) {
      body.classList.add('drop-target');
    }
  });
  kitPanel.addEventListener('dragleave', e => {
    const slot = e.target.closest('.bank-slot');
    const body = e.target.closest('[data-drop="bank"]');
    if (slot) slot.classList.remove('drop-target-slot');
    if (body) body.classList.remove('drop-target');
  });
  kitPanel.addEventListener('drop', e => {
    // Clear all highlights
    kitPanel.querySelectorAll('.drop-target-slot').forEach(el => el.classList.remove('drop-target-slot'));
    kitPanel.querySelectorAll('.drop-target').forEach(el => el.classList.remove('drop-target'));

    // Handle multi-sample drop (selected files or folder)
    const rawMulti = e.dataTransfer.getData('application/x-tbd-samples');
    if (rawMulti) {
      e.preventDefault();
      try {
        const samples = JSON.parse(rawMulti);
        const body = e.target.closest('[data-drop="bank"]');
        if (!body) return;
        const bankIdx = parseInt(body.dataset.bank, 10);
        let added = 0;
        for (const data of samples) {
          const nsamp = nsamples(data.size);
          if (addEntryToBank(bankIdx, data.name, data.path, nsamp)) added++;
        }
        if (added > 0) {
          markDirty();
          renderKitEditor();
          toast(`Added ${added} sample(s) to ${state.banks[bankIdx].name}`, 'success');
        }
      } catch (err) {
        console.error('Multi-drop parse error:', err);
      }
      return;
    }

    const raw = e.dataTransfer.getData('application/x-tbd-sample');
    if (!raw) return;
    e.preventDefault();

    try {
      const data = JSON.parse(raw);
      const nsamp = nsamples(data.size);

      // Check if dropped onto a specific slot (replace)
      const slot = e.target.closest('.bank-slot');
      if (slot && slot.dataset.index !== undefined) {
        const absIdx = parseInt(slot.dataset.index, 10);
        if (absIdx >= 0 && absIdx < state.kitEntries.length) {
          state.kitEntries[absIdx] = { filename: data.name, path: data.path, nsamples: nsamp, sname: '' };
          markMissingKitEntries();
          markDirty();
          renderKitEditor();
          toast(`Replaced slot with ${data.name}`, 'success');
          return;
        }
      }

      // Otherwise add to bank
      const body = e.target.closest('[data-drop="bank"]');
      if (!body) return;
      const bankIdx = parseInt(body.dataset.bank, 10);
      if (addEntryToBank(bankIdx, data.name, data.path, nsamp)) {
        markDirty();
        renderKitEditor();
        toast(`Added ${data.name} to ${state.banks[bankIdx].name}`, 'success');
      }
    } catch (err) {
      console.error('Drop parse error:', err);
    }
  });
}

// ═══════════════════════════════════════════════════════════════
//  DROP ZONE — Drag-and-Drop Folder Helpers
// ═══════════════════════════════════════════════════════════════

/** Read all entries from a FileSystemDirectoryReader (Chrome batches in 100s) */
function readAllDirectoryEntries(dirReader) {
  return new Promise((resolve, reject) => {
    const all = [];
    const readBatch = () => {
      dirReader.readEntries(entries => {
        if (entries.length === 0) resolve(all);
        else { all.push(...entries); readBatch(); }
      }, reject);
    };
    readBatch();
  });
}

/** Get a File object from a FileSystemFileEntry */
function fileFromEntry(entry) {
  return new Promise((resolve, reject) => entry.file(resolve, reject));
}

/** Recursively collect files from FileSystemEntry objects, annotating each with _relativePath */
async function collectFilesFromEntries(entries, relPath, results) {
  for (const entry of entries) {
    if (entry.isFile) {
      try {
        const file = await fileFromEntry(entry);
        file._relativePath = relPath ? relPath + '/' + entry.name : entry.name;
        results.push(file);
      } catch (e) { /* skip unreadable files */ }
    } else if (entry.isDirectory) {
      const subPath = relPath ? relPath + '/' + entry.name : entry.name;
      const reader = entry.createReader();
      const subEntries = await readAllDirectoryEntries(reader);
      await collectFilesFromEntries(subEntries, subPath, results);
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  DROP ZONE — File Upload
// ═══════════════════════════════════════════════════════════════

function setupDropZone() {
  const poolPanel = document.getElementById('pool-panel');
  const overlay = document.getElementById('pool-drop-overlay');
  const bar = document.getElementById('drop-zone-bar');
  const input = document.getElementById('file-input');
  const uploadBtn = document.getElementById('upload-btn');
  let dragCounter = 0;

  // Prevent defaults on whole page
  for (const evt of ['dragenter', 'dragover', 'dragleave', 'drop']) {
    document.body.addEventListener(evt, e => e.preventDefault(), false);
  }

  // Show overlay when dragging files over the pool panel
  poolPanel.addEventListener('dragenter', e => {
    // Only react to external file drags, not internal sample drags
    if (e.dataTransfer.types.includes('Files')) {
      dragCounter++;
      overlay.classList.add('visible');
    }
  });
  poolPanel.addEventListener('dragleave', e => {
    if (e.dataTransfer.types.includes('Files')) {
      dragCounter--;
      if (dragCounter <= 0) {
        dragCounter = 0;
        overlay.classList.remove('visible');
      }
    }
  });
  poolPanel.addEventListener('dragover', e => {
    if (e.dataTransfer.types.includes('Files')) {
      e.preventDefault();
      e.dataTransfer.dropEffect = 'copy';
    }
  });
  poolPanel.addEventListener('drop', async e => {
    dragCounter = 0;
    overlay.classList.remove('visible');
    e.preventDefault();
    e.stopPropagation();

    // Check for dropped folders via File System Entry API
    const items = e.dataTransfer.items ? Array.from(e.dataTransfer.items) : [];
    const entries = items.map(i => i.webkitGetAsEntry ? i.webkitGetAsEntry() : null).filter(Boolean);
    const hasDirs = entries.some(entry => entry && entry.isDirectory);

    if (hasDirs) {
      // Recursively collect files from dropped folders, preserving paths
      const collected = [];
      for (const entry of entries) {
        if (entry.isDirectory) {
          const reader = entry.createReader();
          const children = await readAllDirectoryEntries(reader);
          await collectFilesFromEntries(children, entry.name, collected);
        } else if (entry.isFile) {
          const file = await fileFromEntry(entry);
          file._relativePath = entry.name;
          collected.push(file);
        }
      }
      if (collected.length > 0) handleDroppedFiles(collected);
    } else if (e.dataTransfer.files.length > 0) {
      handleDroppedFiles(e.dataTransfer.files);
    }
  });

  // Bottom bar click and upload button
  const inputFiles = document.getElementById('file-input-files');
  bar.addEventListener('click', () => inputFiles.click());
  uploadBtn.addEventListener('click', e => {
    // Show a context menu with file/folder upload options
    const menu = document.getElementById('upload-menu');
    if (menu) {
      menu.style.display = menu.style.display === 'block' ? 'none' : 'block';
      // Position below the upload button
      const rect = uploadBtn.getBoundingClientRect();
      menu.style.top = (rect.bottom + 2) + 'px';
      menu.style.left = (rect.right - menu.offsetWidth) + 'px';
      e.stopPropagation();
      // Close on outside click
      const closeMenu = () => { menu.style.display = 'none'; document.removeEventListener('click', closeMenu); };
      setTimeout(() => document.addEventListener('click', closeMenu), 0);
    } else {
      inputFiles.click();
    }
  });
  // "Upload Files" option
  const uploadFilesOpt = document.getElementById('upload-files-opt');
  if (uploadFilesOpt) uploadFilesOpt.addEventListener('click', () => { inputFiles.click(); });
  // "Upload Folder" option
  const uploadFolderOpt = document.getElementById('upload-folder-opt');
  if (uploadFolderOpt) uploadFolderOpt.addEventListener('click', () => { input.click(); });
  // File input listeners
  input.addEventListener('change', () => {
    if (input.files.length > 0) {
      handleDroppedFiles(input.files);
      input.value = '';
    }
  });
  inputFiles.addEventListener('change', () => {
    if (inputFiles.files.length > 0) {
      handleDroppedFiles(inputFiles.files);
      inputFiles.value = '';
    }
  });
}

function handleDroppedFiles(fileList) {
  // At root level, reject uploads — user must navigate into a folder first
  if (!state.poolPath) {
    toast('Navigate into a folder before uploading. Uploads go to the currently viewed folder.', 'warning', 5000);
    return;
  }
  const targetPath = state.poolPath;
  // Collect valid audio files from the FileList (supports folder uploads via webkitdirectory)
  const validExts = ['.wav', '.mp3', '.aiff', '.aif', '.ogg', '.flac', '.m4a', '.wma'];
  const validFiles = Array.from(fileList).filter(f => {
    const name = f.name.toLowerCase();
    return validExts.some(ext => name.endsWith(ext)) && f.size > 0;
  });
  if (validFiles.length === 0) {
    toast('No valid audio files found. Supported: WAV, MP3, AIFF, OGG, FLAC', 'warning', 5000);
    return;
  }

  // Check if any files come from subfolders (webkitRelativePath or _relativePath from drag-drop)
  const hasSubfolders = validFiles.some(f => {
    const rel = f.webkitRelativePath || f._relativePath || '';
    const parts = rel.split('/');
    return parts.length > 2; // root-folder / subfolder / file
  });

  if (hasSubfolders) {
    // Auto-preserve folder structure — no confirmation needed
    const subfolderSet = new Set();
    for (const f of validFiles) {
      const rel = f.webkitRelativePath || f._relativePath || '';
      const parts = rel.split('/');
      if (parts.length > 2) subfolderSet.add(parts.slice(1, -1).join('/'));
    }
    toast(`Uploading ${validFiles.length} files — preserving ${subfolderSet.size} subfolder(s)`, 'primary', 4000);
    uploadWithSubfolders(validFiles, targetPath);
  } else {
    addToUploadQueue(validFiles, targetPath, state.targetBank);
  }
}

/**
 * Upload files preserving their subfolder structure.
 * Creates necessary subfolders on the device first, then queues uploads.
 */
async function uploadWithSubfolders(validFiles, basePath) {
  // Collect unique subfolder paths to create
  const subfolderPaths = new Set();
  for (const f of validFiles) {
    const rel = f.webkitRelativePath || f._relativePath || '';
    const parts = rel.split('/');
    if (parts.length > 2) {
      // Build cumulative subfolder paths for recursive creation
      for (let depth = 2; depth < parts.length; depth++) {
        const sub = parts.slice(1, depth).join('/');
        subfolderPaths.add(basePath + '/' + sub);
      }
    }
  }

  // Create subfolders on device (sorted so parents come before children)
  const sortedPaths = Array.from(subfolderPaths).sort();
  for (const folderPath of sortedPaths) {
    try {
      await createFolderOnDevice(folderPath);
    } catch (e) {
      // Folder may already exist — continue
    }
  }

  // Queue each file to its correct subfolder path
  for (const f of validFiles) {
    const rel = f.webkitRelativePath || f._relativePath || '';
    const parts = rel.split('/');
    let uploadPath;
    if (parts.length > 2) {
      uploadPath = basePath + '/' + parts.slice(1, -1).join('/');
    } else {
      uploadPath = basePath;
    }
    addToUploadQueue([f], uploadPath, state.targetBank);
  }

  // Refresh the file list to show new folders
  try {
    await fetchSampleList();
    renderPoolContent();
    updateDropZoneTarget();
  } catch (e) {
    console.error('Refresh after folder upload failed:', e);
  }
}

/**
 * Update the drop-zone bar and overlay text to show the current upload target.
 * Called whenever pool navigation changes.
 */
function updateDropZoneTarget() {
  const barText = document.querySelector('#drop-zone-bar .drop-zone-text');
  const overlayText = document.querySelector('#pool-drop-overlay span');
  const uploadBtn = document.getElementById('upload-btn');
  const newFolderBtn = document.getElementById('new-folder-btn');
  const writable = isInUserFolder();
  if (state.poolPath && writable) {
    const shortPath = '/' + state.poolPath;
    if (barText) barText.textContent = `Drop files to upload → ${shortPath}`;
    if (overlayText) overlayText.textContent = `Drop audio files → ${shortPath}`;
    if (uploadBtn) uploadBtn.removeAttribute('disabled');
  } else if (state.poolPath && !writable) {
    if (barText) barText.textContent = 'Upload to ' + USER_FOLDER + ' folder only';
    if (overlayText) overlayText.textContent = 'Navigate to ' + USER_FOLDER + ' to upload';
    if (uploadBtn) uploadBtn.setAttribute('disabled', '');
  } else {
    if (barText) barText.textContent = 'Navigate into a folder to upload files';
    if (overlayText) overlayText.textContent = 'Navigate into a folder first';
    if (uploadBtn) uploadBtn.setAttribute('disabled', '');
  }
  // Show/hide or disable new-folder button based on writable context
  if (newFolderBtn) {
    // Always show the button, but disable it when not writable
    newFolderBtn.style.display = '';
    if (writable) {
      newFolderBtn.removeAttribute('disabled');
      newFolderBtn.style.opacity = '';
    } else {
      newFolderBtn.setAttribute('disabled', '');
      newFolderBtn.style.opacity = '0.4';
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  EVENT DELEGATION — Pool Actions
// ═══════════════════════════════════════════════════════════════

function setupPoolActions() {
  const pool = document.getElementById('pool-panel');
  pool.addEventListener('click', e => {
    // Handle folder action buttons FIRST (before nav, since they stopPropagation)
    const btn = e.target.closest('[data-act]');
    if (btn) {
      const act = btn.dataset.act;

      if (act === 'rename-folder') {
        e.stopPropagation();
        openRenameFolderDialog(btn.dataset.folderPath, btn.dataset.folderName);
        return;
      }
      if (act === 'delete-folder') {
        e.stopPropagation();
        openDeleteFolderDialog(btn.dataset.folderPath, btn.dataset.folderName);
        return;
      }
      if (act === 'preview') {
        e.stopPropagation();
        playPreview(btn.dataset.path, btn.dataset.name);
        return;
      }
      if (act === 'rename') {
        e.stopPropagation();
        openRenameDialog(btn.dataset.path, btn.dataset.name);
        return;
      }
      if (act === 'delete') {
        e.stopPropagation();
        openDeleteDialog(btn.dataset.path, btn.dataset.name);
        return;
      }
    }

    // Folder navigation via breadcrumb or folder row
    const navEl = e.target.closest('[data-nav]');
    if (navEl) {
      e.stopPropagation();
      navigatePool(navEl.dataset.nav);
      return;
    }
  });
}

// ═══════════════════════════════════════════════════════════════
//  EVENT DELEGATION — Kit Editor Actions
// ═══════════════════════════════════════════════════════════════

function setupKitActions() {
  const kit = document.getElementById('kit-panel');
  kit.addEventListener('click', e => {
    const btn = e.target.closest('[data-act]');
    if (!btn) return;
    const act = btn.dataset.act;

    if (act === 'toggle-bank') {
      const bankIdx = parseInt(btn.dataset.bank, 10);
      state.banks[bankIdx].collapsed = !state.banks[bankIdx].collapsed;
      btn.closest('.bank-card').classList.toggle('collapsed');
      return;
    }
    if (act === 'delete-bank') {
      e.stopPropagation();
      deleteBank(parseInt(btn.dataset.bank, 10));
      return;
    }
    if (act === 'add-bank') {
      e.stopPropagation();
      openAddBankDialog();
      return;
    }
    if (act === 'add-sample') {
      e.stopPropagation();
      openSamplePicker(parseInt(btn.dataset.bank, 10));
      return;
    }
    if (act === 'preview-kit') {
      e.stopPropagation();
      const idx = parseInt(btn.dataset.index, 10);
      const entry = state.kitEntries[idx];
      if (entry) playPreview(entry.path, entry.filename);
      return;
    }
    if (act === 'edit-sname') {
      e.stopPropagation();
      const idx = parseInt(btn.dataset.index, 10);
      const entry = state.kitEntries[idx];
      if (entry) openRenameDisplayName(idx, entry);
      return;
    }
    if (act === 'remove-slot') {
      e.stopPropagation();
      const idx = parseInt(btn.dataset.index, 10);
      removeEntryFromBank(idx);
      markDirty();
      renderKitEditor();
      return;
    }
  });

  kit.addEventListener('change', e => {
    if (e.target.matches('[data-act="edit-bank-name"]')) {
      const bankIdx = parseInt(e.target.dataset.bank, 10);
      const fallback = (DEFAULT_BANKS[bankIdx] && DEFAULT_BANKS[bankIdx].name) || `BANK ${bankIdx + 1}`;
      state.banks[bankIdx].name = e.target.value.trim() || fallback;
      markDirty();
    }
  });
}

// ═══════════════════════════════════════════════════════════════
//  DIALOGS
// ═══════════════════════════════════════════════════════════════

function openRenameDialog(path, filename) {
  state._renameCtx = { path, filename };
  const dlg = document.getElementById('rename-dialog');
  const inp = document.getElementById('rename-input');
  dlg.label = 'Rename';
  inp.value = filename;
  dlg.show();
  setTimeout(() => inp.focus(), 100);
}

function setupRenameDialog() {
  const dlg = document.getElementById('rename-dialog');
  const ok  = document.getElementById('rename-ok');
  const can = document.getElementById('rename-cancel');

  ok.addEventListener('click', async () => {
    const ctx = state._renameCtx;
    if (!ctx) return;
    if (ctx._kitIndex !== undefined) return; // handled by temporary handler
    const newName = sanitizeFilename(document.getElementById('rename-input').value);
    if (!newName || newName === ctx.filename) { dlg.hide(); return; }
    try {
      await renameSample(ctx.path, ctx.filename, newName);
      toast(`Renamed to ${newName}`, 'success');
      await fetchSampleList();
      renderPoolContent();
      renderKitEditor();
    } catch (e) {
      toast(`Rename failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

async function openDeleteDialog(path, filename) {
  state._deleteCtx = { path, filename };
  document.getElementById('delete-msg').textContent =
    `Delete ${filename}.wav from ${path}?`;

  // Check ALL kits for references to this file via firmware
  const refsEl = document.getElementById('delete-kit-refs');
  const listEl = document.getElementById('delete-kit-refs-list');
  const headerEl = refsEl.querySelector('div');
  refsEl.style.display = 'none';
  listEl.innerHTML = '';
  document.getElementById('delete-dialog').show();

  try {
    const result = await apiPost('?action=manage', {
      action: 'checkFileRefs', path, filename
    });
    const refs = result.refs || [];
    if (refs.length > 0) {
      const SLICES = SLICES_PER_BANK;
      headerEl.innerHTML = `<sl-icon name="exclamation-triangle" style="vertical-align:-2px;margin-right:4px;color:var(--sl-color-warning-600);"></sl-icon>Used in Kit(s):`;
      listEl.innerHTML = refs.map(r => {
        const bankIdx = Math.floor(r.slotIndex / SLICES);
        const slotInBank = r.slotIndex - bankIdx * SLICES;
        const kitName = r.kitName || `Kit ${r.kitIndex}`;
        return `<li>${kitName} — Bank ${bankIdx + 1}, Slot ${slotInBank + 1}</li>`;
      }).join('');
      refsEl.style.display = '';
    }
  } catch (e) {
    // Non-critical: fall back to local check
    const refs = findKitReferencesForFile(path, filename);
    if (refs.length > 0) {
      headerEl.innerHTML = `<sl-icon name="exclamation-triangle" style="vertical-align:-2px;margin-right:4px;color:var(--sl-color-warning-600);"></sl-icon>Used in current Kit:`;
      listEl.innerHTML = refs.map(r =>
        `<li>${state.banks[r.bankIdx].name} — Slot ${r.slotIdx + 1}</li>`
      ).join('');
      refsEl.style.display = '';
    }
  }
}

function setupDeleteDialog() {
  const dlg = document.getElementById('delete-dialog');
  const ok  = document.getElementById('delete-ok');
  const can = document.getElementById('delete-cancel');

  ok.addEventListener('click', async () => {
    const ctx = state._deleteCtx;
    if (!ctx) return;
    try {
      await deleteSample(ctx.path, ctx.filename);
      toast(`Deleted ${ctx.filename}`, 'success');
      await fetchSampleList();
      renderPoolContent();
      renderKitEditor();
      updateCapacityBar();
      updateStorageBar();
    } catch (e) {
      toast(`Delete failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// ═══════════════════════════════════════════════════════════════
//  FOLDER MANAGEMENT DIALOGS
// ═══════════════════════════════════════════════════════════════

/**
 * Sanitize a folder name for FAT32 filesystem.
 */
function sanitizeFolderName(name) {
  let clean = name.trim();
  clean = clean.normalize('NFKD').replace(/[^\x00-\x7F]/g, '');
  clean = clean.replace(/[\s]+/g, '_');
  clean = clean.replace(/[^A-Za-z0-9_\-.]/g, '');
  clean = clean.replace(/[_\-.]{2,}/g, '_');
  clean = clean.replace(/^[_\-.]+|[_\-.]+$/g, '');
  if (!clean) clean = 'folder';
  if (clean.length > 32) clean = clean.substring(0, 32);
  return clean;
}

function setupNewFolderDialog() {
  const dlg = document.getElementById('new-folder-dialog');
  const ok  = document.getElementById('new-folder-ok');
  const can = document.getElementById('new-folder-cancel');
  const btn = document.getElementById('new-folder-btn');

  btn.addEventListener('click', () => {
    document.getElementById('new-folder-input').value = '';
    dlg.show();
    setTimeout(() => document.getElementById('new-folder-input').focus(), 100);
  });

  ok.addEventListener('click', async () => {
    const raw = document.getElementById('new-folder-input').value;
    const name = sanitizeFolderName(raw);
    if (!name) { toast('Please enter a folder name.', 'warning'); return; }

    const parentPath = state.poolPath || '';
    const fullPath = parentPath ? `${parentPath}/${name}` : name;

    // Check if folder already exists
    const exists = state.folders.some(f => f === fullPath || f.startsWith(fullPath + '/'));
    if (exists) {
      toast(`Folder "${name}" already exists.`, 'warning');
      return;
    }

    try {
      await createFolderOnDevice(fullPath);
      toast(`Folder "${name}" created.`, 'success');
      await fetchSampleList();
      renderPoolContent();
    } catch (e) {
      toast(`Create folder failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

function openRenameFolderDialog(folderPath, folderName) {
  state._renameFolderCtx = { path: folderPath, name: folderName };
  const dlg = document.getElementById('rename-folder-dialog');
  const inp = document.getElementById('rename-folder-input');
  const warning = document.getElementById('rename-folder-kit-warning');
  const msg = document.getElementById('rename-folder-kit-msg');
  inp.value = folderName;

  // Check kit references
  const refs = findKitReferencesInFolder(folderPath);
  if (refs.length > 0) {
    warning.style.display = '';
    msg.textContent = `${refs.length} sample(s) in the active kit reference this folder. Renaming will automatically update their paths.`;
  } else {
    warning.style.display = 'none';
  }

  dlg.show();
  setTimeout(() => inp.focus(), 100);
}

function setupRenameFolderDialog() {
  const dlg = document.getElementById('rename-folder-dialog');
  const ok  = document.getElementById('rename-folder-ok');
  const can = document.getElementById('rename-folder-cancel');

  ok.addEventListener('click', async () => {
    const ctx = state._renameFolderCtx;
    if (!ctx) return;
    const raw = document.getElementById('rename-folder-input').value;
    const newName = sanitizeFolderName(raw);
    if (!newName || newName === ctx.name) { dlg.hide(); return; }

    // Compute new path: replace the last segment
    const parts = ctx.path.split('/');
    parts[parts.length - 1] = newName;
    const newPath = parts.join('/');

    try {
      // Rename on device
      await renameFolderOnDevice(ctx.path, newPath);

      // Update kit entries in memory and auto-save if needed
      const updated = updateKitPathsAfterRename(ctx.path, newPath);
      if (updated > 0) {
        markDirty();
        await saveKit();
        toast(`Renamed folder and updated ${updated} kit reference(s).`, 'success');
      } else {
        toast(`Folder renamed to "${newName}".`, 'success');
      }

      // If we're currently viewing inside the renamed folder, update navigation
      if (state.poolPath === ctx.path || state.poolPath.startsWith(ctx.path + '/')) {
        state.poolPath = state.poolPath.replace(ctx.path, newPath);
      }

      await fetchSampleList();
      renderPoolContent();
      renderKitEditor();
      updateDropZoneTarget();
    } catch (e) {
      toast(`Rename folder failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

function openDeleteFolderDialog(folderPath, folderName) {
  state._deleteFolderCtx = { path: folderPath, name: folderName };
  const dlg = document.getElementById('delete-folder-dialog');
  const msg = document.getElementById('delete-folder-msg');
  const warning = document.getElementById('delete-folder-kit-warning');
  const kitMsg = document.getElementById('delete-folder-kit-msg');

  msg.textContent = `Delete folder "${folderName}" and all its contents?`;

  // Count files to be deleted
  const fileCount = state.files.filter(f =>
    f.path === folderPath || f.path.startsWith(folderPath + '/')
  ).length;

  // Check kit references
  const refs = findKitReferencesInFolder(folderPath);
  if (refs.length > 0) {
    warning.style.display = '';
    kitMsg.textContent = `${refs.length} sample(s) in the active kit reference files in this folder. Deleting will break those kit assignments and leave empty slots.`;
  } else {
    warning.style.display = 'none';
  }

  if (fileCount > 0) {
    msg.textContent = `Delete folder "${folderName}" containing ${fileCount} file(s)?`;
  }

  dlg.show();
}

function setupDeleteFolderDialog() {
  const dlg = document.getElementById('delete-folder-dialog');
  const ok  = document.getElementById('delete-folder-ok');
  const can = document.getElementById('delete-folder-cancel');

  ok.addEventListener('click', async () => {
    const ctx = state._deleteFolderCtx;
    if (!ctx) return;

    try {
      await deleteFolderOnDevice(ctx.path);

      // Clear out any kit entries that referenced this folder
      let cleared = 0;
      if (state.kitEntries) {
        for (let i = 0; i < state.kitEntries.length; i++) {
          const e = state.kitEntries[i];
          if (!e || !e.path) continue;
          if (e.path === ctx.path || e.path.startsWith(ctx.path + '/')) {
            state.kitEntries[i] = null;
            cleared++;
          }
        }
      }
      if (cleared > 0) {
        markDirty();
        await saveKit();
        toast(`Deleted folder and cleared ${cleared} kit reference(s).`, 'success');
      } else {
        toast(`Folder "${ctx.name}" deleted.`, 'success');
      }

      // If we're inside the deleted folder, navigate up
      if (state.poolPath === ctx.path || state.poolPath.startsWith(ctx.path + '/')) {
        // Go to parent folder
        const parts = ctx.path.split('/');
        parts.pop();
        state.poolPath = parts.join('/');
      }

      await fetchSampleList();
      renderPoolContent();
      renderKitEditor();
      updateCapacityBar();
      updateStorageBar();
      updateDropZoneTarget();
    } catch (e) {
      toast(`Delete folder failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

function openRenameDisplayName(idx, entry) {
  state._renameCtx = { _kitIndex: idx };
  const dlg = document.getElementById('rename-dialog');
  const inp = document.getElementById('rename-input');
  dlg.label = 'Edit Display Name';
  inp.value = entry.sname || entry.filename;
  dlg.show();
  setTimeout(() => inp.focus(), 100);

  const origOk = document.getElementById('rename-ok');
  const handler = async () => {
    const newName = document.getElementById('rename-input').value.trim();
    if (state.kitEntries[idx]) {
      state.kitEntries[idx].sname = newName;
      markDirty();
      renderKitEditor();
    }
    dlg.label = 'Rename';
    dlg.hide();
    origOk.removeEventListener('click', handler);
  };
  origOk.addEventListener('click', handler, { once: true });
}

function setupNewKitDialog() {
  const dlg   = document.getElementById('new-kit-dialog');
  const ok    = document.getElementById('new-kit-ok');
  const can   = document.getElementById('new-kit-cancel');
  const btn   = document.getElementById('new-kit-btn');
  const cloneCheckbox = document.getElementById('new-kit-clone');
  const optionsDiv = document.getElementById('new-kit-options');
  const bankCountInput = document.getElementById('new-kit-bank-count');
  const bankCountGroup = document.getElementById('new-kit-bank-count-group');

  // Bank count button group
  if (bankCountGroup) {
    bankCountGroup.addEventListener('click', e => {
      const clicked = e.target.closest('[data-bank-count]');
      if (!clicked) return;
      // Deselect all, select clicked
      bankCountGroup.querySelectorAll('[data-bank-count]').forEach(b => b.variant = 'default');
      clicked.variant = 'primary';
      bankCountInput.value = clicked.dataset.bankCount;
    });
  }

  // Hide bank options when cloning
  cloneCheckbox.addEventListener('sl-change', () => {
    optionsDiv.style.display = cloneCheckbox.checked ? 'none' : 'flex';
  });

  btn.addEventListener('click', () => {
    document.getElementById('new-kit-input').value = '';
    cloneCheckbox.checked = false;
    optionsDiv.style.display = 'flex';
    bankCountInput.value = '8';
    // Reset button group visual state
    if (bankCountGroup) {
      bankCountGroup.querySelectorAll('[data-bank-count]').forEach(b => {
        b.variant = b.dataset.bankCount === '8' ? 'primary' : 'default';
      });
    }
    document.getElementById('new-kit-naming').value = 'default';
    dlg.show();
  });

  ok.addEventListener('click', async () => {
    const name = document.getElementById('new-kit-input').value.trim();
    if (!name) { toast('Please enter a Kit name', 'warning'); return; }
    const clone = cloneCheckbox.checked;
    try {
      const entries = clone ? getKitForSave() : [];
      const result = await createKitOnDevice(name, entries);
      toast(`Kit "${name}" created`, 'success');
      const newIdx = result.newKitIndex !== undefined
        ? result.newKitIndex
        : (state.kits.smp_bank_names.length);
      state.kits.active_smp_bank = newIdx;
      state.dirty = false;
      await fetchSampleList();

      // If not cloning, apply bank count + naming options
      if (!clone) {
        const bankCount = parseInt(document.getElementById('new-kit-bank-count').value, 10) || 0;
        const naming = document.getElementById('new-kit-naming').value;
        const newBanks = [];
        for (let i = 0; i < bankCount; i++) {
          let bname;
          if (naming === 'default' && DEFAULT_BANKS[i]) {
            bname = DEFAULT_BANKS[i].name;
          } else if (naming === 'numbered' || (naming === 'default' && !DEFAULT_BANKS[i])) {
            bname = `BANK ${i + 1}`;
          } else {
            bname = '';
          }
          newBanks.push({
            name: bname,
            color: BANK_COLORS[i % BANK_COLORS.length],
            collapsed: false,
          });
        }
        state.banks = newBanks;
        // Ensure kitEntries covers all bank slots
        while (state.kitEntries.length < bankCount * SLICES_PER_BANK) {
          state.kitEntries.push(null);
        }
        // Trim kitEntries if fewer banks
        if (bankCount * SLICES_PER_BANK < state.kitEntries.length) {
          state.kitEntries.length = bankCount * SLICES_PER_BANK;
        }
      }

      renderKitSelector();
      renderKitEditor();
      updateCapacityBar();
      updateSaveButton();
    } catch (e) {
      toast(`Create Kit failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// ═══════════════════════════════════════════════════════════════
//  DELETE KIT
// ═══════════════════════════════════════════════════════════════

function openDeleteKitDialog() {
  const dlg = document.getElementById('delete-kit-dialog');
  const inp = document.getElementById('delete-kit-confirm-input');
  const names = state.kits.smp_bank_names || [];
  const kitName = names[state.kits.active_smp_bank] || 'Default';
  document.getElementById('delete-kit-name').textContent = kitName;
  inp.value = '';
  document.getElementById('delete-kit-ok').setAttribute('disabled', '');
  dlg.show();
  setTimeout(() => inp.focus(), 100);
}

function setupDeleteKitDialog() {
  const dlg = document.getElementById('delete-kit-dialog');
  const ok  = document.getElementById('delete-kit-ok');
  const can = document.getElementById('delete-kit-cancel');
  const inp = document.getElementById('delete-kit-confirm-input');

  // Enable button only when user types the exact kit name
  inp.addEventListener('sl-input', () => {
    const names = state.kits.smp_bank_names || [];
    const kitName = names[state.kits.active_smp_bank] || 'Default';
    if (inp.value.trim() === kitName) {
      ok.removeAttribute('disabled');
    } else {
      ok.setAttribute('disabled', '');
    }
  });

  ok.addEventListener('click', async () => {
    const kitIdx = state.kits.active_smp_bank;
    try {
      await deleteKitOnDevice(kitIdx);
      toast('Kit deleted', 'success');
      state.kits.active_smp_bank = 0;
      state.dirty = false;
      await fetchSampleList();
      renderKitSelector();
      renderKitEditor();
      updateCapacityBar();
      updateSaveButton();
    } catch (e) {
      toast(`Delete Kit failed: ${e.message}`, 'danger');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// --- Sample Picker ---
function openSamplePicker(bankIdx) {
  state._pickerBank = bankIdx;
  state._pickerSelected = new Set();
  renderPickerList('');
  document.getElementById('picker-search').value = '';
  document.getElementById('picker-dialog').show();
}

function renderPickerList(filter) {
  const list = document.getElementById('picker-list');
  const lc = filter.toLowerCase();
  const filtered = state.files.filter(f => {
    const stem = f.name.replace(/\.wav$/i, '');
    return !lc || stem.toLowerCase().includes(lc) || f.path.toLowerCase().includes(lc);
  });
  if (filtered.length === 0) {
    list.innerHTML = '<div style="padding:1rem;text-align:center;color:var(--sl-color-neutral-400);">No matching files.</div>';
    return;
  }
  list.innerHTML = filtered.map(f => {
    const stem = f.name.replace(/\.wav$/i, '');
    const key = `${f.path}/${stem}`;
    const sel = state._pickerSelected.has(key) ? ' selected' : '';
    return `<div class="sample-picker-item${sel}" data-key="${esc(key)}" data-path="${esc(f.path)}" data-name="${esc(stem)}" data-size="${f.size}">
      <sl-icon name="file-earmark-music" style="font-size:0.9rem;"></sl-icon>
      <span style="flex:1;">${esc(stem)}</span>
      <span style="font-size:0.72rem;color:var(--sl-color-neutral-500);">${esc(f.path)}</span>
      <span style="font-size:0.72rem;color:var(--sl-color-neutral-500);">${formatBytes(f.size)}</span>
    </div>`;
  }).join('');
}

function setupSamplePicker() {
  const dlg = document.getElementById('picker-dialog');
  const ok  = document.getElementById('picker-ok');
  const can = document.getElementById('picker-cancel');
  const search = document.getElementById('picker-search');
  const list = document.getElementById('picker-list');

  search.addEventListener('sl-input', () => renderPickerList(search.value));

  list.addEventListener('click', e => {
    const item = e.target.closest('.sample-picker-item');
    if (!item) return;
    const key = item.dataset.key;
    if (state._pickerSelected.has(key)) {
      state._pickerSelected.delete(key);
      item.classList.remove('selected');
    } else {
      state._pickerSelected.add(key);
      item.classList.add('selected');
    }
  });

  ok.addEventListener('click', () => {
    const bankIdx = state._pickerBank;
    let added = 0;
    for (const key of state._pickerSelected) {
      const [path, name] = [key.substring(0, key.lastIndexOf('/')), key.substring(key.lastIndexOf('/') + 1)];
      const file = state.files.find(f => f.path === path && f.name === name);
      const nsamp = file ? nsamples(file.size) : 0;
      if (addEntryToBank(bankIdx, name, path, nsamp)) added++;
    }
    if (added > 0) {
      markDirty();
      renderKitEditor();
      toast(`Added ${added} sample(s) to ${state.banks[bankIdx].name}`, 'success');
    }
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// ═══════════════════════════════════════════════════════════════
//  SHOELACE DIALOGS — Bank & Kit confirmations
// ═══════════════════════════════════════════════════════════════

function openAddBankDialog() {
  const dlg = document.getElementById('add-bank-dialog');
  const inp = document.getElementById('add-bank-input');
  const ok  = document.getElementById('add-bank-ok');
  const can = document.getElementById('add-bank-cancel');
  inp.value = getNextUniqueBankName();
  dlg.show();
  setTimeout(() => inp.focus(), 100);

  const onOk = () => {
    const name = inp.value.trim();
    if (name) addBank(name);
    dlg.hide();
    cleanup();
  };
  const onCancel = () => { dlg.hide(); cleanup(); };
  const cleanup = () => {
    ok.removeEventListener('click', onOk);
    can.removeEventListener('click', onCancel);
  };
  ok.addEventListener('click', onOk);
  can.addEventListener('click', onCancel);
}

function openDeleteBankDialog(bankIdx, name, entryCount) {
  const dlg = document.getElementById('delete-bank-dialog');
  const msg = document.getElementById('delete-bank-msg');
  const ok  = document.getElementById('delete-bank-ok');
  const can = document.getElementById('delete-bank-cancel');
  msg.textContent = entryCount > 0
    ? `Delete Bank "${name}"? It contains ${entryCount} sample(s).`
    : `Delete Bank "${name}"?`;
  dlg.show();

  const onOk = () => {
    performDeleteBank(bankIdx, name);
    dlg.hide();
    cleanup();
  };
  const onCancel = () => { dlg.hide(); cleanup(); };
  const cleanup = () => {
    ok.removeEventListener('click', onOk);
    can.removeEventListener('click', onCancel);
  };
  ok.addEventListener('click', onOk);
  can.addEventListener('click', onCancel);
}

function openUnsavedKitDialog() {
  return new Promise(resolve => {
    const dlg = document.getElementById('unsaved-kit-dialog');
    const ok  = document.getElementById('unsaved-kit-ok');
    const can = document.getElementById('unsaved-kit-cancel');
    dlg.show();

    const onOk = () => { dlg.hide(); cleanup(); resolve(true); };
    const onCancel = () => { dlg.hide(); cleanup(); resolve(false); };
    const cleanup = () => {
      ok.removeEventListener('click', onOk);
      can.removeEventListener('click', onCancel);
    };
    ok.addEventListener('click', onOk);
    can.addEventListener('click', onCancel);
  });
}

// ═══════════════════════════════════════════════════════════════
//  TOOLBAR ACTIONS
// ═══════════════════════════════════════════════════════════════

function setupToolbar() {
  // Kit selector
  document.getElementById('kit-select').addEventListener('sl-change', async e => {
    if (state.initializing) return;
    if (state.dirty) {
      const proceed = await openUnsavedKitDialog();
      if (!proceed) {
        const sel = e.target;
        customElements.whenDefined('sl-select').then(() => {
          requestAnimationFrame(() => { sel.value = String(state.kits.active_smp_bank); });
        });
        return;
      }
    }
    const idx = parseInt(e.target.value, 10);
    state.kits.active_smp_bank = idx;
    state.dirty = false;
    try {
      await fetchSampleList();
      renderKitEditor();
      updateCapacityBar();
      updateSaveButton();
    } catch (err) {
      toast(`Failed to switch Kit: ${err.message}`, 'danger');
    }
  });

  // Save Kit
  document.getElementById('save-kit-btn').addEventListener('click', () => saveKit());

  // Delete Kit
  document.getElementById('delete-kit-btn').addEventListener('click', () => {
    const names = state.kits.smp_bank_names || [];
    if (names.length <= 1) {
      toast('Cannot delete the last kit', 'warning');
      return;
    }
    openDeleteKitDialog();
  });

  // View toggle
  document.getElementById('view-toggle').addEventListener('sl-change', e => {
    if (state.initializing) return;
    state.viewMode = e.target.checked ? 'banked' : 'flat';
    renderKitEditor();
  });

  // Export / Import Kit
  document.getElementById('export-kit-btn').addEventListener('click', () => exportKit());
  const importBtn = document.getElementById('import-kit-btn');
  const importInput = document.getElementById('import-kit-input');
  importBtn.addEventListener('click', () => importInput.click());
  importInput.addEventListener('change', () => {
    if (importInput.files.length > 0) {
      importKit(importInput.files[0]);
      importInput.value = '';
    }
  });

  // Theme toggle — delegated to app shell (shared.js) when running in unified mode
  if (!_S) setupThemeToggle();
}

// ═══════════════════════════════════════════════════════════════
//  KIT EXPORT / IMPORT (Backup & Restore)
// ═══════════════════════════════════════════════════════════════

function exportKit() {
  const names = state.kits.smp_bank_names || [];
  const kitName = names[state.kits.active_smp_bank] || 'Default';
  const entries = getKitForSave();
  const banksMeta = getBanksMeta();
  const exportData = {
    _format: 'tbd16-kit-backup',
    _version: 1,
    kitName,
    kitIndex: state.kits.active_smp_bank,
    exportDate: new Date().toISOString(),
    banks: banksMeta,
    entries: entries,
  };
  const json = JSON.stringify(exportData, null, 2);
  const blob = new Blob([json], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `${kitName.replace(/[^A-Za-z0-9_-]/g, '_')}_kit_backup.json`;
  a.click();
  URL.revokeObjectURL(url);
  toast(`Exported "${kitName}" kit backup`, 'success');
}

async function importKit(file) {
  try {
    const text = await file.text();
    const data = JSON.parse(text);
    if (data._format !== 'tbd16-kit-backup' || !data.entries) {
      toast('Invalid kit backup file', 'danger');
      return;
    }
    openImportKitDialog(data);
  } catch (e) {
    toast(`Import failed: ${e.message}`, 'danger');
  }
}

function openImportKitDialog(data) {
  const dlg = document.getElementById('import-kit-dialog');
  const body = document.getElementById('import-kit-body');

  // Build a lookup of available files for auto-mapping
  const available = new Map();
  for (const f of state.files) {
    const stem = f.name.replace(/\.wav$/i, '');
    // Index by filename for fuzzy matching
    if (!available.has(stem)) available.set(stem, []);
    available.get(stem).push(f);
  }

  // Analyze entries
  let totalSlots = 0, matched = 0, missing = 0;
  const entryAnalysis = [];
  for (let i = 0; i < data.entries.length; i++) {
    const e = data.entries[i];
    if (!e) continue;
    totalSlots++;
    const stem = (e.filename || '').replace(/\.wav$/i, '');
    const candidates = available.get(stem) || [];
    // Prefer exact path match, then any match
    const exact = candidates.find(f => f.path === e.path);
    const any = candidates.length > 0 ? candidates[0] : null;
    const found = exact || any;
    if (found) {
      matched++;
      entryAnalysis.push({ idx: i, entry: e, mapped: found, status: exact ? 'exact' : 'remapped' });
    } else {
      missing++;
      entryAnalysis.push({ idx: i, entry: e, mapped: null, status: 'missing' });
    }
  }

  // Build summary
  const bankNames = (data.banks || []).map(b => b.name).join(', ') || 'Unknown';
  const kitDate = data.exportDate ? new Date(data.exportDate).toLocaleDateString() : 'Unknown';

  let html = `<div style="margin-bottom:0.8rem;">
    <div style="font-weight:600;font-size:0.95rem;margin-bottom:0.3rem;">${esc(data.kitName)}</div>
    <div style="font-size:0.78rem;color:var(--sl-color-neutral-500);">Exported: ${esc(kitDate)} &middot; Banks: ${esc(bankNames)}</div>
  </div>
  <div style="display:flex;gap:1rem;margin-bottom:0.8rem;">
    <div style="flex:1;text-align:center;padding:0.5rem;border-radius:6px;background:var(--sl-color-success-100);color:var(--sl-color-success-700);font-size:0.82rem;">
      <strong>${matched}</strong> matched
    </div>
    <div style="flex:1;text-align:center;padding:0.5rem;border-radius:6px;background:${missing > 0 ? 'var(--sl-color-warning-100)' : 'var(--sl-color-neutral-100)'};color:${missing > 0 ? 'var(--sl-color-warning-700)' : 'var(--sl-color-neutral-500)'};font-size:0.82rem;">
      <strong>${missing}</strong> missing
    </div>
    <div style="flex:1;text-align:center;padding:0.5rem;border-radius:6px;background:var(--sl-color-neutral-100);color:var(--sl-color-neutral-600);font-size:0.82rem;">
      <strong>${totalSlots}</strong> total
    </div>
  </div>`;

  if (missing > 0) {
    html += `<div style="font-size:0.78rem;color:var(--sl-color-warning-600);margin-bottom:0.6rem;">
      <sl-icon name="exclamation-triangle" style="vertical-align:-2px;"></sl-icon>
      ${missing} sample(s) not found on SD card. Upload the missing files and re-import, or continue with partial restore.
    </div>`;
  }

  html += `<div style="max-height:240px;overflow-y:auto;border:1px solid var(--sl-color-neutral-200);border-radius:6px;margin-bottom:0.5rem;">
    <table style="width:100%;font-size:0.76rem;border-collapse:collapse;">
      <thead><tr style="background:var(--sl-color-neutral-100);position:sticky;top:0;">
        <th style="padding:0.3rem 0.5rem;text-align:left;">SAMPLE</th>
        <th style="padding:0.3rem 0.5rem;text-align:left;">BANK</th>
        <th style="padding:0.3rem 0.5rem;text-align:left;">STATUS</th>
      </tr></thead><tbody>`;

  for (const a of entryAnalysis) {
    const bankIdx = Math.floor(a.idx / SLICES_PER_BANK);
    const bankName = data.banks && data.banks[bankIdx] ? data.banks[bankIdx].name : `Bank ${bankIdx + 1}`;
    const statusIcon = a.status === 'exact'
      ? '<span style="color:var(--sl-color-success-600);">&#10003; Found</span>'
      : a.status === 'remapped'
      ? '<span style="color:var(--sl-color-primary-600);">&#8634; Remapped</span>'
      : '<span style="color:var(--sl-color-warning-600);">&#10007; Missing</span>';
    const rowBg = a.status === 'missing' ? 'background:color-mix(in srgb, var(--sl-color-warning-500) 8%, transparent);' : '';
    html += `<tr style="${rowBg}">
      <td style="padding:0.25rem 0.5rem;" title="${esc(a.entry.path + '/' + a.entry.filename)}">${esc(a.entry.filename)}</td>
      <td style="padding:0.25rem 0.5rem;">${esc(bankName)}</td>
      <td style="padding:0.25rem 0.5rem;">${statusIcon}</td>
    </tr>`;
  }

  html += '</tbody></table></div>';

  body.innerHTML = html;

  // Store analysis for the OK handler
  state._importData = { data, entryAnalysis };
  dlg.show();
}

function setupImportKitDialog() {
  const dlg = document.getElementById('import-kit-dialog');
  const ok  = document.getElementById('import-kit-ok');
  const can = document.getElementById('import-kit-cancel');

  ok.addEventListener('click', async () => {
    const { data, entryAnalysis } = state._importData || {};
    if (!data) { dlg.hide(); return; }

    // Apply banks metadata
    if (data.banks && data.banks.length > 0) {
      state.banks = data.banks.map((b, i) => ({
        name: b.name || `BANK ${i + 1}`,
        color: b.color || BANK_COLORS[i % BANK_COLORS.length],
        collapsed: false,
      }));
    }

    // Apply entries with auto-mapping
    const maxSlots = state.banks.length * SLICES_PER_BANK;
    state.kitEntries = [];
    for (let i = 0; i < maxSlots; i++) state.kitEntries.push(null);

    for (const a of entryAnalysis) {
      if (a.idx >= maxSlots) continue;
      if (a.mapped) {
        const stem = a.mapped.name.replace(/\.wav$/i, '');
        state.kitEntries[a.idx] = {
          filename: stem,
          path: a.mapped.path,
          nsamples: nsamples(a.mapped.size),
          sname: a.entry.sname || '',
        };
      } else {
        // Keep original entry marked as missing
        state.kitEntries[a.idx] = { ...a.entry };
      }
    }

    markMissingKitEntries();
    markDirty();
    renderKitEditor();

    const matchCount = entryAnalysis.filter(a => a.mapped).length;
    const missCount = entryAnalysis.filter(a => !a.mapped).length;
    toast(`Imported "${data.kitName}" — ${matchCount} mapped${missCount > 0 ? `, ${missCount} missing` : ''}`, 'success');
    dlg.hide();
  });
  can.addEventListener('click', () => dlg.hide());
}

// ═══════════════════════════════════════════════════════════════
//  THEME TOGGLE — standalone fallback (used only on samples.html)
// ═══════════════════════════════════════════════════════════════

// sun-fill SVG for standalone mode
const _SM_SUN_SVG = '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" viewBox="0 0 16 16"><path d="M8 12a4 4 0 1 0 0-8 4 4 0 0 0 0 8M8 0a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 0m0 13a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 13m8-5a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2a.5.5 0 0 1 .5.5M3 8a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2A.5.5 0 0 1 3 8m10.657-5.657a.5.5 0 0 1 0 .707l-1.414 1.415a.5.5 0 1 1-.707-.708l1.414-1.414a.5.5 0 0 1 .707 0m-9.193 9.193a.5.5 0 0 1 0 .707L3.05 13.657a.5.5 0 0 1-.707-.707l1.414-1.414a.5.5 0 0 1 .707 0m9.193 2.121a.5.5 0 0 1-.707 0l-1.414-1.414a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707M3.757 4.464a.5.5 0 0 1-.707 0L1.636 3.05a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707"/></svg>';

function setupThemeToggle() {
  const btn = document.getElementById('theme-toggle');
  if (!btn) return;
  const saved = localStorage.getItem('tbd-theme');
  if (saved === 'light') applyTheme('light');
  btn.addEventListener('click', () => {
    const isDark = document.documentElement.classList.contains('sl-theme-dark');
    applyTheme(isDark ? 'light' : 'dark');
  });
}

function applyTheme(theme) {
  const html = document.documentElement;
  const btn  = document.getElementById('theme-toggle');
  if (theme === 'light') {
    html.classList.remove('sl-theme-dark');
    html.classList.add('sl-theme-light');
    // Use inline SVG data URI to avoid network fetch for sun-fill icon
    btn.name = '';
    btn.src = `data:image/svg+xml,${encodeURIComponent(_SM_SUN_SVG)}`;
  } else {
    html.classList.remove('sl-theme-light');
    html.classList.add('sl-theme-dark');
    btn.src = '';
    btn.name = 'moon-fill';
  }
  const link = document.querySelector('link[href*="/shoelace/themes/"]');
  if (link) link.href = `/shoelace/themes/${theme}.css?v=2`;
  localStorage.setItem('tbd-theme', theme);
}

// ═══════════════════════════════════════════════════════════════
//  INITIALIZATION
// ═══════════════════════════════════════════════════════════════

async function init() {
  // In unified mode, status is managed by app shell; standalone sets it directly
  const status = _S ? null : document.getElementById('status-text');

  // Setup all event handlers
  setupDropZone();
  setupPoolActions();
  setupKitActions();
  setupPoolDragEvents();
  setupToolbar();
  setupRenameDialog();
  setupDeleteDialog();
  setupNewFolderDialog();
  setupRenameFolderDialog();
  setupDeleteFolderDialog();
  setupNewKitDialog();
  setupDeleteKitDialog();
  setupSamplePicker();
  setupTransferBar();
  setupColumnSort();
  setupSelectionToolbar();
  setupBatchDeleteDialog();
  setupImportKitDialog();

  // Select mode button
  const selectBtn = document.getElementById('select-mode-btn');
  if (selectBtn) {
    selectBtn.addEventListener('click', () => toggleSelectionMode());
  }

  // Fetch initial data
  try {
    await fetchSampleList();
    if (status) {
      status.textContent = 'Connected';
      status.style.color = 'var(--sl-color-success-600)';
    }

    // Default to user folder (samples/user on SD card)
    state.poolPath = USER_FOLDER;
  } catch (e) {
    console.error('Initial fetch failed:', e);
    if (status) {
      status.textContent = 'Offline';
      status.style.color = 'var(--sl-color-danger-600)';
    }

    document.getElementById('pool-content').innerHTML =
      '<div style="color:var(--sl-color-neutral-400);font-size:0.8rem;padding:1rem;text-align:center;">Could not connect to device.</div>';
    document.getElementById('bank-container').innerHTML =
      '<div style="color:var(--sl-color-neutral-400);font-size:0.8rem;padding:1rem;text-align:center;">Connect to TBD-16 to manage samples.</div>';
    return;
  }

  // Render everything
  renderPoolContent();
  renderKitEditor();
  updateCapacityBar();
  updateStorageBar();
  updateDropZoneTarget();

  state.initializing = false;
}

// ═══════════════════════════════════════════════════════════════
//  EXPORT / AUTO-INIT
// ═══════════════════════════════════════════════════════════════

// Unified mode: export on window.TBD for lazy init from app.js
// Standalone mode (samples.html): auto-init on DOMContentLoaded
if (typeof window.TBD !== 'undefined' && window.TBD.shared) {
  window.TBD.sampleManager = { init: init, state: state };
} else {
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => setTimeout(init, 200));
  } else {
    setTimeout(init, 200);
  }
}
