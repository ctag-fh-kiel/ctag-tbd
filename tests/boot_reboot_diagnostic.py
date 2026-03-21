#!/usr/bin/env python3
"""
ESP32-P4 Boot Reboot Diagnostic Tool

Interactive diagnostic for investigating ESP32-P4 boot behavior.
Supports two test scenarios:
  1. Cold boot — USB-C cable unplug/replug
  2. Software reboot — via Sequencer UI hardware command

Success criteria checked:
  - Audio is processing (Audio task CPU time reported)
  - SPI real-time stream connected (audio CPU time > 100µs = synth engines
    processing incoming SPI data; idle = ~10µs)
  - No USB NCM errors
  - No unexpected reboots during boot
  - Boot time measured (first ESP log → stable operation)

Usage:
    # Scenario 1: Cold boot testing (3-5 USB-C replug cycles)
    python3 boot_reboot_diagnostic.py --scenario cold --cycles 5

    # Scenario 2: Software reboot testing
    python3 boot_reboot_diagnostic.py --scenario reboot --cycles 5

    # Specify port explicitly:
    python3 boot_reboot_diagnostic.py --port /dev/tty.usbmodem1101 --scenario cold
"""

import argparse
import os
import re
import sys
import time
from datetime import datetime
from pathlib import Path

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("ERROR: pyserial not installed. Install with: pip3 install pyserial")
    sys.exit(1)


# ── ESP32-P4 reset reason codes ──────────────────────────────
RESET_REASONS = {
    0x01: "POWERON_RESET      (power-on / chip reset)",
    0x03: "SW_RESET           (software reset via esp_restart())",
    0x05: "DEEPSLEEP_RESET    (deep sleep wakeup)",
    0x07: "TG0WDT_SYS_RESET  (Timer Group 0 watchdog)",
    0x08: "TG1WDT_SYS_RESET  (Timer Group 1 watchdog)",
    0x09: "RTCWDT_SYS_RESET  (RTC watchdog)",
    0x0B: "RTCWDT_CPU_RESET  (RTC watchdog CPU)",
    0x0C: "EXT_CPU_RESET      (external CPU reset)",
    0x0D: "RTCWDT_BROWN_OUT   (brownout reset)",
    0x0E: "RTCWDT_RTC_RESET  (RTC watchdog RTC)",
    0x0F: "TG0WDT_CPU_RESET  (Timer Group 0 WDT CPU)",
    0x10: "TG1WDT_CPU_RESET  (Timer Group 1 WDT CPU)",
    0x14: "JTAG_RESET         (JTAG reset)",
}

# ── Detection patterns ───────────────────────────────────────

# Critical crash/reboot indicators
REBOOT_PATTERNS = [
    (re.compile(r"rst:(0x[0-9a-fA-F]+)"), "RESET_VECTOR"),
    (re.compile(r"boot:(0x[0-9a-fA-F]+)"), "BOOT_MODE"),
    (re.compile(r"Guru Meditation Error", re.I), "GURU_MEDITATION"),
    (re.compile(r"abort\(\)"), "ABORT"),
    (re.compile(r"assert failed", re.I), "ASSERT_FAIL"),
    (re.compile(r"Backtrace:\s*(.+)"), "BACKTRACE"),
    (re.compile(r"panic", re.I), "PANIC"),
    (re.compile(r"watchdog", re.I), "WATCHDOG"),
    (re.compile(r"stack overflow", re.I), "STACK_OVERFLOW"),
    (re.compile(r"LoadProhibited|StoreProhibited|InstrFetchProhibited", re.I), "MEMORY_FAULT"),
    (re.compile(r"brownout", re.I), "BROWNOUT"),
    (re.compile(r"Rebooting device!", re.I), "SPI_REBOOT_CMD"),
    (re.compile(r"Rebooting device to OTA", re.I), "SPI_OTA_REBOOT"),
    (re.compile(r"esp_restart", re.I), "ESP_RESTART"),
]

