****************************
Initial Device Setup Guide
****************************

Complete interactive setup for your TBD device. Navigate through 7 simple steps.

**Time:** 15–20 minutes | **Requirements:** TBD device, 2× USB-C cables, Chrome/Edge/Opera, 2× SD cards, reader

.. raw:: html

    <style>
      #setupGuide {
        font-family: system-ui, -apple-system, sans-serif;
        max-width: 900px;
      }
      .steps-overview {
        display: grid;
        grid-template-columns: repeat(7, 1fr);
        gap: 0.5em;
        margin: 1.5em 0;
        padding: 1em;
        background: #f8f9fa;
        border-radius: 8px;
        border: 1px solid #e5e7eb;
      }
      .step-overview-item {
        text-align: center;
        padding: 0.6em 0.4em;
        border-radius: 4px;
        background: white;
        border: 2px solid #ddd;
        cursor: pointer;
        transition: all 0.2s;
        min-width: 60px;
      }
      .step-overview-item:hover {
        border-color: #2563EB;
        background: #f0f9ff;
      }
      .step-overview-item.active {
        background: #2563EB;
        color: white;
        border-color: #2563EB;
      }
      .step-overview-num {
        font-weight: 700;
        font-size: 1em;
        margin-bottom: 0.2em;
      }
      .step-overview-label {
        font-size: 0.65em;
        line-height: 1.2;
        opacity: 0.85;
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
        min-width: 36px;
        height: 36px;
        background: #2563EB;
        color: white;
        border-radius: 50%;
        font-weight: 600;
        font-size: 1em;
      }
      .step-title {
        font-size: 1.4em;
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
      .btn-step:hover {
        opacity: 0.85;
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
      <!-- STEP OVERVIEW -->
      <div class="steps-overview">
        <div class="step-overview-item active" onclick="goToStep(1)">
          <div class="step-overview-num">1</div>
          <div class="step-overview-label">Remove<br>SD Cards</div>
        </div>
        <div class="step-overview-item" onclick="goToStep(2)">
          <div class="step-overview-num">2</div>
          <div class="step-overview-label">Flash<br>P4</div>
        </div>
        <div class="step-overview-item" onclick="goToStep(3)">
          <div class="step-overview-num">3</div>
          <div class="step-overview-label">Flash<br>RP2350</div>
        </div>
        <div class="step-overview-item" onclick="goToStep(4)">
          <div class="step-overview-num">4</div>
          <div class="step-overview-label">Prepare<br>SD Cards</div>
        </div>
        <div class="step-overview-item" onclick="goToStep(5)">
          <div class="step-overview-num">5</div>
          <div class="step-overview-label">Insert &<br>Boot</div>
        </div>
        <div class="step-overview-item" onclick="goToStep(6)">
          <div class="step-overview-num">6</div>
          <div class="step-overview-label">Update<br>Possan</div>
        </div>
        <div class="step-overview-item" onclick="goToStep(7)">
          <div class="step-overview-num">7</div>
          <div class="step-overview-label">Verify<br>Setup</div>
        </div>
      </div>

      <!-- STEP 1: Remove SD Cards -->
      <div class="setup-step active" data-step="1">
        <div class="step-header">
          <div class="step-number">1</div>
          <h3 class="step-title">Remove the SD Cards</h3>
        </div>
        <div class="step-content">
          <div class="pcb-image">
            <img src="../_static/assets/dada-tbd-pcb-backside_001.jpg" alt="TBD PCB showing SD card slots" style="max-width: 100%; width: 100%; max-width: 370px; border: 1px solid #ccc; border-radius: 6px;">
            <div class="pcb-caption" style="font-size: 0.85em; color: #666; margin-top: 0.5em; line-height: 1.4;">
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
        <div class="step-nav">
          <button class="btn-step btn-prev" onclick="goToStep(1)">← Previous</button>
          <button class="btn-step btn-next" onclick="goToStep(3)">Next Step →</button>
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
            <li>While holding, press <b>RESET button</b> (to the left of BOOTSEL)</li>
            <li>Release both buttons</li>
          </ul>
          <h4>Flash the firmware:</h4>
          <ul>
            <li>Click <b>Connect</b> in the flasher below</li>
            <li>In the device picker, select <b>"RP2350 Boot"</b></li>
            <li>Click <b>Flash</b>, then <b>Reboot</b> when done</li>
          </ul>
        </div>
        <div class="step-nav">
          <button class="btn-step btn-prev" onclick="goToStep(2)">← Previous</button>
          <button class="btn-step btn-next" onclick="goToStep(4)">Next Step →</button>
        </div>
      </div>

      <!-- STEP 4: Prepare SD Cards -->
      <div class="setup-step" data-step="4">
        <div class="step-header">
          <div class="step-number">4</div>
          <h3 class="step-title">Prepare the SD Cards</h3>
        </div>
        <div class="step-content">
          <h4>ESP32-P4 SD card (middle slot):</h4>
          <ul>
            <li>Format as <b>FAT32</b>, label <b>NO NAME</b>
              <span style="font-size: 0.78em; color: #888;"> — macOS: Disk Utility → MS-DOS (FAT) | Win: right-click → FAT32 | Linux: <code style="font-size: 0.95em;">mkfs.vfat -n "NO NAME"</code></span>
            </li>
            <li>Download <a href="../_static/sdcard_image/tbd-sd-card.zip" target="_blank"><b>tbd-sd-card.zip</b></a> + <a href="../_static/sdcard_image/tbd-sd-card-hash.txt" target="_blank"><b>tbd-sd-card-hash.txt</b></a> (samples &amp; settings)</li>
            <li>Copy <b>both files</b> to the SD card root (do NOT unzip) → eject</li>
          </ul>
          <h4>RP2350 SD card (edge slot):</h4>
          <ul>
            <li>Format as <b>FAT32</b>, label <b>NO NAME</b> — leave empty → eject</li>
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
          <h4>Power on and wait (~1–2 minutes):</h4>
          <ul>
            <li>Device powers on</li>
            <li>OLED shows initialization messages</li>
            <li><b>Do not turn off the device</b> while it initializes</li>
          </ul>
          <h4>When done:</h4>
          <ul>
            <li>Device is ready for the Possan firmware update</li>
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
            <li>Select <b>Possan TBD (Experimental) — 2026-02-14</b> in the RP2350 flasher below</li>
            <li>Click Connect → Flash → Reboot</li>
          </ul>
          <h4>Then ESP32-P4:</h4>
          <ul>
            <li>Select <b>Possan TBD (Experimental) — 2026-02-14</b> in the P4 flasher below</li>
            <li>Click Connect → Flash → Disconnect</li>
          </ul>
          <div class="step-progress">
            Both flashers are visible below so you can easily access both tools
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
          <div class="step-number" style="background: #16A34A;">✓</div>
          <h3 class="step-title" style="color: #16A34A;">Congratulations — You're Done!</h3>
        </div>
        <div class="step-content">
          <p style="font-size: 1.05em; color: #065F46; background: #D1FAE5; padding: 0.8em 1em; border-radius: 6px; margin-bottom: 1em;">
            🎉 <b>Your TBD device is fully set up and ready to go!</b> All firmware has been flashed, SD cards are prepared, and the latest Possan firmware is installed. Time to make some noise!
          </p>
          <h4>Final checks:</h4>
          <ul>
            <li>✓ Power on the device</li>
            <li>✓ OLED shows the groovebox interface</li>
            <li>✓ There is <b>no</b> <code>P4 dead</code> message on the OLED — this confirms both processors are running</li>
            <li>✓ Device responds to controls</li>
          </ul>
          <h4>Troubleshooting:</h4>
          <ul>
            <li><b>"P4 dead" on OLED:</b> Press the <b>top right button</b> and select <b>"Reboot P4"</b></li>
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
        // Hide all steps
        document.querySelectorAll('.setup-step').forEach(el => el.classList.remove('active'));
        // Hide all flasher sections
        document.querySelectorAll('.flasher-section').forEach(el => el.style.display = 'none');
        // Update overview dots
        document.querySelectorAll('.step-overview-item').forEach(el => el.classList.remove('active'));
        const activeOverview = document.querySelector(`.step-overview-item:nth-child(${stepNum})`);
        if (activeOverview) activeOverview.classList.add('active');
        
        // Show current step
        const step = document.querySelector(`[data-step="${stepNum}"]`);
        if (step) {
          step.classList.add('active');
          // Show relevant flasher sections
          if (stepNum === 2) {
            const p4Flasher = document.getElementById('flasher-p4');
            if (p4Flasher) p4Flasher.style.display = 'block';
            // Reset and select CTAG firmware
            if (typeof window.resetEspFlasher === 'function') window.resetEspFlasher();
            var espSel2 = document.getElementById('espFirmwareSelect');
            if (espSel2) espSel2.value = 'ctag';
          } else if (stepNum === 3) {
            const rp2350Flasher = document.getElementById('flasher-rp2350');
            if (rp2350Flasher) rp2350Flasher.style.display = 'block';
            // Reset and select CTAG firmware
            if (typeof window.resetPicoFlasher === 'function') window.resetPicoFlasher();
            var picoSel3 = document.getElementById('picoFirmwareSelect');
            if (picoSel3) picoSel3.value = 'ctag-tbd-2026-02-11.uf2';
          } else if (stepNum === 6) {
            // Show both flashers for step 6
            const p4Flasher = document.getElementById('flasher-p4');
            const rp2350Flasher = document.getElementById('flasher-rp2350');
            if (p4Flasher) p4Flasher.style.display = 'block';
            if (rp2350Flasher) rp2350Flasher.style.display = 'block';
            // Preselect Possan firmware for the update step
            var espSel = document.getElementById('espFirmwareSelect');
            if (espSel) espSel.value = 'possan';
            var picoSel = document.getElementById('picoFirmwareSelect');
            if (picoSel) picoSel.value = 'possan-tbd-2026-02-14.uf2';
            // Reset flasher UI state so it's clean for re-use
            if (typeof window.resetEspFlasher === 'function') window.resetEspFlasher();
            if (typeof window.resetPicoFlasher === 'function') window.resetPicoFlasher();
            // Re-set Possan after reset (reset restores defaults)
            if (espSel) espSel.value = 'possan';
            if (picoSel) picoSel.value = 'possan-tbd-2026-02-14.uf2';
          }
          step.scrollIntoView({ behavior: 'smooth', block: 'start' });
          window.scrollTo(0, step.offsetTop - 100);
        }
      }
      
      // Initialize on load - hide all flasher sections initially
      document.addEventListener('DOMContentLoaded', function() {
        document.querySelectorAll('.flasher-section').forEach(el => el.style.display = 'none');
      });
    </script>

.. raw:: html

    <div id="flasher-rp2350" class="flasher-section">
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
      <label for="picoFirmwareSelect"><b>RP2350 Flasher</b></label>
      <select id="picoFirmwareSelect">        
        <option value="ctag-tbd-2026-02-11.uf2">ctag-tbd-2026-02-11.uf2 — CTAG TBD (Development)</option>        
        <option value="possan-tbd-2026-02-14.uf2">possan-tbd-2026-02-14.uf2 — Possan TBD (Experimental)</option>
      </select>

      <div class="btn-row">
        <button class="btn-connect" id="picoBtnConnect">Connect</button>
        <button class="btn-flash"   id="picoBtnFlash"   disabled>Flash</button>
        <button class="btn-reboot"  id="picoBtnReboot"  disabled>Reboot</button>
        <button class="btn-disconnect" id="picoBtnDisconnect" disabled>Disconnect</button>
      </div>

      <div class="device-info" id="picoDeviceInfo"></div>

      <div class="status-box status-idle" id="picoStatusBox">
        Ready &mdash; put your device in BOOTSEL mode and click <b>Connect</b>.
      </div>

      <div class="progress-wrap" id="picoProgressWrap">
        <div class="progress-bar" id="picoProgressBar"></div>
      </div>
    </div>

    <script type="module">
      import { Picoboot } from '../_static/picoflash/pkg/index.js';
      import { uf2ToFlashBuffer } from '../_static/picoflash/js/uf2.js';

      const FIRMWARE_BASE = '../_static/firmware/pico/';
      const OP_TIMEOUT    = 30000;
      const SHORT_TIMEOUT = 5000;

      const $ = id => document.getElementById(id);

      const btnConnect    = $('picoBtnConnect');
      const btnFlash      = $('picoBtnFlash');
      const btnReboot     = $('picoBtnReboot');
      const btnDisconnect = $('picoBtnDisconnect');
      const fwSelect      = $('picoFirmwareSelect');
      const deviceInfo    = $('picoDeviceInfo');
      const statusBox     = $('picoStatusBox');
      const progressWrap  = $('picoProgressWrap');
      const progressBar   = $('picoProgressBar');

      let picoboot = null;
      let connection = null;

      function setStatus(msg, cls = 'idle') {
        statusBox.className = 'status-box status-' + cls;
        statusBox.innerHTML = msg;
      }
      function showProgress(pct) {
        progressWrap.style.display = 'block';
        progressBar.style.width = pct + '%';
      }
      function hideProgress() {
        progressWrap.style.display = 'none';
        progressBar.style.width = '0%';
      }
      function withTimeout(promise, ms, label) {
        return Promise.race([
          promise,
          new Promise((_, reject) =>
            setTimeout(() => reject(new Error(label + ' timed out after ' + (ms/1000) + 's')), ms)
          )
        ]);
      }
      function updateButtons(busy) {
        if (busy) {
          btnConnect.disabled = btnFlash.disabled = btnReboot.disabled = btnDisconnect.disabled = true;
          return;
        }
        const c = picoboot && picoboot.isConnected();
        btnConnect.disabled = c;
        btnFlash.disabled = !c;
        btnReboot.disabled = !c;
        btnDisconnect.disabled = !c;
      }
      async function tryRecover() {
        try { if (connection) await connection.resetInterface(); return true; }
        catch (_) { await doDisconnect(); return false; }
      }
      async function doDisconnect() {
        try { if (picoboot) await picoboot.disconnect(); } catch (_) {}
        picoboot = null; connection = null;
        deviceInfo.style.display = 'none';
      }

      /* Expose a reset function so goToStep() can reset the UI */
      window.resetPicoFlasher = function() {
        doDisconnect();
        hideProgress();
        fwSelect.disabled = false;
        setStatus('Ready &mdash; put your device in BOOTSEL mode and click <b>Connect</b>.', 'idle');
        updateButtons(false);
      };

      /* ── connect ── */
      btnConnect.addEventListener('click', async () => {
        try {
          updateButtons(true);
          setStatus('Waiting for device selection… choose <b>RP2350 Boot</b>', 'busy');
          picoboot = await Picoboot.requestDevice();
          setStatus('Connecting…', 'busy');
          connection = await withTimeout(picoboot.connect(), SHORT_TIMEOUT, 'Connect');
          setStatus('Resetting interface…', 'busy');
          await withTimeout(connection.resetInterface(), SHORT_TIMEOUT, 'Reset interface');
          const info = picoboot.getUsbDeviceInfo();
          const target = picoboot.getTarget();
          deviceInfo.style.display = 'block';
          deviceInfo.innerHTML =
            '<b>Connected:</b> ' + (info.productName || target.toString()) +
            ' &nbsp;|&nbsp; ' + picoboot.getInfo();
          setStatus('Device connected — select firmware and click <b>Flash</b>.', 'success');
        } catch (e) {
          setStatus('Connection failed: ' + e.message, 'error');
          await doDisconnect();
        }
        updateButtons(false);
      });

      /* ── flash ── */
      btnFlash.addEventListener('click', async () => {
        const fwFile = fwSelect.value;
        if (!fwFile) { setStatus('Please select a firmware.', 'error'); return; }
        try {
          updateButtons(true);
          setStatus('Downloading firmware…', 'busy'); showProgress(10);
          const resp = await fetch(FIRMWARE_BASE + fwFile);
          if (!resp.ok) throw new Error('Download failed: HTTP ' + resp.status);
          const uf2Data = new Uint8Array(await resp.arrayBuffer());
          showProgress(25);
          setStatus('Parsing UF2 file…', 'busy');
          const { address, data } = uf2ToFlashBuffer(uf2Data);
          const sizeKB = (data.length / 1024).toFixed(0);
          setStatus('Erasing & writing ' + sizeKB + ' KB at 0x' + address.toString(16) + '…', 'busy');
          showProgress(35);
          await picoboot.flashEraseAndWrite(address, data);
          showProgress(100);
          setStatus('Flash complete &#10003; — click <b>Reboot</b> to restart the device.', 'success');
        } catch (e) {
          hideProgress();
          const recovered = await tryRecover();
          setStatus('Flash failed: ' + e.message +
            (recovered ? '. Device recovered — you can try again.' : '. Please reconnect the device.'),
            'error');
        }
        updateButtons(false);
      });

      /* ── reboot ── */
      btnReboot.addEventListener('click', async () => {
        try {
          updateButtons(true);
          setStatus('Rebooting device…', 'busy');
          try { await withTimeout(connection.reboot(100), SHORT_TIMEOUT, 'Reboot'); }
          catch (e) { console.warn('Reboot command error (may be expected):', e.message); }
          await doDisconnect(); hideProgress();
          setStatus('Device rebooted and disconnected. Proceed to the next step.', 'success');
        } catch (e) {
          setStatus('Reboot failed: ' + e.message, 'error');
          await doDisconnect();
        }
        updateButtons(false);
      });

      /* ── disconnect ── */
      btnDisconnect.addEventListener('click', async () => {
        await doDisconnect(); hideProgress();
        setStatus('Disconnected.', 'idle');
        updateButtons(false);
      });

      if (!('usb' in navigator)) {
        setStatus('Your browser does not support WebUSB. Please use Chrome, Edge or Opera.', 'error');
        btnConnect.disabled = true;
      }
    </script>
    </div>

.. raw:: html

    <div id="flasher-p4" class="flasher-section">
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
      <label for="espFirmwareSelect"><b>ESP32-P4 Flasher</b></label>
      <select id="espFirmwareSelect">
        <option value="ctag" selected>ctag-tbd-2026-02-11.bin — CTAG TBD (Development)</option>
        <option value="possan">possan-tbd-2026-02-14.bin — Possan TBD (Experimental)</option>
      </select>

      <div class="btn-row">
        <button id="espBtnConnect" class="btn-connect" disabled>Loading…</button>
        <button id="espBtnFlash" class="btn-flash" disabled>Flash</button>
        <button id="espBtnDisconnect" class="btn-disconnect" disabled>Disconnect</button>
      </div>

      <div class="progress-wrap" id="espProgressWrap" style="display:none;">
        <div class="progress-bar" id="espProgressBar"></div>
        <span class="progress-text" id="espProgressText">0 %</span>
      </div>

      <div class="status-box" id="espStatusBox">Loading flash tool…</div>
    </div>

    <script>
      (async function () {
        var btnConnect    = document.getElementById('espBtnConnect');
        var btnFlash      = document.getElementById('espBtnFlash');
        var btnDisconnect = document.getElementById('espBtnDisconnect');
        var sel           = document.getElementById('espFirmwareSelect');
        var progressWrap  = document.getElementById('espProgressWrap');
        var progressBar   = document.getElementById('espProgressBar');
        var progressText  = document.getElementById('espProgressText');
        var statusBox     = document.getElementById('espStatusBox');

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
          ctag:   { url: '../_static/firmware/p4/ctag-tbd-2026-02-11.bin',   name: 'CTAG TBD' },
          possan: { url: '../_static/firmware/p4/possan-tbd-2026-02-14.bin', name: 'Possan TBD' }
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

        /* Expose a reset function so goToStep() can reset the UI */
        window.resetEspFlasher = function() {
          cleanup();
          resetProgress();
          sel.disabled = false;
          setStatus('Select a firmware, then click <b>Connect</b>.');
        };
      })();
    </script>
    </div>
