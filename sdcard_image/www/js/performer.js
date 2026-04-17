// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Unified View Controller
//
// Manages the center panel knob controls and the left sidebar
// preset browser. Uses shared renderKnobGroups() for consistent
// knob rendering across all modes.
//
// Data flow:
//   Track Selection → Machine → Macro Def → Knobs
//   Sound Presets → Load knob values
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    activeTrack: -1,
    activeMachine: '',
    activeMacroDef: null,
    activePreset: null,
    paramValues: [],
    presetSearchTerm: '',
    macroFilter: null,   // null = show all macros, string = filter by def id
    initialized: false,
  };

  // ─── Track Selection (driven by shared track tabs) ────────

  function onTrackSelected(idx, track) {
    state.activeTrack = idx;
    state.activePreset = null;
    state.macroFilter = null; // reset filter on track change

    var availMachines = S.getTrackMachines(track);

    // Check for boot default preset
    var bootPreset = null;
    var bootDef = null;
    if (S.data.trackDefaults && S.data.trackDefaults.tracks) {
      var tdEntry = S.data.trackDefaults.tracks.find(function(t) { return t.index === idx; });
      if (tdEntry && tdEntry.preset) {
        bootPreset = S.data.soundPresets.find(function(p) { return p.id === tdEntry.preset; });
        if (bootPreset) {
          bootDef = S.data.macroDefs.find(function(d) { return d.id === bootPreset.macro; });
        }
      }
    }

    if (bootPreset && bootDef) {
      // Auto-select the boot default preset
      state.activePreset = bootPreset;
      state.activeMacroDef = bootDef;
      state.activeMachine = bootDef.machine;
      state.macroFilter = bootPreset.macro;

      // Load param values from preset
      state.paramValues = [];
      if (bootPreset.values && bootPreset.values.length > 0) {
        state.paramValues = bootPreset.values.slice();
        for (var vi = 0; vi < state.paramValues.length; vi++) {
          if (state.paramValues[vi] === undefined || state.paramValues[vi] === null) {
            state.paramValues[vi] = 0;
          }
        }
      }
      // Fill missing params from def defaults
      if (bootDef.groups) {
        bootDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (state.paramValues[p.idx] === undefined) {
              state.paramValues[p.idx] = p.def || 0;
            }
          });
        });
      }
    } else {
      // Fallback: default to first available machine + allparams def
      var machineId = availMachines.length > 0 ? availMachines[0] : '';
      state.activeMachine = machineId;

      var matchingDefs = S.data.macroDefs.filter(function(d) {
        return d.machine === machineId;
      });
      var allParamsDef = matchingDefs.find(function(d) {
        return d.id.indexOf('allparams') !== -1;
      });
      var def = allParamsDef || matchingDefs[0] || null;
      state.activeMacroDef = def;

      state.paramValues = [];
      if (def && def.groups) {
        def.groups.forEach(function(group) {
          group.parameters.forEach(function(param) {
            state.paramValues[param.idx] = param.def || 0;
          });
        });
      }
    }

    renderMachineSelect(availMachines);
    renderKnobControls(track, state.activeMacroDef);
    renderPresetBrowser();

    // Notify designer of machine change
    if (window.TBD.designer && window.TBD.designer.onMachineChanged) {
      window.TBD.designer.onMachineChanged(state.activeMachine);
    }
  }

  // ─── Machine Select (rendered in track-info-bar) ─────────

  function renderMachineSelect(availMachines) {
    // Machine select is now rendered inside renderTrackInfoBar
    // Keep state updated for external access
    state._availMachines = availMachines;
  }

  // ─── Knob Controls Rendering ─────────────────────────────

  function renderTrackInfoBar(track, macroDef) {
    var trackBar = document.getElementById('track-info-bar');
    if (!trackBar || !track) return;

    var html = '<div class="track-info-header">';

    // Left: Badge + Name
    html += '<div class="track-info-left">';
    html += '<span class="track-badge">CH ' + String(track.index + 1).padStart(2, '0') + '</span>';
    html += '<span class="track-title">' + S.esc(track.name) + '</span>';
    html += '</div>';

    html += '<span class="track-info-separator"></span>';

    // Right: stacked rows
    html += '<div class="track-info-right">';

    // Row 1: MACHINE (+ MACRO in Presets mode)
    var activeTab = (S.getActiveTab && S.getActiveTab()) || 'presets';
    var matchingDefs = S.data.macroDefs.filter(function(d) {
      return d.machine === state.activeMachine;
    });

    html += '<div class="track-info-row">';
    html += '<span class="track-info-label">MACHINE:</span>';
    var availMachines = state._availMachines || S.getTrackMachines(track);
    if (availMachines.length > 1) {
      html += '<sl-select id="machine-select" size="small" value="' + S.esc(state.activeMachine || '') + '" style="min-width:140px;flex:1;max-width:200px;" hoist>';
      availMachines.forEach(function(machId) {
        var info = S.getMachineInfo(machId);
        var label = info ? info.name : machId;
        html += '<sl-option value="' + S.esc(machId) + '">' + S.esc(label) + '</sl-option>';
      });
      html += '</sl-select>';
    } else {
      var machInfo = S.getMachineInfo(state.activeMachine);
      var machName = machInfo ? machInfo.name : state.activeMachine;
      if (machName) {
        html += '<span class="track-machine-label">' + S.esc(machName) + '</span>';
      }
    }
    // In Presets mode, add MACRO dropdown in the same row as MACHINE
    if (activeTab !== 'macros' && matchingDefs.length > 0) {
      if (matchingDefs.length > 1) {
        var filterVal = state.macroFilter || '__all__';
        html += '<span class="track-info-label" style="margin-left:0.5rem;">MACRO:</span>';
        html += '<sl-select id="knobset-select" size="small" value="' + S.esc(filterVal) + '" style="min-width:140px;flex:1;max-width:200px;" hoist>';
        html += '<sl-option value="__all__">All Macros</sl-option>';
        matchingDefs.forEach(function(d) {
          html += '<sl-option value="' + S.esc(d.id) + '">' + S.esc(d.name || d.id) + '</sl-option>';
        });
        html += '</sl-select>';
      } else if (macroDef) {
        html += '<span class="track-info-label" style="margin-left:0.5rem;">MACRO:</span>';
        html += '<span class="track-knobset-label">' + S.esc(macroDef.name) + '</span>';
      }
    }
    html += '</div>'; // .track-info-row

    // Row 2 divider + content
    var hasRow2 = (activeTab === 'macros' && macroDef) || activeTab !== 'macros';
    if (hasRow2) {
      html += '<hr class="track-info-divider" />';
    }

    // Row 2: Context-dependent
    if (activeTab === 'macros') {
      var def = macroDef;
      if (def) {
        var D = window.TBD.designer;
        var isNew = D && D.state && !D.state.selectedDefId;
        html += '<div class="track-info-row" style="flex-wrap:nowrap;gap:0.35rem;">';
        html += '<span class="track-info-label">MACRO NAME:</span>';
        html += '<input class="track-inline-input def-name-input" value="' + S.esc(def.name) + '" placeholder="Definition name" style="flex:1 1 80px;min-width:60px;max-width:200px;" />';
        html += '<span class="track-info-label">ID:</span>';
        html += '<input class="track-inline-input def-id-input" value="' + S.esc(def.id) + '" placeholder="auto-id" style="min-width:0;max-width:12ch;" ' + (isNew ? '' : 'readonly') + ' />';
        var F = window.TBD.factory;
        var isFactoryDef = F && F.isFactoryDefinition(def.id);
        var isFactoryUnlocked = F && F.isUnlocked && F.isUnlocked();
        var volReadonly = isFactoryDef && !isFactoryUnlocked;
        html += '<span class="track-info-label" title="Volume multiplier — compensates for quiet/loud engines. 1.0 = no change.">VOL:</span>';
        html += '<input type="text" inputmode="decimal" class="track-inline-input def-volmult-input" value="' + (def.volmult != null ? def.volmult : 1.0) + '" style="width:5ch;min-width:4ch;max-width:6ch;text-align:center;padding:0.25rem 0.2rem;' + (volReadonly ? 'opacity:0.5;' : '') + '" title="Volume multiplier (0.1–4.0)"' + (volReadonly ? ' readonly' : '') + ' />';
        html += '<div class="track-def-actions" style="flex-shrink:0;">';
        html += '<button class="mapping-btn btn-save-def" title="Save this definition"><sl-icon name="floppy" style="font-size:0.7rem;"></sl-icon> Save</button>';
        html += '<button class="mapping-btn btn-export-def" title="Export as JSON"><sl-icon name="download" style="font-size:0.7rem;"></sl-icon> Export</button>';
        html += '<button class="mapping-btn btn-import-def" title="Import from JSON"><sl-icon name="upload" style="font-size:0.7rem;"></sl-icon> Import</button>';
        var jsonFolder = (isFactoryDef ? 'factory/macros' : 'macros');
        var jsonFile = jsonFolder + '/' + def.id + '.json';
        html += '<a class="mapping-btn btn-viewjson-def" href="/index.html?view=samples&file=' + encodeURIComponent(jsonFile) + '" title="View this macro\'s JSON in Data Manager" style="text-decoration:none;"><sl-icon name="filetype-json" style="font-size:0.7rem;"></sl-icon> View JSON</a>';
        if (!isNew) {
          html += '<button class="mapping-btn btn-delete-def" title="Delete this definition" style="border-color:var(--sl-color-danger-300);color:var(--sl-color-danger-600);"><sl-icon name="trash3" style="font-size:0.7rem;"></sl-icon> Delete</button>';
        }
        html += '</div>';
        html += '</div>';
      }
    } else {
      // Presets mode — row 2: Preset info + action buttons
      html += '<div class="track-info-row" style="flex-wrap:nowrap;gap:0.35rem;">';
      if (state.activePreset) {
        var F = window.TBD.factory;
        var isFactoryPreset = F && F.isFactoryPreset(state.activePreset.id);
        var isFactoryUnlocked = F && F.isUnlocked && F.isUnlocked();
        if (isFactoryPreset && !isFactoryUnlocked) {
          html += '<sl-icon name="lock" style="font-size:0.6rem;opacity:0.45;flex-shrink:0;" title="Factory preset — read-only"></sl-icon>';
        }
        html += '<span class="track-info-label">PRESET:</span>';
        html += '<span class="track-preset-name">' + S.esc(state.activePreset.name) + '</span>';
        html += '<span class="track-info-label" style="margin-left:0.25rem;">ID:</span>';
        html += '<span class="track-preset-id">' + S.esc(state.activePreset.id) + '</span>';
      } else {
        html += '<span class="track-info-label">PRESET:</span>';
        html += '<span class="track-preset-name" style="opacity:0.4;font-style:italic;">No preset loaded</span>';
      }
      // Preset action buttons
      html += '<div class="track-def-actions" style="flex-shrink:0;">';
      if (state.activePreset) {
        var Fsave = window.TBD.factory;
        var isFactoryP = Fsave && Fsave.isFactoryPreset(state.activePreset.id);
        var isUnlockedP = Fsave && Fsave.isUnlocked && Fsave.isUnlocked();
        var canOverwrite = !isFactoryP || isUnlockedP;
        if (canOverwrite) {
          html += '<button class="mapping-btn btn-save-preset" title="Overwrite current preset with current knob values"><sl-icon name="floppy" style="font-size:0.7rem;"></sl-icon> Save</button>';
        }
      }
      html += '<button class="mapping-btn btn-saveas-preset" title="Save current knob values as a new preset"><sl-icon name="floppy" style="font-size:0.7rem;"></sl-icon> Save As\u2026</button>';
      html += '<button class="mapping-btn btn-export-presets" title="Export all presets as JSON"><sl-icon name="download" style="font-size:0.7rem;"></sl-icon> Export</button>';
      html += '<button class="mapping-btn btn-import-presets" title="Import presets from JSON"><sl-icon name="upload" style="font-size:0.7rem;"></sl-icon> Import</button>';
      if (state.activePreset) {
        var FJ = window.TBD.factory;
        var jsonFolder = (FJ && FJ.isFactoryPreset(state.activePreset.id)) ? 'factory/presets' : 'presets';
        var jsonFile = jsonFolder + '/' + state.activePreset.id + '.json';
        html += '<a class="mapping-btn btn-viewjson-presets" href="/index.html?view=samples&file=' + encodeURIComponent(jsonFile) + '" title="View this preset\u2019s JSON in Data Manager" style="text-decoration:none;"><sl-icon name="filetype-json" style="font-size:0.7rem;"></sl-icon> JSON</a>';
      } else {
        html += '<a class="mapping-btn btn-viewjson-presets" href="/index.html?view=samples&browse=presets" title="Browse preset JSON files in Data Manager" style="text-decoration:none;"><sl-icon name="filetype-json" style="font-size:0.7rem;"></sl-icon> JSON</a>';
      }
      html += '</div>';
      html += '</div>';
    }

    html += '</div>'; // .track-info-right
    html += '</div>'; // .track-info-header
    trackBar.innerHTML = html;
    setupMachineSelectEvents();
  }

  function setupMachineSelectEvents() {
    var machineSelect = document.getElementById('machine-select');
    if (machineSelect) {
      machineSelect.addEventListener('sl-change', function() {
        onMachineChange(machineSelect.value);
        if (window.TBD.designer && window.TBD.designer.onMachineChanged) {
          window.TBD.designer.onMachineChanged(machineSelect.value);
        }
      });
    }

    // Knob set dropdown (Presets mode — acts as macro filter)
    var knobsetSelect = document.getElementById('knobset-select');
    if (knobsetSelect) {
      knobsetSelect.addEventListener('sl-change', function() {
        var defId = knobsetSelect.value;
        if (defId === '__all__') {
          // "All Macros" — clear filter, keep current activeMacroDef
          state.macroFilter = null;
          state.activePreset = null;
          var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
          if (track) renderTrackInfoBar(track, state.activeMacroDef);
        } else {
          var def = S.data.macroDefs.find(function(d) { return d.id === defId; });
          if (def) {
            state.macroFilter = defId;
            state.activeMacroDef = def;
            state.activePreset = null;
            state.paramValues = [];
            if (def.groups) {
              def.groups.forEach(function(group) {
                group.parameters.forEach(function(param) {
                  state.paramValues[param.idx] = param.def || 0;
                });
              });
            }
            var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
            if (track) {
              renderKnobControls(track, def);
            }
          }
        }
        renderPresetBrowser();
      });
    }

    // Preset action buttons in header bar (Presets mode)
    var savePresetBtn = document.querySelector('#track-info-bar .btn-save-preset');
    if (savePresetBtn) {
      savePresetBtn.addEventListener('click', function() {
        savePresetOverwrite();
      });
    }
    var saveAsPresetBtn = document.querySelector('#track-info-bar .btn-saveas-preset');
    if (saveAsPresetBtn) {
      saveAsPresetBtn.addEventListener('click', function() {
        if (state.activeTrack < 0 || !state.activeMacroDef) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        savePresetDialog();
      });
    }
    var exportPresetsBtn = document.querySelector('#track-info-bar .btn-export-presets');
    if (exportPresetsBtn) {
      exportPresetsBtn.addEventListener('click', function() {
        exportAllPresets();
      });
    }
    var importPresetsBtn = document.querySelector('#track-info-bar .btn-import-presets');
    if (importPresetsBtn) {
      importPresetsBtn.addEventListener('click', function() {
        importPresetFile();
      });
    }

    // Def header inputs (Macros mode — NAME/ID/Save/Export/Import in track bar)
    setupDefHeaderInBarEvents();
  }

  function setupDefHeaderInBarEvents() {
    var D = window.TBD.designer;
    if (!D || !D.state) return;

    var nameInput = document.querySelector('#track-info-bar .def-name-input');
    var idInput = document.querySelector('#track-info-bar .def-id-input');

    if (nameInput) {
      nameInput.addEventListener('input', function() {
        if (D.state.editDef) {
          D.state.editDef.name = nameInput.value;
          D.state.dirty = true;
          // Auto-generate ID for new definitions (only if user hasn't manually edited ID)
          if (!D.state.selectedDefId && idInput && !idInput.dataset.userEdited) {
            var machinePrefix = D.state.editDef.machine ? (D.state.editDef.machine.substring(0, 2) + '-') : '';
            var slug = nameInput.value.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
            D.state.editDef.id = machinePrefix + slug;
            idInput.value = D.state.editDef.id;
          }
        }
      });
      nameInput.addEventListener('change', function() {
        if (D.state.editDef) {
          D.state.editDef.name = nameInput.value;
          D.state.dirty = true;
        }
      });
    }

    if (idInput) {
      idInput.addEventListener('input', function() {
        idInput.dataset.userEdited = 'true';
      });
      idInput.addEventListener('change', function() {
        if (D.state.editDef) {
          D.state.editDef.id = idInput.value;
          D.state.dirty = true;
        }
      });
    }

    var volmultInput = document.querySelector('#track-info-bar .def-volmult-input');
    if (volmultInput) {
      volmultInput.addEventListener('change', function() {
        if (D.state.editDef) {
          var v = parseFloat(volmultInput.value);
          if (isNaN(v) || v < 0.1) v = 0.1;
          if (v > 4.0) v = 4.0;
          v = Math.round(v * 10) / 10; // round to 1 decimal
          volmultInput.value = v;
          D.state.editDef.volmult = v;
          D.state.dirty = true;
        }
      });
    }

    var saveBtn = document.querySelector('#track-info-bar .btn-save-def');
    if (saveBtn) {
      saveBtn.addEventListener('click', function() {
        if (D.saveDefinition) D.saveDefinition();
      });
    }
    var exportBtn = document.querySelector('#track-info-bar .btn-export-def');
    if (exportBtn) {
      exportBtn.addEventListener('click', function() {
        if (D.exportDefinition) D.exportDefinition();
      });
    }
    var importBtn = document.querySelector('#track-info-bar .btn-import-def');
    if (importBtn) {
      importBtn.addEventListener('click', function() {
        if (D.importDefinitionFile) D.importDefinitionFile();
      });
    }
    var deleteDefBtn = document.querySelector('#track-info-bar .btn-delete-def');
    if (deleteDefBtn) {
      deleteDefBtn.addEventListener('click', function() {
        if (D.state && D.state.selectedDefId) {
          if (D.deleteDefinition) D.deleteDefinition(D.state.selectedDefId);
        }
      });
    }
  }

  function renderKnobControls(track, macroDef) {
    var container = document.getElementById('knob-controls');
    if (!container) return;

    // Render the track info bar (separated from knob content)
    renderTrackInfoBar(track, macroDef);

    if (!macroDef) {
      container.innerHTML =
        '<div class="empty-state" id="knob-empty">' +
        '<sl-icon name="sliders"></sl-icon>' +
        '<h3>No macro definition found</h3>' +
        '<p>No macro definition available for machine "' + S.esc(state.activeMachine) + '"</p>' +
        '</div>';
      return;
    }

    // Render knob groups using the shared renderer
    container.innerHTML = S.renderKnobGroups(macroDef, state.paramValues);
    setupKnobEvents(container);
    setupGroupCollapseEvents(container);
  }

  function setupGroupCollapseEvents(container) {
    container.querySelectorAll('.macro-group-header').forEach(function(header) {
      header.addEventListener('click', function() {
        header.parentElement.classList.toggle('collapsed');
      });
    });
  }

  // ─── Knob Drag Interaction ───────────────────────────────

  function setupKnobEvents(container) {
    container.querySelectorAll('.macro-knob').forEach(function(knob) {
      var cell = knob.closest('.macro-knob-cell');
      var valueEl = cell.querySelector('.macro-knob-value');
      var min = parseInt(knob.getAttribute('data-min'), 10) || 0;
      var max = parseInt(knob.getAttribute('data-max'), 10) || 127;
      var paramIdx = parseInt(knob.getAttribute('data-idx'), 10);
      var startY = 0;
      var startVal = 0;

      function onPointerDown(e) {
        e.preventDefault();
        knob.classList.add('dragging');
        startY = e.clientY;
        startVal = parseInt(knob.getAttribute('data-value'), 10) || 0;
        document.addEventListener('pointermove', onPointerMove);
        document.addEventListener('pointerup', onPointerUp);
      }

      function onPointerMove(e) {
        var dy = startY - e.clientY;
        var range = max - min;
        var sensitivity = range / 200;
        var newVal = Math.round(startVal + dy * sensitivity);
        newVal = Math.max(min, Math.min(max, newVal));

        knob.setAttribute('data-value', newVal);
        valueEl.textContent = newVal;
        state.paramValues[paramIdx] = newVal;

        var knobColor = knob.getAttribute('data-color') || 'normal';
        knob.innerHTML = S.renderKnobSVG({ value: newVal, min: min, max: max, color: knobColor, size: 64 });

        // Update target panel values in real-time
        if (state.activeMacroDef) {
          var panel = cell.querySelector('.knob-target-panel');
          if (panel) {
            var outputs = S.computeMappingOutputs(state.activeMacroDef, paramIdx, newVal);
            var targetDH = window.TBD && window.TBD.displayHints;
            outputs.forEach(function(o) {
              var row = panel.querySelector('.knob-target-row[data-ctrl="' + o.ctrl + '"]');
              if (!row) return;
              var valEl = row.querySelector('.knob-target-val');
              var dotEl = row.querySelector('.knob-target-dot');
              if (valEl) {
                var fmt = String(o.value);
                if (targetDH && state.activeMacroDef.machine) {
                  var pid = state.activeMacroDef.machine + '_' + o.name.replace(/[- ]/g, '_');
                  var hint = targetDH.resolveHint(pid, o.name);
                  if (hint) {
                    var physVal = targetDH.rawToDisplay(o.value, 0, o.max || 127, hint);
                    fmt = targetDH.formatDisplayValue(physVal, hint);
                  }
                }
                valEl.textContent = fmt;
              }
              if (dotEl) dotEl.style.left = (o.value / (o.max || 127) * 100) + '%';
            });
          }
        }
      }

      function onPointerUp() {
        knob.classList.remove('dragging');
        document.removeEventListener('pointermove', onPointerMove);
        document.removeEventListener('pointerup', onPointerUp);
        var value = parseInt(knob.getAttribute('data-value'), 10);
        state.paramValues[paramIdx] = value;
        sendParameterUpdate();
      }

      knob.addEventListener('pointerdown', onPointerDown);
    });
  }

  // ─── API: Send Updates ────────────────────────────────────

  function sendTrackUpdate(body) {
    S.apiPostJSON('/macros?action=update_track', body).then(function() {
      console.log('[Performer] Track update sent:', body);
    }).catch(function(err) {
      console.error('[Performer] Track update failed:', err);
    });
  }

  function sendParameterUpdate() {
    if (state.activeTrack < 0 || !state.activeMacroDef) return;
    var body = {
      track: state.activeTrack,
      machine: state.activeMachine,
      macro: state.activeMacroDef.id,
      parameters: state.paramValues.slice(),
    };
    S.apiPostJSON('/macros?action=update_track', body).then(function() {
      console.log('[Performer] Parameters sent for track', state.activeTrack);
    }).catch(function(err) {
      console.error('[Performer] Parameter send failed:', err);
    });
  }

  // ─── Preset Browser ─────────────────────────────────────

  function renderPresetBrowser() {
    var container = document.getElementById('preset-list');
    if (!container) return;

    // Collect ALL presets for this machine (for total count)
    var machineDefIds = {};
    S.data.macroDefs.forEach(function(d) {
      if (d.machine === state.activeMachine) {
        machineDefIds[d.id] = true;
      }
    });
    var allMachinePresets = S.data.soundPresets.filter(function(p) {
      return machineDefIds[p.macro] || false;
    });
    var totalCount = allMachinePresets.length;

    // Apply macro filter
    var presets;
    if (state.macroFilter) {
      presets = allMachinePresets.filter(function(p) {
        return p.macro === state.macroFilter;
      });
    } else {
      presets = allMachinePresets;
    }

    var term = state.presetSearchTerm.toLowerCase();
    if (term) {
      presets = presets.filter(function(p) {
        return (p.name || '').toLowerCase().indexOf(term) !== -1 ||
               (p.group || '').toLowerCase().indexOf(term) !== -1;
      });
    }

    var filteredCount = presets.length;

    // Build HTML — filter chip + count + list
    var html = '';

    // Active filter chip (dismissible)
    if (state.macroFilter) {
      var filterDef = S.data.macroDefs.find(function(d) { return d.id === state.macroFilter; });
      var filterLabel = filterDef ? (filterDef.name || filterDef.id) : state.macroFilter;
      html += '<div class="preset-filter-bar">';
      html += '<button class="preset-filter-chip" id="clear-macro-filter" title="Show all macros">';
      html += '<span class="preset-filter-chip-label">' + S.esc(filterLabel) + '</span>';
      html += '<sl-icon name="x-lg" style="font-size:0.6rem;"></sl-icon>';
      html += '</button>';
      html += '<span class="preset-count">' + filteredCount + ' of ' + totalCount + '</span>';
      html += '</div>';
    } else if (totalCount > 0) {
      html += '<div class="preset-filter-bar">';
      html += '<span class="preset-count">' + totalCount + ' preset' + (totalCount !== 1 ? 's' : '') + '</span>';
      html += '</div>';
    }

    // Group by category
    var groups = {};
    presets.forEach(function(p) {
      var g = p.group || 'Uncategorized';
      if (!groups[g]) groups[g] = [];
      groups[g].push(p);
    });

    var F = window.TBD.factory;

    Object.keys(groups).sort().forEach(function(groupName) {
      html += '<div class="preset-category">' + S.esc(groupName) + '</div>';
      groups[groupName].forEach(function(p) {
        var isActive = state.activePreset && state.activePreset.id === p.id;
        var isFactory = F && F.isFactoryPreset(p.id);
        html += '<div class="preset-item' + (isActive ? ' active' : '') + '" data-preset-id="' + S.esc(p.id) + '">';
        if (isFactory) {
          html += '<sl-icon name="lock" style="font-size:0.6rem;opacity:0.4;flex-shrink:0;margin-right:0.2rem;" title="Factory preset — use Save As to create a copy"></sl-icon>';
        }
        html += '<span class="preset-item-name" title="' + S.esc(p.name) + '">' + S.esc(p.name) + '</span>';
        html += '<span class="preset-item-machine">' + S.esc(p.macro) + '</span>';
        if (!isFactory || (F && F.isUnlocked && F.isUnlocked())) {
          html += '<button class="preset-item-delete" data-delete-preset-id="' + S.esc(p.id) + '" title="Delete preset">';
          html += '<sl-icon name="trash3"></sl-icon>';
          html += '</button>';
        }
        html += '</div>';
      });
    });

    if (filteredCount === 0) {
      html += '<div class="empty-state" style="padding:1.5rem;">';
      if (state.macroFilter) {
        html += '<p style="font-size:0.78rem;">No presets for this macro</p>';
        html += '<p style="font-size:0.7rem;color:var(--sl-color-neutral-400);">Clear the filter to see all presets</p>';
      } else {
        html += '<p style="font-size:0.78rem;">No presets for ' + S.esc(state.activeMachine || 'this track') + '</p>';
      }
      html += '</div>';
    }

    container.innerHTML = html;

    // Wire up the filter chip dismiss button
    var clearBtn = document.getElementById('clear-macro-filter');
    if (clearBtn) {
      clearBtn.addEventListener('click', function() {
        state.macroFilter = null;
        // Re-render track info bar to sync dropdown to "All Macros"
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        if (track) {
          renderTrackInfoBar(track, state.activeMacroDef);
        }
        renderPresetBrowser();
      });
    }
  }

  function setupPresetBrowserEvents() {

    var list = document.getElementById('preset-list');
    if (list) {
      list.addEventListener('click', function(e) {
        // Handle delete button click
        var deleteBtn = e.target.closest('.preset-item-delete');
        if (deleteBtn) {
          e.stopPropagation();
          var presetId = deleteBtn.getAttribute('data-delete-preset-id');
          if (presetId) deletePreset(presetId);
          return;
        }
        var item = e.target.closest('.preset-item');
        if (!item) return;
        var presetId = item.getAttribute('data-preset-id');
        loadPreset(presetId);
      });
    }
  }

  function loadPreset(presetId) {
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    if (!preset) return;

    state.activePreset = preset;

    var def = S.data.macroDefs.find(function(d) { return d.id === preset.macro; });
    if (def) {
      state.activeMacroDef = def;
      state.activeMachine = def.machine;
    }

    // Sync macro filter to reflect the preset's macro
    state.macroFilter = preset.macro;

    if (preset.values && preset.values.length > 0) {
      state.paramValues = preset.values.slice();
      // Ensure all values are defined (no nulls/undefineds from sparse arrays)
      for (var vi = 0; vi < state.paramValues.length; vi++) {
        if (state.paramValues[vi] === undefined || state.paramValues[vi] === null) {
          state.paramValues[vi] = 0;
        }
      }
    }

    // If definition has more params than the preset, fill missing values with defaults
    if (def && def.groups) {
      def.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) {
          if (state.paramValues[p.idx] === undefined) {
            state.paramValues[p.idx] = p.def || 0;
          }
        });
      });
    }

    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) {
      // Re-render track info bar so MACRO dropdown reflects the preset's macro
      renderTrackInfoBar(track, state.activeMacroDef);
      renderKnobControls(track, state.activeMacroDef);
    }

    // Re-render sidebar (filter chip + filtered list with active highlight)
    renderPresetBrowser();

    sendTrackUpdate({
      track: state.activeTrack,
      machine: state.activeMachine,
      macro: preset.macro,
      parameters: state.paramValues.slice(),
    });

    S.toast('Loaded: ' + preset.name, 'success', 2000);
  }

  // ─── Quick Actions (sidebar buttons) ─────────────────────

  function setupQuickActions() {
    var newPresetBtn = document.getElementById('qa-new-preset');
    if (newPresetBtn) {
      newPresetBtn.addEventListener('click', function() {
        if (state.activeTrack < 0) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        // Check if any macros are available for the current machine
        var availableMacros = S.data.macroDefs.filter(function(d) {
          return d.machine === state.activeMachine;
        });
        if (availableMacros.length === 0) {
          S.toast('No macros available for this machine — create one first in the Macros tab', 'warning', 3000);
          return;
        }
        savePresetDialog();
      });
    }

  }

  // ─── Save Preset Overwrite (existing preset) ──────────────

  function savePresetOverwrite() {
    if (!state.activePreset || !state.activeMacroDef) {
      S.toast('No preset loaded to save', 'warning', 2000);
      return;
    }

    // Double-check: factory presets require unlocked factory edit mode
    var F = window.TBD.factory;
    var isFactory = F && F.isFactoryPreset(state.activePreset.id);
    if (isFactory && !(F.isUnlocked && F.isUnlocked())) {
      S.toast('Factory presets are read-only — use Save As to create a copy', 'warning', 3000);
      return;
    }

    // Gather current knob values
    var paramCount = 0;
    if (state.activeMacroDef.groups) {
      state.activeMacroDef.groups.forEach(function(g) {
        (g.parameters || []).forEach(function(p) {
          if (p.idx >= paramCount) paramCount = p.idx + 1;
        });
      });
    }
    var values = [];
    for (var vi = 0; vi < paramCount; vi++) {
      var raw = state.paramValues[vi];
      values[vi] = (raw !== undefined && raw !== null) ? Math.round(raw) : 0;
    }

    var preset = {
      id: state.activePreset.id,
      name: state.activePreset.name,
      group: state.activePreset.group || 'User',
      macro: state.activeMacroDef.id,
      values: values,
    };

    var jsonStr = JSON.stringify(preset, null, 2);
    var filePath = (isFactory ? 'factory/presets/' : 'presets/') + preset.id + '.json';

    fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved: ' + preset.name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderPresetBrowser();
    }).catch(function(err) {
      S.toast('Save failed: ' + err.message, 'danger', 3000);
    });
  }

  // ─── Save Preset Dialog (Shoelace) ────────────────────────

  function savePresetDialog() {
    // Remove any old dialog
    var old = document.getElementById('save-preset-dialog');
    if (old) old.remove();

    var F = window.TBD.factory;
    var isFromFactory = state.activePreset && F && F.isFactoryPreset(state.activePreset.id);

    var defaultName = state.activePreset ? state.activePreset.name : (state.activeMacroDef ? state.activeMacroDef.name : '');
    if (isFromFactory) defaultName = defaultName + ' (copy)';
    var defaultGroup = state.activePreset ? (state.activePreset.group || '') : '';
    if (isFromFactory && defaultGroup) defaultGroup = 'User';
    if (!defaultGroup) defaultGroup = 'User';

    var trackName = '';
    var trackBadge = '';
    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) {
      trackName = track.name;
      trackBadge = 'CH ' + String(track.index + 1).padStart(2, '0');
    }

    // Available machines for this track
    var availMachines = track ? S.getTrackMachines(track) : [];
    var currentMachine = state.activeMachine || (availMachines.length > 0 ? availMachines[0] : '');
    var currentMacroId = state.activeMacroDef ? state.activeMacroDef.id : '';

    // Dialog state — track selections via sl-change events
    var selectedMachine = currentMachine;
    var selectedMacroId = currentMacroId;

    function getMacrosForMachine(machId) {
      return S.data.macroDefs.filter(function(d) { return d.machine === machId; });
    }

    function buildMacroOptions(machId) {
      var macros = getMacrosForMachine(machId);
      var h = '';
      macros.forEach(function(m) {
        h += '<sl-option value="' + S.esc(m.id) + '">' + S.esc(m.name || m.id) + '</sl-option>';
      });
      return h;
    }

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'save-preset-dialog';
    dialog.label = 'New Sound Preset';
    dialog.setAttribute('style', '--width:28rem;');

    var html = '';
    // Context header
    html += '<div class="save-preset-context">';
    html += '<span class="track-badge" style="font-size:0.72rem;">' + S.esc(trackBadge) + '</span> ';
    html += '<strong>' + S.esc(trackName) + '</strong>';
    html += '</div>';

    html += '<div style="display:flex;flex-direction:column;gap:0.75rem;margin-top:0.75rem;">';
    // Machine dropdown (first)
    if (availMachines.length > 1) {
      html += '<sl-select id="save-preset-machine" label="Machine" value="' + S.esc(currentMachine) + '" hoist>';
      availMachines.forEach(function(machId) {
        var info = S.getMachineInfo(machId);
        var label = info ? info.name : machId;
        html += '<sl-option value="' + S.esc(machId) + '">' + S.esc(label) + '</sl-option>';
      });
      html += '</sl-select>';
    } else {
      var mInfo = S.getMachineInfo(currentMachine);
      var mName = mInfo ? mInfo.name : currentMachine;
      html += '<div style="font-size:0.78rem;color:var(--sl-color-neutral-500);"><strong>Machine:</strong> ' + S.esc(mName) + '</div>';
    }
    // Macro dropdown (second)
    html += '<sl-select id="save-preset-macro" label="Macro" value="' + S.esc(currentMacroId) + '" hoist>';
    html += buildMacroOptions(currentMachine);
    html += '</sl-select>';
    // Preset name
    html += '<sl-input id="save-preset-name" label="Preset Name" value="' + S.esc(defaultName) + '" placeholder="e.g. Fat Punch" required></sl-input>';
    // Category
    html += '<sl-input id="save-preset-group" label="Category / Group" value="' + S.esc(defaultGroup) + '" placeholder="e.g. User" help-text="Presets are grouped by this label in the sidebar"></sl-input>';
    html += '</div>';
    html += '<div style="margin-top:1rem;font-size:0.72rem;color:var(--sl-color-neutral-500);">';
    html += '<sl-icon name="info-circle" style="font-size:0.7rem;"></sl-icon> ';
    html += 'Saves the current knob values as a new sound preset for the selected macro.';
    html += '</div>';

    dialog.innerHTML = html;

    // Wire sl-change events to track selections reliably
    var machineSelectEl = dialog.querySelector('#save-preset-machine');
    if (machineSelectEl) {
      machineSelectEl.addEventListener('sl-change', function() {
        selectedMachine = machineSelectEl.value;
        // Rebuild Macro options for the new machine
        var macros = getMacrosForMachine(selectedMachine);
        var oldMacro = dialog.querySelector('#save-preset-macro');
        if (oldMacro) {
          var newMacro = document.createElement('sl-select');
          newMacro.id = 'save-preset-macro';
          newMacro.label = 'Macro';
          newMacro.setAttribute('hoist', '');
          newMacro.innerHTML = buildMacroOptions(selectedMachine);
          selectedMacroId = macros.length > 0 ? macros[0].id : '';
          newMacro.value = selectedMacroId;
          oldMacro.replaceWith(newMacro);
          newMacro.addEventListener('sl-change', function() {
            selectedMacroId = newMacro.value;
          });
        }
      });
    }
    var macroSelectEl = dialog.querySelector('#save-preset-macro');
    if (macroSelectEl) {
      macroSelectEl.addEventListener('sl-change', function() {
        selectedMacroId = macroSelectEl.value;
      });
    }

    // Footer buttons
    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var saveBtn = document.createElement('sl-button');
    saveBtn.setAttribute('slot', 'footer');
    saveBtn.setAttribute('variant', 'primary');
    saveBtn.innerHTML = '<sl-icon name="floppy" slot="prefix"></sl-icon> Save Preset';

    saveBtn.addEventListener('click', function() {
      var nameInput = dialog.querySelector('#save-preset-name');
      var groupInput = dialog.querySelector('#save-preset-group');
      var name = (nameInput.value || '').trim();
      var group = (groupInput.value || '').trim() || 'User';

      if (!name) {
        nameInput.setAttribute('help-text', 'Please enter a name');
        nameInput.focus();
        return;
      }

      if (!selectedMacroId) {
        S.toast('Please select a macro', 'warning', 2000);
        return;
      }

      // Find the selected macro definition
      var selectedMacroDef = S.data.macroDefs.find(function(d) { return d.id === selectedMacroId; });
      if (!selectedMacroDef) {
        S.toast('Selected macro not found', 'danger', 2000);
        return;
      }

      var id = name.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');

      // Prevent overwriting factory presets (unless Factory Edit Mode unlocked)
      var Fcheck = window.TBD.factory;
      var isFactoryId = Fcheck && Fcheck.isFactoryPreset(id);
      if (isFactoryId && !(Fcheck.isUnlocked && Fcheck.isUnlocked())) {
        nameInput.setAttribute('help-text', 'This name matches a factory preset \u2014 unlock Factory Edit Mode or choose a different name');
        nameInput.focus();
        return;
      }

      // Produce a dense values array trimmed to the definition's parameter count
      var paramCount = 0;
      if (selectedMacroDef && selectedMacroDef.groups) {
        selectedMacroDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (p.idx >= paramCount) paramCount = p.idx + 1;
          });
        });
      }
      var values = [];
      for (var vi = 0; vi < paramCount; vi++) {
        var raw = state.paramValues[vi];
        values[vi] = (raw !== undefined && raw !== null) ? Math.round(raw) : 0;
      }
      var preset = {
        id: id,
        name: name,
        group: group,
        macro: selectedMacroId,
        values: values,
      };

      saveBtn.setAttribute('loading', '');
      var jsonStr = JSON.stringify(preset, null, 2);
      var filePath = (isFactoryId ? 'factory/presets/' : 'presets/') + id + '.json';

      fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: jsonStr,
      }).then(function(r) {
        if (!r.ok) throw new Error('HTTP ' + r.status);
        dialog.hide();
        S.toast('Saved preset: ' + name, 'success', 2000);
        return S.reloadMacroData();
      }).then(function() {
        renderPresetBrowser();
      }).catch(function(err) {
        saveBtn.removeAttribute('loading');
        S.toast('Save failed: ' + err.message, 'danger', 3000);
      });
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(saveBtn);
    document.body.appendChild(dialog);

    // Prevent hoisted sl-select overlay clicks from closing the dialog
    dialog.addEventListener('sl-request-close', function(e) {
      if (e.detail.source === 'overlay') e.preventDefault();
    });

    // Guard: only remove on dialog's own hide, not bubbled sl-after-hide from child sl-selects
    dialog.addEventListener('sl-after-hide', function(e) {
      if (e.target !== dialog) return;
      dialog.remove();
    });

    // Show the dialog
    requestAnimationFrame(function() {
      dialog.show();
    });
  }

  // ─── Delete Preset ────────────────────────────────────────

  function deletePreset(presetId) {
    var F = window.TBD.factory;
    if (F && F.isFactoryPreset(presetId) && !(F.isUnlocked && F.isUnlocked())) {
      S.toast('Factory presets cannot be deleted \u2014 unlock Factory Edit Mode first', 'warning', 3000);
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
      var Fdel = window.TBD.factory;
      var filePath = (Fdel && Fdel.isFactoryPreset(presetId) ? 'factory/presets/' : 'presets/') + presetId + '.json';
      S.apiPostJSON('/storage?action=manage', { action: 'deleteconfig', path: filePath })
      .then(function() {
        dialog.hide();
        // If the deleted preset was active, clear it
        if (state.activePreset && state.activePreset.id === presetId) {
          state.activePreset = null;
        }
        S.toast('Deleted preset: ' + displayName, 'success', 2000);
        return S.reloadMacroData();
      }).then(function() {
        renderPresetBrowser();
        // Also refresh designer if it's active
        if (window.TBD.designer && window.TBD.designer.reload) {
          window.TBD.designer.reload();
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

  // ─── Export / Import (for presets mode) ───────────────────

  function exportAllPresets() {
    // Patch the active preset's values with the current live knob state
    // so the export reflects what the user actually hears right now.
    var presets = S.data.soundPresets.map(function(p) {
      if (state.activePreset && p.id === state.activePreset.id && state.paramValues.length > 0) {
        var patched = {};
        for (var k in p) { if (p.hasOwnProperty(k)) patched[k] = p[k]; }
        var paramCount = 0;
        if (state.activeMacroDef && state.activeMacroDef.groups) {
          state.activeMacroDef.groups.forEach(function(g) {
            (g.parameters || []).forEach(function(pm) {
              if (pm.idx >= paramCount) paramCount = pm.idx + 1;
            });
          });
        }
        var vals = [];
        for (var i = 0; i < paramCount; i++) {
          var raw = state.paramValues[i];
          vals[i] = (raw !== undefined && raw !== null) ? Math.round(raw) : 0;
        }
        patched.values = vals;
        return patched;
      }
      return p;
    });
    var data = {
      macroDefs: S.data.macroDefs,
      soundPresets: presets,
    };
    var blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    var a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = 'tbd16-presets-export.json';
    a.click();
    URL.revokeObjectURL(a.href);
    S.toast('Exported all presets', 'success', 2000);
  }

  function importPresetFile() {
    var input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.addEventListener('change', function() {
      if (!input.files.length) return;
      var reader = new FileReader();
      reader.onload = function() {
        try {
          var data = JSON.parse(reader.result);
          if (data.id && data.macro) {
            importSinglePreset(data);
          } else if (data.macroDefs || data.soundPresets) {
            importBulk(data);
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

  /**
   * Import a bulk export file containing macroDefs and/or soundPresets.
   * Each item is uploaded to the device sequentially.
   */
  function importBulk(data) {
    var defs = Array.isArray(data.macroDefs) ? data.macroDefs : [];
    var presets = Array.isArray(data.soundPresets) ? data.soundPresets : [];
    var total = defs.length + presets.length;
    if (total === 0) {
      S.toast('Nothing to import', 'warning', 2000);
      return;
    }
    if (!confirm('Import ' + defs.length + ' macro definitions and ' + presets.length + ' sound presets? Existing files with the same IDs will be overwritten.')) {
      return;
    }
    S.showLoading('Importing 0/' + total + '\u2026');
    var done = 0;
    var errors = 0;

    function uploadFile(path, obj) {
      return fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(path), {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(obj, null, 2),
      }).then(function(r) {
        if (!r.ok) throw new Error('HTTP ' + r.status);
        done++;
        S.showLoading('Importing ' + done + '/' + total + '\u2026');
      }).catch(function() {
        errors++;
        done++;
        S.showLoading('Importing ' + done + '/' + total + '\u2026');
      });
    }

    // Chain uploads sequentially to avoid overwhelming the device
    var chain = Promise.resolve();
    defs.forEach(function(d) {
      if (!d.id) return;
      chain = chain.then(function() {
        return uploadFile('macros/' + d.id + '.json', d);
      });
    });
    presets.forEach(function(p) {
      if (!p.id) return;
      chain = chain.then(function() {
        return uploadFile('presets/' + p.id + '.json', p);
      });
    });
    chain.then(function() {
      S.hideLoading();
      if (errors > 0) {
        S.toast('Imported with ' + errors + ' error(s)', 'warning', 3000);
      } else {
        S.toast('Imported ' + total + ' items', 'success', 2000);
      }
      return S.reloadFirmwareMacros().then(function() {
        return S.reloadMacroData();
      });
    }).then(function() {
      renderPresetBrowser();
    });
  }

  function importSinglePreset(preset) {
    if (!preset.id || !preset.macro || !Array.isArray(preset.values)) {
      S.toast('Invalid preset: missing id, macro, or values', 'danger', 3000);
      return;
    }
    // Verify the referenced macro definition exists
    var macroDef = S.data.macroDefs.find(function(d) { return d.id === preset.macro; });
    if (!macroDef) {
      S.toast('Macro definition "' + preset.macro + '" not found on device. Import the macro first.', 'warning', 4000);
      return;
    }
    // Count expected params and warn on mismatch
    var expectedCount = 0;
    (macroDef.groups || []).forEach(function(g) { expectedCount += (g.parameters || []).length; });
    if (preset.values.length !== expectedCount) {
      if (!confirm('Preset has ' + preset.values.length + ' values but macro "' + preset.macro + '" has ' + expectedCount + ' parameters. Import anyway?')) return;
    }
    var Fimp = window.TBD.factory;
    var isFactoryId = Fimp && Fimp.isFactoryPreset && Fimp.isFactoryPreset(preset.id);
    var filePath = (isFactoryId && Fimp.isUnlocked && Fimp.isUnlocked() ? 'factory/presets/' : 'presets/') + preset.id + '.json';
    var jsonStr = JSON.stringify(preset, null, 2);

    fetch('/api/v2/storage?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Imported: ' + preset.name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderPresetBrowser();
    }).catch(function(err) {
      S.toast('Import failed: ' + err.message, 'danger', 3000);
    });
  }

  // ─── Machine Change (toolbar) ────────────────────────────

  function onMachineChange(newMachine) {
    if (newMachine === state.activeMachine) return;
    state.activeMachine = newMachine;
    state.macroFilter = null; // reset filter on machine change

    var matchingDefs = S.data.macroDefs.filter(function(d) {
      return d.machine === newMachine;
    });
    var allParamsDef = matchingDefs.find(function(d) {
      return d.id.indexOf('allparams') !== -1;
    });
    var def = allParamsDef || matchingDefs[0] || null;
    state.activeMacroDef = def;

    state.paramValues = [];
    if (def && def.groups) {
      def.groups.forEach(function(group) {
        group.parameters.forEach(function(param) {
          state.paramValues[param.idx] = param.def || 0;
        });
      });
    }

    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    renderKnobControls(track, def);
    renderPresetBrowser();
    sendTrackUpdate({ track: state.activeTrack, machine: newMachine });
  }

  // ─── Initialization ─────────────────────────────────────

  function init() {
    S.onTrackChange(function(idx, track) {
      onTrackSelected(idx, track);
    });

    setupPresetBrowserEvents();
    setupQuickActions();

    // Re-render when factory lock state changes (buttons, delete visibility)
    window.addEventListener('tbd-factory-lock-changed', function() {
      var track = S.data.tracks ? S.data.tracks.find(function(t) { return t.index === state.activeTrack; }) : null;
      if (track) renderTrackInfoBar(track, state.activeMacroDef);
      renderPresetBrowser();
    });

    if (S.data.loaded && S.data.tracks.length > 0) {
      S.selectTrack(S.data.tracks[0].index);
    }

    state.initialized = true;
  }

  // ─── Set Macro Def (called by designer in Macros mode) ────

  function setMacroDef(def) {
    state.activeMacroDef = def;
    if (def) {
      state.activeMachine = def.machine;
    }
    state.paramValues = [];
    if (def && def.groups) {
      def.groups.forEach(function(group) {
        group.parameters.forEach(function(param) {
          state.paramValues[param.idx] = param.def || 0;
        });
      });
    }
    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) {
      renderKnobControls(track, def);
      renderTrackInfoBar(track, def);
    }
  }

  function refreshTrackInfoBar() {
    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) renderTrackInfoBar(track, state.activeMacroDef);
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.performer = {
    init: init,
    state: state,
    setMacroDef: setMacroDef,
    refreshTrackInfoBar: refreshTrackInfoBar,
    renderKnobControls: function() {
      var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
      if (track) renderKnobControls(track, state.activeMacroDef);
    },
    exportAllPresets: exportAllPresets,
    importPresetFile: importPresetFile,
    savePresetDialog: savePresetDialog,
    loadPreset: loadPreset,
    sendParameterUpdate: sendParameterUpdate,
    reload: function() {
      if (state.activeTrack >= 0) {
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        if (track) onTrackSelected(state.activeTrack, track);
      }
    },
  };

})();
