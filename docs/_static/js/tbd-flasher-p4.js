/**
 * tbd-flasher-p4.js — ESP32-P4 flashing via WebSerial (esptool-js)
 *
 * Provides:
 *   - loadEspTool()          → load esptool-js from CDN
 *   - connectP4(callbacks)   → open serial port + detect chip
 *   - flashP4(ctx, url, addr, callbacks) → download + flash firmware
 *   - readPartitionTable(ctx) → parse device partition table
 *   - detectOta1Address(ctx) → find ota_1 offset from partition table
 *   - flashMscAndSwitchOta(ctx, mscUrl, callbacks) → flash tusb_msc + switch to ota_1
 *   - switchOtaSlot(ctx, slot, callbacks) → write OTA data to select boot slot
 *   - resetDevice(ctx)       → hard-reset the device
 *   - disconnectP4(ctx)      → close transport
 *
 * Usage from an RST page:
 *   import { loadEspTool, connectP4, flashP4, disconnectP4 } from '../_static/js/tbd-flasher-p4.js';
 *   var esp = await loadEspTool();
 *   var ctx = await connectP4({ onStatus: msg => ... });
 *   await flashP4(ctx, 'https://cdn.example.com/stable/p4/dada-tbd-16-v0.4.1-unified.bin', 0x0, { onProgress: pct => ... });
 *   await disconnectP4(ctx);
 */

/* ── Constants ── */
var ESPTOOL_CDN = 'https://unpkg.com/esptool-js@0.5.7/bundle.js';
var OTA_DATA_ADDR = 0xd000;
var OTA_DATA_SIZE = 0x2000;   /* 8 KB */
var PT_ADDR       = 0x8000;
var PT_READ_SIZE  = 0xc00;    /* 3 KB */

/* ── Binary helpers ── */

/** Convert Uint8Array to binary string (8 KB chunks to avoid call-stack overflow). */
export function toBinStr(u8) {
  var parts = [], cs = 8192;
  for (var i = 0; i < u8.length; i += cs)
    parts.push(String.fromCharCode.apply(null, u8.subarray(i, i + cs)));
  return parts.join('');
}

/* ── CRC-32 (same polynomial as ESP-IDF / zlib) ── */
var crcTable = null;
function ensureCrcTable() {
  if (!crcTable) {
    crcTable = new Uint32Array(256);
    for (var n = 0; n < 256; n++) {
      var c = n;
      for (var k = 0; k < 8; k++)
        c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
      crcTable[n] = c;
    }
  }
}

/** esp_rom_crc32_le(UINT32_MAX, buf, len) — matches ESP-IDF bootloader. */
export function espCrc32(buf) {
  ensureCrcTable();
  var crc = 0x00000000;
  for (var i = 0; i < buf.length; i++)
    crc = crcTable[(crc ^ buf[i]) & 0xFF] ^ (crc >>> 8);
  return (crc ^ 0xFFFFFFFF) >>> 0;
}

/* ── Partition table parsing ── */

/** Parse ESP-IDF partition table from binary data. */
export function parsePartitionTable(data) {
  var entries = [];
  for (var i = 0; i < data.length; i += 32) {
    var magic = data[i] | (data[i + 1] << 8);
    if (magic === 0xEBEB) break;
    if (magic !== 0xAA50) continue;
    var type    = data[i + 2];
    var subtype = data[i + 3];
    var offset  = (data[i+4] | (data[i+5]<<8) | (data[i+6]<<16) | (data[i+7]<<24)) >>> 0;
    var size    = (data[i+8] | (data[i+9]<<8) | (data[i+10]<<16) | (data[i+11]<<24)) >>> 0;
    var nameBytes = data.subarray(i + 12, i + 28);
    var name = '';
    for (var j = 0; j < nameBytes.length && nameBytes[j] !== 0; j++)
      name += String.fromCharCode(nameBytes[j]);
    entries.push({ name: name, type: type, subtype: subtype, offset: offset, size: size });
  }
  return entries;
}

