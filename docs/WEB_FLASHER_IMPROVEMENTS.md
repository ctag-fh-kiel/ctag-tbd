# Web Flasher Robustness Improvements

Analysis of failure modes in the browser-based flash pages vs CLI tools, and proposed fixes.
This is the single source of truth — every improvement is tracked here with its status.

## Background

The web flash pages (`docs/flash/10_stable_channel.rst`, `20_staging_channel.rst`) use
`tbd-flasher-p4.js` with esptool-js over WebSerial. The CLI approach uses `esptool.py`
and `otatool.py` over pyserial. Both write identical data, but the mechanisms for
partition detection, reset, and error handling differ significantly.

---

## Implementation Status Overview

| # | Improvement | Status | Files Changed |
|---|-------------|--------|---------------|
| 1 | [Silent reset failure → visible](#1-silent-reset-failure-critical) | **DONE** | `tbd-flasher-p4.js`, both `.rst` pages |
| 2 | [Hardcoded ota_1 address → dynamic](#2-hardcoded-ota_1-address-medium-risk) | **DONE** | `tbd-flasher-p4.js` |
| 3 | [Post-reset verification](#3-no-post-reset-verification) | **DEFERRED** (complex, WebSerial limitation) | `tbd-flasher-p4.js` |
| 4 | [Skip-step-1 link more visible](#4-monolithic-step-1-function) | **DONE** | both `.rst` pages |
| 5a | [Countdown timer 20s → 25s](#5-countdown-timer-too-short--no-escalation) | **DONE** | both `.rst` pages |
| 5b | [Countdown message includes 25-30s note](#5-countdown-timer-too-short--no-escalation) | **DONE** | both `.rst` pages |
| 5c | [Post-countdown escalating recovery steps](#5-countdown-timer-too-short--no-escalation) | **DONE** | both `.rst` pages |
| 6 | [Erase SD card before writing](#6-erase-sd-card-before-writing) | **DONE** | both `.rst` pages |
| 7 | [Troubleshooting page: MSC failure guidance](#7-troubleshooting-page-msc-failure-guidance) | **DONE** | `50_troubleshooting.rst` |
| 8 | [Troubleshooting page: boot timing 25-30s](#8-troubleshooting-page-boot-timing) | **DONE** | `50_troubleshooting.rst` |
| 9 | [Retry-reset button after countdown](#9-retry-reset-button) | **DONE** | both `.rst` pages |
| 10 | [Power-cycle messaging over reset button](#10-power-cycle-messaging) | **DONE** | both `.rst` pages, `50_troubleshooting.rst` |
| 11 | [SD erase data loss warning + confirmation](#11-sd-erase-data-loss-warning) | **DONE** | both `.rst` pages |
| 12 | [Port selection guidance](#12-port-selection-guidance) | **DONE** | both `.rst` pages |

---

## Web Flash vs CLI: Structural Differences

| Aspect | Web Flash (esptool-js) | CLI (otatool.py / esptool.py) |
|--------|------------------------|-------------------------------|
| **ota_1 address** | Hardcoded `0x510000` | Reads partition table dynamically |
| **Partition validation** | None — writes blindly | Fails safely if partition not found |
| **Device reset** | DTR/RTS via WebSerial (unreliable) | No auto-reset — user resets manually |
| **Reset failure handling** | Silent `catch` + `console.warn` | N/A (not attempted) |
| **OTA data format** | Correct CRC-32, matches bootloader | Identical format |
| **User feedback on failure** | "Done!" shown even if reset failed | Manual steps = user always knows state |
| **Serial driver** | WebSerial (browser-limited) | pyserial (full OS driver access) |
| **SD card erase** | ~~None — writes over existing~~ Now erases first | Manual `rm -rf` in terminal |

---

## The Improvements

### 1. Silent Reset Failure (Critical)

**Status: DONE**

**Current code** (`tbd-flasher-p4.js` line ~276):
```javascript
export async function resetDevice(ctx) {
  try { await ctx.loader.after('hard_reset'); } catch (e) {
    console.warn('Software reset failed (expected on some setups):', e);
  }
}
```

The `hard_reset` uses DTR/RTS signal toggling via WebSerial's `setSignals()`. This fails
silently on many USB-serial adapters and WebSerial implementations. The error is caught and
logged to `console.warn` — the user never sees it. The UI proceeds to the countdown timer
showing "device should reboot" even when the device hasn't rebooted.

**Fix:** Make reset failure visible instead of silent.

Change `resetDevice()` in `tbd-flasher-p4.js` to return success/failure:
```javascript
export async function resetDevice(ctx) {
  var resetOk = true;
  try {
    await ctx.loader.after('hard_reset');
  } catch (e) {
    console.warn('Software reset failed:', e);
    resetOk = false;
  }
  return resetOk;
}
```

Then in the `btn1Go` handler (both `.rst` pages), use the return value:
```javascript
var resetOk = await resetDevice(ctxB1);
await disconnectP4(ctxB1); ctxB1 = null;
startRebootCountdown(resetOk);
```

And update `startRebootCountdown()` to show a prominent power-cycle banner when
`resetOk === false`:
```javascript
function startRebootCountdown(resetOk) {
  // ... existing countdown logic ...
  if (!resetOk) {
    setStat(stat1,
      '⚠️ <b>Software reset failed.</b> Please power-cycle the device:<br>' +
      '&nbsp;&nbsp;Unplug <b>both</b> USB cables, wait 3 s, replug them.<br>' +
      '<small>The SD card drive should appear within 25–30 seconds.</small>', 'warn');
  }
}
```

Also update the `skip1` handler and `btn3Go` handler (Step 3 switch-back) — they also call
`resetDevice()` and should handle failure the same way.

**Files:** `docs/_static/js/tbd-flasher-p4.js`, `docs/flash/10_stable_channel.rst`,
`docs/flash/20_staging_channel.rst`

### 2. Hardcoded ota_1 Address (Medium Risk)

**Status: DONE**

**Current code** (`flashMscAndSwitchOta`, line ~225):
```javascript
var ota1Addr = 0x510000;
```

This is correct for `partitions_example.csv` (the SD-card config), but `partitions_no_sd.csv`
has **no ota_1 partition at all**. Writing to `0x510000` on that config would corrupt the
`sample_rom` partition (which starts at `0x530000`, close enough that the erase block could
overlap).

The function `detectOta1Address()` already exists in the same file (lines 79–96) — it reads
the partition table from the device and finds ota_1 dynamically. But `flashMscAndSwitchOta`
doesn't use it.

**Fix:** Use dynamic detection in `tbd-flasher-p4.js`:
```javascript
export async function flashMscAndSwitchOta(ctx, mscUrl, callbacks) {
  var cb = callbacks || {};
  var status = cb.onStatus || function () {};

  status('Reading partition table…');
  var ota1Addr = await detectOta1Address(ctx);
  // ... rest unchanged, but now ota1Addr is verified ...
}
```

This also provides a safety check — if ota_1 doesn't exist, `detectOta1Address()` throws
`'ota_1 partition not found'`, which the caller catches and displays as an error. Much better
than silently corrupting flash.

**Risk assessment:** Currently low risk because all web-flash users have the SD-card partition
layout. But if no-SD configs ever reach web flash, this would be a data-corruption bug.

**Files:** `docs/_static/js/tbd-flasher-p4.js`

### 3. No Post-Reset Verification

**Status: DEFERRED** (not practical with current WebSerial permission model)

After `resetDevice()`, there's no way to confirm the device actually rebooted. The countdown
timer just counts down and says "Ready. Look for the 'NO NAME' drive."

**Possible fix:** After reset + disconnect, attempt a brief serial reconnect after ~3 seconds.
If the serial port is no longer available (expected — MSC firmware doesn't expose serial on
the JTAG port), that's a positive signal the device rebooted into MSC mode. If serial
reconnects normally (same chip detected), the reset likely failed.

```javascript
async function verifyReboot() {
  await new Promise(r => setTimeout(r, 3000));
  try {
    var testCtx = await connectP4({});
    await disconnectP4(testCtx);
    return false; // Still responsive = didn't reboot
  } catch (e) {
    return true; // Can't connect = likely rebooted into MSC mode ✓
  }
}
```

**Why deferred:** WebSerial's `requestPort()` requires a user gesture (click) each time.
We can't silently reconnect without the user clicking a button and re-selecting the port.
This makes automatic verification impractical. The visible reset failure (Issue #1) is the
better solution — tell the user when the reset might have failed, rather than trying to
detect it automatically.

### 4. Monolithic Step 1 Function

**Status: DONE**

`flashMscAndSwitchOta()` bundles three operations (flash MSC → write OTA data → reset) into
one function. If the reset fails, the user is in a partially-complete state: OTA data already
points to ota_1, but the device hasn't rebooted yet.

The "Skip step 1" link partially addresses this — it calls `switchOtaSlot(ctx, 1)` + reset
without re-flashing tusb_msc.bin. But it's a small gray text link that's easy to miss.

**Fix:** Make the skip link more visible:
- Increase font size from `0.82em` to `0.9em`
- Change color from `#6B7280` (gray) to `#2563EB` (blue) 
- Add an info icon or border to make it stand out
- Consider showing it always (not only after `resetAllSteps`)

**Files:** `docs/flash/10_stable_channel.rst`, `docs/flash/20_staging_channel.rst`

### 5. Countdown Timer Too Short + No Escalation

**Status: DONE** (all three sub-items complete)

The countdown timer (`REBOOT_WAIT`) was 20 seconds, now updated to 25 seconds. The
real-world SD card initialization can take 25–30 seconds due to:

- UHS-II tuning sequence (speed negotiation with SD card)
- `fs.cpp` retry logic: 5 attempts with escalating settle times (200ms → 300ms → 500ms →
  800ms → 1000ms) plus 500ms between attempts
- macOS volume mount latency after USB MSC enumeration

**5a — DONE:** Timer increased from 20s to 25s (`REBOOT_WAIT = 25` in both `.rst` pages).

**5b — DONE:** Countdown message now includes "The SD card can take 25–30 s to initialize —
this is normal" in both flash pages.

**5c — DONE:** After the countdown reaches 0, shows escalating recovery steps with
power-cycle as the primary action, plus a "Retry software reset" button (see Issue #9).

**Files:** `docs/flash/10_stable_channel.rst`, `docs/flash/20_staging_channel.rst`

### 6. Erase SD Card Before Writing

**Status: DONE**

Previously, Step 2 (Write SD Card) just wrote new files on top of whatever was already on
the SD card. Leftover files from old versions (especially `spm-config.json` with its
`availableProcessors` cache) caused failures where new plugins wouldn't appear.

Users had to manually erase the SD card via Disk Utility or terminal before running Step 2.
This was a friction point and a common source of user-testing failures.

**Fix implemented:** Added `eraseDirectory()` function that recursively deletes all files and
subdirectories from the selected drive before downloading and extracting the ZIP. This happens
automatically — the user just selects the drive and everything is handled.

The Step 2 description was updated to say: "All existing files will be **erased** first, then
the SD card image will be downloaded from GitHub, extracted, and written."

Progress bar allocation was adjusted to account for the erase phase:
- 0–5%: Erase SD card
- 5–45%: Download ZIP
- 45–95%: Extract + write files  
- 95–100%: macOS metadata cleanup + hash files

**Files changed:** `docs/flash/10_stable_channel.rst`, `docs/flash/20_staging_channel.rst`

### 7. Troubleshooting Page: MSC Failure Guidance

**Status: DONE**

The troubleshooting page (`50_troubleshooting.rst`) previously had minimal MSC guidance.
Expanded the "SD Card (Path B)" section with:

- **"SD card drive doesn't appear after Step 1"** — rewritten with correct 25–30s timing,
  explained silent software reset failure, added OS-specific tips (macOS/Windows/Linux)
- **"SD card drive appears but disappears quickly"** — new entry about UHS-II init issues
- **"Permission denied"** — expanded: browser permission vs filesystem-still-initializing
- **"Step 3 connection fails"** — expanded with per-OS eject instructions
- **"Browser crashed or tab closed"** — entirely new entry explaining recovery via "Skip step 1"
- **"Device stuck in MSC mode"** — entirely new entry for OTA data recovery

**Files changed:** `docs/flash/50_troubleshooting.rst`

### 8. Troubleshooting Page: Boot Timing

**Status: DONE**

Updated "Device not reachable at http://192.168.4.1" timing from "15–30 seconds" to
"25–30 seconds" to match real-world boot times. Added hard-refresh tip.

**Files changed:** `docs/flash/50_troubleshooting.rst`

### 9. Retry Reset Button

**Status: DONE**

After the countdown timer finishes, a "🔄 Retry software reset" button appears. When
clicked, it reconnects to the device via WebSerial (the user must re-select the serial port),
sends another DTR/RTS hardware reset, and restarts the countdown.

This helps when:
- The first DTR/RTS reset was a fluke (timing-dependent failures are common)
- The user power-cycled but the device booted back into normal firmware instead of MSC
  (OTA data is still set to ota_1, so a reset will try MSC again)

If the serial port can't be found (device already in MSC mode), the button shows a
fallback message recommending power-cycle.

**Files changed:** `docs/flash/10_stable_channel.rst`, `docs/flash/20_staging_channel.rst`

### 10. Power-Cycle Messaging

**Status: DONE**

All manual reset instructions now prioritize **power-cycling** (unplug both cables, wait 3 s,
replug) over pressing the RESET button. Power-cycling is:
- More intuitive for users unfamiliar with the hardware
- Always works (the reset button requires knowing its location)
- The same action regardless of device variant

References to the RESET button have been removed from all flash page messages and
the troubleshooting page.

**Files changed:** `docs/flash/10_stable_channel.rst`, `docs/flash/20_staging_channel.rst`,
`docs/flash/50_troubleshooting.rst`

### 11. SD Erase Data Loss Warning

**Status: DONE**

Path B (Full SD Card Deploy) erases the entire SD card and writes the factory image. This
destroys user data: audio samples, macro definitions, sound presets, and any custom files.
This is by design — Path B is for fresh installs and recovery, not routine updates. But
users need to understand the consequences before proceeding.

Three layers of warning were added:

1. **Path B option card** — red warning text at the bottom: "⚠️ This erases ALL data on the
   SD card — samples, macros, presets, and any custom files will be permanently deleted."

2. **Step 2 description** — red-bordered callout box listing exactly what gets erased
   (samples, macro definitions, sound presets, custom files) and that the factory image
   replaces them.

3. **Browser `confirm()` dialog** — after the user selects the SD card drive but before
   any files are deleted, a native confirmation dialog appears listing every data type that
   will be lost. The user must click OK to proceed. Clicking Cancel aborts safely.

**Files changed:** `docs/flash/10_stable_channel.rst`, `docs/flash/20_staging_channel.rst`

---

## Would Flashing Normal P4 Firmware First Help?

**No.** The transition to MSC mode depends on:
1. tusb_msc.bin correctly written to ota_1 ✓ (both approaches do this reliably)
2. OTA data correctly written to 0xd000 ✓ (both approaches produce identical data)
3. Device actually resetting ✗ (this is where web flash fails)

Having old vs new firmware on ota_0 doesn't affect the MSC transition. The device just needs
to reboot — it doesn't matter what was running before.

Step 4 (flash P4 firmware after SD card write) also self-recovers: esptool talks to the ROM
bootloader directly over serial, bypassing whatever firmware is running. So even if the device
is still in MSC mode at Step 4, the flash will succeed.

---

## Priority Order for Remaining Work

All items are implemented except Issue #3 (post-reset verification), which is deferred
due to WebSerial permission model limitations.

---

### 12. Port Selection Guidance

**Status: DONE**

The WebSerial port picker shows multiple ports (Bluetooth, debug-console, Debug Probe
CMSIS-DAP, USB JTAG/serial debug unit, etc.) and users frequently select the wrong one.
The correct port for all serial operations (MSC firmware flash and P4 firmware flash)
is **"USB JTAG/serial debug unit"**, not "Debug Probe (CMSIS-DAP)".

Port selection hints were added to every Connect interaction:

1. **Step descriptions** — all steps that have a Connect button now explicitly say
   "select **USB JTAG/serial debug unit** in the port picker (not Debug Probe)".

2. **Static HTML status messages** (`stat3`, `stat4`) — updated from "front JTAG port"
   to "USB JTAG/serial debug unit".

3. **JS-generated status messages** (`statA1`, `stat3` in `resetAllSteps` and after
   SD write completion) — all updated to reference the correct port name.

The physical port descriptions (telling users which cable to connect) still reference
"front JTAG port" since that describes the hardware location, not the browser dialog.

**Files changed:** `docs/flash/10_stable_channel.rst`, `docs/flash/20_staging_channel.rst`

---

## Files Reference

| File | What it does |
|------|-------------|
| `docs/_static/js/tbd-flasher-p4.js` | Shared JS: esptool-js wrappers, `resetDevice()`, `flashMscAndSwitchOta()`, `detectOta1Address()` |
| `docs/flash/10_stable_channel.rst` | Stable channel flash page (HTML + inline JS) |
| `docs/flash/20_staging_channel.rst` | Beta/staging channel flash page (HTML + inline JS, mirrors stable) |
| `docs/flash/50_troubleshooting.rst` | User-facing troubleshooting docs |
