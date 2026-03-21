#!/usr/bin/env python3
"""
Quick cold-boot NCM verification test.

Runs N power-cycle iterations.  Each cycle prompts the user to
unplug → wait 3 s → replug the USB-C cable, then waits for macOS
to enumerate the device and checks that:
  1. en6 gets IP 192.168.4.2  (DHCP from device)
  2. WebUI responds HTTP 200  at http://192.168.4.1
"""

import argparse
import subprocess
import sys
import time


NCM_IFACE = "en6"
EXPECTED_IP = "192.168.4.2"
WEBUI_URL = "http://192.168.4.1/"
SETTLE_SECS = 25          # seconds after replug before first check
RETRY_SECS = 10           # extra seconds to wait on failure before retry


def find_ncm_interface():
    """Find which en* interface has EXPECTED_IP, or fall back to NCM_IFACE."""
    r = subprocess.run(["ifconfig"], capture_output=True, text=True)
    current_iface = None
    for line in r.stdout.splitlines():
        if not line.startswith((" ", "\t")):
            current_iface = line.split(":")[0]
        if EXPECTED_IP in line and current_iface:
            return current_iface
    return NCM_IFACE


def check_ncm():
    """Return (is_active, has_ip, iface, detail_str)."""
    iface = find_ncm_interface()
    r = subprocess.run(["ifconfig", iface], capture_output=True, text=True)
    out = r.stdout
    is_active = "status: active" in out
    has_ip = EXPECTED_IP in out
    detail_lines = [l.strip() for l in out.splitlines()
                    if "inet " in l or "status:" in l]
    return is_active, has_ip, iface, "\n    ".join(detail_lines) if detail_lines else "(no output)"


def check_webui():
    """Return HTTP status code string (e.g. '200')."""
    r = subprocess.run(
        ["curl", "-s", "-o", "/dev/null", "-w", "%{http_code}",
         "--connect-timeout", "5", WEBUI_URL],
        capture_output=True, text=True,
    )
    return r.stdout.strip()


def run_cycle(cycle, total):
    print(f"\n{'='*50}")
    print(f"  CYCLE {cycle}/{total}")
    print(f"{'='*50}")
    print("  1. UNPLUG the USB-C cable NOW")
    print("  2. Wait ~3 seconds")
    print("  3. PLUG it back in")
    input("  Press ENTER right after plugging back in... ")

    print(f"  Waiting {SETTLE_SECS}s for device boot + NCM…")
    time.sleep(SETTLE_SECS)

    is_active, has_ip, iface, detail = check_ncm()
    http_code = check_webui()
    webui_ok = http_code == "200"

    # Retry once if the first check fails — macOS may need more time
    if not (is_active and has_ip and webui_ok):
        print(f"  First check incomplete, retrying in {RETRY_SECS}s…")
        time.sleep(RETRY_SECS)
        is_active, has_ip, iface, detail = check_ncm()
        http_code = check_webui()
        webui_ok = http_code == "200"

    print(f"  --- Results ---")
    if is_active and has_ip:
        print(f"    NCM:   PASS  ({iface} active, IP {EXPECTED_IP})")
    else:
        print(f"    NCM:   FAIL  ({iface}: active={is_active}, has_ip={has_ip})")
        print(f"    {detail}")

    print(f"    WebUI: {'PASS' if webui_ok else 'FAIL'}  (HTTP {http_code})")

    passed = is_active and has_ip and webui_ok
    print(f"    Cycle: {'PASS' if passed else 'FAIL'}")
    return passed


def main():
    ap = argparse.ArgumentParser(description="Cold-boot NCM verification")
    ap.add_argument("-n", "--cycles", type=int, default=5,
                    help="Number of power-cycle iterations (default: 5)")
    args = ap.parse_args()

    print("=" * 50)
    print("  TBD-16 Cold Boot NCM Test")
    print("=" * 50)
    print(f"  Cycles:  {args.cycles}")
    print(f"  Checks:  {NCM_IFACE} → {EXPECTED_IP},  WebUI → {WEBUI_URL}")
    print()

    results = []
    for i in range(1, args.cycles + 1):
        results.append(run_cycle(i, args.cycles))

    # Summary
    passed = sum(results)
    total = len(results)
    print(f"\n{'='*50}")
    print(f"  SUMMARY: {passed}/{total} passed")
    print(f"{'='*50}")
    for i, ok in enumerate(results, 1):
        print(f"    Cycle {i}: {'PASS' if ok else 'FAIL'}")

    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