# Boot milestone patterns (ordered roughly by expected occurrence)
MILESTONE_PATTERNS = [
    (re.compile(r"cpu_start.*app_cpu_up", re.I), "APP_CPU_UP"),
    (re.compile(r"app_main\(\)"), "APP_MAIN_ENTER"),
    (re.compile(r"InitFS|FileSystem::InitFS", re.I), "FILESYSTEM_INIT"),
    (re.compile(r"sd-card mounted", re.I), "SD_CARD_MOUNTED"),
    (re.compile(r"SD card mount failed after", re.I), "SD_CARD_MOUNT_FAILED"),
    (re.compile(r"SD card mount attempt.*failed", re.I), "SD_CARD_RETRY"),
    (re.compile(r"read_tuning_block.*returned", re.I), "UHS1_TUNING_ERROR"),
    (re.compile(r"InitLedRGB|LedRGB::Init", re.I), "LED_INIT"),
    (re.compile(r"StartSoundProcessor"), "SP_MANAGER_START"),
    (re.compile(r"tusb.*Init|TinyUSB|tusb::Init", re.I), "TINYUSB_INIT"),
    (re.compile(r"Codec.*Init|InitCodec", re.I), "CODEC_INIT"),
    (re.compile(r"SynthDefinition", re.I), "SYNTH_DEFS_LOADED"),
    (re.compile(r"MacroDeviceDefinition", re.I), "DEVICE_DEFS_LOADED"),
    (re.compile(r"MacroSoundPreset", re.I), "SOUND_PRESETS_LOADED"),
    (re.compile(r"NCM interface ready|NCM.*ready", re.I), "USB_NCM_READY"),
    (re.compile(r"WaitForNCMReady", re.I), "USB_NCM_WAIT"),
    (re.compile(r"Network.*Up|Network::Up", re.I), "NETWORK_UP"),
    (re.compile(r"DHCP.*started|dhcps.*start", re.I), "DHCP_START"),
    (re.compile(r"DHCP.*assigned|dhcps.*assigned", re.I), "HOST_CONNECTED"),
    (re.compile(r"Starting HTTP|RestServer", re.I), "HTTP_SERVER"),
    (re.compile(r"SpiAPI.*Init|StartSpiAPI", re.I), "SPI_API_INIT"),
    (re.compile(r"rp2350.*Init|rp2350_spi_stream", re.I), "RP2350_SPI_INIT"),
    (re.compile(r"link.*Init|Ableton.*Link", re.I), "ABLETON_LINK"),
    (re.compile(r"Audio task started"), "AUDIO_TASK_START"),
    (re.compile(r"audio_task.*main loop", re.I), "AUDIO_LOOP_START"),
    (re.compile(r"PicoSeqRack", re.I), "PICOSEQRACK_LOADED"),
    (re.compile(r"sample.*rom|ctagSampleRom", re.I), "SAMPLE_ROM"),
    (re.compile(r"Loading ch0|Loading ch1", re.I), "PLUGIN_LOAD"),
    (re.compile(r"setTrackMachine|SetTrackMachine", re.I), "TRACK_MACHINE_SET"),
    (re.compile(r"Audio task CPU time", re.I), "AUDIO_RUNNING"),
    (re.compile(r"Mem freesize internal", re.I), "MEMORY_REPORT"),
    (re.compile(r"Failed to send buffer to USB|USB NCM send failed", re.I), "USB_SEND_ERROR"),
    (re.compile(r"SPI receive timeout", re.I), "SPI_TIMEOUT"),
]

# SPI stream success counter in debug_task output:
#   "counters: tx-err=N queue-err=N parse-err=N success=N"
SPI_COUNTERS_RE = re.compile(
    r"tx-err=(\d+)\s+queue-err=(\d+)\s+parse-err=(\d+)\s+success=(\d+)"
)


