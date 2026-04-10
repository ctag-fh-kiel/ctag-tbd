// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Track Default Presets Editor (Right Panel)
//
// Renders inside #kit-panel as a right-panel editor mode.
// Toggle between File Viewer (JSON) and Editor via header buttons.
// Hierarchy mirrors the main Preset & Macro Manager UI:
//   Track → Machine → Preset (grouped by Macro definition)
//
// Machine names come from S.getMachineInfo() (synthdefinitions.json).
// Machine list uses S.getTrackMachines() which filters out noX empties.
// Presets are grouped into <optgroup>s by their macro definition name,
// so the user sees e.g. "Phat Punch" / "Synth Kick — All knobs" sections.
//
// Data source:  factory/ or user/trackdefaults/
// API:          GET  /api/v2/macros?action=get_trackdefaults&file=<name>
//               POST /api/v2/macros?action=save_trackdefaults
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ─────────────────────────────────────────────────
  var currentFile = null;     // { path, name } of the file being edited

  var trackDefaults = null;   // parsed trackdefaults.json
  var dirty = false;
  var facetedData = null;     // per-track: [ { machine, name, macros: [{id, name, presets}] } ]
  var kitNames = [];          // kit names from sample_rom.json via samples API
  var kitFiles = [];          // kit filenames from smp_banks (e.g. "def_smp.json")
  var kitMeta = [];           // per-kit bank metadata [{banks: [{name, color}]}]
  var activeKitIndex = 0;     // index of the currently active kit in PSRAM
  var activeKitEntries = [];  // sample entries for the active kit
  var activeTemplateName = 'default'; // name of the boot-default template (from P4)

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
      dirty = true;
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
    return S.queuedFetch('/storage')
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
    var fileParam = currentFile && currentFile.name ? '&file=' + encodeURIComponent(currentFile.name) : '';
    return loadKitData().then(function() {
      return S.queuedFetch('/macros?action=get_trackdefaults' + fileParam);
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

  function isFactoryFile() {
    return currentFile && /^factory\//i.test(currentFile.path || '');
  }

  function updateFactoryBadge() {
    var badge = document.getElementById('td-badge-factory');
    var factory = isFactoryFile();
    if (badge) badge.style.display = factory ? '' : 'none';
  }

  /**
   * Get the template name (without .json) for the current file.
   */
  function currentTemplateName() {
    if (!currentFile || !currentFile.name) return 'default';
    return currentFile.name.replace(/\.json$/i, '');
  }

  /**
   * Fetch which template is the active boot default from the P4.
   */
  function fetchActiveTemplate() {
    return S.queuedFetch('/macros?action=get_active_trackdefault')
      .then(function(data) {
        if (data && data.name) activeTemplateName = data.name;
        updateActiveBadge();
      })
      .catch(function() { /* ignore — badge stays as-is */ });
  }

  /**
   * Update the "boot default" badge and "Set as Boot Default" button.
   */
  function updateActiveBadge() {
    var badge = document.getElementById('td-badge-active');
    var btn = document.getElementById('td-setactive-btn');
    var isActive = (currentTemplateName() === activeTemplateName);
    if (badge) badge.style.display = isActive ? '' : 'none';
    if (btn) {
      btn.disabled = isActive;
      if (isActive) {
        btn.innerHTML = '<sl-icon name="star-fill" style="font-size:0.65rem;color:var(--sl-color-success-600);"></sl-icon> Boot Default';
      } else {
        btn.innerHTML = '<sl-icon name="star" style="font-size:0.65rem;"></sl-icon> Set as Boot Default';
      }
    }
  }

  /**
   * Set the current template as the boot default via REST API.
   */
  function setAsBootDefault() {
    var name = currentTemplateName();
    S.queuedPost('/macros?action=set_active_trackdefault', { name: name }, S.API_MUTATION_TIMEOUT_MS)
      .then(function(resp) {
        if (resp && resp.ok) {
          activeTemplateName = name;
          updateActiveBadge();
          S.toast('Boot default set to "' + name + '"', 'success', 3000);
        } else {
          S.toast('Failed to set boot default', 'danger', 4000);
        }
      })
      .catch(function(err) {
        S.toast('Failed to set boot default: ' + err.message, 'danger', 4000);
      });
  }

  function saveTrackDefaults(data, fileName) {
    var file = fileName || (currentFile ? currentFile.name : 'default.json');
    return S.queuedPost('/macros?action=save_trackdefaults&file=' + encodeURIComponent(file), data, S.API_MUTATION_TIMEOUT_MS)
      .then(function(resp) {
        if (resp && resp.ok) {
          S.toast('Track setup saved', 'success', 3000);
          dirty = false;
          updateFactoryBadge();
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

  // ─── Save As Dialog (Shoelace) ───────────────────────────

  function saveAsDialog() {
    var old = document.getElementById('td-saveas-dialog');
    if (old) old.remove();

    var srcName = currentFile ? currentFile.name.replace(/\.json$/i, '') : 'default';
    var defaultName = srcName + '-custom';

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'td-saveas-dialog';
    dialog.label = 'Save Track Setup As…';
    dialog.setAttribute('style', '--width:26rem;');

    // Add footer gap between Cancel and Save buttons
    var footerStyle = document.createElement('style');
    footerStyle.textContent = '#td-saveas-dialog::part(footer){display:flex;gap:0.5rem;justify-content:flex-end;}';

    var html = '';
    html += '<div style="font-size:0.8rem;color:var(--sl-color-neutral-500);margin-bottom:0.75rem;">';
    html += '<sl-icon name="info-circle" style="font-size:0.7rem;"></sl-icon> ';
    html += 'Saves a copy to the user folder. The original factory template is not modified.';
    html += '</div>';
    html += '<sl-input id="td-saveas-name" label="Template Name" value="' + S.esc(defaultName) + '" placeholder="e.g. my-setup" required autofocus></sl-input>';
    html += '<div id="td-saveas-hint" style="font-size:0.72rem;color:var(--sl-color-neutral-400);margin-top:0.35rem;"></div>';

    dialog.innerHTML = html;

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var saveBtn = document.createElement('sl-button');
    saveBtn.setAttribute('slot', 'footer');
    saveBtn.setAttribute('variant', 'primary');
    saveBtn.innerHTML = '<sl-icon name="floppy" slot="prefix"></sl-icon> Save Copy';

    saveBtn.addEventListener('click', function() {
      var nameInput = dialog.querySelector('#td-saveas-name');
      var hint = dialog.querySelector('#td-saveas-hint');
      var raw = (nameInput.value || '').trim();

      if (!raw) {
        hint.textContent = 'Please enter a name';
        hint.style.color = 'var(--sl-color-danger-600)';
        nameInput.focus();
        return;
      }

      var safeName = raw.toLowerCase().replace(/[^a-z0-9_-]+/g, '-').replace(/^-|-$/g, '');
      if (!safeName) {
        hint.textContent = 'Name must contain at least one letter or digit';
        hint.style.color = 'var(--sl-color-danger-600)';
        nameInput.focus();
        return;
      }

      var fileName = safeName + '.json';
      var data = collectFromUI();

      saveBtn.setAttribute('loading', '');
      saveTrackDefaults(data, fileName)
        .then(function() {
          dialog.hide();
          // Switch currentFile to the new user copy
          currentFile = { path: 'user/trackdefaults', name: fileName };
          var nameEl = document.getElementById('td-file-name');
          if (nameEl) nameEl.textContent = fileName;
          dirty = false;
          updateFactoryBadge();
          // Refresh the file browser to show the new file
          if (window.TBD.sampleManager && window.TBD.sampleManager.refreshCurrentDir) {
            window.TBD.sampleManager.refreshCurrentDir();
          }
        })
        .catch(function() {
          saveBtn.removeAttribute('loading');
        });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(saveBtn);
    document.body.appendChild(dialog);
    document.head.appendChild(footerStyle);

    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); footerStyle.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
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
        dirty = true;
      });
      sliceCell.querySelector('.td-slice-select').addEventListener('change', function() { dirty = true; });
    } else {
      bankCell.innerHTML = '<span class="td-na">—</span>';
      sliceCell.innerHTML = '<span class="td-na">—</span>';
    }
  }

  function renderOverlayContent() {
    var body = document.getElementById('td-editor-body');
    if (!body) return;

    facetedData = buildFacetedData();
    var tracks = S.data.tracks || [];

    var html = '';
    html += '<p class="td-intro">Configure which preset each track loads on boot. ';
    html += 'Pick a <strong>machine</strong> first, then choose a <strong>preset</strong>. ';
    html += 'Changes take effect on next power-up.</p>';;

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
       
      });
    });

    // Attach preset change listeners
    body.querySelectorAll('.td-preset-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
       
      });
    });

    // Attach slice change listeners
    body.querySelectorAll('.td-slice-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
       
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
       
      });
    });

    // Attach global kit change listener — also refreshes bank dropdowns
    var kitSel = document.getElementById('td-global-kit');
    if (kitSel) {
      kitSel.addEventListener('change', function() {
        rebuildBankDropdowns();
        dirty = true;
       
      });
    }

    dirty = false;
  }

  // ─── Collect & Save ────────────────────────────────────────

  function collectFromUI() {
    // Read global kit selection
    var kitSel = document.getElementById('td-global-kit');
    var kitIdx = kitSel ? parseInt(kitSel.value, 10) : 0;
    var kitFile = (kitFiles.length > kitIdx) ? kitFiles[kitIdx] : (kitFiles[0] || 'def_smp.json');

    var result = {
      _comment: 'Default preset per track, loaded by the Pico via SPI command 0xA5.',
      _comment2: 'Preset IDs = filenames (without .json) from presets/.',
      _comment3: 'Omit a track entry to let the Pico use the first available preset.',
      _comment4: 'The kit field sets which kit file to activate in PSRAM (matched by filename).',
      _comment5: 'NOTE: all romplers share the same PSRAM kit — only one kit is active at a time.',
      kit: kitFile,
      tracks: []
    };

    var tracks = S.data.tracks || [];
    var presetSelects = document.querySelectorAll('#td-editor-body .td-preset-select');

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

  // ─── Panel switching ────────────────────────────────────────

  function showTDPanel() {
    var panel = document.getElementById('kit-panel');
    panel.classList.remove('viewer-active');
    panel.classList.add('td-editor-active');
    // Update filename in the sub-header tab
    var nameEl = document.getElementById('td-file-name');
    if (nameEl) nameEl.textContent = currentFile ? currentFile.name : 'default.json';
    // Hide the File Viewer nav links when switching to editor
    var fvNav = document.getElementById('fv-td-nav');
    if (fvNav) fvNav.style.display = 'none';
    // Show/hide factory badge and adjust Save button
    updateFactoryBadge();
    // Fetch and show active template badge
    fetchActiveTemplate();
  }

  function closeTDEditor() {
    if (dirty && !confirm('You have unsaved changes. Discard them?')) return;
    var panel = document.getElementById('kit-panel');
    panel.classList.remove('td-editor-active');
    dirty = false;
    // Hide the File Viewer nav links
    var fvNav = document.getElementById('fv-td-nav');
    if (fvNav) fvNav.style.display = 'none';
  }

  /**
   * Switch from editor to JSON file viewer showing the actual file.
   * Fetches the raw file from the server so the user sees the real content.
   */
  function switchToJsonView() {
    var panel = document.getElementById('kit-panel');
    panel.classList.remove('td-editor-active');
    panel.classList.add('viewer-active');

    var fileName = currentFile ? currentFile.name : 'default.json';
    var filePath = currentFile ? currentFile.path : 'factory/trackdefaults';

    // Show the Editor/JSON nav links in File Viewer header
    var fvNav = document.getElementById('fv-td-nav');
    if (fvNav) fvNav.style.display = '';

    // Use the sample-manager openFileViewer to load the real file
    var fullPath = filePath + '/' + fileName;
    var body = document.getElementById('fv-body');
    body.innerHTML = '<sl-spinner></sl-spinner>';

    // Fetch the real file content from server
    fetch('/api/v2/storage?fetch=' + encodeURIComponent(fullPath))
      .then(function(r) { return r.text(); })
      .then(function(text) {
        var smState = window.TBD.sampleManager && window.TBD.sampleManager.state;
        if (smState) {
          smState.fileViewerOpen = true;
          smState.fileViewerData = {
            path: filePath,
            name: fileName,
            size: text.length,
            content: text,
            type: 'json'
          };
        }

        document.getElementById('fv-name').textContent = fileName;
        var iconDiv = document.getElementById('fv-icon');
        iconDiv.className = 'fv-tab-icon json';
        iconDiv.innerHTML = '<sl-icon name="file-earmark-code"></sl-icon>';
        document.getElementById('fv-lang').textContent = 'JSON';
        document.getElementById('fv-size').textContent = text.length + ' B';

        if (window.TBD.sampleManager && window.TBD.sampleManager.renderJson) {
          window.TBD.sampleManager.renderJson(body, text);
        } else {
          body.innerHTML = '<pre style="padding:1rem;color:#d4d4d4;">' + S.esc(text) + '</pre>';
        }
      })
      .catch(function() {
        body.innerHTML = '<div style="padding:1rem;color:#999;">Could not load file.</div>';
      });
  }

  /**
   * Open the track defaults editor for a specific file.
   * Called from: header button (default.json), file viewer toggle (any td file).
   */
  function openTrackDefaultsEditor(filePath, fileName) {
    currentFile = filePath ? { path: filePath, name: fileName || 'default.json' } : null;

    S.showLoading('Loading track setup…');
    var dataReady = S.data.loaded
      ? Promise.resolve()
      : S.loadSharedData();
    dataReady.then(function() {
      return loadTrackDefaults();
    }).then(function() {
      renderOverlayContent();
      S.hideLoading();
      showTDPanel();
    }).catch(function() {
      S.hideLoading();
      S.toast('Could not load track setup', 'danger', 4000);
    });
  }

  /**
   * Open from file viewer — parses the already-loaded JSON content.
   */
  function openFromFileViewer() {
    var smState = window.TBD.sampleManager && window.TBD.sampleManager.state;
    var fvData = smState && smState.fileViewerData;

    if (fvData && fvData.content) {
      currentFile = { path: fvData.path, name: fvData.name };
      try {
        var parsed = JSON.parse(fvData.content);
        trackDefaults = parsed && parsed.tracks ? parsed : { tracks: [] };
      } catch (e) {
        S.toast('Invalid JSON in file', 'danger', 4000);
        return;
      }
    } else {
      currentFile = null;
    }

    S.showLoading('Loading editor…');
    var dataReady = S.data.loaded
      ? Promise.resolve()
      : S.loadSharedData();
    dataReady.then(function() {
      return loadKitData();
    }).then(function() {
      renderOverlayContent();
      S.hideLoading();
      showTDPanel();
    }).catch(function() {
      S.hideLoading();
      S.toast('Could not load editor data', 'danger', 4000);
    });
  }

  // ─── Init ──────────────────────────────────────────────────

  function init() {
    var openBtn = document.getElementById('trackdefaults-btn');
    var closeBtn = document.getElementById('td-close-btn');

    // Header text links — TD editor side
    var tdLinkJson = document.getElementById('td-link-json');
    // Header text links — File Viewer side
    var fvLinkEditor = document.getElementById('fv-link-editor');

    if (openBtn) {
      openBtn.addEventListener('click', function() {
        openTrackDefaultsEditor('factory/trackdefaults', 'default.json');
      });
    }

    var saveAsBtn = document.getElementById('td-saveas-btn');
    if (saveAsBtn) {
      saveAsBtn.addEventListener('click', function() {
        saveAsDialog();
      });
    }

    var setActiveBtn = document.getElementById('td-setactive-btn');
    if (setActiveBtn) {
      setActiveBtn.addEventListener('click', function() {
        setAsBootDefault();
      });
    }

    if (closeBtn) {
      closeBtn.addEventListener('click', function() {
        closeTDEditor();
      });
    }

    if (tdLinkJson) {
      tdLinkJson.addEventListener('click', function() {
        switchToJsonView();
      });
    }

    if (fvLinkEditor) {
      fvLinkEditor.addEventListener('click', function() {
        openFromFileViewer();
      });
    }
  }

  // ─── Export ────────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.trackDefaults = {
    init: init,
    openEditor: openTrackDefaultsEditor,
    openFromFileViewer: openFromFileViewer,
    isTrackDefaultsFile: function(path) {
      return /trackdefaults\//i.test(path || '');
    }
  };

})();
