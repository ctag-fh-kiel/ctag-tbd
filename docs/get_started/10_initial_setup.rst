****************************
Initial Device Setup Guide
****************************

Interactive step-by-step setup for your TBD device. Both flashing tools are built in below.

**Time:** ~15–20 min | **Requirements:** TBD device, 2× USB-C cables, Chrome/Edge/Opera, 2× SD cards + reader

.. raw:: html

    <style>
      #setupGuide {
        font-family: system-ui, -apple-system, sans-serif;
        max-width: 900px;
      }
      .setup-step {
        display: none;
        margin: 2em 0;
        padding: 1.5em;
        border-left: 4px solid #2563EB;
        background: rgba(37, 99, 235, 0.04);
        border-radius: 4px;
      }
      .setup-step.active {
        display: block;
      }
      .step-header {
        display: flex;
        align-items: baseline;
        gap: 0.8em;
        margin-bottom: 1em;
      }
      .step-number {
        display: flex;
        align-items: center;
        justify-content: center;
        min-width: 32px;
        height: 32px;
        background: #2563EB;
        color: white;
        border-radius: 50%;
        font-weight: 600;
        font-size: 0.95em;
      }
      .step-title {
        font-size: 1.3em;
        font-weight: 700;
        color: #1a1a1a;
        margin: 0;
      }
      .step-content {
        margin: 1em 0 0 0;
      }
      .step-content h4 {
        margin: 1.2em 0 0.5em;
        font-size: 0.95em;
        font-weight: 600;
        color: #444;
      }
      .step-content h4:first-child {
        margin-top: 0;
      }
      .step-content p, .step-content li {
        margin: 0.4em 0;
        line-height: 1.5;
        font-size: 0.95em;
      }
      .step-content ul {
        padding-left: 1.3em;
      }
      .step-nav {
        display: flex;
        gap: 1em;
        margin-top: 1.5em;
        padding-top: 1.5em;
        border-top: 1px solid #e5e7eb;
      }
      .btn-step {
        padding: 0.6em 1.4em;
        border: none;
        border-radius: 6px;
        font-weight: 600;
        cursor: pointer;
        font-size: 0.95em;
        transition: opacity 0.15s;
      }
      .btn-step:hover:not(:disabled) {
        opacity: 0.85;
      }
      .btn-step:disabled {
        opacity: 0.4;
        cursor: not-allowed;
      }
      .btn-next {
        background: #2563EB;
        color: white;
      }
      .btn-prev {
        background: #e5e7eb;
        color: #333;
      }
      .step-progress {
        margin: 1em 0;
        padding: 0.8em;
        background: #f0f9ff;
        border-radius: 4px;
        font-size: 0.9em;
        color: #1e40af;
      }
      .pcb-image {
        text-align: center;
        margin: 1.2em 0;
      }
      .pcb-image img {
        max-width: 100%;
        width: 100%;
        max-width: 500px;
        border: 1px solid #ccc;
        border-radius: 6px;
      }
      .pcb-caption {
        font-size: 0.85em;
        color: #666;
        margin-top: 0.5em;
        line-height: 1.4;
      }
    </style>

    <div id="setupGuide">
      <!-- STEP 1: Remove SD Cards -->
      <div class="setup-step active" data-step="1">
        <div class="step-header">
          <div class="step-number">1</div>
          <h3 class="step-title">Remove the SD Cards</h3>
        </div>
        <div class="step-content">
          <div class="pcb-image">
            <img src="/docs/get_started/_static/assets/dada-tbd-pcb-backside_001.jpg" alt="TBD PCB showing SD card slots">
            <div class="pcb-caption">
              <strong>Red circle:</strong> ESP32-P4 SD card slot (middle)<br>
              <strong>Other slot:</strong> RP2350 SD card slot (near edge)
            </div>
          </div>
          <h4>What to do:</h4>
          <ul>
            <li>Gently push each SD card inward until it clicks and pops out</li>
            <li>Set them aside in a safe place</li>
            <li>Label them to avoid mixing them up</li>
          </ul>
          <div class="step-progress">✓ When done, click <b>Next</b> to flash the ESP32-P4</div>
        </div>
        <div class="step-nav">
          <button class="btn-step btn-next" onclick="goToStep(2)">Next Step →</button>
        </div>
      </div>

      <!-- STEP 2: Flash ESP32-P4 -->
      <div class="setup-step" data-step="2">
        <div class="step-header">
          <div class="step-number">2</div>
          <h3 class="step-title">Flash ESP32-P4 Firmware</h3>
        </div>
        <div class="step-content">
          <h4>Physical setup:</h4>
          <ul>
            <li>Plug the <b>JTAG USB-C port</b> (front) into your computer</li>
            <li>Plug a <b>back USB-C port</b> into power or your computer</li>
          </ul>
          <h4>Use the flasher below:</h4>
        </div>
      </div>

      <!-- STEP 3: Flash RP2350 -->
      <div class="setup-step" data-step="3">
        <div class="step-header">
          <div class="step-number">3</div>
          <h3 class="step-title">Flash RP2350 Firmware</h3>
        </div>
        <div class="step-content">
          <h4>Enter BOOTSEL mode:</h4>
          <ul>
            <li>Hold the <b>BOOTSEL button</b> (front, next to JTAG port)</li>
            <li>While holding, press <b>RESET button</b> (to the right)</li>
            <li>Release both buttons</li>
          </ul>
          <h4>Use the flasher below:</h4>
        </div>
      </div>

      <!-- STEP 4: Prepare SD Cards -->
      <div class="setup-step" data-step="4">
        <div class="step-header">
          <div class="step-number">4</div>
          <h3 class="step-title">Prepare the SD Cards</h3>
        </div>
        <div class="step-content">
          <h4>Format ESP32-P4 SD card (middle slot):</h4>
          <ul>
            <li>Insert into your SD card reader</li>
            <li>Erase completely (backup any existing data first)</li>
            <li>Format as <b>FAT32</b> with label <b>NO NAME</b> (exactly)
              <ul style="margin-top: 0.4em;">
                <li><i>macOS:</i> Disk Utility → Erase → Format: MS-DOS (FAT) → Name: NO NAME</li>
                <li><i>Windows:</i> File Explorer → right-click → Format → FAT32 → Label: NO NAME</li>
                <li><i>Linux:</i> <code>mkfs.vfat -n "NO NAME" /dev/sdX</code></li>
              </ul>
            </li>
            <li>Download <a href="../_static/sdcard_image/tbd-sd-card.zip" target="_blank">tbd-sd-card.zip</a></li>
            <li>Extract and copy <b>all files directly to the root</b> of the card (not in a subfolder)</li>
            <li>Eject the card</li>
          </ul>
          <h4>Format RP2350 SD card (edge slot):</h4>
          <ul>
            <li>Insert into your SD card reader</li>
            <li>Erase and format as <b>FAT32</b> with label <b>NO NAME</b></li>
            <li><b>Leave it empty</b> — no files needed</li>
            <li>Eject the card</li>
          </ul>
        </div>
        <div class="step-nav">
          <button class="btn-step btn-prev" onclick="goToStep(3)">← Previous</button>
          <button class="btn-step btn-next" onclick="goToStep(5)">Next Step →</button>
        </div>
      </div>

      <!-- STEP 5: Insert & Boot -->
      <div class="setup-step" data-step="5">
        <div class="step-header">
          <div class="step-number">5</div>
          <h3 class="step-title">Insert SD Cards and Initialize</h3>
        </div>
        <div class="step-content">
          <h4>Insert both cards into the PCB back:</h4>
          <ul>
            <li><b>Middle slot:</b> Insert the ESP32-P4 SD card</li>
            <li><b>Edge slot:</b> Insert the RP2350 SD card</li>
            <li>Push each until it clicks</li>
          </ul>
          <h4>Power on and wait (5–10 minutes):</h4>
          <ul>
            <li>Device powers on</li>
            <li>OLED shows initialization messages</li>
            <li><b>Do not turn off the device</b></li>
          </ul>
          <h4>When done:</h4>
          <ul>
            <li>OLED shows debug information</li>
            <li><b>One line displays</b> <code>SD OK</code> ✓</li>
            <li>Device is ready for firmware update</li>
          </ul>
        </div>
        <div class="step-nav">
          <button class="btn-step btn-prev" onclick="goToStep(4)">← Previous</button>
          <button class="btn-step btn-next" onclick="goToStep(6)">Next Step →</button>
        </div>
      </div>

      <!-- STEP 6: Update to Possan -->
      <div class="setup-step" data-step="6">
        <div class="step-header">
          <div class="step-number">6</div>
          <h3 class="step-title">Update to Latest Firmware (Possan)</h3>
        </div>
        <div class="step-content">
          <h4>RP2350 first:</h4>
          <ul>
            <li>Enter <b>BOOTSEL mode</b> (hold BOOTSEL + press RESET)</li>
            <li>Select <b>Possan TBD (Experimental) — 2026-02-14</b> below</li>
            <li>Click Connect → Flash → Reboot</li>
          </ul>
          <h4>Then ESP32-P4:</h4>
          <ul>
            <li>Select <b>Possan TBD (Experimental) — 2026-02-14</b> in the P4 flasher above</li>
            <li>Click Connect → Flash → Disconnect</li>
          </ul>
          <div class="step-progress">
            Tip: Both flashers stay visible on your screen for easy access to both tools
          </div>
        </div>
        <div class="step-nav">
          <button class="btn-step btn-prev" onclick="goToStep(5)">← Previous</button>
          <button class="btn-step btn-next" onclick="goToStep(7)">Next Step →</button>
        </div>
      </div>

      <!-- STEP 7: Verify -->
      <div class="setup-step" data-step="7">
        <div class="step-header">
          <div class="step-number">7</div>
          <h3 class="step-title">Verify the Setup</h3>
        </div>
        <div class="step-content">
          <h4>Check these:</h4>
          <ul>
            <li>✓ Power on the device</li>
            <li>✓ OLED shows debug info with <code>SD OK</code> visible</li>
            <li>✓ Device responds to controls</li>
            <li>✓ Both firmware versions running correctly</li>
          </ul>
          <h4>Troubleshooting:</h4>
          <ul>
            <li><b>No "SD OK":</b> Reformat P4 card and re-extract files</li>
            <li><b>Won't boot:</b> Remove and reinsert both SD cards, power cycle</li>
            <li><b>Flashing failed:</b> Disconnect between steps and re-enter correct mode</li>
          </ul>
        </div>
        <div class="step-nav">
          <button class="btn-step btn-prev" onclick="goToStep(6)">← Previous</button>
        </div>
      </div>

      <!-- Setup Complete -->
      <div class="setup-step" data-step="complete" style="background: rgba(34, 197, 94, 0.08); border-left-color: #16A34A;">
        <div class="step-header">
          <div class="step-number" style="background: #16A34A;">✓</div>
          <h3 class="step-title" style="color: #16A34A;">Setup Complete!</h3>
        </div>
        <div class="step-content">
          <p>Your device is ready to use. See the <a href="./10_system.html">System</a> documentation for operating instructions and the <a href="../sound_library/index.html">Sound Library</a> to explore available processors.</p>
        </div>
      </div>
    </div>

    <script>
      function goToStep(stepNum) {
        document.querySelectorAll('.setup-step').forEach(el => el.classList.remove('active'));
        const step = document.querySelector(`[data-step="${stepNum}"]`);
        if (step) {
          step.classList.add('active');
          step.scrollIntoView({ behavior: 'smooth', block: 'start' });
          window.scrollTo(0, step.offsetTop - 100);
        }
      }
      
      // Scroll flashers into view when user reaches that step
      document.addEventListener('DOMContentLoaded', function() {
        // Note: flashers will be added below in separate raw:: html blocks
      });
    </script>