def find_serial_port():
    """Auto-detect the ESP32-P4 JTAG debug serial port."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        hwid = (port.hwid or "").upper()
        if "303A:1001" in hwid:
            # Prefer tty variant on macOS (cu variant often busy)
            if "cu." in port.device:
                tty_variant = port.device.replace("/dev/cu.", "/dev/tty.")
                if os.path.exists(tty_variant):
                    return tty_variant
            return port.device
    for port in ports:
        desc = (port.description or "").lower()
        if "jtag" in desc and "serial" in desc:
            return port.device
        if "usbmodem" in port.device.lower():
            return port.device
    for port in ports:
        if "ttyUSB" in port.device or "ttyACM" in port.device:
            return port.device
    return None


def decode_reset_reason(hex_str):
    """Decode an ESP32-P4 reset reason code."""
    try:
        code = int(hex_str, 16)
        return RESET_REASONS.get(code, f"UNKNOWN_RESET (0x{code:02X})")
    except ValueError:
        return f"INVALID ({hex_str})"


class BootDiagnostic:
    """Track state across a single capture cycle."""

    def __init__(self, scenario="cold"):
        self.scenario = scenario
        self.boot_count = 0
        self.current_boot_start = None
        self.reboot_events = []     # (wall_time, type, detail)
        self.milestones = []        # (wall_time, esp_time_ms, label, line)
        self.errors = []            # (wall_time, esp_time_ms, label, line)
        self.reset_reasons = []     # (wall_time, code, decoded)
        self.usb_error_count = 0
        self.spi_timeout_count = 0
        self.memory_report_count = 0
        self.lines = []
        self.serial_disconnects = 0
        # Success criteria tracking
        self.audio_running = False
        self.audio_cpu_times = []       # list of CPU time values in µs
        self.spi_stream_success = 0     # last seen success counter (NOTE: dead metric, never incremented in firmware)
        self.spi_stream_tx_err = 0
        self.spi_stream_queue_err = 0
        self.spi_stream_parse_err = 0
        self.first_esp_time_ms = None   # earliest ESP timestamp seen
        self.last_esp_time_ms = None    # latest ESP timestamp seen
        self.time_to_audio = None       # ESP ms from boot to first audio running
        self.time_to_spi_ok = None      # ESP ms from boot to SPI stream connected (high CPU time)
        self.last_spi_timeout_ms = None # ESP ms of the last SPI timeout (stream connects after this)
        self.spi_stream_connected = False  # True when SPI timeouts stop and audio CPU > 100µs
        self.extra_reboots = 0          # reboots beyond the expected 1 (or 0 for cold)
        self.sd_card_mounted = False     # True when SD card mount succeeds
        self.sd_card_retries = 0        # number of SD card mount retries
        self.sd_card_failed = False     # True when SD card mount failed after all attempts
        self.uhs1_tuning_errors = 0    # UHS-I tuning block errors (should be 0 after fix)

    def process_line(self, wall_ts, line):
        """Process a single log line."""
        self.lines.append((wall_ts, line))

        # Extract ESP-IDF timestamp if present, e.g. "I (12345) tag: message"
        esp_time_ms = None
        m = re.search(r"[EWID] \((\d+)\)", line)
        if m:
            esp_time_ms = int(m.group(1))
            if self.first_esp_time_ms is None:
                self.first_esp_time_ms = esp_time_ms
            self.last_esp_time_ms = esp_time_ms

        # Check for reset vector (indicates a new boot)
        rst_match = re.search(r"rst:(0x[0-9a-fA-F]+)", line)
        if rst_match:
            code = rst_match.group(1)
            decoded = decode_reset_reason(code)
            self.reset_reasons.append((wall_ts, code, decoded))
            self.boot_count += 1
            self.current_boot_start = wall_ts
            self.reboot_events.append((wall_ts, "RESET_VECTOR", f"rst:{code} = {decoded}"))

        # Check reboot/crash patterns
        for pattern, label in REBOOT_PATTERNS:
            if label == "RESET_VECTOR":
                continue
            if pattern.search(line):
                self.reboot_events.append((wall_ts, label, line.strip()[:120]))
                if label == "SPI_REBOOT_CMD":
                    self.errors.append((wall_ts, esp_time_ms, "SPI_REBOOT_CMD",
                                        "RP2350 sent Reboot command to ESP32-P4"))

        # Check SPI stream counters from debug_task
        spi_m = SPI_COUNTERS_RE.search(line)
        if spi_m:
            self.spi_stream_tx_err = int(spi_m.group(1))
            self.spi_stream_queue_err = int(spi_m.group(2))
            self.spi_stream_parse_err = int(spi_m.group(3))
            new_success = int(spi_m.group(4))
            if new_success > 0 and self.spi_stream_success == 0 and esp_time_ms is not None:
                self.time_to_spi_ok = esp_time_ms
            self.spi_stream_success = new_success

        # Check audio CPU time
        audio_cpu_m = re.search(r"Audio task CPU time (\d+) uS", line)
        if audio_cpu_m:
            cpu_us = int(audio_cpu_m.group(1))
            self.audio_cpu_times.append(cpu_us)
            if not self.audio_running:
                self.time_to_audio = esp_time_ms if esp_time_ms is not None else int(wall_ts * 1000)
            self.audio_running = True
            # SPI stream connected = audio CPU > 100µs (synth engines processing SPI data)
            # Idle audio loop (no SPI data) runs at ~10-15µs
            if not self.spi_stream_connected and cpu_us > 100:
                self.spi_stream_connected = True
                # Use ESP timestamp if available, otherwise wall time (ms)
                self.time_to_spi_ok = esp_time_ms if esp_time_ms is not None else int(wall_ts * 1000)

        # Check milestones
        for pattern, label in MILESTONE_PATTERNS:
            if pattern.search(line):
                if label == "USB_SEND_ERROR":
                    # Extract cumulative count from exponential backoff format:
                    # "USB NCM send failed (256) err=-1" → 256 actual failures
                    m_count = re.search(r'\((\d+)\)', line)
                    count = int(m_count.group(1)) if m_count else 1
                    self.usb_error_count = max(self.usb_error_count, count)
                    if len([e for e in self.errors if e[2] == 'USB_SEND_ERROR']) < 3:
                        self.errors.append((wall_ts, esp_time_ms, label, line.strip()[:120]))
                elif label == "SD_CARD_MOUNTED":
                    self.sd_card_mounted = True
                    self.milestones.append((wall_ts, esp_time_ms, label, line.strip()[:120]))
                elif label == "SD_CARD_MOUNT_FAILED":
                    self.sd_card_failed = True
                    self.milestones.append((wall_ts, esp_time_ms, label, line.strip()[:120]))
                elif label == "SD_CARD_RETRY":
                    self.sd_card_retries += 1
                    self.milestones.append((wall_ts, esp_time_ms, label, line.strip()[:120]))
                elif label == "UHS1_TUNING_ERROR":
                    self.uhs1_tuning_errors += 1
                    self.milestones.append((wall_ts, esp_time_ms, label, line.strip()[:120]))
                elif label == "SPI_TIMEOUT":
                    self.spi_timeout_count += 1
                    self.last_spi_timeout_ms = esp_time_ms if esp_time_ms is not None else int(wall_ts * 1000)
                    if self.spi_timeout_count <= 3:
                        self.milestones.append((wall_ts, esp_time_ms, label, line.strip()[:120]))
                elif label == "MEMORY_REPORT":
                    self.memory_report_count += 1
                    if self.memory_report_count <= 3:
                        self.milestones.append((wall_ts, esp_time_ms, label, line.strip()[:120]))
                elif label == "AUDIO_RUNNING":
                    pass  # handled above with value extraction
                else:
                    self.milestones.append((wall_ts, esp_time_ms, label, line.strip()[:120]))

        # Check ESP_LOG errors
        if re.search(r"E \(\d+\)", line):
            if "Failed to send buffer to USB" not in line and "USB NCM send failed" not in line:
                self.errors.append((wall_ts, esp_time_ms, "ESP_ERROR", line.strip()[:120]))

    def on_serial_disconnect(self, wall_ts):
        """Called when serial port disconnects (likely reboot)."""
        self.serial_disconnects += 1
        self.reboot_events.append((wall_ts, "SERIAL_DISCONNECT",
                                    "Port disconnected — device likely rebooting"))

    def grade_boot(self):
        """Grade this boot cycle. Returns (passed, grade_str, details)."""
        checks = []
        all_pass = True

        # 1. Audio running?
        if self.audio_running and self.audio_cpu_times:
            avg_cpu = sum(self.audio_cpu_times) / len(self.audio_cpu_times)
            checks.append(("PASS", f"Audio running (avg {avg_cpu:.0f} µs, "
                           f"{len(self.audio_cpu_times)} samples)"))
        else:
            checks.append(("FAIL", "Audio NOT running — no CPU time reported"))
            all_pass = False

        # 2. SPI real-time stream connected?
        #    NOTE: transferSuccessCount in firmware is dead code (never incremented).
        #    Instead we detect stream connection by audio CPU time > 100µs
        #    (idle loop ~10µs vs active synth processing 300-400µs with SPI data).
        if self.spi_stream_connected:
            last_3 = self.audio_cpu_times[-3:] if len(self.audio_cpu_times) >= 3 else self.audio_cpu_times
            avg_recent = sum(last_3) / len(last_3) if last_3 else 0
            checks.append(("PASS", f"SPI stream connected (recent audio CPU ~{avg_recent:.0f}µs, "
                           f"timeouts stopped after {self.last_spi_timeout_ms/1000:.1f}s)"
                           if self.last_spi_timeout_ms else
                           f"SPI stream connected (recent audio CPU ~{avg_recent:.0f}µs)"))
        else:
            checks.append(("FAIL", "SPI stream NOT connected (audio CPU never exceeded 100µs)"))
            all_pass = False

        # 3. No USB NCM errors?
        if self.usb_error_count == 0:
            checks.append(("PASS", "No USB NCM errors"))
        else:
            checks.append(("FAIL", f"USB NCM errors: {self.usb_error_count} send failures"))
            all_pass = False

        # 4. No unexpected reboots?
        spi_reboots = sum(1 for e in self.reboot_events if e[1] == "SPI_REBOOT_CMD")
        crash_events = sum(1 for e in self.reboot_events
                           if e[1] in ("GURU_MEDITATION", "ABORT", "ASSERT_FAIL",
                                       "BACKTRACE", "PANIC", "MEMORY_FAULT",
                                       "STACK_OVERFLOW"))
        wdt_resets = sum(1 for r in self.reset_reasons
                         if int(r[1], 16) in (0x07, 0x08, 0x09, 0x0B, 0x0F, 0x10))
        brownout_resets = sum(1 for r in self.reset_reasons if r[1].lower() == "0xd")

        if spi_reboots == 0 and crash_events == 0 and wdt_resets == 0 and brownout_resets == 0:
            checks.append(("PASS", "No unexpected reboots"))
        else:
            parts = []
            if spi_reboots:
                parts.append(f"{spi_reboots}x SPI reboot cmd")
            if crash_events:
                parts.append(f"{crash_events}x crash")
            if wdt_resets:
                parts.append(f"{wdt_resets}x watchdog")
            if brownout_resets:
                parts.append(f"{brownout_resets}x brownout")
            checks.append(("FAIL", f"Unexpected reboots: {', '.join(parts)}"))
            all_pass = False

        # 5. SD card mount?
        if self.sd_card_mounted:
            retries_str = f" (after {self.sd_card_retries} retries)" if self.sd_card_retries > 0 else " (first attempt)"
            checks.append(("PASS", f"SD card mounted{retries_str}"))
        elif self.sd_card_failed:
            checks.append(("FAIL", f"SD card mount FAILED after {self.sd_card_retries} retries"))
            all_pass = False
        else:
            checks.append(("WARN", "SD card mount status unknown (no log captured)"))

        # 6. No UHS-I tuning errors? (should be 0 after removing SDMMC_SLOT_FLAG_UHS1)
        if self.uhs1_tuning_errors == 0:
            checks.append(("PASS", "No UHS-I tuning errors (UHS1 flag removed)"))
        else:
            checks.append(("FAIL", f"UHS-I tuning errors: {self.uhs1_tuning_errors} (UHS1 flag should be removed!)"))
            all_pass = False

        # 7. Boot time
        if self.time_to_audio is not None:
            checks.append(("INFO", f"Time to audio task: {self.time_to_audio / 1000:.1f}s "
                           f"(ESP timestamp)"))
        if self.time_to_spi_ok is not None:
            checks.append(("INFO", f"Time to SPI stream connected: {self.time_to_spi_ok / 1000:.1f}s"))
        if self.last_spi_timeout_ms is not None:
            checks.append(("INFO", f"SPI timeouts: {self.spi_timeout_count}x, "
                           f"last at {self.last_spi_timeout_ms / 1000:.1f}s"))

        grade = "PASS" if all_pass else "FAIL"
        return all_pass, grade, checks

    def print_report(self, cycle_num, logfile):
        """Print diagnostic report with success grading."""
        print(f"\n{'='*70}")
        print(f"  DIAGNOSTIC REPORT — Cycle {cycle_num} [{self.scenario.upper()}]")
        print(f"{'='*70}")
        print(f"  Log file: {logfile}")
        print(f"  Lines captured: {len(self.lines)}")
        print(f"  Serial disconnects: {self.serial_disconnects}")

        # ── Reset vectors ─────────────────────
        print(f"\n  ── RESET EVENTS ({len(self.reset_reasons)}) ──")
        if self.reset_reasons:
            for wall_ts, code, decoded in self.reset_reasons:
                print(f"    [{wall_ts:8.2f}s] rst:{code} = {decoded}")
        else:
            print(f"    (none captured — logs may start after bootloader)")

        # ── Reboot triggers ───────────────────
        triggers = [e for e in self.reboot_events if e[1] not in ("RESET_VECTOR",)]
        if triggers:
            print(f"\n  ── REBOOT TRIGGERS ({len(triggers)}) ──")
            for wall_ts, label, detail in triggers:
                print(f"    [{wall_ts:8.2f}s] {label}: {detail}")

        # ── Boot timeline ─────────────────────
        if self.milestones:
            print(f"\n  ── BOOT TIMELINE ──")
            seen = set()
            for wall_ts, esp_ms, label, line in self.milestones:
                if label in seen and label not in ("TRACK_MACHINE_SET", "PLUGIN_LOAD",
                                                     "SPI_TIMEOUT"):
                    continue
                seen.add(label)
                esp_str = f"ESP@{esp_ms}ms" if esp_ms is not None else "         "
                print(f"    [{wall_ts:8.2f}s] {esp_str:>14s}  {label:<25s} {line[:60]}")

        # ── SPI stream status ─────────────────
        print(f"\n  ── SPI STREAM STATUS ──")
        connected_str = "CONNECTED" if self.spi_stream_connected else "NOT CONNECTED"
        print(f"    Stream: {connected_str}")
        print(f"    SPI timeouts: {self.spi_timeout_count}  "
              f"(last at {self.last_spi_timeout_ms/1000:.1f}s)"
              if self.last_spi_timeout_ms else
              f"    SPI timeouts: {self.spi_timeout_count}")
        print(f"    Debug counters (NOTE: success counter is dead code in firmware):")
        print(f"      success={self.spi_stream_success}  tx-err={self.spi_stream_tx_err}  "
              f"queue-err={self.spi_stream_queue_err}  parse-err={self.spi_stream_parse_err}")

        # ── Audio status ──────────────────────
        if self.audio_cpu_times:
            avg = sum(self.audio_cpu_times) / len(self.audio_cpu_times)
            mn = min(self.audio_cpu_times)
            mx = max(self.audio_cpu_times)
            print(f"\n  ── AUDIO STATUS ──")
            print(f"    CPU time: avg={avg:.0f}µs  min={mn}µs  max={mx}µs  "
                  f"({len(self.audio_cpu_times)} samples)")

        # ── Errors (non-reboot) ───────────────
        non_reboot_errors = [e for e in self.errors if e[2] != "SPI_REBOOT_CMD"]
        if non_reboot_errors:
            print(f"\n  ── ERRORS ({len(non_reboot_errors)} unique, "
                  f"{self.usb_error_count} USB send errors, "
                  f"{self.spi_timeout_count} SPI timeouts) ──")
            shown = 0
            for wall_ts, esp_ms, label, detail in non_reboot_errors:
                if shown >= 10:
                    remaining = len(non_reboot_errors) - 10
                    print(f"    ... and {remaining} more")
                    break
                print(f"    [{wall_ts:8.2f}s] {label}: {detail[:80]}")
                shown += 1

        # ── Success grading ───────────────────
        passed, grade, checks = self.grade_boot()
        print(f"\n  ── BOOT GRADE: {'PASS ✓' if passed else 'FAIL ✗'} ──")
        for status, detail in checks:
            marker = "✓" if status == "PASS" else ("✗" if status == "FAIL" else "·")
            print(f"    [{marker}] {detail}")

        print()

        # Collect stats for cross-cycle summary
        sw_resets = [r for r in self.reset_reasons if r[1] == "0x3"]
        wdt_resets = [r for r in self.reset_reasons
                      if int(r[1], 16) in (0x07, 0x08, 0x09, 0x0B, 0x0F, 0x10)]
        brownout_resets = [r for r in self.reset_reasons if r[1].lower() == "0xd"]
        spi_reboots = [e for e in self.reboot_events if e[1] == "SPI_REBOOT_CMD"]

        return {
            "passed": passed,
            "grade": grade,
            "boot_count": self.boot_count,
            "sw_resets": len(sw_resets),
            "wdt_resets": len(wdt_resets),
            "brownout_resets": len(brownout_resets),
            "spi_reboots": len(spi_reboots),
            "serial_disconnects": self.serial_disconnects,
            "usb_errors": self.usb_error_count,
            "spi_timeouts": self.spi_timeout_count,
            "spi_success": self.spi_stream_success,
            "spi_connected": self.spi_stream_connected,
            "last_spi_timeout_s": (self.last_spi_timeout_ms / 1000.0
                                   if self.last_spi_timeout_ms else None),
            "audio_running": self.audio_running,
            "audio_avg_us": (sum(self.audio_cpu_times) / len(self.audio_cpu_times)
                             if self.audio_cpu_times else 0),
            "time_to_audio_s": (self.time_to_audio / 1000.0
                                if self.time_to_audio else None),
            "time_to_spi_s": (self.time_to_spi_ok / 1000.0
                              if self.time_to_spi_ok else None),
            "sd_card_mounted": self.sd_card_mounted,
            "sd_card_retries": self.sd_card_retries,
            "sd_card_failed": self.sd_card_failed,
            "uhs1_tuning_errors": self.uhs1_tuning_errors,
        }


def capture_cycle(port, baud, duration, cycle_num, output_dir, scenario):
    """Run one capture cycle. Returns (diagnostic, logfile)."""
    diag = BootDiagnostic(scenario=scenario)
    logfile = os.path.join(
        output_dir,
        f"boot_{scenario}_{cycle_num}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
    )

    print(f"\n{'='*70}")
    print(f"  CYCLE {cycle_num} [{scenario.upper()}]: Capturing {duration}s on {port}")
    print(f"  Log: {logfile}")
    print(f"{'='*70}")

    try:
        ser = serial.Serial(port, baud, timeout=1, dsrdtr=False, rtscts=False)
    except serial.SerialException as e:
        print(f"  ERROR: Could not open {port}: {e}")
        print(f"  (For cold boot: the port may not exist yet — waiting for device...)")
        # For cold boot, wait for port to appear
        if scenario == "cold":
            print(f"  Waiting up to 30s for device to appear on {port}...")
            for attempt in range(30):
                time.sleep(1)
                try:
                    ser = serial.Serial(port, baud, timeout=1, dsrdtr=False, rtscts=False)
                    print(f"  Device appeared after {attempt+1}s!")
                    break
                except serial.SerialException:
                    if attempt % 5 == 4:
                        print(f"    still waiting... ({attempt+1}s)")
            else:
                print(f"  FATAL: Device did not appear on {port} within 30s")
                return diag, logfile
        else:
            return diag, logfile

    with open(logfile, "w") as f:
        f.write(f"# Boot reboot diagnostic — cycle {cycle_num}\n")
        f.write(f"# Scenario: {scenario}\n")
        f.write(f"# Port: {port}, Baud: {baud}\n")
        f.write(f"# Started: {datetime.now().isoformat()}\n")
        f.write(f"# Duration: {duration}s\n\n")

        start = time.time()
        line_buf = ""

        while time.time() - start < duration:
            try:
                data = ser.read(4096)
            except serial.SerialException:
                wall_ts = time.time() - start
                print(f"  [{wall_ts:8.2f}s] *** SERIAL PORT DISCONNECTED — "
                      f"device may be rebooting ***")
                f.write(f"[{wall_ts:8.2f}] *** SERIAL DISCONNECT ***\n")
                diag.on_serial_disconnect(wall_ts)
                time.sleep(2)
                try:
                    ser.close()
                except Exception:
                    pass
                # Try to reconnect
                for attempt in range(15):
                    try:
                        ser = serial.Serial(port, baud, timeout=1,
                                            dsrdtr=False, rtscts=False)
                        wall_ts = time.time() - start
                        print(f"  [{wall_ts:8.2f}s] *** SERIAL RECONNECTED "
                              f"(attempt {attempt+1}) ***")
                        f.write(f"[{wall_ts:8.2f}] *** SERIAL RECONNECTED ***\n")
                        break
                    except serial.SerialException:
                        time.sleep(1)
                else:
                    print(f"  FATAL: Could not reconnect to {port} after 15 attempts")
                    break
                continue

            if not data:
                continue

            text = data.decode("utf-8", errors="replace")
            line_buf += text
            wall_ts = time.time() - start

            while "\n" in line_buf:
                line, line_buf = line_buf.split("\n", 1)
                line = line.rstrip("\r")
                if not line:
                    continue

                entry = f"[{wall_ts:8.2f}] {line}"
                f.write(entry + "\n")

                # Print to console — highlight important events
                prefix = "  "
                is_reboot = any(p.search(line) for p, _ in REBOOT_PATTERNS)
                is_spi_counter = SPI_COUNTERS_RE.search(line)
                is_audio = re.search(r"Audio task CPU time", line)
                if is_reboot:
                    prefix = "  >>> "
                elif is_spi_counter or is_audio:
                    prefix = "  --- "
                print(f"{prefix}{entry}")

                diag.process_line(wall_ts, line)

        try:
            ser.close()
        except Exception:
            pass

    return diag, logfile


def main():
    parser = argparse.ArgumentParser(
        description="ESP32-P4 Boot Reboot Diagnostic — Interactive Test Session",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Scenarios:
  cold     USB-C cable unplug/replug (full power cycle)
  reboot   Software reboot via Sequencer UI hardware command

Examples:
  %(prog)s --scenario cold --cycles 5
  %(prog)s --scenario reboot --cycles 3
  %(prog)s --port /dev/tty.usbmodem1101 --scenario cold --duration 90
        """
    )
    parser.add_argument("--port", type=str, default=None,
                        help="Serial port (auto-detected if not specified)")
    parser.add_argument("--baud", type=int, default=115200,
                        help="Baud rate (default: 115200)")
    parser.add_argument("--duration", type=int, default=90,
                        help="Capture duration per cycle in seconds (default: 90)")
    parser.add_argument("--cycles", type=int, default=5,
                        help="Number of boot cycles to capture (default: 5)")
    parser.add_argument("--scenario", type=str, choices=["cold", "reboot"],
                        required=True,
                        help="Test scenario: 'cold' (USB replug) or 'reboot' (UI command)")
    parser.add_argument("--output-dir", type=str, default=None,
                        help="Output directory (default: /tmp/tbd_boot_diag)")
    args = parser.parse_args()

    port = args.port
    if not port:
        port = find_serial_port()
        if not port:
            print("ERROR: No serial port found. Available ports:")
            for p in serial.tools.list_ports.comports():
                print(f"  {p.device}: {p.description} [{p.hwid}]")
            print("\nFor cold boot: plug in the device first to detect the port,")
            print("then specify it with --port")
            sys.exit(1)
        print(f"Auto-detected port: {port}")

    output_dir = args.output_dir or "/tmp/tbd_boot_diag"
    Path(output_dir).mkdir(parents=True, exist_ok=True)

    scenario = args.scenario
    scenario_desc = {
        "cold": "Cold Boot (USB-C cable unplug → replug)",
        "reboot": "Software Reboot (Sequencer UI → reboot both chips)",
    }

    print(f"\n{'='*70}")
    print(f"  ESP32-P4 Boot Diagnostic — Interactive Test Session")
    print(f"{'='*70}")
    print(f"  Scenario: {scenario_desc[scenario]}")
    print(f"  Port:     {port}")
    print(f"  Baud:     {args.baud}")
    print(f"  Duration: {args.duration}s per cycle")
    print(f"  Cycles:   {args.cycles}")
    print(f"  Output:   {output_dir}")
    print()
    print(f"  SUCCESS CRITERIA (all must pass):")
    print(f"    1. SD card mounted successfully")
    print(f"    2. No UHS-I tuning errors (UHS1 flag removed)")
    print(f"    3. Audio running (CPU time reported)")
    print(f"    4. SPI stream connected (audio CPU > 100µs = synth processing SPI data)")
    print(f"    5. No USB NCM errors")
    print(f"    6. No unexpected reboots/crashes")
    print()

    all_results = []

    for cycle in range(1, args.cycles + 1):
        # Always prompt before each cycle
        print(f"\n{'*'*70}")
        if scenario == "cold":
            if cycle == 1:
                input(f"  CYCLE {cycle}/{args.cycles}: "
                      f"Unplug the USB-C cable NOW.\n"
                      f"  Then plug it back in and press ENTER immediately... ")
            else:
                input(f"  CYCLE {cycle}/{args.cycles}: "
                      f"Unplug the USB-C cable, wait 3 seconds,\n"
                      f"  then plug it back in and press ENTER immediately... ")
        else:  # reboot
            if cycle == 1:
                input(f"  CYCLE {cycle}/{args.cycles}: "
                      f"Go to the Sequencer UI and trigger the\n"
                      f"  'Reboot' command for both chips.\n"
                      f"  Press ENTER right when you trigger the reboot... ")
            else:
                input(f"  CYCLE {cycle}/{args.cycles}: "
                      f"Trigger the Sequencer UI 'Reboot' command\n"
                      f"  for both chips again.\n"
                      f"  Press ENTER right when you trigger the reboot... ")

        diag, logfile = capture_cycle(
            port, args.baud, args.duration, cycle, output_dir, scenario
        )
        stats = diag.print_report(cycle, logfile)
        all_results.append((cycle, stats, logfile))

    # ── Cross-cycle summary ───────────────────
    print(f"\n{'='*70}")
    print(f"  SESSION SUMMARY — {scenario_desc[scenario]}")
    print(f"  {args.cycles} boot cycles captured")
    print(f"{'='*70}")

    passed_count = sum(1 for _, s, _ in all_results if s["passed"])
    total = len(all_results)

    print(f"\n  PASS RATE: {passed_count}/{total} "
          f"({'%.0f' % (100*passed_count/total)}%)")
    print()

    # Per-cycle summary table
    print(f"  {'Cycle':<6} {'Grade':<6} {'SD Card':<10} {'Audio':<7} {'SPI Stream':<12} "
          f"{'USB Err':<8} {'Reboots':<8} {'Boot→Audio':<12}")
    print(f"  {'-'*5:<6} {'-'*5:<6} {'-'*8:<10} {'-'*5:<7} {'-'*10:<12} "
          f"{'-'*6:<8} {'-'*6:<8} {'-'*10:<12}")
    for cycle_num, stats, _ in all_results:
        sd = "OK" if stats["sd_card_mounted"] else ("FAIL" if stats["sd_card_failed"] else "?")
        if stats["sd_card_retries"] > 0 and stats["sd_card_mounted"]:
            sd = f"OK({stats['sd_card_retries']}r)"
        audio = "YES" if stats["audio_running"] else "NO"
        spi = "YES" if stats["spi_connected"] else "NO"
        usb = str(stats["usb_errors"])
        reboots = str(stats["spi_reboots"] + stats["sw_resets"] +
                      stats["wdt_resets"] + stats["brownout_resets"])
        t_audio = (f"{stats['time_to_audio_s']:.1f}s"
                   if stats["time_to_audio_s"] else "N/A")
        print(f"  {cycle_num:<6} {stats['grade']:<6} {sd:<10} {audio:<7} {spi:<12} "
              f"{usb:<8} {reboots:<8} {t_audio:<12}")

    # Aggregated stats
    print()
    total_sw = sum(s["sw_resets"] for _, s, _ in all_results)
    total_wdt = sum(s["wdt_resets"] for _, s, _ in all_results)
    total_brownout = sum(s["brownout_resets"] for _, s, _ in all_results)
    total_spi_reboot = sum(s["spi_reboots"] for _, s, _ in all_results)
    total_usb_err = sum(s["usb_errors"] for _, s, _ in all_results)
    total_disconnects = sum(s["serial_disconnects"] for _, s, _ in all_results)
    boot_times = [s["time_to_audio_s"] for _, s, _ in all_results if s["time_to_audio_s"]]
    spi_times = [s["time_to_spi_s"] for _, s, _ in all_results if s["time_to_spi_s"]]

    print(f"  Total software resets:  {total_sw}")
    print(f"  Total watchdog resets:  {total_wdt}")
    print(f"  Total brownout resets:  {total_brownout}")
    print(f"  Total SPI reboot cmds:  {total_spi_reboot}")
    print(f"  Total USB NCM errors:   {total_usb_err}")
    print(f"  Serial disconnects:     {total_disconnects}")
    if boot_times:
        print(f"  Boot-to-audio time:     avg={sum(boot_times)/len(boot_times):.1f}s  "
              f"min={min(boot_times):.1f}s  max={max(boot_times):.1f}s")
    if spi_times:
        print(f"  Boot-to-SPI time:       avg={sum(spi_times)/len(spi_times):.1f}s  "
              f"min={min(spi_times):.1f}s  max={max(spi_times):.1f}s")

    # Analysis
    print(f"\n  ── ANALYSIS ──")
    if passed_count == total:
        print(f"  All {total} boots passed! Device is booting reliably.")
    else:
        failed = total - passed_count
        print(f"  {failed}/{total} boots FAILED.")
        if total_spi_reboot > 0:
            print(f"  → RP2350 SPI Reboot command detected {total_spi_reboot}x")
        if total_usb_err > 0:
            print(f"  → USB NCM errors in {sum(1 for _,s,_ in all_results if s['usb_errors']>0)} cycles")
        failed_spi = sum(1 for _,s,_ in all_results if not s["spi_connected"])
        if failed_spi > 0:
            print(f"  → SPI stream never connected in {failed_spi} cycles")
        failed_audio = sum(1 for _,s,_ in all_results if not s["audio_running"])
        if failed_audio > 0:
            print(f"  → Audio never started in {failed_audio} cycles")
        failed_sd = sum(1 for _,s,_ in all_results if s["sd_card_failed"])
        if failed_sd > 0:
            print(f"  → SD card mount failed in {failed_sd} cycles")
        uhs1_total = sum(s["uhs1_tuning_errors"] for _,s,_ in all_results)
        if uhs1_total > 0:
            print(f"  → UHS-I tuning errors in {sum(1 for _,s,_ in all_results if s['uhs1_tuning_errors']>0)} cycles "
                  f"(FIX NOT APPLIED? Should be 0 with UHS1 flag removed)")

    print(f"\n  Log files saved to: {output_dir}")
    print()


if __name__ == "__main__":
    main()
