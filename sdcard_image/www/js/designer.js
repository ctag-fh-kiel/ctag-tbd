// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Macro Definition Editor (Designer)
//
// Manages macro definition editing in the unified view.
// Renders the definition list into the sidebar (#definition-list)
// and the Macro Builder into the center panel (#macro-builder-section).
//
// The Knob Preview is handled by performer.js using the shared
// renderKnobGroups() function — no longer duplicated here.
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    selectedDefId: null,
    editDef: null,
    activeTrack: -1,
    trackMachines: [],
    activeMachine: '',
    dirty: false,
    initialized: false,
  };

  // ─── UI Type Lookup ────────────────────────────────────────
  // Machine-specific CC id → {ui, curve} overrides — shared by
  // createOneToOneMapping() and the per-knob auto-select logic.
  var machineUiMap = {
    ro:  { bank: {ui:'samplebank'}, slice: {ui:'sampleslice'}, start: {ui:'sampleoffset'}, end: {ui:'sampleoffset'}, cutoff: {ui:'filtercutoff',curve:'log'}, reso: {ui:'filterq'}, type: {ui:'filtertype'}, bitcr: {ui:'bignum'}, attack: {ui:'envattack',curve:'exp'}, decay: {ui:'envdecay',curve:'exp'}, speed: {ui:'bignum'}, pitch: {ui:'bignum'}, loop: {ui:'bignum'}, pingpong: {ui:'bignum'}, ppstart: {ui:'sampleoffset'}, eg2fm: {ui:'envamount'}, tsmode: {ui:'bignum'}, tsamt: {ui:'envamount'} },
    db:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, dirt: {ui:'noise'}, 'fm-env': {ui:'envamount'}, 'fm-decay': {ui:'envdecay',curve:'exp'}, 'fm-accent': {ui:'envamount'} },
    ds:  { freq: {ui:'freq',curve:'log'}, decay: {ui:'envdecay',curve:'exp'}, fm: {ui:'shape'}, snap: {ui:'noise'}, accent: {ui:'envamount'} },
    as:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, snap: {ui:'noise'}, accent: {ui:'envamount'} },
    ab:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, 'a-fm': {ui:'shape3'}, 's-fm': {ui:'shape2'}, accent: {ui:'envamount'} },
    hh1: { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, noise: {ui:'distortion'}, accent: {ui:'envamount'} },
    hh2: { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, noise: {ui:'distortion'}, accent: {ui:'envamount'} },
    rs:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, noise: {ui:'distortion'}, accent: {ui:'envamount'} },
    cl:  { freq: {ui:'freq',curve:'log'}, tone: {ui:'shape'}, decay: {ui:'envdecay',curve:'exp'}, scale: {ui:'noise'} },
    fmb: { 'f-b': {ui:'shape2'}, 'd-b': {ui:'shape2',curve:'exp'}, 'f-m': {ui:'shape3'}, 'd-m': {ui:'shape3',curve:'exp'}, 'b-m': {ui:'shape2'}, 'a-f': {ui:'shape3'}, 'd-f': {ui:'shape',curve:'exp'}, i: {ui:'noise'} },
    mo:  { shape: {ui:'shape'}, p0: {ui:'shape3'}, p1: {ui:'shape2'}, waveshap: {ui:'distortion'}, attack: {ui:'envattack',curve:'exp'}, decay: {ui:'envdecay',curve:'exp'} },
    td3: { shape: {ui:'shape'}, p0: {ui:'shape2'}, vca_d: {ui:'envdecay',curve:'exp'}, vcf_d: {ui:'envdecay',curve:'exp'}, cutoff: {ui:'filtercutoff',curve:'log'}, reso: {ui:'filterq'}, envdec: {ui:'envdecay',curve:'exp'}, type: {ui:'filtertype'} },
    pp:  { detune: {ui:'distortion'}, cutoff: {ui:'filtercutoff',curve:'log'}, reso: {ui:'filterq'}, type: {ui:'filtertype'}, attack: {ui:'envattack',curve:'exp'}, decay: {ui:'envdecay',curve:'exp'}, release: {ui:'envdecay',curve:'exp'} },
    wtosc: { type: {ui:'filtertype'}, cutoff: {ui:'filtercutoff',curve:'log'}, reso: {ui:'filterq'}, attack: {ui:'envattack',curve:'exp'}, decay: {ui:'envdecay',curve:'exp'}, release: {ui:'bignum',curve:'exp'} },
    fxmaster: { compatk: {ui:'envattack',curve:'exp'}, comprel: {ui:'envdecay',curve:'exp'}, complpf: {ui:'filtercutoff',curve:'log'} },
    fxreverb: { time: {ui:'bignum',curve:'log'}, lowpass: {ui:'bignum',curve:'log'} },
    extdrum: { note: {ui:'midinote'} },
  };

  // Generic keyword fallback for machines/CCs not in the table above
  var uiKeywordMap = [
    { re: /bank/i,    ui: 'samplebank' },
    { re: /slice/i,   ui: 'sampleslice' },
    { re: /offset|start|end/i, ui: 'sampleoffset' },
    { re: /cutoff|lpf|hpf/i,   ui: 'filtercutoff', curve: 'log' },
    { re: /reso|res\b|q\b/i,   ui: 'filterq' },
    { re: /ftype|filtertype|type/i, ui: 'filtertype' },
    { re: /attack|atk/i,       ui: 'envattack', curve: 'exp' },
    { re: /decay|dec\b/i,      ui: 'envdecay', curve: 'exp' },
    { re: /release|rel\b/i,    ui: 'envdecay', curve: 'exp' },
    { re: /amount|depth|amt/i,  ui: 'envamount' },
    { re: /freq|frequency|pitch|tune/i, ui: 'freq', curve: 'log' },
    { re: /note/i,              ui: 'midinote' },
    { re: /noise/i,             ui: 'noise' },
    { re: /shape|wave/i,        ui: 'shape' },
    { re: /dist|drive|dirt/i,   ui: 'distortion' },
  ];

  // Lookup ui+curve for a given machine + cc id. Uses machineUiMap first,
  // then falls back to generic keyword matching on the CC id string.
  function lookupUiType(machineId, ccId) {
    var mMap = machineUiMap[machineId];
    if (mMap && mMap[ccId]) return mMap[ccId];
    // Generic keyword fallback
    for (var i = 0; i < uiKeywordMap.length; i++) {
      if (uiKeywordMap[i].re.test(ccId)) {
        var r = { ui: uiKeywordMap[i].ui };
        if (uiKeywordMap[i].curve) r.curve = uiKeywordMap[i].curve;
        return r;
      }
    }
    return { ui: 'bignum' };
  }

  // ─── API Helpers ──────────────────────────────────────────

  function apiGet(url) {
    return fetch(url).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    });
  }

  function apiPost(url, data) {
    return fetch(url, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data),
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    });
  }

  // ─── Track Selection (from shared track tabs) ────────────

  function onTrackSelected(idx, track) {
    state.activeTrack = idx;
    state.trackMachines = S.getTrackMachines(track);
    state.selectedDefId = null;
    state.editDef = null;
    state.dirty = false;

    // Check boot default to auto-select the right machine and macro
    var bootMacroId = null;
    if (S.data.trackDefaults && S.data.trackDefaults.tracks) {
      var tdEntry = S.data.trackDefaults.tracks.find(function(t) { return t.index === idx; });
      if (tdEntry && tdEntry.preset) {
        var bootPreset = S.data.soundPresets.find(function(p) { return p.id === tdEntry.preset; });
        if (bootPreset) {
          bootMacroId = bootPreset.macro;
          var bootDef = S.data.macroDefs.find(function(d) { return d.id === bootMacroId; });
          if (bootDef && state.trackMachines.indexOf(bootDef.machine) !== -1) {
            state.activeMachine = bootDef.machine;
          }
        }
      }
    }
    if (!state.activeMachine) {
      state.activeMachine = state.trackMachines.length > 0 ? state.trackMachines[0] : '';
    }

    renderDefinitionList();
    renderMacroBuilderSection();

    // Auto-select boot default's macro if available, otherwise first
    var filteredDefs = getFilteredDefs();
    var targetDef = null;
    if (bootMacroId) {
      targetDef = filteredDefs.find(function(d) { return d.id === bootMacroId; });
    }
    if (!targetDef && filteredDefs.length > 0) {
      targetDef = filteredDefs[0];
    }
    if (targetDef) {
      selectMacroDefinition(targetDef.id);
    }
  }

  // Called by app.js when performer changes the machine select
  function onMachineChanged(machineId) {
    state.activeMachine = machineId;
    state.selectedDefId = null;
    state.editDef = null;
    state.dirty = false;

    renderDefinitionList();
    renderMacroBuilderSection();

    var filteredDefs = getFilteredDefs();
    if (filteredDefs.length > 0) {
      selectMacroDefinition(filteredDefs[0].id);
    }
  }

  function getFilteredDefs() {
    if (!state.trackMachines || state.trackMachines.length === 0) return [];
    if (state.activeMachine) {
      return S.data.macroDefs.filter(function(d) {
        return d.machine === state.activeMachine;
      });
    }
    return S.data.macroDefs.filter(function(d) {
      return state.trackMachines.indexOf(d.machine) !== -1;
    });
  }

  // ─── Definition List (left sidebar — Macros tab) ──────────

  function renderDefinitionList() {
    var container = document.getElementById('definition-list');
    if (!container) return;

    var filteredDefs = getFilteredDefs();

    if (state.trackMachines.length === 0) {
      container.innerHTML =
        '<div class="empty-state" style="padding:1.5rem;">' +
        '<sl-icon name="hdd" style="font-size:1.5rem;"></sl-icon>' +
        '<p style="font-size:0.78rem;">Select a track to see macro definitions</p>' +
        '</div>';
      return;
    }

    var html = '';

    // Create New button
    html += '<button class="sidebar-action-btn" id="create-def-btn" style="margin:0.4rem 0.65rem;width:calc(100% - 1.3rem);">';
    html += '<sl-icon name="plus-lg"></sl-icon> Create New Definition';
    html += '</button>';

    if (filteredDefs.length === 0) {
      html += '<div style="padding:0.5rem 0.85rem;opacity:0.5;font-size:0.75rem;">No definitions yet</div>';
    }

    var F = window.TBD.factory;

    filteredDefs.forEach(function(def) {
      var isActive = state.selectedDefId === def.id;
      var isFactory = F && F.isFactoryDefinition(def.id);
      html += '<div class="preset-item' + (isActive ? ' active' : '') + '" data-def-id="' + S.esc(def.id) + '">';
      if (isFactory) {
        html += '<sl-icon name="lock" style="font-size:0.65rem;opacity:0.45;flex-shrink:0;margin-right:0.25rem;" title="Factory template — clone to edit"></sl-icon>';
      }
      html += '<span class="preset-item-name" title="' + S.esc(def.name || def.id) + '">' + S.esc(def.name || def.id) + '</span>';
      html += '<span class="preset-item-machine">' + S.esc(def.id) + '</span>';
      if (!isFactory) {
        html += '<button class="preset-item-delete" data-delete-def-id="' + S.esc(def.id) + '" title="Delete definition">';
        html += '<sl-icon name="trash3"></sl-icon>';
        html += '</button>';
      }
      html += '</div>';
    });

    container.innerHTML = html;
  }

  function setupDefinitionListEvents() {
    var container = document.getElementById('definition-list');
    if (!container) return;

    container.addEventListener('click', function(e) {
      if (e.target.id === 'create-def-btn' || e.target.closest('#create-def-btn')) {
        createNewDefinition();
        return;
      }
      // Handle delete button click
      var deleteBtn = e.target.closest('.preset-item-delete');
      if (deleteBtn) {
        e.stopPropagation();
        var defId = deleteBtn.getAttribute('data-delete-def-id');
        if (defId) deleteDefinition(defId);
        return;
      }
      var item = e.target.closest('.preset-item');
      if (!item) return;
      var defId = item.getAttribute('data-def-id');
      if (!defId) return;
      selectMacroDefinition(defId);
    });
  }

  // ─── Select / Edit a Macro Definition ────────────────────

  function selectMacroDefinition(defId) {
    var def = S.data.macroDefs.find(function(d) { return d.id === defId; });
    if (!def) return;

    state.selectedDefId = defId;
    state.editDef = JSON.parse(JSON.stringify(def));
    state.dirty = false;

    ensureGroupStructure(state.editDef);

    // Update list active state
    document.querySelectorAll('#definition-list .preset-item').forEach(function(item) {
      item.classList.toggle('active', item.getAttribute('data-def-id') === defId);
    });

    renderMacroBuilderSection();
  }

  function createNewDefinition() {
    var defaultMachine = state.activeMachine || (state.trackMachines.length > 0 ? state.trackMachines[0] : '');

    state.selectedDefId = null;
    state.editDef = {
      id: '',
      name: '',
      machine: defaultMachine,
      volmult: 1.0,
      groups: [],
      mapping: [],
    };
    ensureGroupStructure(state.editDef);
    state.dirty = true;

    document.querySelectorAll('#definition-list .preset-item').forEach(function(item) {
      item.classList.remove('active');
    });

    renderMacroBuilderSection();
  }

  function ensureGroupStructure(def) {
    if (!def.groups) def.groups = [];
    while (def.groups.length < 6) {
      def.groups.push({
        name: 'Page ' + (def.groups.length + 1),
        parameters: [],
      });
    }
    var runningIdx = 0;
    def.groups.forEach(function(group) {
      if (!group.parameters) group.parameters = [];
      group.parameters.forEach(function(p) {
        if (p.idx === undefined) {
          p.idx = runningIdx;
        }
        runningIdx = Math.max(runningIdx, p.idx + 1);
      });
    });
    if (!def.mapping) def.mapping = [];
  }

  /**
   * Re-index all parameters to contiguous 0..N-1 and update mapping src references.
   * Call after removing knobs, on import, or before save to ensure clean idx values.
   */
  function reindexParameters(def) {
    var oldToNew = {};
    var newIdx = 0;
    (def.groups || []).forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        oldToNew[p.idx] = newIdx;
        p.idx = newIdx;
        newIdx++;
      });
    });
    // Update mapping src references
    (def.mapping || []).forEach(function(m) {
      (m.add || []).forEach(function(a) {
        if (oldToNew[a.src] !== undefined) {
          a.src = oldToNew[a.src];
        }
      });
    });
  }

  /**
   * Produce a clean copy of a definition for saving/export.
   * - Re-indexes parameters to contiguous 0..N-1
   * - Strips empty trailing groups (keeps only groups that have parameters)
   * - Removes curve:'linear' from parameters (linear is the default)
   */
  function cleanDefinitionForSave(def) {
    var clean = JSON.parse(JSON.stringify(def));
    // Validate and clamp volmult
    var v = parseFloat(clean.volmult);
    if (isNaN(v) || v < 0.1) v = 1.0;
    if (v > 4.0) v = 4.0;
    clean.volmult = Math.round(v * 10) / 10;
    // Strip empty trailing groups
    while (clean.groups.length > 0 &&
           (!clean.groups[clean.groups.length - 1].parameters ||
            clean.groups[clean.groups.length - 1].parameters.length === 0)) {
      clean.groups.pop();
    }
    // Re-index to close any gaps from removed knobs
    reindexParameters(clean);
    // Remove default curve from parameters
    clean.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        if (p.curve === 'linear') delete p.curve;
      });
    });
    // Enforce key order: id, name, machine, volmult, groups, mapping, then rest
    var ordered = {};
    ['id', 'name', 'machine', 'volmult', 'groups', 'mapping'].forEach(function(k) {
      if (clean[k] !== undefined) ordered[k] = clean[k];
    });
    Object.keys(clean).forEach(function(k) {
      if (!(k in ordered)) ordered[k] = clean[k];
    });
    return ordered;
  }

  // ─── Definition Header (above sub-tabs) ───────────────

  // Definition header is now rendered inline in the track-info-bar by performer.js.
  // This function just keeps the legacy container hidden.
  function renderDefHeader() {
    var container = document.getElementById('macro-def-header');
    if (container) {
      container.innerHTML = '';
      container.classList.add('hidden');
    }
  }

  // ─── Sound Presets Section (Knob Preview extras) ─────────

  function renderSoundPresetsSection() {
    var container = document.getElementById('knob-preview-extras');
    if (!container) return;

    if (!state.editDef) {
      container.innerHTML = '';
      return;
    }

    var html = '<details class="dsp-collapsible" open style="margin-top:1rem;">';
    html += '<summary class="dsp-collapsible-summary">';
    html += '<sl-icon name="music-note-beamed" style="font-size:0.7rem;"></sl-icon> Presets using this Definition';
    html += '</summary>';
    html += '<div class="dsp-collapsible-body">';
    html += renderSoundPresetsForDef(state.editDef);
    html += '</div></details>';

    container.innerHTML = html;
    setupSoundPresetsSectionEvents(container);
  }

  function setupSoundPresetsSectionEvents(container) {
    var createPresetBtn = container.querySelector('.create-preset-btn');
    if (createPresetBtn) {
      createPresetBtn.addEventListener('click', function() {
        createSoundPresetForDef();
      });
    }
    container.querySelectorAll('.save-preset-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        saveEditedPreset(btn.getAttribute('data-preset-id'), container);
      });
    });
    container.querySelectorAll('.delete-preset-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        deleteSoundPreset(btn.getAttribute('data-preset-id'));
      });
    });

    // Sync slider ↔ number input for preset param values
    container.querySelectorAll('.preset-slider-input').forEach(function(slider) {
      slider.addEventListener('input', function() {
        var card = slider.closest('.sp-card');
        if (!card) return;
        var pid = slider.getAttribute('data-preset-id');
        var idx = slider.getAttribute('data-value-idx');
        var numInput = card.querySelector('.preset-value-input[data-preset-id="' + pid + '"][data-value-idx="' + idx + '"]');
        if (numInput) numInput.value = slider.value;
      });
    });
    container.querySelectorAll('.preset-value-input').forEach(function(numInput) {
      numInput.addEventListener('input', function() {
        var card = numInput.closest('.sp-card');
        if (!card) return;
        var pid = numInput.getAttribute('data-preset-id');
        var idx = numInput.getAttribute('data-value-idx');
        var slider = card.querySelector('.preset-slider-input[data-preset-id="' + pid + '"][data-value-idx="' + idx + '"]');
        if (slider) slider.value = numInput.value;
      });
    });
  }

  // ─── Macro Builder Section (center panel) ────────────────

  function renderMacroBuilderSection() {
    var container = document.getElementById('macro-builder-section');
    if (!container) return;

    // Also render the def header and sound presets sections
    renderDefHeader();
    renderSoundPresetsSection();

    // Sync performer's Knob Preview with current editDef
    if (state.editDef && window.TBD.performer && window.TBD.performer.setMacroDef) {
      window.TBD.performer.setMacroDef(state.editDef);
    }

    if (!state.editDef) {
      container.innerHTML =
        '<div class="empty-state" id="mapping-empty">' +
        '<sl-icon name="table"></sl-icon>' +
        '<h3>Select a Macro Definition</h3>' +
        '<p>Pick a definition from the sidebar to edit how DSP parameters are exposed as performer knobs.</p>' +
        '</div>';
      return;
    }

    var def = state.editDef;
    var machineInfo = S.getMachineInfo(def.machine);
    var machineParams = machineInfo ? (machineInfo.parameters || []) : [];

    var html = '';

    // 1:1 Map button at top of macro builder
    html += '<div class="mb-top-actions">';
    html += '<button class="mapping-btn btn-1to1" title="Auto-create a 1:1 mapping from all machine CCs">1:1 Map</button>';
    html += '</div>';

    // Macro Builder description
    html += '<div class="tab-description">';
    html += '<sl-icon name="info-circle"></sl-icon>';
    html += 'Define knobs (up to 6 pages \u00d7 4 knobs) and map them to DSP parameters. Drag knobs to preview. The colored dot on each range track shows the current computed CC value.';
    html += '</div>';

    // DSP Reference (collapsible, at top of builder for quick access)
    if (machineInfo && machineParams.length > 0) {
      html += '<details class="dsp-collapsible dsp-ref-top" style="margin-bottom:0.75rem;">';
      html += '<summary class="dsp-collapsible-summary">';
      html += '<sl-icon name="hdd" style="font-size:0.7rem;"></sl-icon> DSP Reference: ' + S.esc(machineInfo.name);
      html += '</summary>';
      html += '<div class="dsp-collapsible-body">';
      html += renderDSPList(machineInfo, machineParams);
      html += '</div></details>';
    }

    // Macro builder pages and mappings
    html += renderMacroBuilder(def, machineParams);

    container.innerHTML = html;
    setupMappingEditorEvents(container);
  }

  // ─── DSP Reference List (inline collapsible) ─────────────

  function renderDSPList(machineInfo, machineParams) {
    var mappedCtrls = {};
    if (state.editDef && state.editDef.mapping) {
      state.editDef.mapping.forEach(function(m) {
        mappedCtrls[m.ctrl] = true;
      });
    }

    var html = '';
    machineParams.forEach(function(p) {
      var isMapped = mappedCtrls[p.ctrl] || false;
      html += '<div class="dsp-param-row' + (isMapped ? ' mapped' : '') + '" data-ctrl="' + p.ctrl + '">';
      html += '<span class="dsp-param-index">CC ' + p.ctrl + '</span>';
      html += '<span class="dsp-param-name">' + S.esc(p.name) + '</span>';
      html += '<span class="dsp-param-value">def: ' + (p.def || 0) + '</span>';
      if (isMapped) {
        html += '<sl-icon name="check2-square" class="dsp-param-mapped-icon" title="Mapped"></sl-icon>';
      }
      html += '</div>';
    });
    return html;
  }

  // ── Render: Macro Builder (merged Parameter Groups + Output Mappings) ──

  function renderMacroBuilder(def, machineParams) {
    var mappings = def.mapping || [];
    var DH = window.TBD && window.TBD.displayHints;
    var html = '';

    // Build lookup maps
    var paramsByIdx = {};
    def.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        paramsByIdx[p.idx] = p;
      });
    });

    var ccLookup = {};
    machineParams.forEach(function(p) {
      ccLookup[p.ctrl] = p;
    });

    // ── Group mappings by source knob ──
    var paramMappings = {};
    var constants = [];
    var multiSourceMappings = [];

    mappings.forEach(function(m, mi) {
      var sources = m.add || [];
      if (sources.length === 0) {
        constants.push({ mi: mi, mapping: m });
      } else if (sources.length === 1) {
        var src = sources[0].src;
        if (!paramMappings[src]) paramMappings[src] = [];
        paramMappings[src].push({ mi: mi, ai: 0, mapping: m });
      } else {
        multiSourceMappings.push({ mi: mi, mapping: m });
        sources.forEach(function(a, ai) {
          if (!paramMappings[a.src]) paramMappings[a.src] = [];
          paramMappings[a.src].push({ mi: mi, ai: ai, mapping: m });
        });
      }
    });

    function fmtCC(ctrl) {
      return 'CC\u2009' + String(ctrl).padStart(2, '0');
    }

    function getSemanticInfo(ctrl, rangeLow, rangeHigh) {
      var mp = ccLookup[ctrl];
      if (!mp || !DH) return { unit: '', rangeStr: '', hint: null };
      var paramId = (def.machine || '') + '_' + (mp.id || '').replace(/-/g, '_');
      var hint = DH.resolveHint(paramId, mp.name, mp);
      if (!hint) return { unit: '', rangeStr: '', hint: null };
      var physLow = DH.rawToDisplay(rangeLow, 0, 127, hint);
      var physHigh = DH.rawToDisplay(rangeHigh, 0, 127, hint);
      var fmtLow = DH.formatDisplayValue(physLow, hint);
      var fmtHigh = DH.formatDisplayValue(physHigh, hint);
      return {
        unit: hint.unit || '',
        rangeStr: fmtLow + ' \u2192 ' + fmtHigh,
        hint: hint,
        scale: hint.scale || 'lin'
      };
    }

    function renderKnob(paramIdx, param, size, color) {
      var val = param ? (param.def || 0) : 0;
      var mn = param ? (param.min || 0) : 0;
      var mx = param ? (param.max || 127) : 127;
      var name = param ? (param.name || ('P' + paramIdx)) : ('P' + paramIdx);
      var h = '<div class="om-knob om-knob-interactive" data-value="' + val + '" data-min="' + mn + '" data-max="' + mx + '" data-idx="' + paramIdx + '" data-color="' + color + '" title="' + S.esc(name) + ' \u2014 drag up/down">';
      h += S.renderKnobSVG({ value: val, min: mn, max: mx, color: color, size: size });
      h += '</div>';
      return h;
    }

    function computeValueDot(param, mapping, ai) {
      var addEntry = mapping.add[ai];
      if (!addEntry) return null;
      var is14 = mapping.bits === 14;
      var maxCC = is14 ? 16383 : 127;
      var start = mapping.start || 0;
      var mul = addEntry.mul || 1;
      var div = addEntry.div || 1;
      var paramVal = param ? (param.def || 0) : 0;
      var ccVal = start + Math.round(paramVal * mul / div);
      ccVal = Math.max(0, Math.min(maxCC, ccVal));
      return { cc: ccVal, maxCC: maxCC };
    }

    function renderCCRow(mi, ai, m, addEntry) {
      var ctrl = m.ctrl;
      var mp = ccLookup[ctrl];
      var ccName = mp ? mp.name : '?';
      var is14 = m.bits === 14;
      var maxCC = is14 ? 16383 : 127;
      var range = sourceToRange(m, ai);
      var curve = addEntry.curve || 'linear';
      var lowPct = range.low / maxCC * 100;
      var highPct = range.high / maxCC * 100;
      var sem = getSemanticInfo(ctrl, range.low, range.high);

      var srcParam = paramsByIdx[addEntry.src];
      var dot = computeValueDot(srcParam, m, ai);

      var r = '';
      r += '<div class="om-cc-row" data-mapping-idx="' + mi + '" data-add="' + ai + '">';
      r += '<span class="om-cc-label">' + fmtCC(ctrl) + '</span>';
      r += '<span class="om-cc-name">' + S.esc(ccName) + '</span>';
      r += '<input type="number" class="mapping-input om-range-low' + (is14 ? ' is-14bit' : '') + '" value="' + range.low + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" data-add="' + ai + '" title="Low' + (is14 ? ' (0\u201316383)' : ' (0\u2013127)') + '" />';
      r += '<div class="om-range-track" data-mapping="' + mi + '" data-add="' + ai + '">';
      r += '<div class="om-range-fill" style="left:' + lowPct + '%;width:' + (highPct - lowPct) + '%"></div>';
      r += '<div class="om-range-thumb om-thumb-low" style="left:' + lowPct + '%" data-mapping="' + mi + '" data-add="' + ai + '"></div>';
      r += '<div class="om-range-thumb om-thumb-high" style="left:' + highPct + '%" data-mapping="' + mi + '" data-add="' + ai + '"></div>';
      if (dot) {
        var dotPct = (dot.maxCC > 0) ? (dot.cc / dot.maxCC * 100) : 0;
        r += '<div class="om-value-dot" style="left:' + dotPct + '%" data-mapping="' + mi + '" data-add="' + ai + '" title="Current CC value: ' + dot.cc + '"></div>';
      }
      r += '</div>';
      r += '<input type="number" class="mapping-input om-range-high' + (is14 ? ' is-14bit' : '') + '" value="' + range.high + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" data-add="' + ai + '" title="High' + (is14 ? ' (0\u201316383)' : ' (0\u2013127)') + '" />';
      r += '<select class="mapping-select om-curve-select" data-mapping="' + mi + '" data-add="' + ai + '" title="Response curve">';
      ['linear','log','exp','scurve'].forEach(function(c) {
        r += '<option value="' + c + '"' + (curve === c ? ' selected' : '') + '>' + c + '</option>';
      });
      r += '</select>';
      r += '<label class="om-bit-toggle" title="Enable 14-bit CC (0\u201316383) for higher precision">';
      r += '<input type="checkbox" class="om-14bit-check" data-mapping="' + mi + '"' + (is14 ? ' checked' : '') + ' />';
      r += '<span>14-bit</span>';
      r += '</label>';
      r += '<button class="mapping-remove-btn remove-mapping-btn" data-mapping="' + mi + '" title="Remove this CC mapping">\u00d7</button>';
      r += '</div>';

      if (sem.rangeStr) {
        r += '<div class="om-semantic-row">';
        r += '<span class="om-semantic-range">' + sem.rangeStr + '</span>';
        if (sem.scale === 'log' && curve === 'linear') {
          r += '<a class="om-scale-hint om-scale-fix" href="#" data-mapping="' + mi + '" data-add="' + ai + '" title="This DSP parameter has a logarithmic scale. Click to switch the curve to log.">\ud83d\udca1 use log curve</a>';
        }
        r += '</div>';
      }

      return r;
    }

    // ── Render page sections (groups) ──
    def.groups.forEach(function(group, gi) {
      html += '<div class="mb-page-section" data-group-idx="' + gi + '">';
      html += '<div class="mb-page-header">';
      html += '<span class="mb-page-icon"><sl-icon name="table" style="font-size:0.7rem;"></sl-icon></span>';
      html += '<span class="mb-page-label">Page ' + (gi + 1) + '</span>';
      html += '<input class="mapping-input mb-group-name" value="' + S.esc(group.name) + '" data-group="' + gi + '" placeholder="Name (optional)" />';
      html += '<span class="mb-page-info">' + (group.parameters || []).length + '/4 knobs</span>';
      html += '<div class="om-card-spacer"></div>';
      if ((group.parameters || []).length < 4) {
        html += '<button class="mapping-add-btn mb-add-knob-btn" data-group="' + gi + '" title="Add a new knob parameter">+ Add Knob</button>';
      }
      html += '</div>';

      html += '<div class="mb-page-content">';
      (group.parameters || []).forEach(function(param, pi) {
        var paramIdx = param.idx;
        var entries = paramMappings[paramIdx] || [];
        var isMacro = entries.length >= 2;
        var knobColor = isMacro ? 'macro' : 'normal';

        html += '<div class="om-knob-card' + (isMacro ? ' is-macro' : '') + '" data-group="' + gi + '" data-param="' + pi + '" data-param-idx="' + paramIdx + '">';
        html += '<div class="om-knob-header">';
        html += '<span class="om-drag-handle" title="Drag to reorder">\u2AF6</span>';
        html += '<span class="om-knob-badge">Knob ' + (pi + 1) + '</span>';
        html += '<div class="om-knob-cell">';
        html += '<input class="mapping-input mb-param-name" value="' + S.esc(param.name) + '" data-group="' + gi + '" data-param="' + pi + '" placeholder="Knob name" />';
        html += renderKnob(paramIdx, param, 64, knobColor);
        html += '<span class="om-knob-value' + (isMacro ? ' is-macro' : '') + '">' + (param.def || 0) + '</span>';
        html += '</div>';
        html += '<div class="om-card-spacer"></div>';
        if (isMacro) {
          html += '<sl-badge class="om-macro-badge" variant="warning" size="small">MACRO \u00b7 ' + entries.length + '</sl-badge>';
        }
        html += '<select class="mapping-select mapping-add-cc-for-knob" data-src-idx="' + paramIdx + '" title="Map this knob to another CC">';
        html += '<option value="">+ map to CC\u2026</option>';
        machineParams.forEach(function(mp) {
          html += '<option value="' + mp.ctrl + '">' + fmtCC(mp.ctrl) + ' ' + S.esc(mp.name) + '</option>';
        });
        html += '</select>';
        html += '<button class="mapping-remove-btn mb-remove-knob-btn" data-group="' + gi + '" data-param="' + pi + '" data-param-idx="' + paramIdx + '" title="Remove this knob">\u00d7</button>';
        html += '</div>';

        // Properties row
        html += '<div class="mb-props-row">';
        html += '<label class="mb-prop"><span>def</span><input type="number" class="mapping-input mb-prop-def" value="' + (param.def || 0) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>min</span><input type="number" class="mapping-input mb-prop-min" value="' + (param.min || 0) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>max</span><input type="number" class="mapping-input mb-prop-max" value="' + (param.max || 127) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>res</span><input type="number" class="mapping-input mb-prop-res" value="' + (param.res || 64) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>ui</span><select class="mapping-select mb-prop-ui" data-group="' + gi + '" data-param="' + pi + '">';
        ['bignum', 'slider', 'toggle', 'selector', 'knob', 'freq', 'midinote', 'shape', 'shape2', 'shape3', 'noise', 'distortion', 'envattack', 'envdecay', 'envamount', 'filtercutoff', 'filterq', 'filtertype', 'samplebank', 'sampleslice', 'sampleoffset'].forEach(function(ui) {
          html += '<option value="' + ui + '"' + (param.ui === ui ? ' selected' : '') + '>' + ui + '</option>';
        });
        html += '</select></label>';
        html += '</div>';

        // CC mapping rows
        html += '<div class="om-knob-body">';
        if (entries.length === 0) {
          html += '<div class="mb-no-mappings">No CC mappings \u2014 use "+ map to CC\u2026" above</div>';
        }
        entries.forEach(function(entry) {
          html += renderCCRow(entry.mi, entry.ai, entry.mapping, entry.mapping.add[entry.ai]);
        });
        html += '</div>';
        html += '</div>';
      });

      if (!group.parameters || group.parameters.length === 0) {
        html += '<div class="mb-empty-group">No knobs in this page \u2014 click "+ Add Knob" to create one</div>';
      }
      html += '</div>';
      html += '</div>';
    });

    // Add Page button
    if (def.groups.length < 6) {
      html += '<div style="text-align:center;margin:0.6rem 0;">';
      html += '<button class="mapping-add-btn mb-add-group-btn" title="Add a new knob page (up to 6)">+ Add Page</button>';
      html += '</div>';
    }

    // Constants (locked parameters)
    if (constants.length > 0) {
      html += '<div class="om-card om-constants-card">';
      html += '<div class="om-card-header">';
      html += '<sl-icon name="lock" style="font-size:0.85rem;color:var(--sl-color-neutral-500);"></sl-icon>';
      html += '<span class="om-cc-name" style="font-weight:700;">Locked Parameters</span>';
      html += '<div class="om-card-spacer"></div>';
      html += '<sl-badge variant="neutral" size="small">' + constants.length + ' locked</sl-badge>';
      html += '</div>';
      html += '<div class="om-card-body">';

      constants.forEach(function(entry) {
        var m = entry.mapping;
        var mi = entry.mi;
        var ctrl = m.ctrl;
        var mp = ccLookup[ctrl];
        var ccName = mp ? mp.name : '?';
        var is14 = m.bits === 14;
        var maxCC = is14 ? 16383 : 127;
        var fixedVal = m.start || 0;
        var fixedPct = fixedVal / maxCC * 100;
        var sem = getSemanticInfo(ctrl, fixedVal, fixedVal);

        html += '<div class="om-constant-row" data-mapping-idx="' + mi + '">';
        html += '<span class="om-cc-label">' + fmtCC(ctrl) + '</span>';
        html += '<span class="om-cc-name">' + S.esc(ccName) + '</span>';
        html += '<input type="number" class="mapping-input om-fixed-input' + (is14 ? ' is-14bit' : '') + '" value="' + fixedVal + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" />';
        html += '<div class="om-range-track" title="Fixed CC value">';
        html += '<div class="om-range-mark" style="left:' + fixedPct + '%"></div>';
        html += '</div>';
        if (sem.rangeStr) {
          html += '<span class="om-semantic-val">' + S.esc(sem.rangeStr.split(' \u2192 ')[0]) + '</span>';
        }
        html += '<label class="om-bit-toggle" title="Enable 14-bit CC">';
        html += '<input type="checkbox" class="om-14bit-check" data-mapping="' + mi + '"' + (is14 ? ' checked' : '') + ' />';
        html += '<span>14-bit</span>';
        html += '</label>';
        html += '<button class="mapping-remove-btn remove-mapping-btn" data-mapping="' + mi + '" title="Remove"><sl-icon name="x-lg"></sl-icon></button>';
        html += '</div>';
      });

      html += '</div>';
      html += '</div>';
    }

    // Unmapped CC add dropdown
    html += '<div style="margin-top:0.5rem;text-align:center;">';
    html += '<select class="mapping-select add-unmapped-cc-select" title="Add a constant (locked) CC mapping">';
    html += '<option value="">+ add locked CC\u2026</option>';
    machineParams.forEach(function(mp) {
      var alreadyMapped = mappings.some(function(m) { return m.ctrl === mp.ctrl; });
      if (!alreadyMapped) {
        html += '<option value="' + mp.ctrl + '">' + fmtCC(mp.ctrl) + ' ' + S.esc(mp.name) + '</option>';
      }
    });
    html += '</select>';
    html += '</div>';

    return html;
  }

  // ── Helpers: range ↔ start/mul/div conversion ──

  function sourceToRange(mapping, addIdx) {
    var add = (mapping.add || [])[addIdx];
    var maxCC = (mapping.bits === 14) ? 16383 : 127;
    if (!add) return { low: mapping.start || 0, high: mapping.start || 0 };
    var mul = add.mul || 1;
    var div = add.div || 1;
    var singleSource = (mapping.add || []).length === 1;
    if (singleSource) {
      var low = mapping.start || 0;
      var high = low + Math.round(maxCC * mul / div);
      return { low: Math.max(0, Math.min(maxCC, low)), high: Math.max(0, Math.min(maxCC, high)) };
    } else {
      var maxC = Math.round(maxCC * mul / div);
      return { low: 0, high: Math.max(0, Math.min(maxCC, maxC)), base: mapping.start || 0 };
    }
  }

  function rangeToSource(mapping, addIdx, low, high) {
    var maxCC = (mapping.bits === 14) ? 16383 : 127;
    var singleSource = (mapping.add || []).length === 1;
    if (singleSource) {
      mapping.start = low;
      mapping.add[addIdx].mul = high - low;
      mapping.add[addIdx].div = maxCC;
    } else {
      mapping.add[addIdx].mul = high;
      mapping.add[addIdx].div = maxCC;
    }
  }

  // ─── Sortable: Knob reordering within pages ──────────────

  var knobSortableInstances = [];

  function setupKnobSortables(container) {
    knobSortableInstances.forEach(function(s) { try { s.destroy(); } catch(e) {} });
    knobSortableInstances = [];
    if (typeof Sortable === 'undefined') return;

    container.querySelectorAll('.mb-page-content').forEach(function(pageContent) {
      var section = pageContent.closest('.mb-page-section');
      if (!section) return;
      var gi = parseInt(section.getAttribute('data-group-idx'), 10);

      var inst = Sortable.create(pageContent, {
        handle: '.om-drag-handle',
        animation: 150,
        ghostClass: 'sortable-ghost',
        chosenClass: 'sortable-chosen',
        draggable: '.om-knob-card',
        onEnd: function(evt) {
          if (evt.oldIndex !== evt.newIndex) {
            reorderKnobInGroup(gi, evt.oldIndex, evt.newIndex);
          }
        },
      });
      knobSortableInstances.push(inst);
    });
  }

  function reorderKnobInGroup(groupIdx, oldPos, newPos) {
    if (!state.editDef) return;
    var group = state.editDef.groups[groupIdx];
    if (!group || !group.parameters) return;
    var moved = group.parameters.splice(oldPos, 1)[0];
    group.parameters.splice(newPos, 0, moved);
    state.dirty = true;
    renderMacroBuilderSection();
  }

  // ── Render: Sound Presets ──

  function renderSoundPresetsForDef(def) {
    var matching = S.data.soundPresets.filter(function(p) {
      return p.macro === def.id;
    });

    var html = '';
    html += '<div class="mapping-output-header">';
    html += '<span>Presets using \u201c' + S.esc(def.name || def.id) + '\u201d (' + matching.length + ')</span>';
    html += '<button class="mapping-add-btn create-preset-btn" title="Create new sound preset">+ New Preset</button>';
    html += '</div>';

    if (matching.length === 0) {
      html += '<div style="text-align:center;opacity:0.4;padding:1rem;">No sound presets reference this definition</div>';
      return html;
    }

    // Build page-grouped param structure for labeling values
    var pageParams = [];
    def.groups.forEach(function(g, gi) {
      var params = (g.parameters || []);
      if (params.length > 0) {
        pageParams.push({
          pageName: g.name || ('Page ' + (gi + 1)),
          params: params,
        });
      }
    });

    var FP = window.TBD.factory;

    matching.forEach(function(preset) {
      var isFactory = FP && FP.isFactoryPreset(preset.id);
      html += '<div class="sp-card" data-preset-id="' + S.esc(preset.id) + '">';

      // Card header: labeled fields + action buttons
      html += '<div class="sp-card-header">';
      html += '<div class="sp-card-fields">';
      if (isFactory) {
        html += '<sl-icon name="lock" style="font-size:0.6rem;opacity:0.45;margin-right:0.25rem;" title="Factory preset — read-only"></sl-icon>';
      }
      html += '<label class="sp-field-label">Preset Name</label>';
      html += '<input class="mapping-input sp-name-input preset-name-input" value="' + S.esc(preset.name || preset.id) + '" data-preset-id="' + S.esc(preset.id) + '" placeholder="e.g. Fat Punch"' + (isFactory ? ' readonly' : '') + ' />';
      html += '<label class="sp-field-label" style="margin-left:0.5rem;">Group</label>';
      html += '<input class="mapping-input sp-group-input preset-group-input" value="' + S.esc(preset.group || '') + '" data-preset-id="' + S.esc(preset.id) + '" placeholder="e.g. User"' + (isFactory ? ' readonly' : '') + ' />';
      html += '</div>';
      html += '<div class="sp-card-actions">';
      if (!isFactory) {
        html += '<button class="mapping-btn save-preset-btn" data-preset-id="' + S.esc(preset.id) + '" title="Save changes"><sl-icon name="floppy" style="font-size:0.7rem;"></sl-icon> Save</button>';
        html += '<button class="mapping-btn delete-preset-btn" data-preset-id="' + S.esc(preset.id) + '" title="Delete preset"><sl-icon name="trash3" style="font-size:0.7rem;"></sl-icon> Delete</button>';
      }
      html += '</div>';
      html += '</div>';

      // Card body: values organized by page with knob labels
      html += '<div class="sp-card-values">';
      pageParams.forEach(function(page) {
        html += '<div class="sp-page-group">';
        html += '<span class="sp-page-label">' + S.esc(page.pageName) + '</span>';
        html += '<div class="sp-params">';
        page.params.forEach(function(param) {
          var val = preset.values && preset.values[param.idx] !== undefined ? preset.values[param.idx] : '';
          var pMin = param.min !== undefined ? param.min : 0;
          var pMax = param.max !== undefined ? param.max : 127;
          var numVal = val !== '' ? val : param.def || 0;
          html += '<div class="sp-param">';
          html += '<span class="sp-param-name">' + S.esc(param.name || ('P' + param.idx)) + '</span>';
          html += '<input class="sp-param-slider preset-slider-input" type="range" min="' + pMin + '" max="' + pMax + '" value="' + numVal + '" data-preset-id="' + S.esc(preset.id) + '" data-value-idx="' + param.idx + '" />';
          html += '<input class="sp-param-value preset-value-input" type="number" min="' + pMin + '" max="' + pMax + '" value="' + val + '" data-preset-id="' + S.esc(preset.id) + '" data-value-idx="' + param.idx + '" />';
          html += '<span class="sp-param-range">' + pMin + '–' + pMax + '</span>';
          html += '</div>';
        });
        html += '</div>';
        html += '</div>';
      });
      html += '</div>';

      html += '</div>'; // .sp-card
    });

    return html;
  }

  // ─── Mapping Editor Events ───────────────────────────────

  function setupMappingEditorEvents(container) {
    // 1:1 mapping button
    var btn1to1 = container.querySelector('.btn-1to1');
    if (btn1to1) {
      btn1to1.addEventListener('click', function() {
        createOneToOneMapping();
      });
    }

    // Group name changes
    container.querySelectorAll('.mb-group-name').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        if (state.editDef && state.editDef.groups[gi]) {
          // Rompler macros: first page must always be called SAMPLE
          if (gi === 0 && state.editDef.machine === 'ro' && input.value !== 'SAMPLE') {
            S.toast('Rompler macros require the first page to be named SAMPLE', 'warning', 3000);
            input.value = 'SAMPLE';
            return;
          }
          state.editDef.groups[gi].name = input.value;
          state.dirty = true;
        }
      });
    });

    // Parameter property changes
    container.querySelectorAll('.mb-param-name').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        var pi = parseInt(input.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (param) { param.name = input.value; state.dirty = true; }
      });
    });
    container.querySelectorAll('.mb-prop-def, .mb-prop-min, .mb-prop-max, .mb-prop-res').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        var pi = parseInt(input.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (!param) return;
        var v = parseInt(input.value, 10) || 0;
        if (input.classList.contains('mb-prop-def')) param.def = v;
        if (input.classList.contains('mb-prop-min')) param.min = v;
        if (input.classList.contains('mb-prop-max')) param.max = v;
        if (input.classList.contains('mb-prop-res')) param.res = v;
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });
    container.querySelectorAll('.mb-prop-ui').forEach(function(select) {
      select.addEventListener('change', function() {
        var gi = parseInt(select.getAttribute('data-group'), 10);
        var pi = parseInt(select.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (param) { param.ui = select.value; state.dirty = true; }
      });
    });

    // Add knob
    container.querySelectorAll('.mb-add-knob-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        if ((state.editDef.groups[gi].parameters || []).length >= 4) return;
        var maxIdx = -1;
        state.editDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (p.idx > maxIdx) maxIdx = p.idx;
          });
        });
        state.editDef.groups[gi].parameters.push({
          idx: maxIdx + 1,
          name: 'New Knob',
          def: 0, min: 0, max: 127, res: 64, ui: 'bignum',
        });
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Remove knob
    container.querySelectorAll('.mb-remove-knob-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        var pi = parseInt(btn.getAttribute('data-param'), 10);
        var paramIdx = parseInt(btn.getAttribute('data-param-idx'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        state.editDef.groups[gi].parameters.splice(pi, 1);
        if (state.editDef.mapping) {
          state.editDef.mapping = state.editDef.mapping.filter(function(m) {
            if (!m.add || m.add.length === 0) return true;
            m.add = m.add.filter(function(a) { return a.src !== paramIdx; });
            return m.add.length > 0 || (m.start !== undefined);
          });
        }
        // Re-index remaining parameters to close the gap
        reindexParameters(state.editDef);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Sortable
    setupKnobSortables(container);

    // Add page
    container.querySelectorAll('.mb-add-group-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        if (!state.editDef) return;
        if (state.editDef.groups.length >= 6) return;
        state.editDef.groups.push({
          name: 'Page ' + (state.editDef.groups.length + 1),
          parameters: [],
        });
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Interactive knob drag
    container.querySelectorAll('.om-knob-interactive').forEach(function(knob) {
      var paramIdx = parseInt(knob.getAttribute('data-idx'), 10);
      var min = parseInt(knob.getAttribute('data-min'), 10) || 0;
      var max = parseInt(knob.getAttribute('data-max'), 10) || 127;
      var startY = 0;
      var startVal = 0;

      knob.addEventListener('pointerdown', function(e) {
        e.preventDefault();
        knob.classList.add('dragging');
        startY = e.clientY;
        startVal = parseInt(knob.getAttribute('data-value'), 10) || 0;

        function onMove(ev) {
          var dy = startY - ev.clientY;
          var range = max - min;
          var sensitivity = range / 200;
          var newVal = Math.round(startVal + dy * sensitivity);
          newVal = Math.max(min, Math.min(max, newVal));

          knob.setAttribute('data-value', newVal);
          var color = knob.getAttribute('data-color') || 'normal';
          var size = knob.querySelector('.knob-svg') ? parseInt(knob.querySelector('.knob-svg').getAttribute('width'), 10) : 32;
          knob.innerHTML = S.renderKnobSVG({ value: newVal, min: min, max: max, color: color, size: size });

          var card = knob.closest('.om-knob-card');
          if (card) {
            var valEl = card.querySelector('.om-knob-value');
            if (valEl) valEl.textContent = newVal;
            var defInput = card.querySelector('.mb-prop-def');
            if (defInput) defInput.value = newVal;

            card.querySelectorAll('.om-value-dot').forEach(function(dot) {
              var mi = parseInt(dot.getAttribute('data-mapping'), 10);
              var ai = parseInt(dot.getAttribute('data-add'), 10);
              if (!state.editDef || !state.editDef.mapping[mi]) return;
              var mapping = state.editDef.mapping[mi];
              var addEntry = mapping.add && mapping.add[ai];
              if (!addEntry) return;
              var is14 = mapping.bits === 14;
              var maxCC = is14 ? 16383 : 127;
              var start = mapping.start || 0;
              var mul = addEntry.mul || 1;
              var div = addEntry.div || 1;
              var ccVal = start + Math.round(newVal * mul / div);
              ccVal = Math.max(0, Math.min(maxCC, ccVal));
              dot.style.left = (ccVal / maxCC * 100) + '%';
              dot.title = 'Current CC value: ' + ccVal;
            });
          }

          if (state.editDef) {
            state.editDef.groups.forEach(function(g) {
              (g.parameters || []).forEach(function(p) {
                if (p.idx === paramIdx) p.def = newVal;
              });
            });
            state.dirty = true;
          }
        }

        function onUp() {
          knob.classList.remove('dragging');
          document.removeEventListener('pointermove', onMove);
          document.removeEventListener('pointerup', onUp);
        }

        document.addEventListener('pointermove', onMove);
        document.addEventListener('pointerup', onUp);
      });
    });

    // Range low/high input changes
    container.querySelectorAll('.om-range-low, .om-range-high').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        var ai = parseInt(input.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var mapping = state.editDef.mapping[mi];
        var maxCC = (mapping.bits === 14) ? 16383 : 127;
        var row = input.closest('.om-cc-row, .om-source-row');
        var lowInput = row ? row.querySelector('.om-range-low') : null;
        var highInput = row ? row.querySelector('.om-range-high') : null;
        var low = Math.max(0, Math.min(maxCC, parseInt(lowInput ? lowInput.value : 0, 10) || 0));
        var high = Math.max(0, Math.min(maxCC, parseInt(highInput ? highInput.value : maxCC, 10) || 0));
        if (low > high) { var tmp = low; low = high; high = tmp; }
        rangeToSource(mapping, ai, low, high);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Curve select
    container.querySelectorAll('.om-curve-select').forEach(function(select) {
      select.addEventListener('change', function() {
        var mi = parseInt(select.getAttribute('data-mapping'), 10);
        var ai = parseInt(select.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var addEntry = state.editDef.mapping[mi].add[ai];
        if (addEntry) { addEntry.curve = select.value; state.dirty = true; }
      });
    });

    // Scale hint fix
    container.querySelectorAll('.om-scale-fix').forEach(function(link) {
      link.addEventListener('click', function(e) {
        e.preventDefault();
        var mi = parseInt(link.getAttribute('data-mapping'), 10);
        var ai = parseInt(link.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var addEntry = state.editDef.mapping[mi].add[ai];
        if (addEntry) { addEntry.curve = 'log'; state.dirty = true; renderMacroBuilderSection(); }
      });
    });

    // Fixed value inputs
    container.querySelectorAll('.om-fixed-input').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var maxCC = (state.editDef.mapping[mi].bits === 14) ? 16383 : 127;
        state.editDef.mapping[mi].start = Math.max(0, Math.min(maxCC, parseInt(input.value, 10) || 0));
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Multi-source base start
    container.querySelectorAll('.om-multi-card .mapping-start').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var maxCC = (state.editDef.mapping[mi].bits === 14) ? 16383 : 127;
        state.editDef.mapping[mi].start = Math.max(0, Math.min(maxCC, parseInt(input.value, 10) || 0));
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // 14-bit toggle
    container.querySelectorAll('.om-14bit-check').forEach(function(checkbox) {
      checkbox.addEventListener('change', function() {
        var mi = parseInt(checkbox.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var mapping = state.editDef.mapping[mi];
        if (checkbox.checked) {
          mapping.bits = 14;
        } else {
          delete mapping.bits;
          if (mapping.start > 127) mapping.start = 127;
          (mapping.add || []).forEach(function(a) { if (a.mul > 127) a.mul = 127; });
        }
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Range slider thumb drag
    container.querySelectorAll('.om-range-thumb').forEach(function(thumb) {
      thumb.addEventListener('pointerdown', function(e) {
        e.preventDefault();
        thumb.setPointerCapture(e.pointerId);
        thumb.classList.add('dragging');

        var mi = parseInt(thumb.getAttribute('data-mapping'), 10);
        var ai = parseInt(thumb.getAttribute('data-add'), 10);
        var isLow = thumb.classList.contains('om-thumb-low');
        var track = thumb.closest('.om-range-track');
        if (!track) return;
        var parentRow = thumb.closest('.om-cc-row, .om-source-row');

        function onMove(ev) {
          if (!state.editDef || !state.editDef.mapping[mi]) return;
          var mapping = state.editDef.mapping[mi];
          var maxCC = (mapping.bits === 14) ? 16383 : 127;
          var rect = track.getBoundingClientRect();
          var pct = (ev.clientX - rect.left) / rect.width;
          pct = Math.max(0, Math.min(1, pct));
          var ccVal = Math.round(pct * maxCC);
          var range = sourceToRange(mapping, ai);
          var low = range.low, high = range.high;
          if (isLow) { low = Math.min(ccVal, high); } else { high = Math.max(ccVal, low); }
          rangeToSource(mapping, ai, low, high);
          state.dirty = true;

          var newRange = sourceToRange(mapping, ai);
          var lowPct = newRange.low / maxCC * 100;
          var highPct = newRange.high / maxCC * 100;
          var fill = track.querySelector('.om-range-fill');
          var thumbLow = track.querySelector('.om-thumb-low');
          var thumbHigh = track.querySelector('.om-thumb-high');
          if (fill) { fill.style.left = lowPct + '%'; fill.style.width = (highPct - lowPct) + '%'; }
          if (thumbLow) thumbLow.style.left = lowPct + '%';
          if (thumbHigh) thumbHigh.style.left = highPct + '%';
          if (parentRow) {
            var lowInput = parentRow.querySelector('.om-range-low');
            var highInput = parentRow.querySelector('.om-range-high');
            if (lowInput) lowInput.value = newRange.low;
            if (highInput) highInput.value = newRange.high;
          }
        }

        function onUp() {
          thumb.classList.remove('dragging');
          thumb.removeEventListener('pointermove', onMove);
          thumb.removeEventListener('pointerup', onUp);
          renderMacroBuilderSection();
        }

        thumb.addEventListener('pointermove', onMove);
        thumb.addEventListener('pointerup', onUp);
      });
    });

    // Add CC mapping for knob
    container.querySelectorAll('.mapping-add-cc-for-knob').forEach(function(select) {
      select.addEventListener('change', function() {
        if (!state.editDef) return;
        var ctrl = parseInt(select.value, 10);
        var srcIdx = parseInt(select.getAttribute('data-src-idx'), 10);
        if (isNaN(ctrl) || isNaN(srcIdx)) return;
        var addEntry = { src: srcIdx, mul: 1, div: 1 };

        // Auto-fill name + ui type for this knob based on the CC being mapped
        var machine = state.editDef.machine;
        if (machine) {
          var machineInfo = S.getMachineInfo(machine);
          if (machineInfo && machineInfo.parameters) {
            var ccParam = machineInfo.parameters.find(function(p) { return p.ctrl === ctrl; });
            if (ccParam) {
              var uiInfo = lookupUiType(machine, ccParam.id);
              // Find the knob parameter and update its name, ui + curve
              state.editDef.groups.forEach(function(g) {
                (g.parameters || []).forEach(function(p) {
                  if (p.idx === srcIdx) {
                    // Auto-fill name if still generic (user can still rename later)
                    if (!p.name || p.name === 'New Knob' || /^CC\d+$/.test(p.name)) {
                      p.name = ccParam.name || ('CC' + ctrl);
                    }
                    p.ui = uiInfo.ui;
                    if (uiInfo.curve) { p.curve = uiInfo.curve; addEntry.curve = uiInfo.curve; }
                    else { delete p.curve; }
                  }
                });
              });
            }
          }
        }

        state.editDef.mapping.push({ ctrl: ctrl, start: 0, add: [addEntry] });
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Add unmapped CC as constant
    var unmappedSelect = container.querySelector('.add-unmapped-cc-select');
    if (unmappedSelect) {
      unmappedSelect.addEventListener('change', function() {
        if (!state.editDef) return;
        var ctrl = parseInt(unmappedSelect.value, 10);
        if (isNaN(ctrl)) return;
        state.editDef.mapping.push({ ctrl: ctrl, start: 0, add: [] });
        state.dirty = true;
        renderMacroBuilderSection();
      });
    }

    // Remove output mapping
    container.querySelectorAll('.remove-mapping-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var mi = parseInt(btn.getAttribute('data-mapping'), 10);
        if (!state.editDef) return;
        state.editDef.mapping.splice(mi, 1);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Add/remove source
    container.querySelectorAll('.mapping-add-src-select').forEach(function(select) {
      select.addEventListener('change', function() {
        var mi = parseInt(select.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var src = parseInt(select.value, 10);
        if (isNaN(src)) return;
        state.editDef.mapping[mi].add.push({ src: src, mul: 1, div: 1 });
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });
    container.querySelectorAll('.mapping-remove-src-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var mi = parseInt(btn.getAttribute('data-mapping'), 10);
        var ai = parseInt(btn.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        state.editDef.mapping[mi].add.splice(ai, 1);
        state.dirty = true;
        renderMacroBuilderSection();
      });
    });

    // Create preset and Save/delete preset handlers are now in setupSoundPresetsSectionEvents
  }

  // ─── Create 1:1 Mapping ──────────────────────────────────

  function createOneToOneMapping() {
    if (!state.editDef || !state.editDef.machine) {
      S.toast('Select a machine first', 'warning', 2000);
      return;
    }
    var machineInfo = S.getMachineInfo(state.editDef.machine);
    if (!machineInfo || !machineInfo.parameters || machineInfo.parameters.length === 0) {
      S.toast('Machine has no CC parameters', 'warning', 2000);
      return;
    }
    if (!confirm('This will replace all parameter groups and output mappings with a 1:1 mapping from all ' + machineInfo.parameters.length + ' CCs. Continue?')) {
      return;
    }

    var params = machineInfo.parameters;
    if (!state.editDef.id) { state.editDef.id = state.editDef.machine + '-allparams'; }
    if (!state.editDef.name) { state.editDef.name = (machineInfo.name || state.editDef.machine) + ' All params'; }

    var machine = state.editDef.machine;

    state.editDef.groups = [];
    var paramIdx = 0;
    var isRompler = machine === 'ro';
    for (var g = 0; g < 6 && paramIdx < params.length; g++) {
      var groupParams = [];
      for (var p = 0; p < 4 && paramIdx < params.length; p++) {
        var cc = params[paramIdx];
        var uiInfo = lookupUiType(machine, cc.id);
        var ccMax = (cc.type === 'nrpm') ? 16383 : 127;
        var paramObj = {
          idx: paramIdx,
          name: cc.name || ('CC' + cc.ctrl),
          def: cc.def || 0,
          min: 0, max: ccMax, res: Math.round(ccMax / 2), ui: uiInfo.ui,
        };
        if (uiInfo.curve) paramObj.curve = uiInfo.curve;
        groupParams.push(paramObj);
        paramIdx++;
      }
      var pageName = (isRompler && g === 0) ? 'SAMPLE' : 'Page ' + (g + 1);
      state.editDef.groups.push({ name: pageName, parameters: groupParams });
    }

    state.editDef.mapping = [];
    var allParams = [];
    state.editDef.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) { allParams.push(p); });
    });
    params.forEach(function(cc, i) {
      if (i < allParams.length) {
        var mapStart = 0;
        var mapMul = 1;
        var mapCurve = allParams[i].curve;
        if (isRompler) {
          // Attack/Decay need start:1 to avoid div-by-zero in envelope
          if (cc.id === 'attack' || cc.id === 'decay') mapStart = 1;
          // TSMode needs mul:64 for proper 0/1/2 integer mapping
          if (cc.id === 'tsmode') mapMul = 64;
        }
        var addEntry = { src: allParams[i].idx, mul: mapMul, div: 1 };
        if (mapCurve) addEntry.curve = mapCurve;
        var mapEntry = { ctrl: cc.ctrl, start: mapStart, add: [addEntry] };
        if (cc.type === 'nrpm') mapEntry.bits = 14;
        state.editDef.mapping.push(mapEntry);
      }
    });

    state.dirty = true;
    renderMacroBuilderSection();

    // Sync with performer for knob preview
    if (window.TBD.performer && window.TBD.performer.setMacroDef) {
      window.TBD.performer.setMacroDef(state.editDef);
    }

    S.toast('Created 1:1 mapping with ' + params.length + ' parameters', 'success', 2000);
  }

  // ─── Sound Preset CRUD ──────────────────────────────────

  function createSoundPresetForDef() {
    if (!state.editDef || !state.editDef.id) {
      S.toast('Save the definition first', 'warning', 2000);
      return;
    }

    var old = document.getElementById('create-preset-dialog');
    if (old) old.remove();

    var macroName = state.editDef.name || state.editDef.id;
    var defaultGroup = state.editDef.machine || 'User';
    var paramCount = 0;
    if (state.editDef.groups) {
      state.editDef.groups.forEach(function(g) { paramCount += (g.parameters || []).length; });
    }

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'create-preset-dialog';
    dialog.label = 'New Sound Preset';
    dialog.setAttribute('style', '--width:26rem;');

    var html = '';
    html += '<div style="display:flex;flex-direction:column;gap:0.75rem;">';
    html += '<sl-input id="new-preset-name" label="Preset Name" placeholder="e.g. Fat Bass" required autofocus></sl-input>';
    html += '<sl-input id="new-preset-group" label="Category / Group" value="' + S.esc(defaultGroup) + '" placeholder="e.g. User" help-text="Presets are grouped by this label in the sidebar"></sl-input>';
    html += '</div>';
    html += '<div style="margin-top:1rem;font-size:0.72rem;color:var(--sl-color-neutral-500);">';
    html += '<sl-icon name="info-circle" style="font-size:0.7rem;"></sl-icon> ';
    html += 'Creates a new preset with default values (' + paramCount + ' params) for the <strong>' + S.esc(macroName) + '</strong> macro.';
    html += '</div>';
    dialog.innerHTML = html;

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var createBtn = document.createElement('sl-button');
    createBtn.setAttribute('slot', 'footer');
    createBtn.setAttribute('variant', 'primary');
    createBtn.innerHTML = '<sl-icon name="plus-lg" slot="prefix"></sl-icon> Create Preset';
    createBtn.addEventListener('click', function() {
      var nameInput = dialog.querySelector('#new-preset-name');
      var groupInput = dialog.querySelector('#new-preset-group');
      var name = (nameInput.value || '').trim();
      var group = (groupInput.value || '').trim() || 'User';

      if (!name) {
        nameInput.setAttribute('help-text', 'Please enter a name');
        nameInput.focus();
        return;
      }

      var id = name.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');

      // Prevent overwriting factory presets
      var Fcheck = window.TBD.factory;
      if (Fcheck && Fcheck.isFactoryPreset(id)) {
        nameInput.setAttribute('help-text', 'This name matches a factory preset \u2014 choose a different name');
        nameInput.focus();
        return;
      }

      // Build a dense values array — fill any gaps with 0
      var maxIdx = -1;
      state.editDef.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) { if (p.idx > maxIdx) maxIdx = p.idx; });
      });
      var values = [];
      for (var vi = 0; vi <= maxIdx; vi++) values[vi] = 0;
      state.editDef.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) { values[p.idx] = p.def || 0; });
      });

      var preset = { id: id, name: name, group: group, macro: state.editDef.id, values: values };
      var jsonStr = JSON.stringify(preset, null, 2);
      var filePath = 'presets/' + id + '.json';

      createBtn.setAttribute('loading', '');
      fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: jsonStr,
      }).then(function(r) {
        if (!r.ok) throw new Error('HTTP ' + r.status);
        dialog.hide();
        S.toast('Created preset: ' + name, 'success', 2000);
        return S.reloadMacroData();
      }).then(function() {
        renderMacroBuilderSection();
      }).catch(function(err) {
        createBtn.removeAttribute('loading');
        S.toast('Create failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(createBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
  }

  function saveEditedPreset(presetId, container) {
    var F = window.TBD.factory;
    if (F && F.isFactoryPreset(presetId)) {
      S.toast('Factory presets are read-only', 'warning', 3000);
      return;
    }
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    if (!preset) { S.toast('Preset not found', 'danger', 2000); return; }

    var nameInput = container.querySelector('.preset-name-input[data-preset-id="' + presetId + '"]');
    if (nameInput) preset.name = nameInput.value;
    var groupInput = container.querySelector('.preset-group-input[data-preset-id="' + presetId + '"]');
    if (groupInput) preset.group = groupInput.value;
    container.querySelectorAll('.preset-value-input[data-preset-id="' + presetId + '"]').forEach(function(input) {
      var vi = parseInt(input.getAttribute('data-value-idx'), 10);
      preset.values[vi] = parseInt(input.value, 10) || 0;
    });

    var jsonStr = JSON.stringify(preset, null, 2);
    var filePath = 'presets/' + presetId + '.json';

    fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved preset: ' + preset.name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderMacroBuilderSection();
    }).catch(function(err) {
      S.toast('Save failed: ' + err.message, 'danger', 3000);
    });
  }

  function deleteSoundPreset(presetId) {
    var F = window.TBD.factory;
    if (F && F.isFactoryPreset(presetId)) {
      S.toast('Factory presets cannot be deleted', 'warning', 3000);
      return;
    }
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    var displayName = preset ? preset.name : presetId;

    var old = document.getElementById('delete-preset-dialog');
    if (old) old.remove();

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'delete-preset-dialog';
    dialog.label = 'Delete Sound Preset';
    dialog.setAttribute('style', '--width:24rem;');

    dialog.innerHTML = '<p style="font-size:0.85rem;margin:0;">Are you sure you want to delete <strong>' + S.esc(displayName) + '</strong>?</p>'
      + '<p style="font-size:0.75rem;color:var(--sl-color-neutral-500);margin:0.5rem 0 0;">This action cannot be undone.</p>';

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var deleteBtn = document.createElement('sl-button');
    deleteBtn.setAttribute('slot', 'footer');
    deleteBtn.setAttribute('variant', 'danger');
    deleteBtn.innerHTML = '<sl-icon name="trash3" slot="prefix"></sl-icon> Delete';
    deleteBtn.addEventListener('click', function() {
      deleteBtn.setAttribute('loading', '');
      var filePath = 'presets/' + presetId + '.json';
      apiPost('/api/v2/storage?action=manage', { action: 'deleteconfig', path: filePath })
      .then(function() {
        dialog.hide();
        S.toast('Deleted preset: ' + displayName, 'success', 2000);
        return S.reloadMacroData();
      }).then(function() {
        renderMacroBuilderSection();
      }).catch(function(err) {
        deleteBtn.removeAttribute('loading');
        S.toast('Delete failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(deleteBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
  }

  // ─── Delete Macro Definition ────────────────────────────────

  function deleteDefinition(defId) {
    var F = window.TBD.factory;
    if (F && F.isFactoryDefinition(defId)) {
      S.toast('Factory definitions cannot be deleted — clone it instead', 'warning', 3000);
      return;
    }
    var def = S.data.macroDefs.find(function(d) { return d.id === defId; });
    var displayName = def ? (def.name || def.id) : defId;

    // Check if any sound presets use this definition
    var dependentPresets = S.data.soundPresets.filter(function(p) { return p.macro === defId; });

    var old = document.getElementById('delete-def-dialog');
    if (old) old.remove();

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'delete-def-dialog';
    dialog.label = 'Delete Macro Definition';
    dialog.setAttribute('style', '--width:26rem;');

    var warningHtml = '';
    if (dependentPresets.length > 0) {
      warningHtml = '<div style="margin-top:0.75rem;padding:0.5rem 0.65rem;background:var(--sl-color-warning-100);border:1px solid var(--sl-color-warning-300);border-radius:4px;font-size:0.75rem;color:var(--sl-color-warning-700);">'
        + '<sl-icon name="exclamation-triangle" style="font-size:0.75rem;"></sl-icon> '
        + '<strong>' + dependentPresets.length + ' sound preset' + (dependentPresets.length > 1 ? 's' : '') + '</strong> use this macro definition and will become orphaned.'
        + '</div>';
    }

    dialog.innerHTML = '<p style="font-size:0.85rem;margin:0;">Are you sure you want to delete the macro definition <strong>' + S.esc(displayName) + '</strong>?</p>'
      + '<p style="font-size:0.75rem;color:var(--sl-color-neutral-500);margin:0.5rem 0 0;">This action cannot be undone.</p>'
      + warningHtml;

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var deleteBtn = document.createElement('sl-button');
    deleteBtn.setAttribute('slot', 'footer');
    deleteBtn.setAttribute('variant', 'danger');
    deleteBtn.innerHTML = '<sl-icon name="trash3" slot="prefix"></sl-icon> Delete';
    deleteBtn.addEventListener('click', function() {
      deleteBtn.setAttribute('loading', '');
      var filePath = 'macros/' + defId + '.json';
      apiPost('/api/v2/storage?action=manage', { action: 'deleteconfig', path: filePath })
      .then(function() {
        dialog.hide();
        // If the deleted def was being edited, clear state
        if (state.selectedDefId === defId) {
          state.selectedDefId = null;
          state.editDef = null;
          state.dirty = false;
        }
        S.toast('Deleted macro: ' + displayName, 'success', 2000);
        // Reload firmware macro state, then refresh UI data
        return S.reloadFirmwareMacros(defId).then(function() {
          return S.reloadMacroData();
        });
      }).then(function() {
        renderDefinitionList();
        renderMacroBuilderSection();
        // Refresh performer preset browser and track info
        if (window.TBD.performer && window.TBD.performer.reload) {
          window.TBD.performer.reload();
        }
      }).catch(function(err) {
        deleteBtn.removeAttribute('loading');
        S.toast('Delete failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(deleteBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() { dialog.show(); });
  }

  // ─── Toolbar Actions (called by app.js) ──────────────────

  function saveDefinition() {
    if (!state.editDef) { S.toast('Nothing to save', 'warning', 2000); return; }
    if (!state.editDef.id) { S.toast('Definition ID is required', 'warning', 2000); return; }
    if (!state.editDef.machine) { S.toast('Select a machine for this definition', 'warning', 2000); return; }

    // Factory definitions: if unlocked, allow in-place save; otherwise prompt for clone
    var F = window.TBD.factory;
    if (F && F.isFactoryDefinition(state.editDef.id)) {
      if (F.isUnlocked && F.isUnlocked()) {
        // Unlocked — allow in-place save of factory definition
      } else {
        var newId = prompt('Factory definitions are read-only.\nEnter a new ID to save as a copy:', state.editDef.id + '-custom');
        if (!newId) return;
        newId = newId.trim().toLowerCase().replace(/[^a-z0-9_-]/g, '-').replace(/^-|-$/g, '');
        if (!newId) { S.toast('Invalid ID', 'warning', 2000); return; }
        if (F.isFactoryDefinition(newId)) { S.toast('That ID is also a factory definition', 'warning', 2000); return; }
        state.editDef.id = newId;
        state.selectedDefId = newId;
      }
    }

    var cleanDef = cleanDefinitionForSave(state.editDef);
    var jsonStr = JSON.stringify(cleanDef, null, 2);
    var filePath = 'macros/' + state.editDef.id + '.json';

    S.showLoading('Saving definition\u2026');
    fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved: ' + state.editDef.name, 'success', 2000);
      state.dirty = false;
      // Reload firmware macro state, then refresh UI data
      return S.reloadFirmwareMacros(state.editDef.id).then(function() {
        return S.reloadMacroData();
      });
    }).then(function() {
      renderDefinitionList();
      // Sync performer with the saved definition
      if (window.TBD.performer && window.TBD.performer.setMacroDef) {
        window.TBD.performer.setMacroDef(state.editDef);
      }
      S.hideLoading();
    }).catch(function(err) {
      S.hideLoading();
      S.toast('Save failed: ' + err.message, 'danger', 3000);
    });
  }

  function exportDefinition() {
    if (!state.editDef) { S.toast('Nothing to export', 'warning', 2000); return; }
    var cleanDef = cleanDefinitionForSave(state.editDef);
    var blob = new Blob([JSON.stringify(cleanDef, null, 2)], { type: 'application/json' });
    var a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = (state.editDef.id || 'definition') + '.json';
    a.click();
    URL.revokeObjectURL(a.href);
    S.toast('Exported: ' + (state.editDef.name || state.editDef.id), 'success', 2000);
  }

  function importDefinitionFile() {
    var input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.addEventListener('change', function() {
      if (!input.files.length) return;
      var reader = new FileReader();
      reader.onload = function() {
        try {
          var data = JSON.parse(reader.result);
          if (data.groups && data.id) {
            // Validate machine matches current track (if a track is selected)
            if (state.editDef && state.editDef.machine && data.machine && data.machine !== state.editDef.machine) {
              if (!confirm('This definition is for machine "' + data.machine + '" but the current track uses "' + state.editDef.machine + '". Import anyway?')) return;
            }
            state.editDef = data;
            state.selectedDefId = data.id || null;
            ensureGroupStructure(state.editDef);
            reindexParameters(state.editDef);
            state.dirty = true;
            renderMacroBuilderSection();
            // Sync performer with the imported definition
            if (window.TBD.performer && window.TBD.performer.setMacroDef) {
              window.TBD.performer.setMacroDef(state.editDef);
            }
            S.toast('Imported definition: ' + (data.name || data.id || 'unknown'), 'success', 2000);
          } else if (data.id && data.macro && data.values) {
            var filePath = 'presets/' + data.id + '.json';
            fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
              method: 'POST',
              headers: { 'Content-Type': 'application/json' },
              body: JSON.stringify(data, null, 2),
            }).then(function(r) {
              if (!r.ok) throw new Error('HTTP ' + r.status);
              S.toast('Imported preset: ' + data.name, 'success', 2000);
              return S.reloadMacroData();
            }).then(function() {
              renderMacroBuilderSection();
            }).catch(function(err) {
              S.toast('Import failed: ' + err.message, 'danger', 3000);
            });
          } else {
            S.toast('Unrecognized JSON format', 'warning', 3000);
          }
        } catch (err) {
          S.toast('Invalid JSON: ' + err.message, 'danger', 3000);
        }
      };
      reader.readAsText(input.files[0]);
    });
    input.click();
  }

  // ─── Initialization ─────────────────────────────────────

  function init() {
    S.onTrackChange(function(idx, track) {
      onTrackSelected(idx, track);
    });

    setupDefinitionListEvents();

    if (S.data.activeTrack >= 0) {
      var track = S.data.tracks.find(function(t) { return t.index === S.data.activeTrack; });
      if (track) onTrackSelected(S.data.activeTrack, track);
    } else if (S.data.tracks.length > 0) {
      S.selectTrack(S.data.tracks[0].index);
    }

    state.initialized = true;
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.designer = {
    init: init,
    state: state,
    onMachineChanged: onMachineChanged,
    saveDefinition: saveDefinition,
    deleteDefinition: deleteDefinition,
    exportDefinition: exportDefinition,
    importDefinitionFile: importDefinitionFile,
    selectMacroDefinition: selectMacroDefinition,
    reload: function() {
      if (state.activeTrack >= 0) {
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        if (track) onTrackSelected(state.activeTrack, track);
      }
    },
  };

})();