---

Flash ESP32-P4 (Main Processor)
===============================

.. raw:: html

    <style>
      .esp-flasher {
        border: 1px solid var(--color-background-border, #d1d5db);
        border-radius: 8px;
        padding: 1.5em 1.8em;
        margin: 1em 0 1.5em;
        background: var(--color-background-secondary, #fafafa);
      }
      .esp-flasher label {
        display: block;
        font-weight: 600;
        margin-bottom: 0.35em;
        font-size: 0.95em;
        color: var(--color-foreground-secondary, #555);
      }
      .esp-flasher select {
        width: 100%;
        padding: 0.5em 0.7em;
        border-radius: 4px;
        border: 1px solid var(--color-background-border, #ccc);
        margin-bottom: 1em;
        font-size: 0.95em;
        background: var(--color-background-primary, #fff);
        color: var(--color-foreground-primary, #222);
      }
      .esp-flasher .btn-row {
        display: flex;
        gap: 0.5em;
        flex-wrap: wrap;
      }
      .esp-flasher button {
        padding: 0.5em 1.2em;
        border: none;
        border-radius: 4px;
        font-size: 0.9em;
        font-weight: 600;
        cursor: pointer;
        color: #fff;
        transition: opacity 0.15s;
      }
      .esp-flasher button:disabled {
        opacity: 0.4;
        cursor: not-allowed;
      }
      .esp-flasher button:not(:disabled):hover {
        opacity: 0.85;
      }
      .esp-flasher .btn-connect { background: #2563EB; }
      .esp-flasher .btn-flash { background: #16A34A; }
      .esp-flasher .btn-disconnect { background: #6B7280; }
      .esp-flasher .progress-wrap {
        margin-top: 0.8em;
        background: #E5E7EB;
        border-radius: 4px;
        overflow: hidden;
        height: 22px;
        position: relative;
      }
      .esp-flasher .progress-bar {
        height: 100%;
        background: #2563EB;
        width: 0%;
        transition: width 0.15s;
      }
      .esp-flasher .progress-text {
        position: absolute;
        top: 0; left: 0; right: 0;
        text-align: center;
        line-height: 22px;
        font-size: 0.8em;
        font-weight: 600;
        color: #374151;
      }
      .esp-flasher .status-box {
        padding: 0.6em 0.9em;
        border-radius: 4px;
        font-size: 0.88em;
        min-height: 1.3em;
        margin-top: 0.8em;
        word-break: break-word;
        background: #F3F4F6;
        color: #374151;
      }
      .esp-flasher .status-success {
        background: #DEF7EC;
        color: #065F46;
      }
      .esp-flasher .status-error {
        background: #FEE2E2;
        color: #991B1B;
      }
    </style>

    <div class="esp-flasher" id="espFlasher">
      <label for="espFirmwareSelect">Firmware</label>
      <select id="espFirmwareSelect">
        <option value="ctag" selected>CTAG TBD (Development) — 2026-02-11</option>
        <option value="possan">Possan TBD (Experimental) — 2026-02-14</option>
      </select>

      <div class="btn-row">
        <button id="btnConnect" class="btn-connect" disabled>Loading…</button>
        <button id="btnFlash" class="btn-flash" disabled>Flash</button>
        <button id="btnDisconnect" class="btn-disconnect" disabled>Disconnect</button>
      </div>

      <div class="progress-wrap" id="progressWrap" style="display:none;">
        <div class="progress-bar" id="progressBar"></div>
        <span class="progress-text" id="progressText">0 %</span>
      </div>

      <div class="status-box" id="statusBox">Loading flash tool…</div>
    </div>

    <script>
      (async function () {
        var btnConnect    = document.getElementById('btnConnect');
        var btnFlash      = document.getElementById('btnFlash');
        var btnDisconnect = document.getElementById('btnDisconnect');
        var sel           = document.getElementById('espFirmwareSelect');
        var progressWrap  = document.getElementById('progressWrap');
        var progressBar   = document.getElementById('progressBar');
        var progressText  = document.getElementById('progressText');
        var statusBox     = document.getElementById('statusBox');

        function setStatus(msg, type) {
          statusBox.innerHTML = msg;
          statusBox.className = 'status-box' + (type ? ' status-' + type : '');
        }
        function setProgress(pct) {
          progressWrap.style.display = 'block';
          progressBar.style.width = pct + '%';
          progressText.textContent = pct + ' %';
        }
        function resetProgress() {
          progressWrap.style.display = 'none';
          progressBar.style.width = '0%';
          progressText.textContent = '0 %';
        }

        if (!('serial' in navigator)) {
          setStatus('Your browser does not support WebSerial. Please use <b>Chrome</b>, <b>Edge</b> or <b>Opera</b> on desktop.', 'error');
          return;
        }

        var ESPLoader, Transport;
        try {
          var mod = await import('https://unpkg.com/esptool-js@0.5.7/bundle.js');
          ESPLoader = mod.ESPLoader;
          Transport = mod.Transport;
        } catch (e) {
          setStatus('Failed to load flash tool: ' + e.message, 'error');
          return;
        }

        btnConnect.textContent = 'Connect';
        btnConnect.disabled = false;
        setStatus('Select a firmware, then click <b>Connect</b>.');

        var FIRMWARE = {
          ctag:   { url: '_static/firmware/p4/ctag-tbd-2026-02-11.bin',   name: 'CTAG TBD' },
          possan: { url: '_static/firmware/p4/possan-tbd-2026-02-14.bin', name: 'Possan TBD' }
        };

        var device    = null;
        var transport = null;
        var esploader = null;
        var connected = false;

        async function cleanup() {
          connected = false;
          if (transport) { try { await transport.disconnect(); } catch (_) {} }
          device = null;  transport = null;  esploader = null;
        }

        btnConnect.addEventListener('click', async function () {
          try {
            btnConnect.disabled = true;
            setStatus('Requesting serial port…');

            device    = await navigator.serial.requestPort({});
            transport = new Transport(device, true);

            var terminal = {
              clean:     function () {},
              writeLine: function (d) { console.log(d); },
              write:     function (d) { console.log(d); }
            };
            esploader = new ESPLoader({
              transport: transport,
              baudrate:  460800,
              terminal:  terminal
            });

            setStatus('Connecting…');
            var chip = await esploader.main();
            connected = true;

            btnFlash.disabled      = false;
            btnDisconnect.disabled = false;
            sel.disabled           = true;
            setStatus('Connected to <b>' + chip + '</b>. Click <b>Flash</b> to program.', 'success');
          } catch (e) {
            console.error(e);
            setStatus('Connection failed: ' + e.message, 'error');
            btnConnect.disabled = false;
            await cleanup();
          }
        });

        btnFlash.addEventListener('click', async function () {
          if (!connected || !esploader) return;
          try {
            btnFlash.disabled      = true;
            btnDisconnect.disabled = true;
            btnConnect.disabled    = true;

            var fw = FIRMWARE[sel.value];
            setStatus('Downloading <b>' + fw.name + '</b> firmware…');

            var resp = await fetch(fw.url);
            if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
            var data = new Uint8Array(await resp.arrayBuffer());

            var chunks = [];
            var chunkSize = 8192;
            for (var i = 0; i < data.length; i += chunkSize) {
              chunks.push(String.fromCharCode.apply(null, data.subarray(i, i + chunkSize)));
            }
            var binaryString = chunks.join('');

            var sizeMB = (data.length / 1024 / 1024).toFixed(1);
            setStatus('Flashing <b>' + fw.name + '</b> (' + sizeMB + ' MB) — do not unplug the device…');

            var flashOptions = {
              fileArray:      [{ data: binaryString, address: 0x0 }],
              flashSize:      '16MB',
              flashMode:      'dio',
              flashFreq:      '80m',
              eraseAll:       false,
              compress:       true,
              reportProgress: function (fileIndex, written, total) {
                var pct = Math.round((written / total) * 100);
                setProgress(pct);
              }
            };

            await esploader.writeFlash(flashOptions);
            setProgress(100);
            setStatus('Flash complete — resetting device…', 'success');

            try { await esploader.after(); } catch (_) {}

            await cleanup();
            btnConnect.disabled = false;
            sel.disabled        = false;
            setStatus('&#10003; Flash complete. Proceed to the next step.', 'success');
          } catch (e) {
            console.error(e);
            setStatus('Flash failed: ' + e.message, 'error');
            btnDisconnect.disabled = false;
            if (connected) btnFlash.disabled = false;
          }
        });

        btnDisconnect.addEventListener('click', async function () {
          await cleanup();
          btnConnect.disabled    = false;
          btnFlash.disabled      = true;
          btnDisconnect.disabled = true;
          sel.disabled           = false;
          resetProgress();
          setStatus('Disconnected. Click <b>Connect</b> to start again.');
        });
      })();
    </script>



Flash RP2350 (Secondary Processor)
==================================

.. raw:: html

    <style>
      .pico-flasher {
        border: 1px solid var(--color-background-border, #d1d5db);
        border-radius: 8px;
        padding: 1.5em 1.8em;
        margin: 1em 0 1.5em;
        background: var(--color-background-secondary, #fafafa);
      }
      .pico-flasher label {
        display: block;
        font-weight: 600;
        margin-bottom: 0.35em;
        font-size: 0.95em;
        color: var(--color-foreground-secondary, #555);
      }
      .pico-flasher select {
        width: 100%;
        padding: 0.5em 0.7em;
        border-radius: 4px;
        border: 1px solid var(--color-background-border, #ccc);
        margin-bottom: 1em;
        font-size: 0.95em;
        background: var(--color-background-primary, #fff);
        color: var(--color-foreground-primary, #222);
      }
      .pico-flasher .btn-row {
        display: flex;
        gap: 0.5em;
        flex-wrap: wrap;
        margin-bottom: 0.8em;
      }
      .pico-flasher button {
        padding: 0.5em 1.2em;
        border: none;
        border-radius: 4px;
        font-size: 0.9em;
        font-weight: 600;
        cursor: pointer;
        transition: opacity 0.15s;
      }
      .pico-flasher button:disabled {
        opacity: 0.35;
        cursor: not-allowed;
      }
      .pico-flasher .btn-connect    { background: #2563EB; color: #fff; }
      .pico-flasher .btn-flash      { background: #16A34A; color: #fff; }
      .pico-flasher .btn-reboot     { background: #6B7280; color: #fff; }
      .pico-flasher .btn-disconnect { background: #6B7280; color: #fff; }
      .pico-flasher button:hover:not(:disabled) { opacity: 0.85; }
      .pico-flasher .status-box {
        padding: 0.6em 0.9em;
        border-radius: 4px;
        font-size: 0.88em;
        min-height: 1.3em;
        word-break: break-word;
      }
      .pico-flasher .status-idle    { background: #F3F4F6; color: #374151; }
      .pico-flasher .status-busy    { background: #DBEAFE; color: #1E40AF; }
      .pico-flasher .status-success { background: #D1FAE5; color: #065F46; }
      .pico-flasher .status-error   { background: #FEE2E2; color: #991B1B; }
      .pico-flasher .device-info {
        font-size: 0.82em;
        margin-bottom: 0.8em;
        padding: 0.4em 0.7em;
        border-radius: 4px;
        background: var(--color-background-primary, #fff);
        border: 1px solid var(--color-background-border, #e5e7eb);
        display: none;
      }
      .pico-flasher .progress-wrap {
        height: 5px;
        background: #E5E7EB;
        border-radius: 3px;
        margin-top: 0.5em;
        overflow: hidden;
        display: none;
      }
      .pico-flasher .progress-bar {
        height: 100%;
        background: #2563EB;
        border-radius: 3px;
        width: 0%;
        transition: width 0.3s;
      }
    </style>

    <div class="pico-flasher" id="picoFlasher">
      <label for="firmwareSelect">Firmware</label>
      <select id="firmwareSelect">        
        <option value="ctag-tbd-2026-02-11.uf2">CTAG TBD (Development) — 2026-02-11</option>        
        <option value="possan-tbd-2026-02-14.uf2">Possan TBD (Experimental) — 2026-02-14</option>
      </select>

      <div class="btn-row">
        <button class="btn-connect" id="btnConnect">Connect</button>
        <button class="btn-flash"   id="btnFlash"   disabled>Flash</button>
        <button class="btn-reboot"  id="btnReboot"  disabled>Reboot</button>
        <button class="btn-disconnect" id="btnDisconnect" disabled>Disconnect</button>
      </div>

      <div class="device-info" id="deviceInfo"></div>

      <div class="status-box status-idle" id="statusBox">
        Ready &mdash; put your device in BOOTSEL mode and click <b>Connect</b>.
      </div>

      <div class="progress-wrap" id="progressWrap">
        <div class="progress-bar" id="progressBar"></div>
      </div>
    </div>

    <script type="module">
      import { Picoboot } from '_static/picoflash/pkg/index.js';
      import { uf2ToFlashBuffer } from '_static/picoflash/js/uf2.js';

      const FIRMWARE_BASE = '_static/firmware/pico/';
      const OP_TIMEOUT    = 30000;
      const SHORT_TIMEOUT = 5000;

      const $ = id => document.getElementById(id);

      const btnConnect    = $('btnConnect');
      const btnFlash      = $('btnFlash');
      const btnReboot     = $('btnReboot');
      const btnDisconnect = $('btnDisconnect');
      const fwSelect      = $('firmwareSelect');
      const deviceInfo    = $('deviceInfo');
      const statusBox     = $('statusBox');
      const progressWrap  = $('progressWrap');
      const progressBar   = $('progressBar');

      let picoboot = null;

      function setStatus(msg, status = 'idle') {
        statusBox.innerHTML = msg;
        statusBox.className = 'status-box status-' + status;
      }

      if (!navigator.usb) {
        setStatus('Your browser does not support WebUSB. Please use <b>Chrome</b>, <b>Edge</b>, or <b>Opera</b>.', 'error');
      } else {
        btnConnect.addEventListener('click', async () => {
          try {
            btnConnect.disabled = true;
            setStatus('Requesting device…', 'busy');

            const device = await navigator.usb.requestDevice({ filters: [] });
            picoboot = new Picoboot(device);

            setStatus('Connecting…', 'busy');
            await picoboot.connect();

            deviceInfo.style.display = 'block';
            deviceInfo.textContent = 'Device: RP2350 | Ready to flash';

            btnFlash.disabled = false;
            btnReboot.disabled = false;
            btnDisconnect.disabled = false;
            fwSelect.disabled = true;

            setStatus('Connected to <b>RP2350</b>. Click <b>Flash</b> to program.', 'success');
          } catch (e) {
            console.error(e);
            setStatus('Connection failed: ' + e.message, 'error');
            btnConnect.disabled = false;
          }
        });

        btnFlash.addEventListener('click', async () => {
          if (!picoboot) return;
          try {
            btnFlash.disabled = true;
            btnReboot.disabled = true;
            btnConnect.disabled = true;

            const fwFile = fwSelect.value;
            setStatus('Downloading firmware…', 'busy');

            const resp = await fetch(FIRMWARE_BASE + fwFile);
            if (!resp.ok) throw new Error('Download failed: ' + resp.statusText);
            const buffer = await resp.arrayBuffer();

            setStatus('Processing UF2…', 'busy');
            const flashBuffer = uf2ToFlashBuffer(new Uint8Array(buffer));

            progressWrap.style.display = 'block';
            setStatus('Flashing…', 'busy');

            await picoboot.flash(flashBuffer, (offset, length) => {
              const pct = Math.round((offset / length) * 100);
              progressBar.style.width = pct + '%';
            }, OP_TIMEOUT);

            progressBar.style.width = '100%';
            setStatus('Flash complete. Click <b>Reboot</b> to restart.', 'success');
          } catch (e) {
            console.error(e);
            setStatus('Flash failed: ' + e.message, 'error');
            btnFlash.disabled = false;
            btnReboot.disabled = false;
          }
        });

        btnReboot.addEventListener('click', async () => {
          if (!picoboot) return;
          try {
            btnReboot.disabled = true;
            setStatus('Rebooting…', 'busy');
            await picoboot.reboot(SHORT_TIMEOUT);
            setStatus('Device rebooted. Proceed to the next step.', 'success');
            await picoboot.disconnect();
            btnConnect.disabled = false;
            fwSelect.disabled = false;
            deviceInfo.style.display = 'none';
            picoboot = null;
          } catch (e) {
            console.error(e);
            setStatus('Reboot failed: ' + e.message, 'error');
            btnReboot.disabled = false;
          }
        });

        btnDisconnect.addEventListener('click', async () => {
          if (picoboot) {
            try { await picoboot.disconnect(); } catch (_) {}
          }
          picoboot = null;
          btnConnect.disabled = false;
          btnFlash.disabled = true;
          btnReboot.disabled = true;
          btnDisconnect.disabled = true;
          fwSelect.disabled = false;
          deviceInfo.style.display = 'none';
          progressWrap.style.display = 'none';
          setStatus('Disconnected. Click <b>Connect</b> to start again.', 'idle');
        });
      }
    </script>
