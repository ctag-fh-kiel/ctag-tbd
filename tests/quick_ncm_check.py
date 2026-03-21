#!/usr/bin/env python3
"""Quick boot log capture - filters for NCM/USB/Network messages."""
import serial, time, subprocess, sys

PORT = '/dev/tty.usbmodem1101'
CU_PORT = '/dev/cu.usbmodem1101'
DURATION = 30

# Reset device
print('Triggering device reset via esptool...')
try:
    subprocess.run(
        ['esptool.py', '--chip', 'esp32p4', '-p', CU_PORT,
         '--before=default_reset', '--after=hard_reset', 'read_mac'],
        capture_output=True, timeout=15
    )
except Exception as e:
    print(f'  esptool reset failed: {e}')

print(f'Waiting 3s for device to restart...')
time.sleep(3)

print(f'Capturing boot log for {DURATION}s (filtered for NCM/USB/Network)...\n')
for attempt in range(10):
    try:
        ser = serial.Serial(PORT, 115200, timeout=1, dsrdtr=False, rtscts=False)
        break
    except serial.SerialException:
        time.sleep(1)
else:
    print(f'FATAL: Cannot open {PORT}')
    sys.exit(1)

start = time.time()
keywords = ('NCM', 'link', 'TUSB', 'network', 'DHCP', 'Network', 'USB', 'mount', 'tud_', 'abort', 'Guru', 'Reboot')

while time.time() - start < DURATION:
    try:
        data = ser.read(4096)
    except serial.SerialException:
        elapsed = time.time() - start
        print(f'[{elapsed:6.1f}s] *** SERIAL DISCONNECT ***')
        time.sleep(2)
        try:
            ser.close()
        except:
            pass
        for i in range(10):
            try:
                ser = serial.Serial(PORT, 115200, timeout=1, dsrdtr=False, rtscts=False)
                print(f'[{time.time()-start:6.1f}s] *** RECONNECTED ***')
                break
            except:
                time.sleep(1)
        else:
            print('FATAL: Cannot reconnect')
            break
        continue

    if not data:
        continue
    text = data.decode('utf-8', errors='replace')
    for line in text.split('\n'):
        line = line.strip()
        if not line:
            continue
        if any(kw in line for kw in keywords):
            elapsed = time.time() - start
            print(f'[{elapsed:6.1f}s] {line}')

ser.close()
print('\nDone. Checking NCM from Mac side...')

# Check NCM
import subprocess as sp
r = sp.run(['ifconfig', 'en6'], capture_output=True, text=True)
print(r.stdout)

r = sp.run(['curl', '-s', '-o', '/dev/null', '-w', '%{http_code}',
            '--connect-timeout', '5', 'http://192.168.4.1/'],
           capture_output=True, text=True)
print(f'WebUI HTTP: {r.stdout}')
