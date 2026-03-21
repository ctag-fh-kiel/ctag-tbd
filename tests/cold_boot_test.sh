#!/bin/bash
# Cold Boot Test — 5 cycles with USB NCM verification
# Run this script, then follow the prompts to unplug/replug USB-C
#
# Tests per cycle:
#   1. Serial capture for 75s (crashes, USB errors, audio, SPI)
#   2. Mac-side NCM check (en6 gets 192.168.4.2)
#   3. WebUI check (curl http://192.168.4.1)

PORT="${1:-/dev/tty.usbmodem1101}"
CYCLES="${2:-5}"
DURATION=75
RESULTS_DIR="/tmp/tbd_cold_boot_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$RESULTS_DIR"

echo "==========================================="
echo "  TBD-16 Cold Boot Test — $CYCLES cycles"
echo "==========================================="
echo "  Port:     $PORT"
echo "  Duration: ${DURATION}s per cycle"
echo "  Results:  $RESULTS_DIR"
echo ""
echo "  Tests per cycle:"
echo "    1. No crashes / abort()"
echo "    2. No USB NCM errors"
echo "    3. Audio running (CPU > 100µs)"
echo "    4. SPI stream connected"
echo "    5. Mac gets IP 192.168.4.2 (NCM)"
echo "    6. WebUI responds at http://192.168.4.1"
echo ""

PASS_COUNT=0
FAIL_COUNT=0

for cycle in $(seq 1 "$CYCLES"); do
    echo ""
    echo "***************************************************"
    echo "  CYCLE $cycle/$CYCLES"
    echo "***************************************************"
    
    if [ "$cycle" -eq 1 ]; then
        echo "  Unplug the USB-C cable NOW."
        echo "  Wait 3 seconds, then plug it back in."
    else
        echo "  Unplug the USB-C cable, wait 3 seconds,"
        echo "  then plug it back in."
    fi
    read -p "  Press ENTER right after plugging back in... "
    
    LOGFILE="$RESULTS_DIR/cycle_${cycle}.txt"
    
    # Wait for serial port to appear
    echo "  Waiting for device on $PORT..."
    for i in $(seq 1 30); do
        if [ -e "$PORT" ]; then
            echo "  Device appeared after ${i}s"
            break
        fi
        sleep 1
        if [ "$i" -eq 30 ]; then
            echo "  ERROR: Device did not appear on $PORT within 30s"
            echo "CYCLE $cycle: SKIP (no device)" >> "$RESULTS_DIR/summary.txt"
            continue 2
        fi
    done
    
    # Small delay to let port settle
    sleep 1
    
    # Capture serial output
    echo "  Capturing serial output for ${DURATION}s..."
    python3 -c "
import serial, time, re, sys, os

port = '$PORT'
duration = $DURATION
logfile = '$LOGFILE'

try:
    ser = serial.Serial(port, 115200, timeout=1, dsrdtr=False, rtscts=False)
except Exception as e:
    print(f'  Cannot open {port}: {e}')
    sys.exit(1)

usb_errors = 0
audio_times = []
spi_timeouts = 0
crashes = 0
abort_lines = []
spi_reboot = 0
last_spi_timeout_ms = None
lines = []

start = time.time()
reconnect_count = 0

while time.time() - start < duration:
    try:
        data = ser.read(4096)
    except serial.SerialException:
        elapsed = time.time() - start
        print(f'  [{elapsed:.1f}s] *** Serial disconnect — reconnecting... ***')
        reconnect_count += 1
        try:
            ser.close()
        except:
            pass
        time.sleep(2)
        for attempt in range(15):
            try:
                ser = serial.Serial(port, 115200, timeout=1, dsrdtr=False, rtscts=False)
                elapsed = time.time() - start
                print(f'  [{elapsed:.1f}s] *** Reconnected ***')
                break
            except:
                time.sleep(1)
        else:
            print(f'  FATAL: Cannot reconnect')
            break
        continue
    
    if not data:
        continue
    text = data.decode('utf-8', errors='replace')
    for line in text.split('\n'):
        line = line.strip()
        if not line:
            continue
        lines.append(line)
        elapsed = time.time() - start
        
        if 'Failed to send buffer to USB' in line or 'USB NCM send failed' in line:
            # Extract cumulative count from exponential backoff format:
            # 'USB NCM send failed (256) err=-1' -> 256 actual failures
            m_usb = re.search(r'\((\d+)\)', line)
            count = int(m_usb.group(1)) if m_usb else 1
            usb_errors = max(usb_errors, count)
        if 'abort()' in line or 'Guru Meditation' in line or 'assert failed' in line:
            crashes += 1
            abort_lines.append(line[:120])
        if 'Rebooting device!' in line:
            spi_reboot += 1
        m = re.search(r'Audio task CPU time (\d+) uS', line)
        if m:
            audio_times.append(int(m.group(1)))
        if 'SPI receive timeout' in line:
            spi_timeouts += 1
            tm = re.search(r'\((\d+)\)', line)
            if tm:
                last_spi_timeout_ms = int(tm.group(1))

