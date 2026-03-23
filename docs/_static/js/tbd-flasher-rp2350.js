/**
 * tbd-flasher-rp2350.js — RP2350 flashing via WebUSB (Picoboot)
 *
 * Provides:
 *   - loadPicoboot(basePath)  → load Picoboot + UF2 parser from local modules
 *   - connectRP2350(callbacks) → WebUSB device selection + connect
 *   - flashRP2350(ctx, url, callbacks) → download UF2 + erase + write
 *   - rebootRP2350(ctx, callbacks) → reboot the device
 *   - disconnectRP2350(ctx)   → close USB connection
 *
 * Usage from an RST page:
 *   import { loadPicoboot, connectRP2350, flashRP2350, rebootRP2350 } from '../_static/js/tbd-flasher-rp2350.js';
 *   await loadPicoboot('../_static/picoflash');
 *   var ctx = await connectRP2350({ onStatus: msg => ... });
 *   await flashRP2350(ctx, 'https://...firmware.uf2', { onProgress: pct => ... });
 *   await rebootRP2350(ctx);
 */

/* ── Module state ── */
var _Picoboot = null;
var _PicobootCmd = null;
var _PicobootCmdId = null;
var _uf2ToFlashBuffer = null;

/* ── Loader ── */

/**
 * Load Picoboot and UF2 parser from local modules.
 * @param {string} basePath — path to the picoflash directory (e.g. '../_static/picoflash')
 */
export async function loadPicoboot(basePath) {
  if (_Picoboot && _uf2ToFlashBuffer) return;
  /* Resolve basePath relative to the HTML page (document.baseURI),
     not relative to this module — avoids double-_static path bug. */
  var base = new URL(basePath, document.baseURI).href;
  var picoMod = await import(base + '/pkg/index.js');
  _Picoboot = picoMod.Picoboot;
  _PicobootCmd = picoMod.PicobootCmd;
  _PicobootCmdId = picoMod.PicobootCmdId;
  var uf2Mod = await import(base + '/js/uf2.js');
  _uf2ToFlashBuffer = uf2Mod.uf2ToFlashBuffer;
}

/* ── Connect ── */

/**
 * Request a Picoboot device via WebUSB and connect.
 * The RP2350 must be in BOOTSEL mode.
 * @param {object} [callbacks]
 * @param {function} [callbacks.onStatus]
 * @returns {object} ctx — { pico, connection, info }
 */
export async function connectRP2350(callbacks) {
  var cb = callbacks || {};
  var status = cb.onStatus || function () {};

  if (!('usb' in navigator))
    throw new Error('WebUSB not supported. Use Chrome, Edge, or Opera.');

  if (!_Picoboot)
    throw new Error('Picoboot not loaded. Call loadPicoboot() first.');

  status('Waiting for device selection… choose RP2350 Boot');
  var pico = await _Picoboot.requestDevice();

  status('Connecting…');
  var connection = await pico.connect();
  await connection.resetInterface();

  var info = pico.getUsbDeviceInfo();
  return { pico: pico, connection: connection, info: info };
}

/* ── Flash ── */

/**
 * Download a UF2 file and flash it to the RP2350.
 * @param {object} ctx — from connectRP2350()
 * @param {string} url — URL of the .uf2 file
 * @param {object} [callbacks]
 * @param {function} [callbacks.onStatus]
 * @param {function} [callbacks.onProgress] — receives percent (0–100)
 */
export async function flashRP2350(ctx, url, callbacks) {
  var cb = callbacks || {};
  var status = cb.onStatus || function () {};
  var progress = cb.onProgress || function () {};

  if (!_uf2ToFlashBuffer)
    throw new Error('UF2 parser not loaded. Call loadPicoboot() first.');

  status('Downloading UF2 firmware…');
  progress(10);
  var resp = await fetch(url);
  if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
  var uf2Data = new Uint8Array(await resp.arrayBuffer());
  progress(25);

  status('Parsing UF2 file…');
  var parsed = _uf2ToFlashBuffer(uf2Data);
  var sizeKB = (parsed.data.length / 1024).toFixed(0);
  var isRam = parsed.address >= 0x20000000;

  /* Tag context so rebootRP2350 knows how to restart the device */
  ctx._ramAddress = isRam ? parsed.address : null;

  if (isRam) {
    status('Writing ' + sizeKB + ' KB to SRAM…');
    progress(35);
    await ctx.connection.resetInterface();
    await ctx.connection.exitXip();
    await ctx.connection.flashWrite(parsed.address, parsed.data);
    progress(100);
  } else {
    status('Erasing & writing ' + sizeKB + ' KB to flash…');
    progress(35);
    await ctx.pico.flashEraseAndWrite(parsed.address, parsed.data);
    progress(100);
  }
}

/* ── Reboot ── */

/**
 * Reboot the RP2350 device.
 * @param {object} ctx — from connectRP2350()
 * @param {object} [callbacks]
 * @param {function} [callbacks.onStatus]
 */
export async function rebootRP2350(ctx, callbacks) {
  var cb = callbacks || {};
  var status = cb.onStatus || function () {};

  try {
    if (ctx._ramAddress && _PicobootCmd && _PicobootCmdId) {
      /* RAM UF2 was loaded — use EXEC to jump directly to the SRAM code.
         Standard reboot would boot from flash, ignoring the SRAM data. */
      status('Executing SRAM firmware…');
      var args = new Uint8Array(16);
      new DataView(args.buffer).setUint32(0, ctx._ramAddress, true);
      var cmd = new _PicobootCmd(_PicobootCmdId.EXEC, 4, 0, args);
      await ctx.connection.sendCmd(cmd, null);
    } else {
      /* Flash UF2 — normal reboot picks it up from flash */
      status('Rebooting device…');
      await ctx.connection.reboot(100);
    }
  } catch (e) {
    /* Expected: device disconnects immediately on exec/reboot */
    console.warn('Reboot/exec command error (may be expected):', e.message);
  }
  await disconnectRP2350(ctx);
}

/* ── Disconnect ── */

/** Disconnect from the RP2350. Safe to call multiple times. */
export async function disconnectRP2350(ctx) {
  if (!ctx) return;
  try {
    if (ctx.pico) await ctx.pico.disconnect();
  } catch (_) {}
  ctx.pico = null;
  ctx.connection = null;
}
