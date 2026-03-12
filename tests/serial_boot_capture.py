#!/usr/bin/env python3
"""
ESP32-P4 Serial Boot Capture Tool

Captures serial debug output from the ESP32-P4 JTAG debug port during boot.
Can run multiple power cycles and analyze boot logs for reboots, errors,
and USB NCM initialization issues.

Usage:
    # Single boot capture (120s default):
    python3 serial_boot_capture.py

    # Multiple boot cycles:
    python3 serial_boot_capture.py --cycles 5

    # Custom port and baud:
    python3 serial_boot_capture.py --port /dev/cu.usbmodem1201 --baud 115200

    # Custom capture duration per cycle:
    python3 serial_boot_capture.py --duration 90

    # Save logs to specific directory:
    python3 serial_boot_capture.py --output-dir /tmp/boot_logs
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


# Known error/warning patterns to flag
ERROR_PATTERNS = [
    (re.compile(r"Guru Meditation Error", re.IGNORECASE), "CRASH"),
    (re.compile(r"abort\(\)", re.IGNORECASE), "ABORT"),
    (re.compile(r"assert failed", re.IGNORECASE), "ASSERT"),
    (re.compile(r"Backtrace:", re.IGNORECASE), "BACKTRACE"),
    (re.compile(r"rst:0x[0-9a-f]+", re.IGNORECASE), "RESET"),
    (re.compile(r"boot:0x[0-9a-f]+", re.IGNORECASE), "BOOT"),
    (re.compile(r"E \(\d+\)"), "ESP_ERROR"),
    (re.compile(r"W \(\d+\)"), "ESP_WARNING"),
    (re.compile(r"panic", re.IGNORECASE), "PANIC"),
    (re.compile(r"watchdog", re.IGNORECASE), "WATCHDOG"),
    (re.compile(r"stack overflow", re.IGNORECASE), "STACK_OVERFLOW"),
    (re.compile(r"LoadProhibited|StoreProhibited|InstrFetchProhibited"), "MEMORY_FAULT"),
]

# Key boot milestones to track
MILESTONE_PATTERNS = [
    (re.compile(r"NCM interface ready"), "USB_NCM_READY"),
    (re.compile(r"Starting USB NCM"), "USB_NCM_START"),
    (re.compile(r"DHCP server started"), "DHCP_READY"),
    (re.compile(r"DHCP server assigned IP"), "HOST_CONNECTED"),
    (re.compile(r"Starting HTTP Server"), "HTTP_SERVER"),
    (re.compile(r"SpiAPI: Init"), "SPI_INIT"),
    (re.compile(r"SPManager: Init"), "SPMANAGER_INIT"),
    (re.compile(r"Audio task started"), "AUDIO_STARTED"),
    (re.compile(r"Returned from app_main"), "APP_MAIN_DONE"),
    (re.compile(r"Ableton Link: Enabled"), "LINK_ENABLED"),
]


def find_serial_port():
    """Auto-detect the ESP32-P4 JTAG debug serial port."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        desc = (port.description or "").lower()
        if "jtag" in desc and "serial" in desc:
            return port.device
        if "usbmodem" in port.device.lower():
            return port.device
    # Fallback
    for port in ports:
        if "ttyUSB" in port.device or "ttyACM" in port.device or "usbmodem" in port.device:
            return port.device
    return None


def capture_boot(port, baud, duration, cycle_num, output_dir):
    """Capture one boot cycle. Returns (log_lines, issues, milestones)."""
    log_lines = []
    issues = []
    milestones = []
    reboot_count = 0

    logfile = os.path.join(output_dir, f"boot_cycle_{cycle_num}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt")

    print(f"\n{'='*60}")
    print(f"  CYCLE {cycle_num}: Capturing for {duration}s on {port} @ {baud}")
    print(f"  Log file: {logfile}")
    print(f"{'='*60}")
    print("  >>> POWER CYCLE THE DEVICE NOW <<<")
    print()

    try:
        ser = serial.Serial(port, baud, timeout=1, dsrdtr=False, rtscts=False)
    except serial.SerialException as e:
        print(f"  ERROR: Could not open {port}: {e}")
        print("  The device may need to be power-cycled first to make the port available.")
        return [], [("SERIAL_ERROR", 0, str(e))], [], logfile

    with open(logfile, "w") as f:
        f.write(f"# Boot capture cycle {cycle_num}\n")
        f.write(f"# Port: {port}, Baud: {baud}\n")
        f.write(f"# Started: {datetime.now().isoformat()}\n")
        f.write(f"# Duration: {duration}s\n\n")

        start = time.time()
        while time.time() - start < duration:
            try:
                data = ser.read(4096)
            except serial.SerialException:
                print("  [serial port disconnected - device may have rebooted]")
                time.sleep(1)
                try:
                    ser.close()
                    ser = serial.Serial(port, baud, timeout=1, dsrdtr=False, rtscts=False)
                    reboot_count += 1
                    issues.append(("REBOOT_DETECTED", time.time() - start, "Serial port reconnected"))
                except serial.SerialException:
                    continue
                continue

            if not data:
                continue

            text = data.decode("utf-8", errors="replace")
            ts = time.time() - start

            for line in text.split("\n"):
                line = line.rstrip()
                if not line:
                    continue

                entry = f"[{ts:8.2f}] {line}"
                log_lines.append(entry)
                print(f"  {entry}")
                f.write(entry + "\n")

                # Check for errors
                for pattern, label in ERROR_PATTERNS:
                    if pattern.search(line):
                        issues.append((label, ts, line.strip()))

                # Check for milestones
                for pattern, label in MILESTONE_PATTERNS:
                    if pattern.search(line):
                        milestones.append((label, ts, line.strip()))

                # Detect reset reasons (indicates reboot)
                if re.search(r"rst:0x[0-9a-f]+", line, re.IGNORECASE):
                    reboot_count += 1

        ser.close()

    if reboot_count > 1:
        issues.append(("MULTIPLE_REBOOTS", 0, f"Detected {reboot_count} reset events"))

    return log_lines, issues, milestones, logfile


