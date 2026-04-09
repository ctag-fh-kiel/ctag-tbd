// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — App Shell
// Vanilla JS · Shoelace Web Components
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
//
// Licensed under the GNU Lesser General Public License (LGPL 3.0).
// https://www.gnu.org/licenses/lgpl-3.0.txt
//
// Part of the dadamachines additions to the CTAG TBD platform.
// See LICENSE in the repository root for full terms.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── Configuration Dialog ────────────────────────────────
  var currentConfig = null;

  async function loadConfiguration() {
    try {
      currentConfig = await S.queuedFetch('/device?action=getConfig');
      populateConfigDialog(currentConfig);
    } catch (e) {
      console.error('Failed to load config:', e);
    }
  }

  function populateConfigDialog(config) {
    // Connection tab
    var apiUrl = document.getElementById('cfg-api-url');
    if (apiUrl) apiUrl.value = config.apiEndpoint || window.location.origin;

    // MIDI tab
    var midiEnable = document.getElementById('cfg-midi-enable');
    if (midiEnable) midiEnable.checked = !!config.midiEnabled;

    var midiChannel = document.getElementById('cfg-midi-channel');
    if (midiChannel) {
      if (midiChannel.querySelectorAll('sl-option').length === 0) {
        var html = '';
        for (var i = 1; i <= 16; i++) {
          html += '<sl-option value="' + i + '">Channel ' + i + '</sl-option>';
        }
        midiChannel.innerHTML = html;
      }
      midiChannel.value = String(config.midiChannel || 1);
    }

    // Enumerate MIDI devices if Web MIDI is available
    populateMidiDevices();

    // WiFi tab — firmware stores in nested config.wifi object
    var wifi = config.wifi || {};
    var wifiSsid = document.getElementById('cfg-wifi-ssid');
    if (wifiSsid) wifiSsid.value = wifi.ssid || '';

    var wifiPassword = document.getElementById('cfg-wifi-password');
    if (wifiPassword) wifiPassword.value = wifi.pwd || '';

    var wifiMdns = document.getElementById('cfg-wifi-mdns');
    if (wifiMdns) wifiMdns.value = wifi.mdns_name || '';

    var wifiMode = wifi.mode || 'ap';
    var wifiAp = document.getElementById('cfg-wifi-ap');
    var wifiSta = document.getElementById('cfg-wifi-sta');
    var wifiUsbncm = document.getElementById('cfg-wifi-usbncm');
    if (wifiAp) wifiAp.checked = (wifiMode === 'ap');
    if (wifiSta) wifiSta.checked = (wifiMode === 'sta');
    if (wifiUsbncm) wifiUsbncm.checked = (wifiMode === 'usbncm');

    // Audio tab — firmware uses ch0_codecLvlOut (0-63) and ch0_outputSoftClip ("on"/"off")
    var ch0Level = document.getElementById('cfg-input-gain');
    var ch0LevelVal = document.getElementById('cfg-input-gain-val');
    if (ch0Level) {
      var lvl0 = parseInt(config.ch0_codecLvlOut) || 58;
      ch0Level.min = 0; ch0Level.max = 63;
      ch0Level.value = lvl0;
      if (ch0LevelVal) ch0LevelVal.textContent = lvl0 + ' / 63';
    }
    var ch1Level = document.getElementById('cfg-output-gain');
    var ch1LevelVal = document.getElementById('cfg-output-gain-val');
    if (ch1Level) {
      var lvl1 = parseInt(config.ch1_codecLvlOut) || 58;
      ch1Level.min = 0; ch1Level.max = 63;
      ch1Level.value = lvl1;
      if (ch1LevelVal) ch1LevelVal.textContent = lvl1 + ' / 63';
    }
    var noiseGate = document.getElementById('cfg-noise-gate');
    if (noiseGate) noiseGate.checked = !!config.noiseGate;
    var softClipCh0 = document.getElementById('cfg-soft-clip-ch0');
    if (softClipCh0) softClipCh0.checked = config.ch0_outputSoftClip === 'on';
    var softClipCh1 = document.getElementById('cfg-soft-clip-ch1');
    if (softClipCh1) softClipCh1.checked = config.ch1_outputSoftClip === 'on';

    // Channel routing
    var daisyChain = document.getElementById('cfg-daisy-chain');
    if (daisyChain) daisyChain.value = config.ch01_daisy || 'off';
    var ch0Stereo = document.getElementById('cfg-ch0-stereo');
    if (ch0Stereo) ch0Stereo.value = config.ch0_toStereo || 'off';
    var ch1Stereo = document.getElementById('cfg-ch1-stereo');
    if (ch1Stereo) ch1Stereo.value = config.ch1_toStereo || 'off';

    // Appearance tab
    var compact = document.getElementById('cfg-compact');
    if (compact) compact.checked = !!config.compactLayout;

    // System tab — fetch version info from IOCaps and AppInfo endpoints
    fetchSystemInfo();
  }

  async function fetchSystemInfo() {
    try {
      var iocaps = await S.queuedFetch('/device?action=getIOCaps');
      var firmware = document.getElementById('cfg-firmware');
      if (firmware) firmware.textContent = iocaps.FWV || '—';
      var hardware = document.getElementById('cfg-hardware');
      if (hardware) hardware.textContent = iocaps.HWV || '—';
    } catch (e) {
      console.warn('Failed to fetch IOCaps:', e);
    }
    try {
      var appInfo = await S.queuedFetch('/device?action=getAppInfo');
      var picoFw = document.getElementById('cfg-pico-firmware');
      if (picoFw) picoFw.textContent = appInfo.pico_version || '—';
    } catch (e) {
      console.warn('Failed to fetch AppInfo:', e);
    }
  }

  function populateMidiDevices() {
    var container = document.getElementById('cfg-midi-devices');
    if (!container) return;

    if (navigator.requestMIDIAccess) {
      navigator.requestMIDIAccess().then(function(access) {
        var html = '';
        var inputs = access.inputs;
        var outputs = access.outputs;
        var deviceNames = {};

        inputs.forEach(function(input) {
          deviceNames[input.name] = deviceNames[input.name] || { input: false, output: false };
          deviceNames[input.name].input = true;
        });
        outputs.forEach(function(output) {
          deviceNames[output.name] = deviceNames[output.name] || { input: false, output: false };
          deviceNames[output.name].output = true;
        });

        var names = Object.keys(deviceNames);
        if (names.length === 0) {
          container.innerHTML = '<div style="font-size:0.82rem;color:var(--sl-color-neutral-500);padding:0.5rem 0;">No MIDI devices detected</div>';
          return;
        }

        names.forEach(function(name) {
          var d = deviceNames[name];
          var badges = '';
          if (d.input) badges += '<span class="midi-device-badge">Input</span>';
          if (d.output) badges += '<span class="midi-device-badge">Output</span>';
          html += '<div class="midi-device-item"><span style="font-size:0.85rem;">' + S.esc(name) + '</span><div>' + badges + '</div></div>';
        });
        container.innerHTML = html;
      }).catch(function() {
        container.innerHTML = '<div style="font-size:0.82rem;color:var(--sl-color-neutral-500);padding:0.5rem 0;">MIDI access denied</div>';
      });
    } else {
      container.innerHTML = '<div style="font-size:0.82rem;color:var(--sl-color-neutral-500);padding:0.5rem 0;">Web MIDI not supported in this browser</div>';
    }
  }

  function setupConfigDialog() {
    // Config tab switching
    var configTabs = document.getElementById('config-tabs');
    if (configTabs) {
      configTabs.addEventListener('click', function(e) {
        var tab = e.target.closest('.config-tab');
        if (!tab) return;
        var target = tab.getAttribute('data-tab');
        configTabs.querySelectorAll('.config-tab').forEach(function(t) { t.classList.remove('active'); });
        tab.classList.add('active');
        document.querySelectorAll('#config-content .config-tab-panel').forEach(function(p) { p.classList.remove('active'); });
        var panel = document.getElementById(target);
        if (panel) panel.classList.add('active');
      });
    }

    // Config button
    var configBtn = document.getElementById('config-btn');
    if (configBtn) {
      configBtn.addEventListener('click', function() {
        loadConfiguration();
        document.getElementById('config-dialog').show();
      });
    }

    // Sample Manager Info button
    var samplesInfoBtn = document.getElementById('samples-info-btn');
    var samplesInfoDialog = document.getElementById('samples-info-dialog');
    if (samplesInfoBtn && samplesInfoDialog) {
      samplesInfoBtn.addEventListener('click', function() { samplesInfoDialog.show(); });
      var samplesInfoClose = document.getElementById('samples-info-close-btn');
      if (samplesInfoClose) samplesInfoClose.addEventListener('click', function() { samplesInfoDialog.hide(); });
    }

    // Save
    var configSave = document.getElementById('config-save');
    if (configSave) configSave.addEventListener('click', saveConfiguration);

    // Cancel
    var configCancel = document.getElementById('config-cancel');
    if (configCancel) {
      configCancel.addEventListener('click', function() {
        document.getElementById('config-dialog').hide();
      });
    }

    // Reboot button in system tab
    var cfgReboot = document.getElementById('cfg-reboot');
    if (cfgReboot) {
      cfgReboot.addEventListener('click', function() {
        document.getElementById('config-dialog').hide();
        document.getElementById('reboot-dialog').show();
      });
    }

    // Test Connection button
    var testConn = document.getElementById('cfg-test-connection');
    if (testConn) {
      testConn.addEventListener('click', async function() {
        var dot = document.getElementById('cfg-conn-dot');
        var statusEl = document.getElementById('cfg-conn-status');
        if (dot) dot.style.background = '#ff9800';
        if (statusEl) statusEl.textContent = 'Testing…';
        try {
          var apiUrl = document.getElementById('cfg-api-url');
          var url = (apiUrl ? apiUrl.value : window.location.origin) + '/api/v2/device?action=getIOCaps';
          var resp = await S.apiQueue.enqueue(function() {
            return fetch(url, { method: 'GET', signal: AbortSignal.timeout(5000) });
          });
          if (resp.ok) {
            if (dot) dot.style.background = '#4caf50';
            if (statusEl) statusEl.textContent = 'Connected';
          } else {
            if (dot) dot.style.background = '#f44336';
            if (statusEl) statusEl.textContent = 'Error (' + resp.status + ')';
          }
        } catch (e) {
          if (dot) dot.style.background = '#f44336';
          if (statusEl) statusEl.textContent = 'Unreachable';
        }
      });
    }

    // WiFi Save button
    var wifiSave = document.getElementById('cfg-wifi-save');
    if (wifiSave) {
      wifiSave.addEventListener('click', async function() {
        if (!currentConfig) {
          S.toast('Configuration not loaded yet', 'warning');
          return;
        }
        var mode = 'ap';
        if (document.getElementById('cfg-wifi-sta') && document.getElementById('cfg-wifi-sta').checked) mode = 'sta';
        if (document.getElementById('cfg-wifi-usbncm') && document.getElementById('cfg-wifi-usbncm').checked) mode = 'usbncm';
        var ssid = document.getElementById('cfg-wifi-ssid');
        var password = document.getElementById('cfg-wifi-password');
        var mdns = document.getElementById('cfg-wifi-mdns');
        // Validate password length (firmware requires >= 8 or empty)
        var pwd = password ? password.value : '';
        if (pwd.length > 0 && pwd.length < 8) {
          S.toast('Password must be at least 8 characters, or empty', 'warning');
          return;
        }
        // Merge WiFi into firmware's nested wifi object (config.wifi)
        var config = currentConfig;
        if (!config.wifi) config.wifi = {};
        config.wifi.mode = mode;
        config.wifi.ssid = ssid ? ssid.value : '';
        config.wifi.pwd = pwd;
        config.wifi.mdns_name = mdns ? mdns.value : '';
        try {
          await S.queuedPost('/device?action=setConfig', config);
          currentConfig = config;
          S.toast('WiFi settings saved. Reboot for changes to take effect.', 'warning', 5000);
        } catch (e) {
          S.toast('Failed to save WiFi settings', 'danger');
        }
      });
    }

    // Firmware Update — inline OTA panel in System tab
    var fwUpdate = document.getElementById('cfg-firmware-update');
    var otaPanel = document.getElementById('ota-panel');
    var otaInfo = document.getElementById('ota-info');
    var otaStepSelect = document.getElementById('ota-step-select');
    var otaStepConfirm = document.getElementById('ota-step-confirm');
    var otaStepProgress = document.getElementById('ota-step-progress');
    var otaStepDone = document.getElementById('ota-step-done');
    var otaFileName = document.getElementById('ota-file-name');
    var otaProgress = document.getElementById('ota-progress');
    var otaProgressText = document.getElementById('ota-progress-text');
    var _otaFile = null;
    var _otaInfo = null;

    function otaReset() {
      otaStepSelect.style.display = '';
      otaStepConfirm.style.display = 'none';
      otaStepProgress.style.display = 'none';
      otaStepDone.style.display = 'none';
      otaFileName.textContent = '';
      _otaFile = null;
      otaProgress.value = 0;
    }

    if (fwUpdate && otaPanel) {
      fwUpdate.addEventListener('click', function() {
        otaReset();
        otaPanel.style.display = '';
        otaInfo.textContent = 'Loading partition info…';
        fetch('/api/v2/ota')
          .then(function(r) { return r.json(); })
          .then(function(info) {
            _otaInfo = info;
            var maxMB = (info.maxSize / (1024 * 1024)).toFixed(1);
            otaInfo.innerHTML = 'Running: <b>' + info.running + '</b> &nbsp;→&nbsp; Target: <b>' + info.next + '</b> (max ' + maxMB + ' MB)';
          })
          .catch(function(e) {
            otaInfo.textContent = 'Could not query OTA status: ' + e.message;
          });
      });

      document.getElementById('ota-panel-close').addEventListener('click', function() {
        otaPanel.style.display = 'none';
        otaReset();
      });

      document.getElementById('ota-pick-file').addEventListener('click', function() {
        var input = document.createElement('input');
        input.type = 'file';
        input.accept = '.bin';
        input.addEventListener('change', function() {
          if (!input.files || !input.files[0] || !_otaInfo) return;
          _otaFile = input.files[0];
          var maxMB = (_otaInfo.maxSize / (1024*1024)).toFixed(1);
          var fileMB = (_otaFile.size / (1024*1024)).toFixed(1);
          if (_otaFile.size > _otaInfo.maxSize) {
            S.toast('Firmware too large (' + fileMB + ' MB > ' + maxMB + ' MB)', 'danger');
            _otaFile = null;
            return;
          }
          otaFileName.textContent = _otaFile.name + ' (' + fileMB + ' MB)';
          otaStepSelect.style.display = 'none';
          otaStepConfirm.style.display = '';
        });
        input.click();
      });

      document.getElementById('ota-cancel-file').addEventListener('click', function() {
        otaStepConfirm.style.display = 'none';
        otaStepSelect.style.display = '';
        otaFileName.textContent = '';
        _otaFile = null;
      });

      document.getElementById('ota-start-upload').addEventListener('click', function() {
        if (!_otaFile) return;
        otaStepConfirm.style.display = 'none';
        otaStepProgress.style.display = '';
        otaProgress.value = 0;
        otaProgressText.textContent = 'Uploading… 0%';

        var xhr = new XMLHttpRequest();
        xhr.open('POST', '/api/v2/ota', true);
        xhr.setRequestHeader('Content-Type', 'application/octet-stream');

        xhr.upload.addEventListener('progress', function(e) {
          if (e.lengthComputable) {
            var pct = Math.round(100 * e.loaded / e.total);
            otaProgress.value = pct;
            otaProgressText.textContent = 'Uploading… ' + pct + '%';
          }
        });

        xhr.addEventListener('load', function() {
          if (xhr.status === 200) {
            otaStepProgress.style.display = 'none';
            otaStepDone.style.display = '';
            try { var resp = JSON.parse(xhr.responseText); } catch(e) {}
            var part = (resp && resp.partition) || 'OTA partition';
            document.getElementById('ota-done-text').textContent = '✓ Firmware flashed to ' + part;
          } else {
            otaStepProgress.style.display = 'none';
            otaStepSelect.style.display = '';
            S.toast('OTA failed: ' + xhr.responseText, 'danger', 8000);
          }
        });

        xhr.addEventListener('error', function() {
          otaStepProgress.style.display = 'none';
          otaStepSelect.style.display = '';
          S.toast('Upload failed — connection error', 'danger');
        });

        xhr.addEventListener('timeout', function() {
          otaStepProgress.style.display = 'none';
          otaStepSelect.style.display = '';
          S.toast('Upload timed out', 'danger');
        });

        xhr.timeout = 300000;
        xhr.send(_otaFile);
      });

      document.getElementById('ota-reboot').addEventListener('click', function() {
        fetch('/api/v2/device', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ action: 'reboot' })
        }).catch(function() {});
        otaPanel.style.display = 'none';
        otaReset();
        S.toast('Rebooting device…', 'primary', 5000);
      });
    }

    // Backup / Restore buttons
    var backupBtn = document.getElementById('cfg-backup');
    if (backupBtn) {
      backupBtn.addEventListener('click', async function() {
        try {
          S.showLoading('Creating backup…');
          var backup = {};

          // 1) Configuration
          backup.configuration = await S.queuedFetch('/device?action=getConfig');

          // 2) Favorites
          backup.favorites = await S.queuedFetch('/device?action=getFavorites');

          // 3) All plugin preset data
          var plugins = await S.queuedFetch('/plugins?action=list');
          backup.presets = {};
          for (var i = 0; i < plugins.length; i++) {
            var pid = plugins[i].id;
            if (pid === 'Void') continue;
            try {
              backup.presets[pid] = await S.queuedFetch('/plugins?action=getPresetData&id=' + encodeURIComponent(pid));
            } catch (e) {
              // Plugin may not have preset data — skip silently
            }
          }

          // 4) Write as JSON download
          var data = JSON.stringify(backup, null, 2);
          var blob = new Blob([data], { type: 'application/json' });
          var url = URL.createObjectURL(blob);
          var a = document.createElement('a');
          a.href = url;
          var dateStr = new Date().toISOString().slice(0, 10);
          a.download = 'ctag-tbd-backup-' + dateStr + '.json';
          document.body.appendChild(a);
          a.click();
          document.body.removeChild(a);
          URL.revokeObjectURL(url);

          S.hideLoading();
          S.toast('Backup created', 'success', 2000);
        } catch (e) {
          S.hideLoading();
          S.toast('Backup failed: ' + e.message, 'danger');
        }
      });
    }
    var restoreBtn = document.getElementById('cfg-restore');
    if (restoreBtn) {
      restoreBtn.addEventListener('click', function() {
        var input = document.createElement('input');
        input.type = 'file';
        input.accept = '.json';
        input.addEventListener('change', async function() {
          if (!input.files || !input.files[0]) return;
          if (!confirm('This will overwrite ALL settings, presets, and favorites. Continue?')) return;
          try {
            S.showLoading('Restoring backup…');
            var text = await input.files[0].text();
            var backup = JSON.parse(text);

            // 1) Restore configuration
            if (backup.configuration) {
              await S.queuedPost('/device?action=setConfig', backup.configuration);
            }

            // 2) Restore favorites
            if (Array.isArray(backup.favorites)) {
              for (var i = 0; i < backup.favorites.length; i++) {
                if (backup.favorites[i] && backup.favorites[i].plug_0) {
                  await S.queuedPost('/device?action=storeFavorite&id=' + i, backup.favorites[i]);
                }
              }
            }

            // 3) Restore preset data
            if (backup.presets) {
              var pluginIds = Object.keys(backup.presets);
              for (var j = 0; j < pluginIds.length; j++) {
                var pid = pluginIds[j];
                try {
                  await S.queuedPost('/plugins?action=setPresetData&id=' + encodeURIComponent(pid), backup.presets[pid]);
                } catch (e) {
                  // Non-critical — plugin may not exist on this device
                }
              }
            }

            S.hideLoading();
            S.toast('Backup restored — please reload the page', 'success', 5000);
          } catch (e) {
            S.hideLoading();
            S.toast('Restore failed: ' + e.message, 'danger');
          }
        });
        input.click();
      });
    }

    // Audio tab - codec level range labels
    var ch0Level = document.getElementById('cfg-input-gain');
    var ch0LevelVal = document.getElementById('cfg-input-gain-val');
    if (ch0Level && ch0LevelVal) {
      ch0Level.addEventListener('input', function() {
        var v = parseInt(ch0Level.value);
        ch0LevelVal.textContent = v + ' / 63';
      });
    }
    var ch1Level = document.getElementById('cfg-output-gain');
    var ch1LevelVal = document.getElementById('cfg-output-gain-val');
    if (ch1Level && ch1LevelVal) {
      ch1Level.addEventListener('input', function() {
        var v = parseInt(ch1Level.value);
        ch1LevelVal.textContent = v + ' / 63';
      });
    }
    // Audio save button
    var audioSave = document.getElementById('cfg-audio-save');
    if (audioSave) {
      audioSave.addEventListener('click', async function() {
        if (!currentConfig) {
          S.toast('Configuration not loaded yet', 'warning');
          return;
        }
        // Merge audio settings using firmware's actual keys
        var config = currentConfig;
        config.ch0_codecLvlOut = ch0Level ? String(parseInt(ch0Level.value)) : '58';
        config.ch1_codecLvlOut = ch1Level ? String(parseInt(ch1Level.value)) : '58';
        var scCh0 = document.getElementById('cfg-soft-clip-ch0');
        var scCh1 = document.getElementById('cfg-soft-clip-ch1');
        config.ch0_outputSoftClip = (scCh0 && scCh0.checked) ? 'on' : 'off';
        config.ch1_outputSoftClip = (scCh1 && scCh1.checked) ? 'on' : 'off';
        // Channel routing
        var daisy = document.getElementById('cfg-daisy-chain');
        if (daisy) config.ch01_daisy = daisy.value;
        var ch0s = document.getElementById('cfg-ch0-stereo');
        if (ch0s) config.ch0_toStereo = ch0s.value;
        var ch1s = document.getElementById('cfg-ch1-stereo');
        if (ch1s) config.ch1_toStereo = ch1s.value;
        try {
          await S.queuedPost('/device?action=setConfig', config);
          currentConfig = config;
          S.toast('Audio settings saved', 'success');
        } catch (e) {
          S.toast('Failed to save audio settings', 'danger');
        }
      });
    }

    // Palette selection
    var paletteGrid = document.getElementById('palette-grid');
    if (paletteGrid) {
      paletteGrid.addEventListener('click', function(e) {
        var card = e.target.closest('.palette-card');
        if (!card) return;
        paletteGrid.querySelectorAll('.palette-card').forEach(function(c) { c.classList.remove('active'); });
        card.classList.add('active');
        var paletteName = card.getAttribute('data-palette');
        localStorage.setItem('tbd-palette', paletteName);
        applyPalette(paletteName);
      });
      // Restore saved palette
      var saved = localStorage.getItem('tbd-palette');
      if (saved) {
        var savedCard = paletteGrid.querySelector('[data-palette="' + saved + '"]');
        if (savedCard) {
          paletteGrid.querySelectorAll('.palette-card').forEach(function(c) { c.classList.remove('active'); });
          savedCard.classList.add('active');
          applyPalette(saved);
        }
      }
    }

    // Control Mode toggle
    var controlModeSwitch = document.getElementById('cfg-control-mode');
    if (controlModeSwitch) {
      // Restore saved state
      if (S.isControlMode()) {
        controlModeSwitch.checked = true;
        S.loadWebAudioControls().catch(function(e) {
          console.warn('webaudio-controls load failed:', e);
        });
      }
      controlModeSwitch.addEventListener('sl-change', function() {
        S.setControlMode(controlModeSwitch.checked);
        // Re-render params in both slots to reflect mode change
        if (window.TBD.pluginManager && window.TBD.pluginManager.rerenderParams) {
          window.TBD.pluginManager.rerenderParams();
        }
      });
    }
  }

  var palettes = {
    'rams-warm': {
      primary: '#c87533',
      neutral: '#5c5c4a',
    },
    'rams-contrast': {
      primary: '#d04a35',
      neutral: '#2d2d2d',
    },
    'rams-muted': {
      primary: '#c05a3c',
      neutral: '#3d3d2e',
    },
  };

  function applyPalette(name) {
    var p = palettes[name];
    if (!p) return;
    // Apply primary accent color as CSS custom property on :root
    document.documentElement.style.setProperty('--sl-color-primary-600', p.primary);
    // Lighter variant for hover
    document.documentElement.style.setProperty('--sl-color-primary-500', lightenColor(p.primary, 15));
    document.documentElement.style.setProperty('--sl-color-primary-700', darkenColor(p.primary, 15));
  }

  function lightenColor(hex, pct) {
    var r = parseInt(hex.slice(1,3), 16);
    var g = parseInt(hex.slice(3,5), 16);
    var b = parseInt(hex.slice(5,7), 16);
    r = Math.min(255, Math.round(r + (255 - r) * pct / 100));
    g = Math.min(255, Math.round(g + (255 - g) * pct / 100));
    b = Math.min(255, Math.round(b + (255 - b) * pct / 100));
    return '#' + [r, g, b].map(function(c) { return c.toString(16).padStart(2, '0'); }).join('');
  }

  function darkenColor(hex, pct) {
    var r = parseInt(hex.slice(1,3), 16);
    var g = parseInt(hex.slice(3,5), 16);
    var b = parseInt(hex.slice(5,7), 16);
    r = Math.max(0, Math.round(r * (1 - pct / 100)));
    g = Math.max(0, Math.round(g * (1 - pct / 100)));
    b = Math.max(0, Math.round(b * (1 - pct / 100)));
    return '#' + [r, g, b].map(function(c) { return c.toString(16).padStart(2, '0'); }).join('');
  }

  async function saveConfiguration() {
    if (!currentConfig) {
      S.toast('Configuration not loaded yet', 'warning');
      return;
    }
    var config = currentConfig;

    var apiUrl = document.getElementById('cfg-api-url');
    if (apiUrl) config.apiEndpoint = apiUrl.value;

    var midiEnable = document.getElementById('cfg-midi-enable');
    if (midiEnable) config.midiEnabled = midiEnable.checked;

    var midiChannel = document.getElementById('cfg-midi-channel');
    if (midiChannel) config.midiChannel = parseInt(midiChannel.value, 10) || 1;

    var compact = document.getElementById('cfg-compact');
    if (compact) config.compactLayout = compact.checked;

    try {
      await S.queuedPost('/device?action=setConfig', config);
      S.toast('Configuration saved', 'success');
      document.getElementById('config-dialog').hide();
    } catch (e) {
      S.toast('Failed to save configuration', 'danger');
    }
  }

  // ─── Debug Panel ──────────────────────────────────────────

  var debugOpen = false;

  function setupDebugPanel() {
    var toggle = document.getElementById('debug-toggle');
    var panel = document.getElementById('debug-panel');
    var closeBtn = document.getElementById('debug-close');
    var refreshBtn = document.getElementById('debug-refresh');

    if (toggle && panel) {
      toggle.addEventListener('click', function() {
        debugOpen = !debugOpen;
        panel.classList.toggle('expanded', debugOpen);
        if (debugOpen) refreshDebugPanel();
      });
    }

    if (closeBtn && panel) {
      closeBtn.addEventListener('click', function() {
        debugOpen = false;
        panel.classList.remove('expanded');
      });
    }

    if (refreshBtn) {
      refreshBtn.addEventListener('click', refreshDebugPanel);
    }

    // Debug tab switching
    var debugTabs = panel ? panel.querySelectorAll('.debug-tab') : [];
    debugTabs.forEach(function(tab) {
      tab.addEventListener('click', function() {
        debugTabs.forEach(function(t) { t.classList.remove('active'); });
        tab.classList.add('active');
        panel.querySelectorAll('.debug-tab-panel').forEach(function(p) { p.classList.remove('active'); });
        var target = document.getElementById(tab.getAttribute('data-tab'));
        if (target) target.classList.add('active');
      });
    });
  }

  // ─── API Call Tracking ─────────────────────────────────────
  var debugApiCalls = 0;
  var debugApiErrors = 0;
  var debugLastApi = '—';
  var debugLastError = 'None';
  var debugParamChanges = 0;
  var debugApiLog = [];
  var MAX_API_LOG = 50;

  // Wrap S.apiFetch to track API calls
  (function() {
    if (!S || !S.apiFetch) return;
    var origFetch = S.apiFetch;
    S.apiFetch = async function(url) {
      debugApiCalls++;
      var shortUrl = url.replace(/^\/api\/v2/, '');
      debugLastApi = shortUrl;
      var ts = new Date().toLocaleTimeString();
      debugApiLog.unshift(ts + ' ' + shortUrl);
      if (debugApiLog.length > MAX_API_LOG) debugApiLog.pop();
      try {
        var result = await origFetch.apply(this, arguments);
        return result;
      } catch (e) {
        debugApiErrors++;
        debugLastError = shortUrl + ' — ' + (e.message || String(e));
        throw e;
      }
    };
  })();

  // Track parameter changes from plugin-manager
  S.onParamChange = function() { debugParamChanges++; };

  function refreshDebugPanel() {
    var pm = window.TBD.pluginManager;
    var pmState = pm ? pm.state : {};
    var sm = window.TBD.sampleManager;

    function set(id, val) {
      var el = document.getElementById(id);
      if (el) el.textContent = val;
    }

    // App State
    set('dbg-active-view', activeView || '—');
    set('dbg-slot-a', (pmState.activePlugin && pmState.activePlugin[0]) ? pmState.activePlugin[0].id : 'None');
    set('dbg-slot-b', (pmState.activePlugin && pmState.activePlugin[1]) ? pmState.activePlugin[1].id : 'None');
    set('dbg-stereo', pmState.stereoLocked ? 'Yes' : 'No');
    set('dbg-plugin-count', pmState.plugins ? pmState.plugins.length : '0');

    // Preset info
    var presetA = '—', presetB = '—';
    if (pmState.presets && pmState.presets[0] && pmState.activePreset) {
      var pa = pmState.presets[0].find(function(p) { return p.number === pmState.activePreset[0]; });
      presetA = pa ? pa.name + ' (#' + pa.number + ')' : (pmState.activePreset[0] >= 0 ? '#' + pmState.activePreset[0] : '—');
    }
    if (pmState.presets && pmState.presets[1] && pmState.activePreset) {
      var pb = pmState.presets[1].find(function(p) { return p.number === pmState.activePreset[1]; });
      presetB = pb ? pb.name + ' (#' + pb.number + ')' : (pmState.activePreset[1] >= 0 ? '#' + pmState.activePreset[1] : '—');
    }
    set('dbg-preset-a', presetA);
    set('dbg-preset-b', presetB);

    // Favorites
    set('dbg-fav-count', pm && pm.favoritesCache ? Object.keys(pm.favoritesCache).length : '0');

    // Kit info
    if (sm && sm.state) {
      var kitNames = sm.state.kits ? sm.state.kits.smp_bank_names || [] : [];
      var activeKit = sm.state.kits ? sm.state.kits.active_smp_bank : 0;
      var kitName = kitNames[activeKit] || 'Kit ' + activeKit;
      var bankCount = sm.state.banks ? sm.state.banks.length : 0;
      var sampleCount = sm.state.kitEntries ? sm.state.kitEntries.filter(Boolean).length : 0;
      set('dbg-kit-info', kitName + ' — ' + bankCount + ' banks, ' + sampleCount + ' samples');
    } else {
      set('dbg-kit-info', 'Not loaded');
    }

    // Page load time
    if (window.performance && window.performance.timing) {
      var t = window.performance.timing;
      var loadMs = t.loadEventEnd - t.navigationStart;
      set('dbg-load-time', loadMs > 0 ? loadMs + ' ms' : '—');
    }

    // Network
    set('dbg-ws-status', S.connectionState ? (S.connectionState.connected ? '● Connected' : '○ Disconnected') : '—');
    set('dbg-api-url', S.API_BASE || window.location.origin);
    set('dbg-api-calls', String(debugApiCalls));
    set('dbg-api-errors', String(debugApiErrors));
    set('dbg-last-api', debugLastApi);
    set('dbg-last-error', debugLastError);

    // API log
    var logEl = document.getElementById('dbg-api-log');
    if (logEl) {
      logEl.textContent = debugApiLog.length > 0 ? debugApiLog.join('\n') : 'No API calls logged yet.';
    }

    // Parameters
    var paramCountA = 0, paramCountB = 0, groupCountA = 0, groupCountB = 0;
    if (pmState.params && pmState.params[0] && pmState.params[0].params) {
      paramCountA = pmState.params[0].params.length;
      groupCountA = pmState.params[0].params.filter(function(p) { return p.type === 'group'; }).length;
    }
    if (pmState.params && pmState.params[1] && pmState.params[1].params) {
      paramCountB = pmState.params[1].params.length;
      groupCountB = pmState.params[1].params.filter(function(p) { return p.type === 'group'; }).length;
    }
    set('dbg-params-a', String(paramCountA));
    set('dbg-groups-a', String(groupCountA));
    set('dbg-params-b', String(paramCountB));
    set('dbg-groups-b', String(groupCountB));
    set('dbg-param-changes', String(debugParamChanges));
  }

  // ─── View Switching ───────────────────────────────────────
  var activeView = 'view-plugins';
  var sampleManagerInited = false;

  // ─── RP2350 App Awareness ─────────────────────────────────
  // Stores the active RP2350 app ID and capability flags
  window.TBD.rp2350App = '';
  window.TBD.pluginLock = false;
  window.TBD.redirectSamples = false;

  /** Fetch active RP2350 app from ESP32. Non-critical — silently tolerates failure. */
  async function fetchAppInfo() {
    try {
      var info = await S.queuedFetch('/device?action=getAppInfo');
      window.TBD.rp2350App = (info && info.rp2350_app) ? info.rp2350_app : '';
      window.TBD.pluginLock = !!(info && info.plugin_lock);
      window.TBD.redirectSamples = !!(info && info.redirect_samples);
    } catch (e) {
      window.TBD.rp2350App = '';
      window.TBD.pluginLock = false;
      window.TBD.redirectSamples = false;
    }
  }

  /** Show or hide the plugin lock overlay based on RP2350 plugin_lock flag. */
  function updatePluginLock() {
    var overlay = document.getElementById('plugin-lock-overlay');
    if (!overlay) return;
    if (window.TBD.pluginLock) {
      // Update overlay text with the app name
      var h3 = overlay.querySelector('h3');
      if (h3) {
        var appName = window.TBD.rp2350App || 'RP2350 firmware';
        h3.textContent = 'Plugins are managed by ' + appName;
      }
      overlay.classList.remove('hidden');
    } else {
      overlay.classList.add('hidden');
    }
  }

  function switchView(viewId) {
    if (viewId === activeView) return;

    // Toggle active class on view containers
    var views = document.querySelectorAll('#view-plugins, #view-samples');
    views.forEach(function(v) { v.classList.remove('active'); });
    var target = document.getElementById(viewId);
    if (target) target.classList.add('active');

    // Toggle active class on nav tabs
    var tabs = document.querySelectorAll('.nav-tab');
    tabs.forEach(function(t) { t.classList.remove('active'); });
    var tab = document.querySelector('.nav-tab[data-view="' + viewId + '"]');
    if (tab) tab.classList.add('active');

    // Show/hide storage bar (only visible in sample view)
    var storageBar = document.getElementById('header-storage');
    if (storageBar) {
      if (viewId === 'view-samples') {
        storageBar.classList.remove('hidden');
      } else {
        storageBar.classList.add('hidden');
      }
    }

    // Show/hide favorites bar (only visible in plugin view)
    var favBar = document.getElementById('favorites-bar');
    if (favBar) {
      if (viewId === 'view-samples') {
        favBar.classList.add('hidden');
      } else {
        favBar.classList.remove('hidden');
      }
    }

    // Show/hide Sample Manager header buttons (only in sample view)
    var tdBtn = document.getElementById('trackdefaults-btn');
    var siBtn = document.getElementById('samples-info-btn');
    if (tdBtn) tdBtn.style.display = (viewId === 'view-samples') ? '' : 'none';
    if (siBtn) siBtn.style.display = (viewId === 'view-samples') ? '' : 'none';

    activeView = viewId;

    // Lazy-init Sample Manager on first switch
    if (viewId === 'view-samples' && !sampleManagerInited) {
      sampleManagerInited = true;
      if (window.TBD.sampleManager) {
        window.TBD.sampleManager.init();
      }
      if (window.TBD.trackDefaults && window.TBD.trackDefaults.init) {
        window.TBD.trackDefaults.init();
      }
    }

    // Update URL without reload
    var url = new URL(window.location);
    url.searchParams.set('view', viewId === 'view-samples' ? 'samples' : 'plugins');
    history.replaceState(null, '', url);
  }

  // ─── Setup ───────────────────────────────────────────────
  function setup() {
    // Theme toggle
    S.setupThemeToggle('theme-toggle');

    // Nav tab click handlers
    var navTabs = document.querySelectorAll('.nav-tab[data-view]');
    navTabs.forEach(function(tab) {
      tab.addEventListener('click', function() {
        switchView(tab.getAttribute('data-view'));
      });
    });

    // Config dialog (tabbed)
    setupConfigDialog();

    // Debug panel
    setupDebugPanel();

    // Reboot confirm
    var rebootOk = document.getElementById('reboot-ok');
    if (rebootOk) {
      rebootOk.addEventListener('click', async function() {
        document.getElementById('reboot-dialog').hide();
        try {
          await S.queuedPost('/device?action=reboot', null);
          S.toast('Rebooting…', 'warning', 6000);
          S.setDisconnected();
        } catch (e) {
          // Expected — device reboots immediately
          S.toast('Rebooting…', 'warning', 6000);
          S.setDisconnected();
        }
      });
    }
    var rebootCancel = document.getElementById('reboot-cancel');
    if (rebootCancel) {
      rebootCancel.addEventListener('click', function() {
        document.getElementById('reboot-dialog').hide();
      });
    }

    // Connection monitor — only used for reconnect detection
    S.startConnectionMonitor(
      async function onConnect() {
        // Skip the first connect (handled by init() below)
        if (!S.connectionState._firstConnectDone) return;
        // Refresh on reconnect — sequential to avoid socket exhaustion
        if (window.TBD.pluginManager) {
          await window.TBD.pluginManager.init();
        }
        // Refresh sample manager if it was initialized
        if (sampleManagerInited && window.TBD.sampleManager) {
          await window.TBD.sampleManager.init();
        }
        // Re-check RP2350 app (user may have rebooted with different firmware)
        await fetchAppInfo();
        updatePluginLock();
      },
      function onDisconnect() {
        // Nothing extra needed — UI updates via shared.js
      }
    );
  }

  // ─── Init ────────────────────────────────────────────────
  function init() {
    setup();

    // Check URL for initial view
    var params = new URLSearchParams(window.location.search);
    var requestedView = params.get('view');

    // Initialize plugin manager (first load)
    if (window.TBD.pluginManager) {
      window.TBD.pluginManager.init().then(async function() {
        // Only set connected if circuit breaker didn't trigger disconnect during init
        if (S.connectionState.status !== 'disconnected') {
          S.setConnected();
        }
        // Mark first connect done after a tick so onConnect callback doesn't re-trigger
        setTimeout(function() {
          S.connectionState._firstConnectDone = true;
        }, 100);

        // Fetch RP2350 app info and apply plugin lock / redirect
        await fetchAppInfo();
        updatePluginLock();
        if (window.TBD.redirectSamples && !requestedView) {
          switchView('view-samples');
        }
      }).catch(function() {
        S.setDisconnected();
      });
    }

    // If ?view=samples was requested, switch to it after a short delay
    if (requestedView === 'samples') {
      setTimeout(function() { switchView('view-samples'); }, 300);
    }
  }

  // Boot
  document.addEventListener('DOMContentLoaded', function() {
    setTimeout(init, 150);
  });

})();
