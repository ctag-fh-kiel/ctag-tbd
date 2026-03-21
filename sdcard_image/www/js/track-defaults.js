// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Track Default Presets Editor (Overlay)
//
// Opens as a Shoelace dialog from the header nav.
// Hierarchy mirrors the main Preset & Macro Manager UI:
//   Track → Machine → Preset (grouped by Macro definition)
//
// Machine names come from S.getMachineInfo() (synthdefinitions.json).
// Machine list uses S.getTrackMachines() which filters out noX empties.
// Presets are grouped into <optgroup>s by their macro definition name,
// so the user sees e.g. "Phat Punch" / "Synth Kick — All knobs" sections.
//
// Data source:  /sdcard/data/trackdefaults.json
// API:          GET  /api/v2/macros?action=get_trackdefaults
//               POST /api/v2/macros?action=save_trackdefaults
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ─────────────────────────────────────────────────
  var trackDefaults = null;   // parsed trackdefaults.json
  var dirty = false;
  var facetedData = null;     // per-track: [ { machine, name, macros: [{id, name, presets}] } ]
  var kitNames = [];          // kit names from sample_rom.json via samples API
  var kitFiles = [];          // kit filenames from smp_banks (e.g. "def_smp.json")
  var kitMeta = [];           // per-kit bank metadata [{banks: [{name, color}]}]
  var activeKitIndex = 0;     // index of the currently active kit in PSRAM
  var activeKitEntries = [];  // sample entries for the active kit

  var SLICES_PER_BANK = 32;
  var DEFAULT_BANKS = [
    'KICK', 'SNARE', 'HIHAT CL', 'HIHAT OP',
    'CLAP', 'RIM', 'PERC', 'OTHER'
  ];

  // ─── Helpers ───────────────────────────────────────────────

  /**
   * Get display name for a machine ID using S.getMachineInfo()
   * (same source as the main UI's MACHINE: dropdown).
   */
  function getMachineName(machineId) {
    var info = S.getMachineInfo(machineId);
    return info ? info.name : machineId;
  }

  /**
   * Build faceted data for every track.
   * Uses S.getTrackMachines() to get the same machine list as the main UI
   * (filters out nodrum/nosynth/nofx).
   *
   * Returns { trackIndex: [ { machine, name, macros: [ { id, name, presets } ] } ] }
   */
  function buildFacetedData() {
    var tracks = S.data.tracks || [];
    var allDefs = S.data.macroDefs || [];
    var allPresets = S.data.soundPresets || [];

    var result = {};

    tracks.forEach(function(track) {
      // Use S.getTrackMachines() — same filter as the main UI MACHINE: dropdown
      var machines = S.getTrackMachines(track);
      var facets = [];

      machines.forEach(function(machineId) {
        // Find all macro definitions for this machine
        var defs = allDefs.filter(function(d) { return d.machine === machineId; });
        if (defs.length === 0) return;

        var macros = [];
        defs.forEach(function(def) {
          // Find all sound presets that use this macrodefinition
          var presets = allPresets.filter(function(p) { return p.macro === def.id; });
          if (presets.length === 0) return;
          presets.sort(function(a, b) {
            var na = (a.name || a.id).toLowerCase();
            var nb = (b.name || b.id).toLowerCase();
            return na < nb ? -1 : na > nb ? 1 : 0;
          });
          macros.push({ id: def.id, name: def.name || def.id, presets: presets });
        });

        // Only include machine if it has at least one preset
        if (macros.length > 0) {
          facets.push({
            machine: machineId,
            name: getMachineName(machineId),
            macros: macros
          });
        }
      });

      result[track.index] = facets;
    });

    return result;
  }

  /**
   * Given a preset ID, find which machine it belongs to within a track's facets.
   */
  function findMachineForPreset(presetId, facets) {
    for (var i = 0; i < facets.length; i++) {
      for (var j = 0; j < facets[i].macros.length; j++) {
        for (var k = 0; k < facets[i].macros[j].presets.length; k++) {
          if (facets[i].macros[j].presets[k].id === presetId) {
            return facets[i].machine;
          }
        }
      }
    }
    return '';
  }

  /**
   * Get the current default preset ID for a track from the loaded defaults.
   */
  function getDefaultPreset(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return '';
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry ? (entry.preset || '') : '';
  }

  /**
   * Get the saved sampleSlice for a track (rompler tracks only).
   */
  function getDefaultSlice(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return 0;
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry && typeof entry.sampleSlice === 'number' ? entry.sampleSlice : 0;
  }

  /**
   * Get the saved sampleBank (group index) for a track (rompler tracks only).
   */
  function getDefaultBank(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return 0;
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry && typeof entry.sampleBank === 'number' ? entry.sampleBank : 0;
  }

  /**
   * Get the number of populated banks for a kit index.
   * For the active kit, count from actual entries. Otherwise use metadata or default 8.
   */
  function getBankCountForKit(kitIndex) {
    if (kitIndex === activeKitIndex && activeKitEntries.length > 0) {
      return Math.ceil(activeKitEntries.length / SLICES_PER_BANK);
    }
    if (kitMeta && kitMeta[kitIndex] &&
        kitMeta[kitIndex].banks && kitMeta[kitIndex].banks.length > 0) {
      return kitMeta[kitIndex].banks.length;
    }
    return DEFAULT_BANKS.length;
  }

  /**
   * Get the number of valid slices for a given bank in a kit.
   * For the active kit, count actual entries. Otherwise return max 32.
   */
  function getSliceCountForBank(kitIndex, bankIndex) {
    if (kitIndex === activeKitIndex && activeKitEntries.length > 0) {
      var bankStart = bankIndex * SLICES_PER_BANK;
      var bankEnd = Math.min(bankStart + SLICES_PER_BANK, activeKitEntries.length);
      if (bankStart >= activeKitEntries.length) return 0;
      return bankEnd - bankStart;
    }
    return SLICES_PER_BANK;
  }

  /**
   * Get bank/group names for the currently selected kit index.
   * Uses smp_bank_meta from the samples API, with fallback to DEFAULT_BANKS.
   * Only returns names for banks that actually have entries.
   */
  function getBankNamesForKit(kitIndex) {
    var bankCount = getBankCountForKit(kitIndex);
    if (kitMeta && kitMeta[kitIndex] &&
        kitMeta[kitIndex].banks && kitMeta[kitIndex].banks.length > 0) {
      return kitMeta[kitIndex].banks.slice(0, bankCount).map(function(b) { return b.name; });
    }
    return DEFAULT_BANKS.slice(0, bankCount);
  }

  /**
   * Get display name for a slice: show sample filename if available, otherwise just the index.
   */
  function getSliceName(kitIndex, bankIndex, sliceIndex) {
    if (kitIndex === activeKitIndex && activeKitEntries.length > 0) {
      var entryIdx = bankIndex * SLICES_PER_BANK + sliceIndex;
      if (entryIdx < activeKitEntries.length) {
        var entry = activeKitEntries[entryIdx];
        if (entry && entry.filename) {
          return sliceIndex + ' — ' + entry.filename;
        }
      }
    }
    return String(sliceIndex);
  }

  /**
   * Rebuild the slice dropdown for a specific rompler track based on selected bank.
   */
  function rebuildSliceDropdown(trackIdx, kitIndex, bankIndex, selectedSlice) {
    var sliceCell = document.querySelector('.td-col-slice.td-rompler-cell[data-track="' + trackIdx + '"]');
    if (!sliceCell) return;
    var sliceCount = getSliceCountForBank(kitIndex, bankIndex);
    var html = '<select class="td-select td-slice-select" data-track="' + trackIdx + '">';
    for (var sl = 0; sl < sliceCount; sl++) {
      var slSel = (sl === selectedSlice) ? ' selected' : '';
      var sliceName = getSliceName(kitIndex, bankIndex, sl);
      html += '<option value="' + sl + '"' + slSel + '>' + S.esc(sliceName) + '</option>';
    }
    html += '</select>';
    sliceCell.innerHTML = html;
    sliceCell.querySelector('.td-slice-select').addEventListener('change', function() {
      dirty = true; updateSaveButton();
    });
  }

  /**
   * Rebuild all rompler Bank dropdowns when Kit changes.
   */
  function rebuildBankDropdowns() {
    var kitSel = document.getElementById('td-global-kit');
    var kitIdx = kitSel ? parseInt(kitSel.value, 10) : 0;
    var bankNames = getBankNamesForKit(kitIdx);
    var bankCount = bankNames.length;
    var selects = document.querySelectorAll('.td-bank-select');
    selects.forEach(function(sel) {
      var current = parseInt(sel.value, 10) || 0;
      if (current >= bankCount) current = 0;
      var html = '';
      bankNames.forEach(function(name, i) {
        var selected = (i === current) ? ' selected' : '';
        html += '<option value="' + i + '"' + selected + '>' + S.esc(name) + '</option>';
      });
      sel.innerHTML = html;
      // Also rebuild the slice dropdown for this track
      var trackIdx = parseInt(sel.getAttribute('data-track'), 10);
      var sliceSel = document.querySelector('.td-slice-select[data-track="' + trackIdx + '"]');
      var currentSlice = sliceSel ? parseInt(sliceSel.value, 10) || 0 : 0;
      rebuildSliceDropdown(trackIdx, kitIdx, current, currentSlice);
    });
  }

  /**
   * Check if a track supports the rompler machine (has 'ro' in machines list).
   */
  function trackHasRompler(track) {
    return (track.machines || []).indexOf('ro') !== -1;
  }

  // ─── API ───────────────────────────────────────────────────

  /**
   * Fetch kit names and per-kit bank metadata from the samples API.
   */
  function loadKitData() {
    return S.queuedFetch('/samples')
      .then(function(data) {
        if (data && data.kits && data.kits.smp_bank_names) {
          kitNames = data.kits.smp_bank_names;
        } else {
          kitNames = [];
        }
        if (data && data.kits && data.kits.smp_banks) {
          kitFiles = data.kits.smp_banks;
        } else {
          kitFiles = [];
        }
        if (data && data.kits && data.kits.smp_bank_meta) {
          kitMeta = data.kits.smp_bank_meta;
        } else {
          kitMeta = [];
        }
        if (data && data.kits && typeof data.kits.active_smp_bank === 'number') {
          activeKitIndex = data.kits.active_smp_bank;
        } else {
          activeKitIndex = 0;
        }
        if (data && data.active_kit_entries && Array.isArray(data.active_kit_entries)) {
          activeKitEntries = data.active_kit_entries;
        } else {
          activeKitEntries = [];
        }
        return kitNames;
      })
      .catch(function() {
        kitNames = [];
        kitFiles = [];
        kitMeta = [];
        activeKitEntries = [];
        return kitNames;
      });
  }

  function loadTrackDefaults() {
    return loadKitData().then(function() {
      return S.queuedFetch('/macros?action=get_trackdefaults');
    }).then(function(data) {
        trackDefaults = data && data.tracks ? data : { tracks: [] };
        return trackDefaults;
      })
      .catch(function(err) {
        console.warn('[TrackDefaults] Load failed, using empty defaults:', err);
        trackDefaults = { tracks: [] };
        return trackDefaults;
      });
  }

  function saveTrackDefaults(data) {
    return S.queuedPost('/macros?action=save_trackdefaults', data, S.API_MUTATION_TIMEOUT_MS)
      .then(function(resp) {
        if (resp && resp.ok) {
          S.toast('Boot defaults saved', 'success', 3000);
          dirty = false;
        } else {
          S.toast('Save failed', 'danger', 4000);
        }
        return resp;
      })
      .catch(function(err) {
        S.toast('Save failed: ' + err.message, 'danger', 4000);
        throw err;
      });
  }

  // ─── Preset dropdown builder ───────────────────────────────

  /**
   * Rebuild the preset <select> options for a given track row
   * when the machine dropdown changes.
   * Presets are grouped by their macro definition name.
   */
  function rebuildPresetDropdown(trackIdx, machineId, currentPresetId) {
    var presetSel = document.querySelector('.td-preset-select[data-track="' + trackIdx + '"]');
    if (!presetSel) return;

    var facets = facetedData[trackIdx] || [];
    var matchingFacet = null;
    for (var i = 0; i < facets.length; i++) {
      if (facets[i].machine === machineId) { matchingFacet = facets[i]; break; }
    }

    var html = '<option value="">(auto — first available)</option>';

    if (matchingFacet) {
      var macros = matchingFacet.macros;

      macros.forEach(function(macro) {
        // Clean up macro definition name for the optgroup label
        var label = (macro.name || macro.id);
        // If it ends with "All param(s)", make it clearer
        if (/All\s*param/i.test(label)) {
          label = label.replace(/\s*All\s*param(s)?\s*$/i, '') + ' — All knobs';
        }

        // Always show optgroup so user can see which macro def each preset belongs to
        html += '<optgroup label="' + S.esc(label) + '">';
        macro.presets.forEach(function(p) {
          var sel = p.id === currentPresetId ? ' selected' : '';
          var pName = p.name || p.id;
          html += '<option value="' + S.esc(p.id) + '"' + sel + '>';
          html += S.esc(pName) + ' (' + S.esc(p.id) + ') \u2014 Macro: ' + S.esc(macro.id);
          html += '</option>';
        });
        html += '</optgroup>';
      });
    }

    presetSel.innerHTML = html;
    presetSel.disabled = !machineId;

    // If the current saved preset wasn't found in the new machine, reset
    if (currentPresetId && presetSel.value !== currentPresetId) {
      presetSel.value = '';
    }

  }

  // ─── Rendering ─────────────────────────────────────────────

  /**
   * Toggle Bank & Slice cells when machine changes to/from Rompler.
   */
  function updateRomplerCells(trackIdx, machineId) {
    var bankCell = document.querySelector('.td-col-bank.td-rompler-cell[data-track="' + trackIdx + '"]');
    var sliceCell = document.querySelector('.td-col-slice.td-rompler-cell[data-track="' + trackIdx + '"]');
    if (!bankCell || !sliceCell) return;

    if (machineId === 'ro') {
      // Build bank dropdown
      var kitSel = document.getElementById('td-global-kit');
      var kitIdx = kitSel ? parseInt(kitSel.value, 10) : 0;
      var bankNames = getBankNamesForKit(kitIdx);
      var bankHtml = '<select class="td-select td-bank-select" data-track="' + trackIdx + '">';
      bankNames.forEach(function(name, i) {
        bankHtml += '<option value="' + i + '">' + S.esc(name) + '</option>';
      });
      bankHtml += '</select>';
      bankCell.innerHTML = bankHtml;

      // Build slice dropdown for bank 0
      rebuildSliceDropdown(trackIdx, kitIdx, 0, 0);

      // Attach change listeners to new selects
      bankCell.querySelector('.td-bank-select').addEventListener('change', function() {
        var newBank = parseInt(this.value, 10);
        rebuildSliceDropdown(trackIdx, kitIdx, newBank, 0);
        dirty = true; updateSaveButton();
      });
      sliceCell.querySelector('.td-slice-select').addEventListener('change', function() { dirty = true; updateSaveButton(); });
    } else {
      bankCell.innerHTML = '<span class="td-na">—</span>';
      sliceCell.innerHTML = '<span class="td-na">—</span>';
    }
  }

  function renderOverlayContent() {
    var body = document.getElementById('trackdefaults-body');
    if (!body) return;

    facetedData = buildFacetedData();
    var tracks = S.data.tracks || [];

    var html = '';
    html += '<p class="td-intro">Configure which preset each track loads on boot. ';
    html += 'Pick a <strong>machine</strong> first, then choose a <strong>preset</strong>. ';
    html += 'Changes take effect on next power-up.</p>';

    // ─── Global kit selector ─────────────────────────────────
    if (kitNames.length > 0) {
      var savedKitFile = (trackDefaults && typeof trackDefaults.kit === 'string')
        ? trackDefaults.kit : (kitFiles.length > 0 ? kitFiles[0] : '');
      var savedKit = kitFiles.indexOf(savedKitFile);
      if (savedKit < 0) savedKit = 0;
      html += '<div class="td-sample-bank-section">';
      html += '<label><strong>Kit (PSRAM)</strong> ';
      html += '<select class="td-select" id="td-global-kit">';
      kitNames.forEach(function(name, i) {
        var sel = (i === savedKit) ? ' selected' : '';
        html += '<option value="' + i + '"' + sel + '>' + S.esc(name) + '</option>';
      });
      html += '</select></label>';
      html += '<span class="td-hint"> All rompler tracks share this kit. ';
      html += 'Switching reloads PSRAM from SD card at boot.</span>';
      html += '</div>';
    }

    html += '<div class="td-table">';
    html += '<div class="td-row td-header">';
    html += '<span class="td-col-idx">#</span>';
    html += '<span class="td-col-name">Track</span>';
    html += '<span class="td-col-type">Type</span>';
    html += '<span class="td-col-engine">Machine</span>';
    html += '<span class="td-col-preset">Preset</span>';
    html += '<span class="td-col-bank">Bank</span>';
    html += '<span class="td-col-slice">Slice</span>';
    html += '</div>';

    tracks.forEach(function(track) {
      var idx = track.index;
      var currentPreset = getDefaultPreset(idx);
      var facets = facetedData[idx] || [];

      // Determine current machine from the saved preset
      var currentMachine = currentPreset ? findMachineForPreset(currentPreset, facets) : '';

      // Track type badge
      var typeClass = 'td-type-badge';
      if (track.type === 'drum')  typeClass += ' td-type-drum';
      else if (track.type === 'synth') typeClass += ' td-type-synth';
      else if (track.type === 'fx')    typeClass += ' td-type-fx';

      html += '<div class="td-row" data-track="' + idx + '">';
      html += '<span class="td-col-idx">' + String(idx + 1).padStart(2, '0') + '</span>';
      html += '<span class="td-col-name">' + S.esc(track.name) + '</span>';
      html += '<span class="td-col-type"><span class="' + typeClass + '">' + S.esc(track.type) + '</span></span>';

      // Machine dropdown — same options as the main UI MACHINE: dropdown
      html += '<span class="td-col-engine">';
      html += '<select class="td-select td-machine-select" data-track="' + idx + '">';
      html += '<option value="">(auto)</option>';
      facets.forEach(function(f) {
        var sel = f.machine === currentMachine ? ' selected' : '';
        html += '<option value="' + S.esc(f.machine) + '"' + sel + '>' + S.esc(f.name) + '</option>';
      });
      html += '</select>';
      html += '</span>';

      // Preset dropdown (populated dynamically based on machine selection)
      html += '<span class="td-col-preset">';
      html += '<select class="td-select td-preset-select" data-track="' + idx + '"';
      if (!currentMachine) html += ' disabled';
      html += '>';
      html += '<option value="">(auto — first available)</option>';
      html += '</select>';
      html += '</span>';

      // Bank + Slice selectors — only shown when selected machine is Rompler
      var isSelectedRompler = (currentMachine === 'ro');
      var savedBank = getDefaultBank(idx);
      var savedSlice = getDefaultSlice(idx);
      var savedKitFile2 = (trackDefaults && typeof trackDefaults.kit === 'string')
        ? trackDefaults.kit : (kitFiles.length > 0 ? kitFiles[0] : '');
      var kitIdx = kitFiles.indexOf(savedKitFile2);
      if (kitIdx < 0) kitIdx = 0;
      var bankNames = getBankNamesForKit(kitIdx);

      html += '<span class="td-col-bank td-rompler-cell" data-track="' + idx + '">';
      if (isSelectedRompler) {
        html += '<select class="td-select td-bank-select" data-track="' + idx + '">';
        bankNames.forEach(function(name, bi) {
          var bSel = (bi === savedBank) ? ' selected' : '';
          html += '<option value="' + bi + '"' + bSel + '>' + S.esc(name) + '</option>';
        });
        html += '</select>';
      } else {
        html += '<span class="td-na">—</span>';
      }
      html += '</span>';

      html += '<span class="td-col-slice td-rompler-cell" data-track="' + idx + '">';
      if (isSelectedRompler) {
        var sliceCount = getSliceCountForBank(kitIdx, savedBank);
        html += '<select class="td-select td-slice-select" data-track="' + idx + '">';
        for (var sl = 0; sl < sliceCount; sl++) {
          var slSel = (sl === savedSlice) ? ' selected' : '';
          var sliceName = getSliceName(kitIdx, savedBank, sl);
          html += '<option value="' + sl + '"' + slSel + '>' + sliceName + '</option>';
        }
        html += '</select>';
      } else {
        html += '<span class="td-na">—</span>';
      }
      html += '</span>';

      html += '</div>';
    });

    html += '</div>'; // .td-table

    body.innerHTML = html;

    // Populate preset dropdowns for tracks that have a saved machine
    tracks.forEach(function(track) {
      var currentPreset = getDefaultPreset(track.index);
      var facets = facetedData[track.index] || [];
      var currentMachine = currentPreset ? findMachineForPreset(currentPreset, facets) : '';
      if (currentMachine) {
        rebuildPresetDropdown(track.index, currentMachine, currentPreset);
      }
    });

    // Attach machine change listeners
    body.querySelectorAll('.td-machine-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var idx = parseInt(sel.getAttribute('data-track'), 10);
        var machineId = sel.value;
        rebuildPresetDropdown(idx, machineId, '');

        // Auto-select first available preset when machine changes
        var presetSel = document.querySelector('.td-preset-select[data-track="' + idx + '"]');
        if (presetSel && presetSel.options.length > 1) {
          presetSel.selectedIndex = 1; // first real preset (index 0 is "(auto)")
        }
        // Show/hide Bank & Slice cells based on whether machine is Rompler
        updateRomplerCells(idx, machineId);

        dirty = true;
        updateSaveButton();
      });
    });

    // Attach preset change listeners
    body.querySelectorAll('.td-preset-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
        updateSaveButton();
      });
    });

    // Attach slice change listeners
    body.querySelectorAll('.td-slice-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
        updateSaveButton();
      });
    });

    // Attach bank (group) change listeners — also rebuilds slice dropdown
    body.querySelectorAll('.td-bank-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var trackIdx = parseInt(sel.getAttribute('data-track'), 10);
        var kitSel2 = document.getElementById('td-global-kit');
        var kitIdx2 = kitSel2 ? parseInt(kitSel2.value, 10) : 0;
        var newBank = parseInt(sel.value, 10);
        rebuildSliceDropdown(trackIdx, kitIdx2, newBank, 0);
        dirty = true;
        updateSaveButton();
      });
    });

    // Attach global kit change listener — also refreshes bank dropdowns
    var kitSel = document.getElementById('td-global-kit');
    if (kitSel) {
      kitSel.addEventListener('change', function() {
        rebuildBankDropdowns();
        dirty = true;
        updateSaveButton();
      });
    }

    dirty = false;
    updateSaveButton();
  }

  function updateSaveButton() {
    var btn = document.getElementById('td-save-btn');
    if (btn) {
      btn.disabled = !dirty;
    }
  }

  // ─── Collect & Save ────────────────────────────────────────

  function collectFromUI() {
    // Read global kit selection
    var kitSel = document.getElementById('td-global-kit');
    var kitIdx = kitSel ? parseInt(kitSel.value, 10) : 0;
    var kitFile = (kitFiles.length > kitIdx) ? kitFiles[kitIdx] : (kitFiles[0] || 'def_smp.json');

    var result = {
      _comment: 'Default preset per track, loaded by the Pico via SPI command 0xA5.',
      _comment2: 'Preset IDs = filenames (without .json) from data/macrosoundpresets/.',
      _comment3: 'Omit a track entry to let the Pico use the first available preset.',
      _comment4: 'The kit field sets which kit file to activate in PSRAM (matched by filename).',
      _comment5: 'NOTE: all romplers share the same PSRAM kit — only one kit is active at a time.',
      kit: kitFile,
      tracks: []
    };

    var tracks = S.data.tracks || [];
    var presetSelects = document.querySelectorAll('#trackdefaults-body .td-preset-select');

    presetSelects.forEach(function(sel) {
      var idx = parseInt(sel.getAttribute('data-track'), 10);
      var presetId = sel.value;
      if (presetId) {
        var track = tracks.find(function(t) { return t.index === idx; });
        var trackName = track ? track.name : ('Track ' + idx);

        // Include machine name in the comment for readability
        var machineSel = document.querySelector('.td-machine-select[data-track="' + idx + '"]');
        var machineName = machineSel ? machineSel.options[machineSel.selectedIndex].text : '';

        var entry = {
          index: idx,
          preset: presetId,
          _name: trackName + ' — ' + machineName + ' — ' + presetId
        };

        // Add rompler-specific fields only when machine is set to Rompler
        var machineVal = machineSel ? machineSel.value : '';
        if (machineVal === 'ro') {
          var bankGroupSel = document.querySelector('.td-bank-select[data-track="' + idx + '"]');
          entry.sampleBank = bankGroupSel ? parseInt(bankGroupSel.value, 10) : 0;
          var sliceSel = document.querySelector('.td-slice-select[data-track="' + idx + '"]');
          entry.sampleSlice = sliceSel ? parseInt(sliceSel.value, 10) : 0;
        }

        result.tracks.push(entry);
      }
    });

    return result;
  }

  // ─── Init ──────────────────────────────────────────────────

  function init() {
    // Create the dialog DOM dynamically so it can be shared across pages
    // (index.html Sample Manager + preset-macro-manager.html)
    var dialog = document.getElementById('trackdefaults-dialog');
    if (!dialog) {
      dialog = document.createElement('sl-dialog');
      dialog.label = 'Boot Default Presets';
      dialog.id = 'trackdefaults-dialog';
      dialog.style.cssText = '--width:64rem;';
      dialog.innerHTML =
        '<div id="trackdefaults-body"></div>' +
        '<sl-button slot="footer" variant="default" id="td-close-btn">Close</sl-button>' +
        '<sl-button slot="footer" variant="primary" id="td-save-btn" disabled>Save to SD Card</sl-button>';
      document.body.appendChild(dialog);
    }

    var openBtn = document.getElementById('trackdefaults-btn');
    var saveBtn = document.getElementById('td-save-btn');
    var closeBtn = document.getElementById('td-close-btn');

    if (!openBtn) {
      console.warn('[TrackDefaults] Trigger button not found');
      return;
    }

    openBtn.addEventListener('click', function() {
      S.showLoading('Loading boot defaults…');
      // Ensure shared data (tracks, machines, macros) is loaded — on the Macros
      // page this is already done, on the Samples page it may not be.
      var dataReady = S.data.loaded
        ? Promise.resolve()
        : S.loadSharedData();
      dataReady.then(function() {
        return loadTrackDefaults();
      }).then(function() {
        renderOverlayContent();
        S.hideLoading();
        dialog.show();
      }).catch(function() {
        S.hideLoading();
        S.toast('Could not load boot defaults', 'danger', 4000);
      });
    });

    if (saveBtn) {
      saveBtn.addEventListener('click', function() {
        var data = collectFromUI();
        S.showLoading('Saving boot defaults…');
        saveTrackDefaults(data).then(function() {
          trackDefaults = data;
          S.hideLoading();
        }).catch(function() {
          S.hideLoading();
        });
      });
    }

    if (closeBtn) {
      closeBtn.addEventListener('click', function() {
        dialog.hide();
      });
    }

    // Confirm unsaved changes on close
    dialog.addEventListener('sl-request-close', function(e) {
      if (dirty) {
        if (!confirm('You have unsaved changes. Discard them?')) {
          e.preventDefault();
        }
      }
    });
  }

  // ─── Export ────────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.trackDefaults = {
    init: init,
  };

})();