def print_summary(cycle_num, issues, milestones, logfile):
    """Print analysis summary for one cycle."""
    print(f"\n{'─'*60}")
    print(f"  CYCLE {cycle_num} SUMMARY")
    print(f"{'─'*60}")
    print(f"  Log: {logfile}")

    if milestones:
        print(f"\n  Boot Milestones:")
        for label, ts, _ in milestones:
            print(f"    [{ts:8.2f}s] {label}")

    if issues:
        errors = [i for i in issues if i[0] not in ("ESP_WARNING",)]
        warnings = [i for i in issues if i[0] == "ESP_WARNING"]

        if errors:
            print(f"\n  ⚠ ERRORS ({len(errors)}):")
            for label, ts, detail in errors:
                print(f"    [{ts:8.2f}s] {label}: {detail[:100]}")

        if warnings:
            print(f"\n  Warnings ({len(warnings)}):")
            for label, ts, detail in warnings[:5]:
                print(f"    [{ts:8.2f}s] {detail[:100]}")
            if len(warnings) > 5:
                print(f"    ... and {len(warnings) - 5} more")
    else:
        print(f"\n  ✓ No issues detected")

    # Check key milestones
    milestone_labels = {m[0] for m in milestones}
    critical = ["USB_NCM_READY", "AUDIO_STARTED"]
    missing = [c for c in critical if c not in milestone_labels]
    if missing:
        print(f"\n  ⚠ Missing milestones: {', '.join(missing)}")
    elif all(c in milestone_labels for c in critical):
        print(f"\n  ✓ All critical milestones reached")

    print()


def main():
    parser = argparse.ArgumentParser(description="ESP32-P4 Serial Boot Capture Tool")
    parser.add_argument("--port", type=str, default=None,
                        help="Serial port (auto-detected if not specified)")
    parser.add_argument("--baud", type=int, default=115200,
                        help="Baud rate (default: 115200)")
    parser.add_argument("--duration", type=int, default=120,
                        help="Capture duration per cycle in seconds (default: 120)")
    parser.add_argument("--cycles", type=int, default=1,
                        help="Number of boot cycles to capture (default: 1)")
    parser.add_argument("--output-dir", type=str, default=None,
                        help="Output directory for log files (default: /tmp/tbd_boot_logs)")
    args = parser.parse_args()

    # Find port
    port = args.port
    if not port:
        port = find_serial_port()
        if not port:
            print("ERROR: No serial port found. Available ports:")
            for p in serial.tools.list_ports.comports():
                print(f"  {p.device}: {p.description}")
            sys.exit(1)
        print(f"Auto-detected serial port: {port}")

    # Create output dir
    output_dir = args.output_dir or "/tmp/tbd_boot_logs"
    Path(output_dir).mkdir(parents=True, exist_ok=True)

    print(f"\nESP32-P4 Serial Boot Capture")
    print(f"  Port:     {port}")
    print(f"  Baud:     {args.baud}")
    print(f"  Duration: {args.duration}s per cycle")
    print(f"  Cycles:   {args.cycles}")
    print(f"  Output:   {output_dir}")

    all_results = []

    for cycle in range(1, args.cycles + 1):
        if cycle > 1:
            print(f"\n{'*'*60}")
            print(f"  Ready for cycle {cycle}/{args.cycles}")
            print(f"  >>> POWER CYCLE THE DEVICE NOW <<<")
            input("  Press ENTER when ready to start capture...")

        log_lines, issues, milestones, logfile = capture_boot(
            port, args.baud, args.duration, cycle, output_dir
        )
        all_results.append((cycle, issues, milestones, logfile))
        print_summary(cycle, issues, milestones, logfile)

    # Final summary across all cycles
    if args.cycles > 1:
        print(f"\n{'='*60}")
        print(f"  OVERALL SUMMARY ({args.cycles} cycles)")
        print(f"{'='*60}")

        total_errors = 0
        cycles_with_reboots = 0
        cycles_ncm_ok = 0
        cycles_audio_ok = 0

        for cycle, issues, milestones, logfile in all_results:
            errors = [i for i in issues if i[0] not in ("ESP_WARNING",)]
            total_errors += len(errors)
            if any(i[0] == "MULTIPLE_REBOOTS" for i in issues):
                cycles_with_reboots += 1
            milestone_labels = {m[0] for m in milestones}
            if "USB_NCM_READY" in milestone_labels:
                cycles_ncm_ok += 1
            if "AUDIO_STARTED" in milestone_labels:
                cycles_audio_ok += 1

        print(f"  Total errors:          {total_errors}")
        print(f"  Cycles with reboots:   {cycles_with_reboots}/{args.cycles}")
        print(f"  USB NCM success:       {cycles_ncm_ok}/{args.cycles}")
        print(f"  Audio start success:   {cycles_audio_ok}/{args.cycles}")
        print(f"  Log directory:         {output_dir}")
        print()


if __name__ == "__main__":
    main()