ser.close()

# Write log
with open(logfile, 'w') as f:
    for l in lines:
        f.write(l + '\n')

# Report
print()
print(f'  ── Serial Results ──')
print(f'    Lines: {len(lines)}')
print(f'    USB NCM errors: {usb_errors}')
print(f'    Crashes/aborts: {crashes}')
print(f'    SPI reboots: {spi_reboot}')
print(f'    SPI timeouts: {spi_timeouts}')
if audio_times:
    avg = sum(audio_times)/len(audio_times)
    print(f'    Audio CPU: avg={avg:.0f}µs min={min(audio_times)}µs max={max(audio_times)}µs ({len(audio_times)} samples)')
    print(f'    SPI stream: {\"CONNECTED\" if avg > 100 else \"NOT CONNECTED\"}')
else:
    print(f'    Audio CPU: NO DATA')
if abort_lines:
    for al in abort_lines[:3]:
        print(f'    CRASH: {al}')
print(f'    Serial disconnects: {reconnect_count}')

# Write machine-readable result
result = {
    'usb_errors': usb_errors,
    'crashes': crashes,
    'spi_reboot': spi_reboot,
    'audio_avg': sum(audio_times)/len(audio_times) if audio_times else 0,
    'audio_connected': (sum(audio_times)/len(audio_times) > 100) if audio_times else False,
    'spi_timeouts': spi_timeouts,
    'serial_disconnects': reconnect_count,
}

# Exit code: 0=pass serial checks, 1=fail
passed = (crashes == 0 and usb_errors == 0 and 
          len(audio_times) > 0 and sum(audio_times)/len(audio_times) > 100)
sys.exit(0 if passed else 1)
"
    SERIAL_RESULT=$?
    
    # Wait a moment for NCM to settle
    echo ""
    echo "  ── NCM Check (waiting 10s for Mac to get IP) ──"
    sleep 10
    
    # Check NCM from Mac side
    NCM_IP=$(ifconfig en6 2>/dev/null | grep "inet " | awk '{print $2}')
    if [ "$NCM_IP" = "192.168.4.2" ]; then
        echo "    NCM: PASS — en6 has IP $NCM_IP"
        NCM_PASS=0
    else
        # Try other interfaces
        NCM_IP=$(ifconfig 2>/dev/null | grep "192.168.4.2" | head -1)
        if [ -n "$NCM_IP" ]; then
            echo "    NCM: PASS — found 192.168.4.2 on another interface"
            NCM_PASS=0
        else
            echo "    NCM: FAIL — no 192.168.4.2 found on any interface"
            NCM_PASS=1
        fi
    fi
    
    # Check WebUI
    echo "  ── WebUI Check ──"
    HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" --connect-timeout 5 http://192.168.4.1/ 2>/dev/null)
    if [ "$HTTP_CODE" = "200" ]; then
        echo "    WebUI: PASS — HTTP $HTTP_CODE"
        WEBUI_PASS=0
    else
        echo "    WebUI: FAIL — HTTP $HTTP_CODE (expected 200)"
        WEBUI_PASS=1
    fi
    
    # Grade cycle
    if [ $SERIAL_RESULT -eq 0 ] && [ $NCM_PASS -eq 0 ] && [ $WEBUI_PASS -eq 0 ]; then
        GRADE="PASS"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        GRADE="FAIL"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
    
    echo ""
    echo "  ══════════════════════════════════════"
    echo "  CYCLE $cycle GRADE: $GRADE"
    echo "  ══════════════════════════════════════"
    
    # Write to summary
    echo "Cycle $cycle: $GRADE (serial=$SERIAL_RESULT ncm=$NCM_PASS webui=$WEBUI_PASS)" >> "$RESULTS_DIR/summary.txt"
done

# Final summary
echo ""
echo "==========================================="
echo "  FINAL RESULTS: $PASS_COUNT/$CYCLES PASS"
echo "==========================================="
cat "$RESULTS_DIR/summary.txt"
echo ""
echo "Logs saved in: $RESULTS_DIR"