/** Read partition table from device flash and find ota_1 offset. */
export async function detectOta1Address(ctx) {
  var ptStr = await ctx.loader.readFlash(PT_ADDR, PT_READ_SIZE);
  var ptData = new Uint8Array(ptStr.length);
  for (var i = 0; i < ptStr.length; i++) ptData[i] = ptStr.charCodeAt(i);
  var parts = parsePartitionTable(ptData);
  for (var p = 0; p < parts.length; p++) {
    if (parts[p].name === 'ota_1') return parts[p].offset;
  }
  throw new Error('ota_1 partition not found in device partition table');
}

/* ── OTA data blob builder ── */

/** Build the 8 KB OTA data blob that selects a given slot (0, 1, or null = erased). */
export function buildOtaData(slot) {
  var blob = new Uint8Array(OTA_DATA_SIZE);
  blob.fill(0xFF);
  if (slot === null) return blob;
  var seq = slot + 1;
  var entry = new Uint8Array(32);
  entry.fill(0xFF);
  entry[0] = seq & 0xFF;
  entry[1] = (seq >> 8) & 0xFF;
  entry[2] = (seq >> 16) & 0xFF;
  entry[3] = (seq >> 24) & 0xFF;
  var c = espCrc32(entry.subarray(0, 4));
  entry[28] = c & 0xFF;
  entry[29] = (c >> 8) & 0xFF;
  entry[30] = (c >> 16) & 0xFF;
  entry[31] = (c >> 24) & 0xFF;
  blob.set(entry, 0);
  return blob;
}

/* ── esptool-js loader ── */

var _ESPLoader = null, _Transport = null;

/** Load esptool-js from CDN. Returns { ESPLoader, Transport }. */
export async function loadEspTool() {
  if (_ESPLoader && _Transport) return { ESPLoader: _ESPLoader, Transport: _Transport };
  var mod = await import(ESPTOOL_CDN);
  _ESPLoader = mod.ESPLoader;
  _Transport = mod.Transport;
  return { ESPLoader: _ESPLoader, Transport: _Transport };
}

/* ── Null terminal (suppresses esptool-js console noise) ── */
var nullTerm = {
  clean: function () {},
  writeLine: function (d) { console.log(d); },
  write: function () {} /* suppress raw byte dumps from readFlash/writeFlash */
};

/* ── Connect / disconnect ── */

/**
 * Open serial port and detect chip.
 * @param {object} [callbacks]
 * @param {function} [callbacks.onStatus] — status message callback
 * @returns {object} ctx — { loader, transport, chip } — pass to other functions
 */
export async function connectP4(callbacks) {
  var cb = callbacks || {};
  var status = cb.onStatus || function () {};

  if (!('serial' in navigator))
    throw new Error('WebSerial not supported. Use Chrome, Edge, or Opera.');

  var tools = await loadEspTool();
  status('Requesting serial port…');
  var port = await navigator.serial.requestPort({});
  var transport = new tools.Transport(port, true);

  status('Connecting to device…');
  var loader = new tools.ESPLoader({ transport: transport, baudrate: 460800, terminal: nullTerm });
  var chip = await loader.main();

  return { loader: loader, transport: transport, chip: chip };
}

/** Close serial transport. Safe to call multiple times. */
export async function disconnectP4(ctx) {
  if (!ctx) return;
  if (ctx.transport) {
    try { await ctx.transport.disconnect(); } catch (_) {}
  }
  ctx.loader = null;
  ctx.transport = null;
}

/* ── Flash firmware ── */

/**
 * Download a firmware binary and flash it to the given address.
 * @param {object} ctx — from connectP4()
 * @param {string} url — URL of the .bin file
 * @param {number} address — flash address (e.g. 0x0 for app)
 * @param {object} [callbacks]
 * @param {function} [callbacks.onStatus]
 * @param {function} [callbacks.onProgress] — receives percent (0–100)
 */
