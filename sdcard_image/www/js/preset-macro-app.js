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
    var btn = document.getElementById('settings-btn');
    if (btn) {
      btn.addEventListener('click', function() {
        S.toast('Settings panel (coming soon)', 'neutral', 2000);
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
