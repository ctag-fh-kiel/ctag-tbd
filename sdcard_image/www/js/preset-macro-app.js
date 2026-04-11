// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — App Controller
//
// Replaces app-persona.js. No persona switching.
// Handles: boot sequence, theme, connection monitor,
// sidebar tab switching, toolbar button context.
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // Current sidebar tab: 'presets' or 'macros'
  var activeTab = 'presets';

  // ─── Theme (delegated to shared.js) ────────────────────────

  // ─── Settings ─────────────────────────────────────────────

  function setupSettings() {
    var btn = document.getElementById('config-btn');
    if (btn) {
      btn.addEventListener('click', function() {
        window.location.href = '/index.html?view=plugins&openConfig=1';
      });
    }
  }

  // ─── Quick Start Guide ───────────────────────────────────

  function setupQuickStart() {
    var dialog = document.getElementById('quickstart-dialog');
    var openBtn = document.getElementById('quickstart-btn');
    var closeBtn = document.getElementById('qs-close-btn');

    if (openBtn && dialog) {
      openBtn.addEventListener('click', function() {
        dialog.show();
      });
    }
    if (closeBtn && dialog) {
      closeBtn.addEventListener('click', function() {
        dialog.hide();
        localStorage.setItem('tbd-qs-seen', '1');
      });
    }

    // Auto-show on first visit
    if (!localStorage.getItem('tbd-qs-seen') && dialog) {
      setTimeout(function() { dialog.show(); }, 800);
    }
  }

  // ─── Connection Monitor (delegated to shared.js) ─────────

  // ─── Sidebar Tab Switching ───────────────────────────────

  function setupSidebarTabs() {
    document.querySelectorAll('.sidebar-tab').forEach(function(tab) {
      tab.addEventListener('click', function() {
        var tabName = tab.getAttribute('data-tab');
        switchTab(tabName);
      });
    });
  }

  function switchTab(tabName) {
    activeTab = tabName;

    // Update shared state so performer knows the mode
    if (S.setActiveTab) S.setActiveTab(tabName);

    // Update tab active states
    document.querySelectorAll('.sidebar-tab').forEach(function(tab) {
      tab.classList.toggle('active', tab.getAttribute('data-tab') === tabName);
    });

    // Update sidebar content visibility
    document.querySelectorAll('.sidebar-content').forEach(function(content) {
      content.classList.toggle('active', content.getAttribute('data-tab') === tabName);
    });

    // Show/hide center panel sub-tabs
    var subtabs = document.getElementById('center-subtabs');
    if (subtabs) {
      subtabs.classList.toggle('hidden', tabName !== 'macros');
    }

    // Update center panel content visibility
    updateCenterPanelVisibility();

    // Re-render knob controls (to update knob set dropdown visibility)
    if (window.TBD.performer && window.TBD.performer.renderKnobControls) {
      window.TBD.performer.renderKnobControls();
    }

    // When switching to macros, sync performer with designer's selected definition
    if (tabName === 'macros') {
      if (window.TBD.designer && window.TBD.designer.state && window.TBD.designer.state.editDef) {
        if (window.TBD.performer && window.TBD.performer.setMacroDef) {
          window.TBD.performer.setMacroDef(window.TBD.designer.state.editDef);
        }
      }
    }
  }

  // ─── Center Panel Sub-tabs (Macros mode) ─────────────────

  var activeSubTab = 'knob-preview';

  function setupCenterSubTabs() {
    document.querySelectorAll('.center-subtab').forEach(function(tab) {
      tab.addEventListener('click', function() {
        activeSubTab = tab.getAttribute('data-subtab');
        document.querySelectorAll('.center-subtab').forEach(function(t) {
          t.classList.toggle('active', t.getAttribute('data-subtab') === activeSubTab);
        });
        updateCenterPanelVisibility();

        // When switching to Knob Preview, re-render with the latest editDef
        if (activeSubTab === 'knob-preview' && activeTab === 'macros') {
          var D = window.TBD.designer;
          if (D && D.state && D.state.editDef) {
            if (window.TBD.performer && window.TBD.performer.setMacroDef) {
              window.TBD.performer.setMacroDef(D.state.editDef);
            }
          }
        }
      });
    });
  }

  function updateCenterPanelVisibility() {
    var knobControls = document.getElementById('knob-controls');
    var macroBuilder = document.getElementById('macro-builder-section');
    var defHeader = document.getElementById('macro-def-header');
    var knobPreviewExtras = document.getElementById('knob-preview-extras');

    if (activeTab === 'presets') {
      // Presets mode: always show knobs, hide macro builder and macro-specific panels
      if (knobControls) knobControls.classList.remove('hidden');
      if (macroBuilder) macroBuilder.classList.add('hidden');
      if (defHeader) defHeader.classList.add('hidden');
      if (knobPreviewExtras) knobPreviewExtras.classList.add('hidden');
    } else {
      // Macros mode: show based on selected sub-tab
      if (knobControls) knobControls.classList.toggle('hidden', activeSubTab !== 'knob-preview');
      if (macroBuilder) macroBuilder.classList.toggle('hidden', activeSubTab !== 'macro-builder');
      // Def header always visible in macros mode (renderDefHeader handles empty state)
      if (defHeader) defHeader.classList.remove('hidden');
      // Sound presets for def only visible in knob-preview sub-tab
      if (knobPreviewExtras) knobPreviewExtras.classList.toggle('hidden', activeSubTab !== 'knob-preview');
    }
  }

  // ─── Sidebar Preset Actions (Export/Import) ──────────────

  function setupPresetExportImport() {
    var exportBtn = document.getElementById('qa-export-presets');
    if (exportBtn) {
      exportBtn.addEventListener('click', function() {
        if (window.TBD.performer && window.TBD.performer.exportAllPresets) {
          window.TBD.performer.exportAllPresets();
        }
      });
    }

    var importBtn = document.getElementById('qa-import-presets');
    if (importBtn) {
      importBtn.addEventListener('click', function() {
        if (window.TBD.performer && window.TBD.performer.importPresetFile) {
          window.TBD.performer.importPresetFile();
        }
      });
    }
  }

  // ─── Keyboard Shortcuts ───────────────────────────────────

  function setupKeyboard() {
    document.addEventListener('keydown', function(e) {
      // Ctrl+1 / Ctrl+2 to switch sidebar tabs
      if (e.ctrlKey || e.metaKey) {
        if (e.key === '1') { e.preventDefault(); switchTab('presets'); }
        if (e.key === '2') { e.preventDefault(); switchTab('macros'); }
      }
    });
  }

  // ─── URL Parameter Handling (cross-links from FILE VIEWER) ─

  function handleUrlParams() {
    var params = new URLSearchParams(window.location.search);
    var tab = params.get('tab');
    var openDef = params.get('openDef');
    var openPreset = params.get('openPreset');

    if (tab === 'macros' || openDef) {
      switchTab('macros');
    } else if (tab === 'presets' || openPreset) {
      switchTab('presets');
    }

    if (openDef && S.data.macroDefs) {
      // Find the macro definition by ID
      var def = S.data.macroDefs.find(function(d) { return d.id === openDef; });
      if (def) {
        // Find a track that has the matching machine
        var targetTrack = S.data.tracks.find(function(t) {
          return (t.machines || []).indexOf(def.machine) >= 0;
        });
        if (targetTrack) {
          S.selectTrack(targetTrack.index);
        }
        // Give UI time to settle after track selection, then select the definition
        setTimeout(function() {
          if (window.TBD.designer && window.TBD.designer.selectMacroDefinition) {
            window.TBD.designer.selectMacroDefinition(openDef);
          }
        }, 300);
      }
    }

    if (openPreset && S.data.soundPresets) {
      // Find preset by ID and select its macro to show it in context
      var preset = S.data.soundPresets.find(function(p) { return p.id === openPreset; });
      if (preset && preset.macro) {
        var pDef = S.data.macroDefs.find(function(d) { return d.id === preset.macro; });
        if (pDef) {
          var targetTrack = S.data.tracks.find(function(t) {
            return (t.machines || []).indexOf(pDef.machine) >= 0;
          });
          if (targetTrack) {
            S.selectTrack(targetTrack.index);
          }
        }
        // Give UI time to settle, then select the macro + scroll to preset
        setTimeout(function() {
          if (window.TBD.designer && window.TBD.designer.selectMacroDefinition && preset.macro) {
            window.TBD.designer.selectMacroDefinition(preset.macro);
          }
          // Highlight the target preset card after a short additional delay
          setTimeout(function() {
            var presetCard = document.querySelector('.sp-card[data-preset-id="' + openPreset + '"]');
            if (presetCard) {
              presetCard.scrollIntoView({ behavior: 'smooth', block: 'center' });
              presetCard.style.outline = '2px solid var(--sl-color-primary-500)';
              setTimeout(function() { presetCard.style.outline = ''; }, 2000);
            }
          }, 200);
        }, 300);
      }
    }

    // Clean URL params without triggering a reload
    if (params.toString()) {
      window.history.replaceState({}, '', window.location.pathname);
    }
  }

  // ─── Boot Sequence ────────────────────────────────────────

  function boot() {
    console.log('[TBD-16] Booting unified UI…');

    S.setupThemeToggle('theme-toggle');
    setupSettings();
    setupQuickStart();
    S.startConnectionMonitor();
    setupSidebarTabs();
    setupCenterSubTabs();
    setupPresetExportImport();
    setupKeyboard();

    // Factory lock button in footer
    if (window.TBD.factory && window.TBD.factory.setupFooterLock) {
      window.TBD.factory.setupFooterLock();
    }

    // Load shared data, then init modules
    S.loadSharedData().then(function() {
      console.log('[TBD-16] Shared data loaded');

      // Render track overview
      S.renderTrackOverview();
      S.setupTrackOverviewEvents();

      // Init modules
      if (window.TBD.performer && window.TBD.performer.init) {
        window.TBD.performer.init();
      }
      if (window.TBD.designer && window.TBD.designer.init) {
        window.TBD.designer.init();
      }
      if (window.TBD.trackDefaults && window.TBD.trackDefaults.init) {
        window.TBD.trackDefaults.init();
      }

      // Auto-select first track
      if (S.data.tracks.length > 0) {
        S.selectTrack(S.data.tracks[0].index);
      }

      // Handle URL params (cross-links from FILE VIEWER)
      handleUrlParams();

      S.hideLoading();
      console.log('[TBD-16] Boot complete');
    }).catch(function(err) {
      console.error('[TBD-16] Boot failed:', err);
      S.toast('Boot failed: ' + err.message, 'danger', 5000);
      S.hideLoading();
    });
  }

  // ─── Start on DOM ready ──────────────────────────────────

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', boot);
  } else {
    boot();
  }

})();