export async function flashP4(ctx, url, address, callbacks) {
  var cb = callbacks || {};
  var status = cb.onStatus || function () {};
  var progress = cb.onProgress || function () {};

  status('Downloading firmware…');
  var resp = await fetch(url);
  if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
  var fw = new Uint8Array(await resp.arrayBuffer());
  var sizeMB = (fw.length / 1024 / 1024).toFixed(1);

  status('Flashing ' + sizeMB + ' MB to 0x' + address.toString(16) + ' — do not unplug…');
  await ctx.loader.writeFlash({
    fileArray: [{ data: toBinStr(fw), address: address }],
    flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
    eraseAll: false, compress: true,
    reportProgress: function (_, written, total) {
      progress(Math.round(written / total * 100));
    }
  });
  progress(100);
}

/* ── MSC + OTA operations ── */

/**
 * Flash tusb_msc.bin to ota_1, switch OTA to boot from it.
 * Used for Full SD Card Deploy (puts device into Mass Storage mode).
 * @param {object} ctx — from connectP4()
 * @param {string} mscUrl — URL of tusb_msc.bin
 * @param {object} [callbacks]
 */
export async function flashMscAndSwitchOta(ctx, mscUrl, callbacks) {
  var cb = callbacks || {};
  var status = cb.onStatus || function () {};
  var progress = cb.onProgress || function () {};

  /* Try to read partition table; fall back to known TBD-16 address */
  var ota1Addr;
  try {
    status('Reading partition table…');
    ota1Addr = await detectOta1Address(ctx);
  } catch (e) {
    console.warn('Partition table read failed, using default ota_1 address:', e);
    ota1Addr = 0x510000;
  }

  /* Download + flash tusb_msc.bin */
  status('Downloading MSC firmware…');
  var resp = await fetch(mscUrl);
  if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
  var fw = new Uint8Array(await resp.arrayBuffer());
  var sizeMB = (fw.length / 1024 / 1024).toFixed(1);

  status('Flashing tusb_msc.bin (' + sizeMB + ' MB) to ota_1 @ 0x' + ota1Addr.toString(16) + '…');
  await ctx.loader.writeFlash({
    fileArray: [{ data: toBinStr(fw), address: ota1Addr }],
    flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
    eraseAll: false, compress: true,
    reportProgress: function (_, written, total) {
      progress(Math.round(written / total * 100));
    }
  });
  progress(100);

  /* Switch OTA to slot 1 (tusb_msc) */
  status('Switching boot partition to ota_1…');
  var otaBlob = buildOtaData(1);
  await ctx.loader.writeFlash({
    fileArray: [{ data: toBinStr(otaBlob), address: OTA_DATA_ADDR }],
    flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
    eraseAll: false, compress: true
  });
}

/**
 * Write OTA data to select a boot slot.
 * @param {object} ctx
 * @param {number|null} slot — 0 = ota_0 (normal), 1 = ota_1 (MSC), null = erased (default)
 * @param {object} [callbacks]
 */
export async function switchOtaSlot(ctx, slot, callbacks) {
  var cb = callbacks || {};
  var status = cb.onStatus || function () {};

  status(slot === null ? 'Erasing OTA data (selects normal firmware)…' : 'Switching to ota_' + slot + '…');
  var otaBlob = buildOtaData(slot);
  await ctx.loader.writeFlash({
    fileArray: [{ data: toBinStr(otaBlob), address: OTA_DATA_ADDR }],
    flashSize: '16MB', flashMode: 'dio', flashFreq: '80m',
    eraseAll: false, compress: true
  });
}

/** Hard-reset the device. Returns true on success, false if reset signal failed. */
export async function resetDevice(ctx) {
  try {
    await ctx.loader.after('hard_reset');
    return true;
  } catch (e) {
    console.warn('Software reset failed:', e);
    return false;
  }
}
