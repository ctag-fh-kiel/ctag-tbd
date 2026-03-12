// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Plugin Manager
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

  // ─── State ───────────────────────────────────────────────
  var state = {
    plugins: [],          // full list from getPlugins
    activePlugin: [null, null],  // {id, name, isStereo, hint} for ch 0 and ch 1
    params: [null, null], // parameter trees for ch 0 and ch 1
    presets: [null, null], // preset lists for ch 0 and ch 1
    activePreset: [-1, -1],
    searchTerm: '',
    stereoLocked: false,      // true when slot A has a stereo plugin
    initialized: false,
    ioCaps: null,             // cached IO capabilities {t: [...], cv: [...]}
  };

  // ─── Plugin List ─────────────────────────────────────────

  /**
   * Categorize plugins by type heuristic using hint.
   * Returns Map<category, plugin[]>
   */
  function categorizePlugins(plugins) {
    var categories = new Map();
    var order = ['OSCILLATORS', 'SYNTH VOICES', 'EFFECTS', 'INSTRUMENTS', 'NOISE & GENERATORS', 'UTILITY'];

    // Keyword-based categorization from hint field
    function classify(p) {
      var hint = (p.hint || '').toLowerCase();
      var id = (p.id || '').toLowerCase();
      var name = (p.name || '').toLowerCase();

      // Utility
      if (id === 'void' || id === 'simplevca') return 'UTILITY';

      // Noise & Generators
      if (/noise|dust|bytebeat/.test(hint) || /pnoise|dust|msx|bbeats/.test(id)) return 'NOISE & GENERATORS';

      // Instruments (drums, samplers, sequencers)
      if (/drum|sample|rompl|sequen|hihat|plays back/.test(hint) || /drumrack|hihat|rompler|recnplay|picoseq/.test(id)) return 'INSTRUMENTS';

      // Effects (reverb, delay, chorus, filter, etc.)
      if (/reverb|delay|chorus|filter|diffus|ensemble|pitch\s*shift|bit\s*crush|strip|trim|effect|feedback|space|antique/.test(hint) ||
          /verb|delay|chorus|filt|strip|trim|crush|diffu|ensemble|pshft|spacefx|antique|retroact/.test(id)) return 'EFFECTS';

      // Oscillators
      if (/wavetable|oscillat|braids|sine\s*gen|svf/.test(hint) || /wtosc|macosc|sinecr|tbd03|misvf/.test(id)) return 'OSCILLATORS';

      // Default: Synth Voices
      return 'SYNTH VOICES';
    }

    // Initialize ordered categories
    order.forEach(function(cat) { categories.set(cat, []); });

    plugins.forEach(function(p) {
      var cat = classify(p);
      if (!categories.has(cat)) categories.set(cat, []);
      categories.get(cat).push(p);
    });

    // Remove empty categories
    categories.forEach(function(list, cat) {
      if (list.length === 0) categories.delete(cat);
    });

    return categories;
  }

  function renderPluginList() {
    var el = document.getElementById('plugin-list');
    var countEl = document.getElementById('plugin-count');
    var term = state.searchTerm.toLowerCase();

    var filtered = state.plugins.filter(function(p) {
      if (!term) return true;
      return p.name.toLowerCase().indexOf(term) !== -1 ||
             p.id.toLowerCase().indexOf(term) !== -1;
    });

    countEl.textContent = filtered.length + ' / ' + state.plugins.length;

    if (filtered.length === 0) {
      el.innerHTML = '<div class="param-empty">No plugins found</div>';
      return;
    }

    var categories = categorizePlugins(filtered);
    var html = '';

    categories.forEach(function(plugins, cat) {
      html += '<div class="plugin-category">' + S.esc(cat) + '</div>';
      plugins.forEach(function(p) {
        var classes = 'plugin-item';
        if (state.activePlugin[0] && state.activePlugin[0].id === p.id) {
          classes += ' active active-a';
        }
        if (state.activePlugin[1] && state.activePlugin[1].id === p.id) {
          classes += ' active active-b';
        }
        var stereo = (p.isStereo === 'true' || p.isStereo === true);
        html += '<div class="' + classes + '" data-plugin-id="' + S.esc(p.id) + '">';
        html += '<span class="plugin-item-name">' + S.esc(p.name) + '</span>';
        if (stereo) {
          html += '<span class="stereo-badge">Stereo</span>';
        } else {
          html += '<span class="mono-badge">Mono</span>';
        }
        html += '</div>';
      });
    });

    el.innerHTML = html;
  }

  function setupPluginListEvents() {
    var list = document.getElementById('plugin-list');
    var search = document.getElementById('plugin-search');

    // Search filtering
    if (search) {
      search.addEventListener('sl-input', function() {
        state.searchTerm = search.value || '';
        renderPluginList();
      });
      search.addEventListener('sl-clear', function() {
        state.searchTerm = '';
        renderPluginList();
      });
    }

    // Plugin click — load into slot
    list.addEventListener('click', function(e) {
      var item = e.target.closest('.plugin-item');
      if (!item) return;
      var pluginId = item.getAttribute('data-plugin-id');
      if (!pluginId) return;

      var pluginInfo = state.plugins.find(function(p) { return p.id === pluginId; });
      var isStereo = pluginInfo && (pluginInfo.isStereo === 'true' || pluginInfo.isStereo === true);

      // Stereo plugins always go to slot A (they occupy both)
      if (isStereo) {
        setActivePlugin(0, pluginId);
        return;
      }

      // If stereo locked, mono plugins can only go to slot A (replaces stereo)
      if (state.stereoLocked) {
        setActivePlugin(0, pluginId);
        return;
      }

      // Determine target slot for mono plugins
      if (e.shiftKey) {
        // Shift+click always targets slot B
        setActivePlugin(1, pluginId);
      } else if (!state.activePlugin[0] || !state.activePlugin[0].id || state.activePlugin[0].id === 'Void') {
        // Slot A is empty — use it
        setActivePlugin(0, pluginId);
      } else if (!state.activePlugin[1] || !state.activePlugin[1].id || state.activePlugin[1].id === 'Void') {
        // Slot A occupied, Slot B is empty — use B
        setActivePlugin(1, pluginId);
      } else {
        // Both slots occupied — show dialog
        showSlotSelectDialog(pluginId);
      }
    });
  }

  // ─── Slot Selection Dialog ────────────────────────────────

  var pendingPluginId = null;

  function showSlotSelectDialog(pluginId) {
    pendingPluginId = pluginId;
    var pluginInfo = state.plugins.find(function(p) { return p.id === pluginId; });
    var nameEl = document.getElementById('slot-select-plugin-name');
    if (nameEl) nameEl.textContent = pluginInfo ? pluginInfo.name : pluginId;
    // Disable Slot B when stereo locked
    var btnB = document.getElementById('slot-select-b');
    if (btnB) btnB.disabled = state.stereoLocked;
    document.getElementById('slot-select-dialog').show();
  }

  function setupSlotSelectDialog() {
    var btnA = document.getElementById('slot-select-a');
    var btnB = document.getElementById('slot-select-b');
    if (btnA) {
      btnA.addEventListener('click', function() {
        document.getElementById('slot-select-dialog').hide();
        if (pendingPluginId) setActivePlugin(0, pendingPluginId);
        pendingPluginId = null;
      });
    }
    if (btnB) {
      btnB.addEventListener('click', function() {
        document.getElementById('slot-select-dialog').hide();
        if (pendingPluginId) setActivePlugin(1, pendingPluginId);
        pendingPluginId = null;
      });
    }
  }

  // ─── Active Plugin Management ────────────────────────────

  var _switching = false;  // mutex — prevents re-entry during plugin switch

  // WP-F: Delays removed.  The real crash cause was Connection:close on
  // API responses (fixed in RestServer.cpp).  Without that header, the
  // browser reuses the same TCP connection (HTTP/1.1 keep-alive), so
  // sequential API calls use only 1 socket.  No artificial delays needed.

  // Plugins that use ctagSampleRom (loads ALL wavetable/sample bank data
  // from SD card into PSRAM on first use).  The firmware's setActivePlugin
  // handler BLOCKS the HTTP response until this completes — 15-30+ seconds.
  var _heavyPlugins = ['WTOsc', 'WTOscDuo', 'Freakwaves', 'VctrSnt'];

  function _isHeavyPlugin(pluginId) {
    return _heavyPlugins.indexOf(pluginId) !== -1;
  }

  async function setActivePlugin(ch, pluginId) {
    if (_switching) {
      console.warn('Plugin switch already in progress — ignoring click');
      return;
    }
    _switching = true;
    var heavy = _isHeavyPlugin(pluginId);
    S.showLoading(heavy
      ? 'Loading plugin — reading wavetable data from SD card, this may take up to 30 seconds…'
      : 'Switching plugin…');
    try {
      var pluginInfo = state.plugins.find(function(p) { return p.id === pluginId; });
      var isStereo = pluginInfo && (pluginInfo.isStereo === 'true' || pluginInfo.isStereo === true);

      // 1. Tell firmware to switch plugin (heavy: allocates plugin memory on RT core)
      // Use the longer plugin-switch timeout for heavy plugins.
      // Skip circuit breaker for this call — a timeout here means the device
      // is busy loading sample ROM from SD card, NOT that it is offline.
      var switchTimeout = heavy ? S.API_PLUGIN_SWITCH_TIMEOUT_MS : S.API_MUTATION_TIMEOUT_MS;
      try {
        await S.queuedPost(
          '/plugins?action=setActive&ch=' + ch + '&id=' + encodeURIComponent(pluginId),
          null, switchTimeout,
          true  /* skipCircuitBreaker */
        );
      } catch (firstErr) {
        // Retry once on timeout — the device may still be loading sample ROM.
        // Wait a moment then try again with a fresh timeout window.
        if (firstErr.name === 'TimeoutError' && heavy) {
          console.warn('Plugin switch timed out (' + (switchTimeout/1000) + 's) — retrying once…');
          S.showLoading('Still loading wavetable data — retrying…');
          await new Promise(function(r) { setTimeout(r, 3000); });
          await S.queuedPost(
            '/plugins?action=setActive&ch=' + ch + '&id=' + encodeURIComponent(pluginId),
            null, switchTimeout,
            true  /* skipCircuitBreaker */
          );
        } else {
          throw firstErr;
        }
      }

      // 2. Fetch the new plugin's params + presets (sequential through queue)
      await loadSlotData(ch, pluginId);

      // If loading a stereo plugin into slot A, clear slot B
      if (ch === 0 && isStereo) {
        await S.queuedPost('/plugins?action=setActive&ch=1&id=Void', null, S.API_MUTATION_TIMEOUT_MS);
        state.activePlugin[1] = null;
        state.params[1] = null;
        state.presets[1] = [];
        state.activePreset[1] = -1;
      }

      renderPluginList(); // update active indicators
    } catch (e) {
      console.error('Failed to set plugin on slot ' + (ch === 0 ? 'A' : 'B'), e);
      if (e.name === 'TimeoutError') {
        S.toast('Plugin switch timed out — the device may still be loading. Try again in a moment.', 'warning');
      } else {
        S.toast('Plugin switch failed — device may be busy or offline', 'danger');
      }
    } finally {
      S.hideLoading();
      _switching = false;
    }
  }

  async function clearSlot(ch) {
    S.showLoading('Clearing slot…');
    try {
      await S.queuedPost('/plugins?action=setActive&ch=' + ch + '&id=Void', null, S.API_MUTATION_TIMEOUT_MS);
      state.activePlugin[ch] = null;
      state.params[ch] = null;
      state.presets[ch] = [];
      state.activePreset[ch] = -1;
      renderSlotHeader(ch);
      renderEmptyCTA(ch);
      updateStereoMode();
      updateSlotVisibility();
      var presetSelect = document.getElementById('preset-select-' + ch);
      if (presetSelect) presetSelect.innerHTML = '';
      var presetList = document.getElementById('preset-list-' + ch);
      if (presetList) presetList.innerHTML = '<div class="param-empty" style="font-size:0.78rem;">No presets</div>';
      renderPluginList();
    } catch (e) {
      console.error('Failed to clear slot ' + (ch === 0 ? 'A' : 'B'), e);
      S.toast('Failed to clear slot \u2014 device may be busy or offline', 'danger');
    } finally {
      S.hideLoading();
    }
  }

  /**
   * Load slot data from the device.
   * @param {number} ch - channel (0 or 1)
   * @param {string} [knownPluginId] - if we already know the plugin ID
   *   (e.g. because we just called setActivePlugin), skip the redundant
   *   getActivePlugin round-trip.  This saves 1 API call per switch —
   *   critical on ESP32 with limited sockets and a busy RT core.
   */
  async function loadSlotData(ch, knownPluginId) {
    try {
      // Sequential fetches through global queue — ESP32 httpd has only
      // 7 sockets (4 usable). Queue ensures max 1 in-flight at a time.
      //
      // If we already know the plugin ID (caller just set it), skip the
      // redundant getActivePlugin call.  The legacy Onsen UI never made
      // this call at all — it used page navigation to separate concerns.
      var activeId;
      if (knownPluginId) {
        activeId = knownPluginId;
      } else {
        var activeData = await S.queuedFetch('/plugins?action=getActive&ch=' + ch);
        activeId = activeData.id;
      }
      var paramsData = await S.queuedFetch('/plugins?action=getParams&ch=' + ch);
      var presetsData = await S.queuedFetch('/plugins?action=getPresets&ch=' + ch);

      // Find full plugin info
      var pluginInfo = state.plugins.find(function(p) { return p.id === activeId; });
      state.activePlugin[ch] = {
        id: activeId,
        name: pluginInfo ? pluginInfo.name : activeId,
        isStereo: pluginInfo ? (pluginInfo.isStereo === 'true' || pluginInfo.isStereo === true) : false,
        hint: pluginInfo ? (pluginInfo.hint || '') : '',
      };

      state.params[ch] = paramsData;
      state.presets[ch] = presetsData.presets || [];
      state.activePreset[ch] = presetsData.activePresetNumber !== undefined
        ? presetsData.activePresetNumber : -1;

      renderSlotHeader(ch);
      renderPluginInfoBar(ch);
      renderParams(ch);
      renderPresets(ch);
      updateStereoMode();
      updateSlotVisibility();
    } catch (e) {
      console.error('Failed to load slot ' + ch + ' data:', e);
    }
  }

  // ─── Slot Header Rendering ──────────────────────────────

  function renderSlotHeader(ch) {
    var nameEl = document.getElementById('slot-name-' + ch);
    var presetPluginEl = document.getElementById('preset-plugin-' + ch);
    var removeBtn = document.getElementById('remove-plugin-' + ch);
    var subheader = document.getElementById('slot-subheader-' + ch);

    var hasPlugin = state.activePlugin[ch] && state.activePlugin[ch].id && state.activePlugin[ch].id !== 'Void';

    if (hasPlugin) {
      nameEl.textContent = state.activePlugin[ch].name;
      if (presetPluginEl) presetPluginEl.textContent = state.activePlugin[ch].name;
      if (removeBtn) removeBtn.style.display = '';
      if (subheader) subheader.style.display = '';
    } else {
      nameEl.textContent = '—';
      if (presetPluginEl) presetPluginEl.textContent = '—';
      if (removeBtn) removeBtn.style.display = 'none';
      if (subheader) subheader.style.display = 'none';
    }
  }

  // ─── Plugin Info Bar ─────────────────────────────────────

  function renderPluginInfoBar(ch) {
    var container = document.getElementById('plugin-info-' + ch);
    if (!container) return;

    var plugin = state.activePlugin[ch];
    if (!plugin || !plugin.id || plugin.id === 'Void') {
      container.innerHTML = '';
      return;
    }

    var info = state.plugins.find(function(p) { return p.id === plugin.id; });
    if (!info) {
      container.innerHTML = '';
      return;
    }

    var isStereo = (info.isStereo === 'true' || info.isStereo === true);
    var category = classifyPlugin(info);

    var html = '<div class="plugin-info-bar">';
    html += '<div class="info-badges">';
    html += '<span class="info-badge ' + (isStereo ? 'badge-stereo' : 'badge-mono') + '">' +
      (isStereo ? 'Stereo' : 'Mono') + '</span>';
    html += '<span class="info-badge badge-category">' + S.esc(category) + '</span>';
    html += '</div>';
    if (info.hint) {
      html += '<div class="info-description">' + S.esc(info.hint) + '</div>';
    }
    html += '</div>';
    container.innerHTML = html;
  }

  function classifyPlugin(p) {
    var hint = (p.hint || '').toLowerCase();
    var id = (p.id || '').toLowerCase();
    if (id === 'void' || id === 'simplevca') return 'Utility';
    if (/noise|dust|bytebeat/.test(hint) || /pnoise|dust|msx|bbeats/.test(id)) return 'Noise/Gen';
    if (/drum|sample|rompl|sequen|hihat|plays back/.test(hint) || /drumrack|hihat|rompler|recnplay|picoseq/.test(id)) return 'Instrument';
    if (/reverb|delay|chorus|filter|diffus|ensemble|pitch\s*shift|bit\s*crush|strip|trim|effect|feedback|space|antique/.test(hint) ||
        /verb|delay|chorus|filt|strip|trim|crush|diffu|ensemble|pshft|spacefx|antique|retroact/.test(id)) return 'Effect';
    if (/wavetable|oscillat|braids|sine\s*gen|svf/.test(hint) || /wtosc|macosc|sinecr|tbd03|misvf/.test(id)) return 'Oscillator';
    return 'Synth Voice';
  }

  // ─── Empty Slot CTA ──────────────────────────────────────

  function renderEmptyCTA(ch) {
    var container = document.getElementById('params-' + ch);
    if (!container) return;

    container.innerHTML = '<div class="plugin-info-container" id="plugin-info-' + ch + '"></div>' +
      '<div class="slot-empty-cta">' +
      '<div class="cta-icon"><sl-icon name="plus-lg"></sl-icon></div>' +
      '<h3>No Plugin Loaded</h3>' +
      '<p style="color:var(--sl-color-neutral-500);font-size:0.82rem;">Select a plugin from the sidebar to get started</p>' +
      '<button class="cta-btn" data-ch="' + ch + '">Add Plugin</button>' +
      '</div>';

    // Wire CTA button — open plugin sidebar if collapsed
    container.querySelector('.cta-btn').addEventListener('click', function() {
      // If stereo locked and this is Slot B, do nothing
      if (ch === 1 && state.stereoLocked) return;

      var sidebar = document.querySelector('.plugin-sidebar');
      if (sidebar && sidebar.classList.contains('collapsed')) {
        sidebar.classList.remove('collapsed');
        var icon = document.querySelector('#plugin-sidebar-toggle sl-icon');
        if (icon) icon.name = 'chevron-left';
        localStorage.setItem('tbd-plugin-sidebar-collapsed', '0');
      }
      var search = document.getElementById('plugin-search');
      if (search) search.focus();

      // Flash the slot panel to hint which slot will receive the plugin
      var panel = document.getElementById('slot-panel-' + ch);
      if (panel) {
        panel.style.outline = '2px solid var(--sl-color-primary-500)';
        panel.style.outlineOffset = '-2px';
        setTimeout(function() {
          panel.style.outline = '';
          panel.style.outlineOffset = '';
        }, 2000);
      }
    });
  }

  // ─── Stereo Mode ─────────────────────────────────────────

  function updateStereoMode() {
    var slotA = state.activePlugin[0];
    var panelA = document.getElementById('slot-panel-0');
    var panelB = document.getElementById('slot-panel-1');
    var swapBtn = document.getElementById('swap-slots-btn');

    if (slotA && slotA.isStereo) {
      state.stereoLocked = true;

      // Update slot label to A+B
      var slotLabel = panelA ? panelA.querySelector('.slot-label') : null;
      if (slotLabel) {
        slotLabel.textContent = 'A+B';
        slotLabel.classList.remove('slot-label-a');
        slotLabel.classList.add('slot-label-ab');
      }

      // Slot A gets full width
      if (panelA) panelA.classList.add('stereo-active');

      // Slot B is hidden entirely via CSS (display: none)
      if (panelB) panelB.classList.add('stereo-locked');

      // Also hide swap button
      if (swapBtn) swapBtn.classList.add('stereo-hidden');
    } else {
      state.stereoLocked = false;

      // Restore slot label to A
      var slotLabel = panelA ? panelA.querySelector('.slot-label') : null;
      if (slotLabel) {
        slotLabel.textContent = 'A';
        slotLabel.classList.remove('slot-label-ab');
        slotLabel.classList.add('slot-label-a');
      }

      // Restore normal layout
      if (panelA) panelA.classList.remove('stereo-active');

      if (panelB) {
        panelB.classList.remove('stereo-locked');

        // If Slot B is empty after unlocking, show the CTA
        var hasPluginB = state.activePlugin[1] && state.activePlugin[1].id && state.activePlugin[1].id !== 'Void';
        if (!hasPluginB) {
          renderSlotHeader(1);
          renderEmptyCTA(1);
        }
      }
      if (swapBtn) swapBtn.classList.remove('stereo-hidden');
    }
  }

  // ─── Slot Visibility (show/hide empty state) ──────────

  function updateSlotVisibility() {
    var hasA = state.activePlugin[0] && state.activePlugin[0].id && state.activePlugin[0].id !== 'Void';
    var hasB = state.activePlugin[1] && state.activePlugin[1].id && state.activePlugin[1].id !== 'Void';

    var panelA = document.getElementById('slot-panel-0');
    var panelB = document.getElementById('slot-panel-1');
    var swapBtn = document.getElementById('swap-slots-btn');

    // If neither slot has a plugin, show single unified CTA
    if (!hasA && !hasB && !state.stereoLocked) {
      // Hide slot B and swap entirely
      if (panelB) panelB.style.display = 'none';
      if (swapBtn) swapBtn.style.display = 'none';

      // Hide Slot A title bar and subheader, show full CTA
      if (panelA) {
        var titleBarA = panelA.querySelector('.slot-title-bar');
        var subheaderA = document.getElementById('slot-subheader-0');
        if (titleBarA) titleBarA.style.display = 'none';
        if (subheaderA) subheaderA.style.display = 'none';
      }
    } else {
      // Restore individual visibility
      if (!state.stereoLocked) {
        if (panelB) panelB.style.display = '';
        if (swapBtn) swapBtn.style.display = '';
      }

      for (var ch = 0; ch < 2; ch++) {
        var hasPlugin = ch === 0 ? hasA : hasB;
        var removeBtn = document.getElementById('remove-plugin-' + ch);
        var subheader = document.getElementById('slot-subheader-' + ch);
        var panel = document.getElementById('slot-panel-' + ch);
        var titleBar = panel ? panel.querySelector('.slot-title-bar') : null;

        if (titleBar) titleBar.style.display = '';
        if (hasPlugin) {
          if (removeBtn) removeBtn.style.display = '';
          if (subheader) subheader.style.display = '';
        } else {
          if (removeBtn) removeBtn.style.display = 'none';
          if (subheader) subheader.style.display = 'none';
        }
      }
    }
  }

  // ─── Parameter Rendering ─────────────────────────────────

  function renderParams(ch) {
    var container = document.getElementById('params-' + ch);
    var data = state.params[ch];

    // Preserve plugin-info-container — only replace param content
    var infoContainer = container.querySelector('.plugin-info-container');
    var infoHtml = infoContainer ? infoContainer.outerHTML : '<div class="plugin-info-container" id="plugin-info-' + ch + '"></div>';

    if (!data || !data.params || data.params.length === 0) {
      container.innerHTML = infoHtml + '<div class="param-empty">No parameters</div>';
      return;
    }

    // Wrap consecutive ungrouped params into synthetic groups
    // so the stereo two-column flex layout works for all plugins
    var normalized = [];
    var ungrouped = [];

    function flushUngrouped() {
      if (ungrouped.length > 0) {
        normalized.push({
          type: 'group',
          id: '_ungrouped_' + normalized.length,
          name: 'Parameters',
          params: ungrouped
        });
        ungrouped = [];
      }
    }

    data.params.forEach(function(p) {
      if (p.type === 'group') {
        flushUngrouped();
        normalized.push(p);
      } else {
        ungrouped.push(p);
      }
    });
    flushUngrouped();

    var html = '';
    var channelInfo = detectChannelGroups(normalized);

    if (channelInfo) {
      // Render channel containers
      channelInfo.channels.forEach(function(channel) {
        html += renderChannelContainer(ch, channel);
      });
      // Wrap remaining groups (FX/Master) in master bus container
      if (channelInfo.others.length > 0) {
        html += '<div class="master-bus-container">';
        html += '<div class="master-bus-header">';
        html += '<sl-icon name="sliders"></sl-icon>';
        html += '<span>Master Bus</span>';
        html += '</div>';
        html += '<div class="master-bus-body">';
        channelInfo.others.forEach(function(g) {
          html += renderParamGroup(ch, g);
        });
        html += '</div>';
        html += '</div>';
      }
    } else {
      // Normal rendering — no channel pattern detected
      normalized.forEach(function(p) {
        html += renderParamGroup(ch, p);
      });
    }

    container.innerHTML = infoHtml + html;
    setupParamEvents(ch, container);
    if (channelInfo) setupChannelEvents(ch, container);
  }

  // ─── Channel Group Detection & Rendering ────────────────

  /**
   * Detect if normalized groups form a channel-based layout.
   * Looks for group IDs matching ch{N}_ pattern.
   * Returns { channels: [...], others: [...] } or null.
   */
  function detectChannelGroups(normalizedGroups) {
    var channelMap = {};
    var channelOrder = [];
    var others = [];

    normalizedGroups.forEach(function(g) {
      var m = g.id.match(/^ch(\d+)_/);
      if (m) {
        var num = parseInt(m[1], 10);
        if (!channelMap[num]) {
          channelMap[num] = { num: num, mixer: null, machines: [] };
          channelOrder.push(num);
        }
        if (g.id === 'ch' + num + '_group') {
          channelMap[num].mixer = g;
        } else {
          // Extract machine index and short name from group name
          var machMatch = g.name.match(/Machine\s+(\d+)\s*-\s*(.*)/);
          var idx, shortName;
          if (machMatch) {
            idx = parseInt(machMatch[1], 10);
            shortName = machMatch[2].trim();
          } else {
            idx = channelMap[num].machines.length;
            // Remove "Channel N - " prefix if present
            shortName = g.name.replace(/^Channel\s+\d+\s*-\s*/, '');
          }
          channelMap[num].machines.push({ group: g, idx: idx, name: shortName });
        }
      } else {
        others.push(g);
      }
    });

    if (channelOrder.length === 0) return null;

    var channels = channelOrder.map(function(num) { return channelMap[num]; });
    return { channels: channels, others: others };
  }

  /**
   * Render a channel container: engine tabs → machine params → mixer strip.
   * Layout follows audio console convention: primary controls (engine) on top,
   * mixer strip (level/pan/sends) at bottom.
   */
  function renderChannelContainer(ch, channel) {
    var mixer = channel.mixer;
    var machines = channel.machines;

    // Extract channel display name from mixer group name
    // "Channel 1 - Drum group - Kicks" → name:"Kicks", category:"Drum group"
    var parts = mixer ? mixer.name.split(' - ') : [];
    var channelName = parts.length >= 3 ? parts.slice(2).join(' - ') :
                      parts.length >= 2 ? parts[1] : 'Channel ' + channel.num;
    var categoryName = parts.length >= 3 ? parts[1] : '';
    var displayNum = String(channel.num).padStart(2, '0');

    var html = '<div class="channel-container" data-channel="' + channel.num + '">';

    // Find Mute param in mixer for header rendering
    var muteParam = null;
    if (mixer && mixer.params) {
      for (var mi = 0; mi < mixer.params.length; mi++) {
        if (mixer.params[mi].name === 'Mute') {
          muteParam = mixer.params[mi];
          break;
        }
      }
    }

    // Channel header
    html += '<div class="channel-header">';
    html += '<sl-icon name="chevron-down" class="channel-chevron"></sl-icon>';
    html += '<span class="channel-num">CH ' + displayNum + '</span>';
    html += '<span class="channel-name">' + S.esc(channelName) + '</span>';
    if (categoryName) {
      html += '<span class="channel-category">' + S.esc(categoryName) + '</span>';
    }
    if (muteParam) {
      var muteChecked = muteParam.current ? ' checked' : '';
      html += '<sl-switch size="small" class="channel-mute-switch"' + muteChecked + ' ';
      html += 'data-param-id="' + S.esc(muteParam.id) + '" data-ch="' + ch + '" ';
      html += 'title="Mute">';
      html += '</sl-switch>';
    }
    html += '</div>';

    // Channel body
    html += '<div class="channel-body">';

    // Show engine tabs if channel has multiple machines
    var hasSelector = machines.length > 1;

    // Find Device param and determine active machine
    var deviceParam = null;
    if (mixer && mixer.params) {
      for (var i = 0; i < mixer.params.length; i++) {
        if (mixer.params[i].name === 'Device') {
          deviceParam = mixer.params[i];
          break;
        }
      }
    }
    var deviceValue = deviceParam ? (deviceParam.current || 0) : 0;
    var activeMachineIdx = 0;
    if (hasSelector && deviceParam) {
      var maxVal = deviceParam.max || 4095;
      activeMachineIdx = deviceValue > (maxVal / 2) ? 1 : 0;
    }

    // ── Engine tab bar (only if multiple machines)
    if (hasSelector) {
      html += '<div class="machine-tab-bar" data-channel="' + channel.num + '">';
      machines.forEach(function(m, tabIdx) {
        var isActive = tabIdx === activeMachineIdx;
        html += '<button class="machine-tab' + (isActive ? ' active' : '') + '" ';
        html += 'data-machine-idx="' + tabIdx + '" data-channel="' + channel.num + '">';
        html += S.esc(m.name);
        html += '</button>';
      });
      html += '</div>';
    }

    // ── Machine param panels (render params directly, no nested group chrome)
    machines.forEach(function(m, tabIdx) {
      var isVisible = hasSelector ? (tabIdx === activeMachineIdx) : true;
      var machineGridCls = S.isControlMode() ? ' knob-grid' : '';
      html += '<div class="machine-params' + machineGridCls + '" data-machine-idx="' + tabIdx + '" data-channel="' + channel.num + '"';
      if (!isVisible) html += ' style="display:none;"';
      html += '>';
      // Render params directly — no collapsible group wrapper
      if (m.group.params) {
        var machParamIdx = 0;
        m.group.params.forEach(function(p) {
          html += renderParamRow(ch, p);
          machParamIdx++;
          if (S.isControlMode() && machParamIdx % 4 === 0) {
            html += '<div class="knob-row-divider"></div>';
          }
        });
      }
      html += '</div>';
    });

    // ── Mixer strip (bottom — level, pan, sends)
    if (mixer && mixer.params) {
      html += '<div class="channel-mixer">';
      html += '<div class="channel-mixer-label">';
      html += '<sl-icon name="sliders"></sl-icon>';
      html += '<span>Track Mix</span>';
      html += '<sl-icon name="chevron-down" class="mixer-chevron"></sl-icon>';
      html += '</div>';
      var mixerGridCls = S.isControlMode() ? ' knob-grid' : '';
      html += '<div class="channel-mixer-body' + mixerGridCls + '">';
      var mixerParamIdx = 0;
      mixer.params.forEach(function(p) {
        // Skip Device param — it's an engine selector, not a mixer control
        if (p.name === 'Device') return;
        // Skip Mute — moved to channel header
        if (p.name === 'Mute') return;
        html += renderParamRow(ch, p);
        mixerParamIdx++;
        if (S.isControlMode() && mixerParamIdx % 4 === 0) {
          html += '<div class="knob-row-divider"></div>';
        }
      });
      html += '</div>';
      html += '</div>';
    }

    html += '</div>'; // .channel-body
    html += '</div>'; // .channel-container
    return html;
  }

  /**
   * Set up event handlers for channel containers:
   * - Channel header collapse/expand
   * - Machine tab switching
   */
  function setupChannelEvents(ch, container) {
    // Channel collapse/expand
    container.querySelectorAll('.channel-header').forEach(function(header) {
      header.addEventListener('click', function(e) {
        // Don't toggle collapse when clicking the mute switch
        if (e.target.closest('.channel-mute-switch')) return;
        header.parentElement.classList.toggle('collapsed');
      });
    });

    // Channel mute switches in header
    container.querySelectorAll('.channel-mute-switch').forEach(function(sw) {
      sw.addEventListener('sl-change', function(e) {
        e.stopPropagation();
        var paramId = sw.getAttribute('data-param-id');
        var slotCh = parseInt(sw.getAttribute('data-ch'), 10);
        sendParamValue(slotCh, paramId, sw.checked ? 1 : 0);
      });
    });

    // Mixer collapse/expand
    container.querySelectorAll('.channel-mixer-label').forEach(function(label) {
      label.addEventListener('click', function() {
        label.parentElement.classList.toggle('mixer-collapsed');
      });
    });

    // Machine tab switching
    container.querySelectorAll('.machine-tab').forEach(function(tab) {
      tab.addEventListener('click', function(e) {
        e.stopPropagation();
        var channelNum = tab.getAttribute('data-channel');
        var tabIdx = parseInt(tab.getAttribute('data-machine-idx'), 10);
        var channelEl = tab.closest('.channel-container');

        // Update active tab
        channelEl.querySelectorAll('.machine-tab').forEach(function(t) {
          t.classList.toggle('active', t === tab);
        });

        // Show/hide machine params
        channelEl.querySelectorAll('.machine-params').forEach(function(mp) {
          var idx = parseInt(mp.getAttribute('data-machine-idx'), 10);
          mp.style.display = (idx === tabIdx) ? '' : 'none';
        });

        // Send Device param value
        var deviceParamId = 'ch' + channelNum + '_device';
        // Map tab index to Device value: 0 → 0, 1 → max (4095)
        var deviceValue = tabIdx === 0 ? 0 : 4095;
        sendParamValue(ch, deviceParamId, deviceValue);
      });
    });
  }

  // ─── Param Group & Row Rendering ────────────────────────

  function renderParamGroup(ch, group, nested) {
    var controlMode = S.isControlMode();
    var cls = nested ? 'param-group param-group-nested' : 'param-group';
    var html = '<div class="' + cls + '" data-group-id="' + S.esc(group.id) + '">';
    html += '<div class="param-group-header">';
    html += '<sl-icon name="chevron-down" class="param-group-chevron"></sl-icon>';
    html += '<span class="param-group-name" title="' + S.esc(group.name) + '">' + S.esc(group.name) + '</span>';
    html += '</div>';
    html += '<div class="param-group-body' + (controlMode ? ' knob-grid' : '') + '">';

    if (group.params) {
      var groupParamIdx = 0;
      group.params.forEach(function(p) {
        if (p.type === 'group') {
          // Recursively render nested groups
          html += renderParamGroup(ch, p, true);
        } else {
          html += renderParamRow(ch, p);
          groupParamIdx++;
          if (controlMode && groupParamIdx % 4 === 0) {
            html += '<div class="knob-row-divider"></div>';
          }
        }
      });
    }

    html += '</div>';
    html += '</div>';
    return html;
  }

  function renderParamRow(ch, param) {
    // If a group somehow ends up here, render it as a nested group
    if (param.type === 'group') {
      return renderParamGroup(ch, param, true);
    }

    var controlMode = S.isControlMode();
    var knobClass = controlMode ? ' knob-mode' : '';

    var html = '<div class="param-row' + knobClass + '" data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '">';
    html += '<span class="param-name" title="' + S.esc(param.name) + '">' + S.esc(param.name) + '</span>';
    html += '<div class="param-control">';

    if (param.type === 'bool') {
      html += renderBoolParam(ch, param);
    } else if (controlMode) {
      html += renderKnobParam(ch, param);
    } else {
      // int or float — Config Mode (raw sliders)
      html += renderSliderParam(ch, param);
    }

    html += '</div>';

    // CV/TRIG routing dropdown (Config Mode only — not shown in Control Mode knob grid)
    if (!controlMode && state.ioCaps) {
      html += renderCVTrigDropdown(ch, param);
    }

    html += '</div>';

    return html;
  }

  function renderSliderParam(ch, param) {
    var min = param.min !== undefined ? param.min : 0;
    var max = param.max !== undefined ? param.max : 4095;
    var step = param.step !== undefined ? param.step : 1;
    var current = param.current !== undefined ? param.current : min;

    var html = '<input type="range" class="param-slider" ';
    html += 'min="' + min + '" max="' + max + '" step="' + step + '" ';
    html += 'value="' + current + '" ';
    html += 'data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '">';
    html += '<span class="param-value">' + current + '</span>';
    return html;
  }

  function renderBoolParam(ch, param) {
    var checked = param.current ? ' checked' : '';
    var html = '<div class="param-bool">';
    html += '<sl-switch size="small"' + checked + ' ';
    html += 'data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '">';
    html += '</sl-switch>';
    html += '</div>';
    return html;
  }

  function renderKnobParam(ch, param) {
    var DH = window.TBD.displayHints;
    var min = param.min !== undefined ? param.min : 0;
    var max = param.max !== undefined ? param.max : 4095;
    var current = param.current !== undefined ? param.current : min;

    var hint = DH ? DH.resolveHint(param.id, param.name, param) : null;

    if (hint) {
      // Control Mode: knob operates in physical units
      var physMin = hint.physMin;
      var physMax = hint.physMax;
      var step = DH.computeStep(hint);
      var displayVal = DH.rawToDisplay(current, min, max, hint);
      var formatted = DH.formatDisplayValue(displayVal, hint);

      var html = '<webaudio-knob class="param-knob" ';
      html += 'diameter="52" ';
      html += 'min="' + physMin + '" max="' + physMax + '" ';
      html += 'step="' + step + '" ';
      html += 'value="' + displayVal.toFixed(4) + '" ';
      html += 'colors="#ccc;#484848;#525252" ';
      html += 'valuetip="0" sensitivity="0.5" ';
      html += 'data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '" ';
      html += 'data-raw-min="' + min + '" data-raw-max="' + max + '" ';
      html += 'data-hint="1">';
      html += '</webaudio-knob>';
      html += '<span class="param-value">' + S.esc(formatted) + '</span>';
      return html;
    }

    // No hint: render a knob with raw values
    var html = '<webaudio-knob class="param-knob" ';
    html += 'diameter="52" ';
    html += 'min="' + min + '" max="' + max + '" ';
    html += 'step="1" ';
    html += 'value="' + current + '" ';
    html += 'colors="#ccc;#484848;#525252" ';
    html += 'valuetip="0" sensitivity="0.5" ';
    html += 'data-param-id="' + S.esc(param.id) + '" data-ch="' + ch + '" ';
    html += 'data-raw-min="' + min + '" data-raw-max="' + max + '">';
    html += '</webaudio-knob>';
    html += '<span class="param-value">' + current + '</span>';
    return html;
  }

  // ─── CV / TRIG Routing Dropdown ─────────────────────────

  /**
   * Render a compact CV or TRIG assignment dropdown for a parameter.
   * - int params get a CV dropdown (sources from ioCaps.cv)
   * - bool params get a TRIG dropdown (sources from ioCaps.t)
   */
  function renderCVTrigDropdown(ch, param) {
    if (!state.ioCaps) return '';

    var isBool = param.type === 'bool';
    var sources = isBool ? (state.ioCaps.t || []) : (state.ioCaps.cv || []);
    var currentIdx = isBool ? (param.trig !== undefined ? param.trig : -1)
                            : (param.cv !== undefined ? param.cv : -1);
    var label = isBool ? 'TRIG' : 'CV';
    var dataAttr = isBool ? 'data-trig-param' : 'data-cv-param';

    var html = '<div class="param-routing">';
    var assignedClass = currentIdx >= 0 ? ' assigned' : '';
    html += '<select class="param-routing-select' + assignedClass + '" ' + dataAttr + '="' + S.esc(param.id) + '" data-ch="' + ch + '" title="' + label + ' routing for ' + S.esc(param.name) + '">';
    html += '<option value="-1"' + (currentIdx === -1 ? ' selected' : '') + '>—</option>';
    for (var i = 0; i < sources.length; i++) {
      var selected = (i === currentIdx) ? ' selected' : '';
      html += '<option value="' + i + '"' + selected + '>' + S.esc(sources[i]) + '</option>';
    }
    html += '</select>';
    html += '</div>';
    return html;
  }

  // ─── Parameter Events ───────────────────────────────────

  function setupParamEvents(ch, container) {
    // Group collapse/expand
    container.querySelectorAll('.param-group-header').forEach(function(header) {
      header.addEventListener('click', function() {
        header.parentElement.classList.toggle('collapsed');
      });
    });

    // Slider input (live update value display)
    container.querySelectorAll('.param-slider').forEach(function(slider) {
      var valueEl = slider.nextElementSibling;

      slider.addEventListener('input', function() {
        if (valueEl) valueEl.textContent = slider.value;
      });

      // Send on change (release or final value)
      slider.addEventListener('change', function() {
        var paramId = slider.getAttribute('data-param-id');
        var slotCh = parseInt(slider.getAttribute('data-ch'), 10);
        sendParamValue(slotCh, paramId, slider.value);
      });
    });

    // Boolean switches
    container.querySelectorAll('sl-switch[data-param-id]').forEach(function(sw) {
      sw.addEventListener('sl-change', function() {
        var paramId = sw.getAttribute('data-param-id');
        var slotCh = parseInt(sw.getAttribute('data-ch'), 10);
        sendParamValue(slotCh, paramId, sw.checked ? 1 : 0);
      });
    });

    // webaudio-knob elements (Control Mode)
    container.querySelectorAll('webaudio-knob[data-param-id]').forEach(function(knob) {
      var valueEl = knob.nextElementSibling;
      var DH = window.TBD.displayHints;
      var hasHint = knob.getAttribute('data-hint') === '1';
      var rawMin = parseInt(knob.getAttribute('data-raw-min'), 10) || 0;
      var rawMax = parseInt(knob.getAttribute('data-raw-max'), 10) || 4095;
      var paramId = knob.getAttribute('data-param-id');
      var paramName = '';
      var nameEl = knob.closest('.param-row');
      if (nameEl) {
        var nameSpan = nameEl.querySelector('.param-name');
        if (nameSpan) paramName = nameSpan.textContent;
      }
      var hint = hasHint && DH ? DH.resolveHint(paramId, paramName) : null;

      // Suppress hover on other param-rows while dragging this knob
      var paramRow = knob.closest('.param-row');
      var isDragging = false;

      // Live update display value on input
      knob.addEventListener('input', function() {
        // Activate drag suppression on first input
        if (!isDragging) {
          isDragging = true;
          document.body.classList.add('knob-dragging');
          if (paramRow) paramRow.classList.add('knob-active');
        }
        if (!valueEl) return;
        if (hint && DH) {
          valueEl.textContent = DH.formatDisplayValue(parseFloat(knob.value), hint);
        } else {
          valueEl.textContent = Math.round(parseFloat(knob.value));
        }
      });

      // Send raw value on change (release)
      knob.addEventListener('change', function() {
        isDragging = false;
        document.body.classList.remove('knob-dragging');
        if (paramRow) paramRow.classList.remove('knob-active');
        var slotCh = parseInt(knob.getAttribute('data-ch'), 10);
        var rawValue;
        if (hint && DH) {
          rawValue = DH.displayToRaw(parseFloat(knob.value), rawMin, rawMax, hint);
        } else {
          rawValue = Math.round(parseFloat(knob.value));
        }
        sendParamValue(slotCh, paramId, rawValue);
      });
    });

    // CV routing selects
    container.querySelectorAll('select[data-cv-param]').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var paramId = sel.getAttribute('data-cv-param');
        var slotCh = parseInt(sel.getAttribute('data-ch'), 10);
        var val = parseInt(sel.value, 10);
        sel.classList.toggle('assigned', val >= 0);
        sendCVValue(slotCh, paramId, val);
      });
    });

    // TRIG routing selects
    container.querySelectorAll('select[data-trig-param]').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var paramId = sel.getAttribute('data-trig-param');
        var slotCh = parseInt(sel.getAttribute('data-ch'), 10);
        var val = parseInt(sel.value, 10);
        sel.classList.toggle('assigned', val >= 0);
        sendTrigValue(slotCh, paramId, val);
      });
    });

  }

  // ─── Debounced Param Sending ─────────────────────────────
  // Per-param debounce timers: rapid knob/slider changes collapse into
  // a single API call (WLED pattern — 50ms debounce on continuous input).
  var _paramTimers = {};
  var PARAM_DEBOUNCE_MS = 50;

  function sendParamValue(ch, paramId, value) {
    // Notify debug panel
    if (S.onParamChange) S.onParamChange(ch, paramId, value);

    // Debounce: cancel previous pending send for this exact param
    var key = ch + ':' + paramId;
    if (_paramTimers[key]) clearTimeout(_paramTimers[key]);

    _paramTimers[key] = setTimeout(function() {
      delete _paramTimers[key];
      // Route directly through apiQueue — no need for double-serialization
      // through paramQueue→apiQueue.  Single queue is sufficient and
      // reduces overhead on the constrained ESP32 httpd.
      S.queuedPost('/plugins?action=setParam&ch=' + ch + '&id=' +
        encodeURIComponent(paramId) + '&key=current&val=' + encodeURIComponent(value),
        null
      ).catch(function(e) {
        console.error('Failed to set param:', paramId, e);
      });
    }, PARAM_DEBOUNCE_MS);
  }

  function sendCVValue(ch, paramId, value) {
    S.queuedPost('/plugins?action=setParam&ch=' + ch + '&id=' +
      encodeURIComponent(paramId) + '&key=cv&val=' + encodeURIComponent(value),
      null
    ).catch(function(e) {
      console.error('Failed to set CV:', paramId, e);
    });
  }

  function sendTrigValue(ch, paramId, value) {
    S.queuedPost('/plugins?action=setParam&ch=' + ch + '&id=' +
      encodeURIComponent(paramId) + '&key=trig&val=' + encodeURIComponent(value),
      null
    ).catch(function(e) {
      console.error('Failed to set TRIG:', paramId, e);
    });
  }


  // ─── Preset Rendering ───────────────────────────────────

  function renderPresets(ch) {
    // Inline preset select in slot title bar
    var selectEl = document.getElementById('preset-select-' + ch);
    if (selectEl) {
      var html = '';
      if (state.presets[ch] && state.presets[ch].length > 0) {
        state.presets[ch].forEach(function(p) {
          html += '<sl-option value="' + p.number + '">' +
            S.esc(p.name || ('Preset ' + p.number)) + '</sl-option>';
        });
      }
      selectEl.innerHTML = html;
      if (state.activePreset[ch] >= 0) {
        selectEl.value = String(state.activePreset[ch]);
      }
    }

    // Sidebar preset list — removed (using in-slot preset controls only)
  }

  function setupPresetEvents() {
    // Preset selection from slot title selects
    for (var ch = 0; ch < 2; ch++) {
      (function(ch) {
        var sel = document.getElementById('preset-select-' + ch);
        if (sel) {
          sel.addEventListener('sl-change', function() {
            var num = parseInt(sel.value, 10);
            if (!isNaN(num)) loadPreset(ch, num);
          });
        }

        var saveBtn = document.getElementById('save-preset-' + ch);
        if (saveBtn) {
          saveBtn.addEventListener('click', function() {
            openSavePresetDialog(ch);
          });
        }
      })(ch);
    }
  }

  async function loadPreset(ch, number) {
    S.showLoading('Loading preset…');
    try {
      await S.queuedPost('/plugins?action=loadPreset&ch=' + ch + '&number=' + number, null, S.API_MUTATION_TIMEOUT_MS);
      state.activePreset[ch] = number;
      // Reload params to reflect preset values
      var paramsData = await S.queuedFetch('/plugins?action=getParams&ch=' + ch);
      state.params[ch] = paramsData;
      renderParams(ch);
      renderPresets(ch);
    } catch (e) {
      console.error('Failed to load preset on slot ' + (ch === 0 ? 'A' : 'B'), e);
    } finally {
      S.hideLoading();
    }
  }

  // ─── Save Preset Dialog ──────────────────────────────────

  var savePresetCh = 0;

  function openSavePresetDialog(ch) {
    savePresetCh = ch;
    var nameInput = document.getElementById('save-preset-name');
    var slotSelect = document.getElementById('save-preset-slot');

    nameInput.value = '';

    // Populate slot options — dynamic count based on existing presets
    // Show all existing slots + 1 extra slot for creating a new preset
    var maxSlot = 0;
    if (state.presets[ch] && state.presets[ch].length > 0) {
      state.presets[ch].forEach(function(p) {
        if (p.number >= maxSlot) maxSlot = p.number + 1;
      });
    }
    var slotCount = Math.max(maxSlot + 1, 10); // minimum 10 slots, always offer +1
    var html = '';
    for (var i = 0; i < slotCount; i++) {
      var preset = state.presets[ch] ? state.presets[ch].find(function(p) {
        return p.number === i;
      }) : null;
      var label = i + (preset ? ' — ' + S.esc(preset.name) : ' — (empty)');
      var selected = (i === state.activePreset[ch]) ? ' selected' : '';
      html += '<sl-option value="' + i + '">' + label + '</sl-option>';
    }
    slotSelect.innerHTML = html;
    if (state.activePreset[ch] >= 0) {
      slotSelect.value = String(state.activePreset[ch]);
    } else {
      slotSelect.value = '0';
    }

    document.getElementById('save-preset-dialog').show();
  }

  function setupSavePresetDialog() {
    var okBtn = document.getElementById('save-preset-ok');
    var cancelBtn = document.getElementById('save-preset-cancel');

    if (okBtn) {
      okBtn.addEventListener('click', async function() {
        var name = document.getElementById('save-preset-name').value.trim();
        var slot = parseInt(document.getElementById('save-preset-slot').value, 10);
        if (!name) {
          S.toast('Enter a preset name', 'warning');
          return;
        }
        try {
          await S.queuedPost('/plugins?action=savePreset&ch=' + savePresetCh +
            '&number=' + slot + '&name=' + encodeURIComponent(name), null);
          document.getElementById('save-preset-dialog').hide();
          // Reload presets
          var presetsData = await S.queuedFetch('/plugins?action=getPresets&ch=' + savePresetCh);
          state.presets[savePresetCh] = presetsData.presets || [];
          state.activePreset[savePresetCh] = slot;
          renderPresets(savePresetCh);
          S.toast('Preset saved: ' + name, 'success');
        } catch (e) {
          S.toast('Failed to save preset', 'danger');
        }
      });
    }
    if (cancelBtn) {
      cancelBtn.addEventListener('click', function() {
        document.getElementById('save-preset-dialog').hide();
      });
    }
  }

  // ─── Favorites Bar (Header) ──────────────────────────────

  var favoritesCache = null;

  function setupFavoritesBar() {
    var bar = document.getElementById('favorites-bar');
    if (!bar) return;

    // Load favorites data to populate tooltips
    loadFavoritesCache();

    // Close any open popover when clicking outside
    document.addEventListener('click', function(e) {
      if (!e.target.closest('.fav-slot-btn') && !e.target.closest('.fav-popover')) {
        closeFavPopover();
      }
    });

    bar.addEventListener('click', function(e) {
      // Handle popover button clicks
      var popBtn = e.target.closest('.fav-popover-btn');
      if (popBtn) {
        e.stopPropagation();
        var action = popBtn.getAttribute('data-action');
        var idx = parseInt(popBtn.getAttribute('data-fav'), 10);
        closeFavPopover();
        if (action === 'recall') {
          recallFavorite(idx);
        } else if (action === 'store') {
          storeFavorite(idx);
        } else if (action === 'export-all') {
          exportFavorites();
        } else if (action === 'import-all') {
          importFavorites();
        }
        return;
      }

      var btn = e.target.closest('.fav-slot-btn');
      if (!btn) return;
      var idx = parseInt(btn.getAttribute('data-fav'), 10);

      // Close existing popover if clicking same button
      var existing = btn.querySelector('.fav-popover');
      if (existing) {
        closeFavPopover();
        return;
      }

      // Close any other open popover first
      closeFavPopover();

      // Show popover
      showFavPopover(btn, idx);
    });
  }

  function closeFavPopover() {
    var pop = document.querySelector('.fav-popover');
    if (pop) pop.remove();
  }

  function showFavPopover(btn, idx) {
    var fav = favoritesCache ? favoritesCache[idx] : null;
    var hasData = fav && fav.plug_0 && fav.plug_0 !== 'Void';

    var infoText = hasData
      ? 'A: ' + fav.plug_0 + (fav.plug_1 && fav.plug_1 !== 'Void' ? ' | B: ' + fav.plug_1 : '')
      : 'Empty slot';

    var pop = document.createElement('div');
    pop.className = 'fav-popover';
    pop.innerHTML =
      '<button class="fav-popover-btn' + (hasData ? '' : ' disabled') + '" data-action="recall" data-fav="' + idx + '"' + (hasData ? '' : ' disabled') + '>' +
        '<sl-icon name="play-fill"></sl-icon> Recall' +
      '</button>' +
      '<button class="fav-popover-btn store" data-action="store" data-fav="' + idx + '">' +
        '<sl-icon name="floppy"></sl-icon> Store Current' +
      '</button>' +
      '<div class="fav-popover-divider"></div>' +
      '<button class="fav-popover-btn" data-action="export-all" data-fav="' + idx + '">' +
        '<sl-icon name="download"></sl-icon> Export All' +
      '</button>' +
      '<button class="fav-popover-btn" data-action="import-all" data-fav="' + idx + '">' +
        '<sl-icon name="upload"></sl-icon> Import All' +
      '</button>' +
      '<div class="fav-popover-info">' + S.esc(infoText) + '</div>';

    btn.appendChild(pop);
  }

  async function loadFavoritesCache() {
    try {
      favoritesCache = await S.queuedFetch('/device?action=getFavorites');
      updateFavoritesBarTooltips();
    } catch (e) {
      favoritesCache = null;
    }
  }

  function updateFavoritesBarTooltips() {
    if (!favoritesCache) return;
    var btns = document.querySelectorAll('.fav-slot-btn');
    btns.forEach(function(btn) {
      var idx = parseInt(btn.getAttribute('data-fav'), 10);
      var fav = favoritesCache[idx];
      if (fav && fav.plug_0 && fav.plug_0 !== 'Void') {
        btn.title = 'Fav ' + (idx + 1) + ': A=' + fav.plug_0 + ' B=' + (fav.plug_1 || 'Void');
        btn.classList.add('has-data');
      } else {
        btn.title = 'Fav ' + (idx + 1) + ': (empty)';
        btn.classList.remove('has-data');
      }
    });
  }

  async function recallFavorite(idx) {
    S.showLoading('Recalling favorite…');
    try {
      await S.queuedPost('/device?action=recallFavorite&id=' + idx, null, S.API_MUTATION_TIMEOUT_MS);
      // Sequential — never use Promise.all against ESP32
      await loadSlotData(0);
      await loadSlotData(1);
      renderPluginList();
      await loadFavoritesCache();
    } catch (e) {
      console.error('Failed to recall favorite ' + (idx + 1), e);
    } finally {
      S.hideLoading();
    }
  }

  async function storeFavorite(idx) {
    var favData = {
      name: 'Favorite ' + (idx + 1),
      plug_0: state.activePlugin[0] ? state.activePlugin[0].id : 'Void',
      pre_0:  state.activePreset[0] >= 0 ? state.activePreset[0] : 0,
      plug_1: state.activePlugin[1] ? state.activePlugin[1].id : 'Void',
      pre_1:  state.activePreset[1] >= 0 ? state.activePreset[1] : 0,
      ustring: '',
    };
    try {
      await S.queuedPost('/device?action=storeFavorite&id=' + idx, favData);
      S.toast('Stored favorite ' + (idx + 1), 'success', 2000);
      await loadFavoritesCache();
    } catch (e) {
      S.toast('Failed to store favorite ' + (idx + 1), 'danger');
    }
  }

  async function exportFavorites() {
    try {
      if (!favoritesCache) await loadFavoritesCache();
      var data = JSON.stringify(favoritesCache, null, 2);
      var blob = new Blob([data], { type: 'application/json' });
      var url = URL.createObjectURL(blob);
      var a = document.createElement('a');
      a.href = url;
      a.download = 'ctag-tbd-favorites.json';
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
      S.toast('Favorites exported', 'success', 2000);
    } catch (e) {
      S.toast('Export failed: ' + e.message, 'danger');
    }
  }

  async function importFavorites() {
    var input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.addEventListener('change', async function() {
      if (!input.files || !input.files[0]) return;
      try {
        var text = await input.files[0].text();
        var data = JSON.parse(text);
        if (!Array.isArray(data)) {
          S.toast('Invalid favorites file — expected array', 'danger');
          return;
        }
        S.showLoading('Importing favorites…');
        for (var i = 0; i < data.length; i++) {
          if (data[i] && data[i].plug_0) {
            await S.queuedPost('/device?action=storeFavorite&id=' + i, data[i]);
          }
        }
        await loadFavoritesCache();
        S.hideLoading();
        S.toast('Imported ' + data.length + ' favorites', 'success', 2000);
      } catch (e) {
        S.hideLoading();
        S.toast('Import failed: ' + e.message, 'danger');
      }
    });
    input.click();
  }

  // ─── Swap Slots ──────────────────────────────────────────

  function setupSwapSlots() {
    var btn = document.getElementById('swap-slots-btn');
    if (!btn) return;

    btn.addEventListener('click', async function() {
      if (state.stereoLocked) {
        S.toast('Cannot swap — stereo plugin active', 'warning');
        return;
      }

      var plugA = state.activePlugin[0] ? state.activePlugin[0].id : 'Void';
      var plugB = state.activePlugin[1] ? state.activePlugin[1].id : 'Void';

      if (plugA === 'Void' && plugB === 'Void') {
        S.toast('Both slots are empty', 'warning', 2000);
        return;
      }

      try {
        S.showLoading('Swapping slots…');
        // Set A to B's plugin and B to A's plugin
        await S.queuedPost('/plugins?action=setActive&ch=0&id=' + encodeURIComponent(plugB), null, S.API_MUTATION_TIMEOUT_MS);
        await S.queuedPost('/plugins?action=setActive&ch=1&id=' + encodeURIComponent(plugA), null, S.API_MUTATION_TIMEOUT_MS);
        // Sequential — never use Promise.all against ESP32
        await loadSlotData(0, plugB);
        await loadSlotData(1, plugA);
        renderPluginList();
      } catch (e) {
        console.error('Failed to swap slots', e);
        S.toast('Swap failed \u2014 device may be busy or offline', 'danger');
      } finally {
        S.hideLoading();
      }
    });
  }

  // ─── Sidebar Collapse/Expand ──────────────────────────────

  function setupSidebarToggles() {
    // Plugin sidebar (left)
    var pluginToggle = document.getElementById('plugin-sidebar-toggle');
    var pluginSidebar = document.querySelector('.plugin-sidebar');
    if (pluginToggle && pluginSidebar) {
      var pluginIcon = pluginToggle.querySelector('sl-icon');
      // Restore state from localStorage
      if (localStorage.getItem('tbd-plugin-sidebar-collapsed') === '1') {
        pluginSidebar.classList.add('collapsed');
        if (pluginIcon) pluginIcon.name = 'chevron-right';
      }
      pluginToggle.addEventListener('click', function() {
        pluginSidebar.classList.toggle('collapsed');
        var collapsed = pluginSidebar.classList.contains('collapsed');
        if (pluginIcon) pluginIcon.name = collapsed ? 'chevron-right' : 'chevron-left';
        localStorage.setItem('tbd-plugin-sidebar-collapsed', collapsed ? '1' : '0');
      });
    }

    // Preset sidebar removed — presets managed in-slot only
  }

  // ─── Initialization ─────────────────────────────────────

  function setupRemoveButtons() {
    for (var ch = 0; ch < 2; ch++) {
      (function(ch) {
        var btn = document.getElementById('remove-plugin-' + ch);
        if (btn) {
          btn.addEventListener('click', function() {
            if (state.activePlugin[ch]) {
              clearSlot(ch);
            }
          });
        }
      })(ch);
    }
  }

  async function init() {
    if (state.initialized) {
      // Refresh data on reconnect
      await refreshAll();
      return;
    }

    // getPlugins is critical — let it throw so app.js can detect failure
    // and trigger the reconnection monitor.
    state.plugins = await S.queuedFetch('/plugins?action=list') || [];

    // Fetch IO capabilities (CV/TRIG sources) — non-critical, tolerate failure
    try {
      state.ioCaps = await S.queuedFetch('/device?action=getIOCaps');
    } catch (e) {
      console.warn('Failed to load IO capabilities:', e);
      state.ioCaps = null;
    }

    renderPluginList();
    setupPluginListEvents();
    setupPresetEvents();
    setupSavePresetDialog();
    setupFavoritesBar();
    setupRemoveButtons();
    setupSidebarToggles();
    setupSwapSlots();
    setupSlotSelectDialog();

    // Load slots sequentially — ESP32 httpd has limited sockets,
    // so we avoid concurrent API requests during init.
    // Individual slot failures are tolerated (loadSlotData catches internally).
    await loadSlotData(0);
    await loadSlotData(1);
    renderPluginList(); // re-render with active indicators

    state.initialized = true;
  }

  async function refreshAll() {
    try {
      await loadSlotData(0);
      await loadSlotData(1);
      renderPluginList();
    } catch (e) {
      console.error('Refresh failed:', e);
    }
  }

  /**
   * Re-render parameter UI for both slots without refetching data.
   * Called when display mode toggles (Config ↔ Control).
   */
  function rerenderParams() {
    for (var ch = 0; ch < 2; ch++) {
      if (state.params[ch]) {
        renderParams(ch);
      }
    }
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.pluginManager = {
    init: init,
    state: state,
    favoritesCache: null,
    rerenderParams: rerenderParams,
  };

  // Keep reference updated when cache loads
  var origLoadCache = loadFavoritesCache;
  loadFavoritesCache = async function() {
    await origLoadCache();
    window.TBD.pluginManager.favoritesCache = favoritesCache;
  };

})();
