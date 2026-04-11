// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Factory Manifest
//
// Read-only registry of factory macro definitions and sound presets
// shipped with the device. Factory items cannot be overwritten or
// deleted — users may only clone/save-as with a new name.
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var FACTORY_DEFINITIONS = [
    'ab-allparams',
    'as-allparams',
    'cl-allparams',
    'db-allparams',
    'db-phatpunch',
    'db-submorph',
    'ds-allparams',
    'ds-snappy',
    'extdrum-allparams',
    'extsynth-allparams',
    'fmb-allparams',
    'fmb-deepfm',
    'fmb-metallic',
    'fxdelay-allparams',
    'fxmaster-allparams',
    'fxreverb-allparams',
    'hh1-allparams',
    'hh2-allparams',
    'inp-allparams',
    'mo-allparams',
    'nodrum-allparams',
    'nofx-allparams',
    'nosynth-allparams',
    'pp-allparams',
    'pp-darkchord',
    'pp-lushpad',
    'ro-allparams',
    'ro-fullrompler',
    'rs-allparams',
    'td3-acidbass',
    'td3-allparams',
    'wtosc-allparams',
    'wtosc-morphpad',
  ];

  var FACTORY_PRESETS = [
    'ab-all-def',
    'as-all-def',
    'cl-all-def',
    'db-all-def',
    'ds-all-def',
    'ds-snap1',
    'extdrum-all-def',
    'extsynth-all-def',
    'fmb-all-def',
    'fxdelay-all-def',
    'fxmaster-all-def',
    'fxreverb-all-def',
    'golem',
    'hh1-all-def',
    'hh2-all-def',
    'inp-all-def',
    'mo-all-def',
    'msp-acidbass1',
    'msp-acidbass2',
    'msp-darkchord1',
    'msp-darkchord2',
    'msp-deepfm1',
    'msp-deepfm2',
    'msp-lushpad1',
    'msp-lushpad2',
    'msp-metallic1',
    'msp-morphpad1',
    'msp-morphpad2',
    'msp-phatpunch1',
    'msp-phatpunch2',
    'msp-submorph1',
    'new-kick',
    'nodrum-all-def',
    'nofx-all-def',
    'nosynth-all-def',
    'pp-all-def',
    'punch-testing',
    'ro-all-def',
    'ro-fullrompler-def',
    'rs-all-def',
    'sub-morphing-preset-',
    'td3-all-def',
    'wtosc-all-def',
  ];

  // Build lookup sets for O(1) checks
  var defSet = {};
  FACTORY_DEFINITIONS.forEach(function(id) { defSet[id] = true; });
  var presetSet = {};
  FACTORY_PRESETS.forEach(function(id) { presetSet[id] = true; });

  // Factory edit unlock state — persisted across pages via sessionStorage
  var _unlocked = sessionStorage.getItem('tbd-factory-unlocked') === '1';
  var FACTORY_PIN = '0000';

  function _persistUnlock() {
    if (_unlocked) {
      sessionStorage.setItem('tbd-factory-unlocked', '1');
    } else {
      sessionStorage.removeItem('tbd-factory-unlocked');
    }
    // Dispatch event so footer lock icon can update
    window.dispatchEvent(new CustomEvent('tbd-factory-lock-changed', { detail: { unlocked: _unlocked } }));
  }

  /**
   * Show a Shoelace dialog asking for the factory PIN.
   * On success, sets unlocked=true and calls onSuccess().
   */
  function showPinDialog(onSuccess) {
    var old = document.getElementById('factory-pin-dialog');
    if (old) old.remove();

    var dialog = document.createElement('sl-dialog');
    dialog.id = 'factory-pin-dialog';
    dialog.label = 'Factory Edit Mode';
    dialog.setAttribute('style', '--width:22rem;');

    dialog.innerHTML = '<p style="font-size:0.85rem;margin:0 0 0.75rem;">Enter the factory PIN to edit protected definitions.</p>'
      + '<sl-input id="factory-pin-input" type="password" placeholder="PIN" size="medium" '
      + 'style="width:100%;" autocomplete="off" autofocus></sl-input>'
      + '<p id="factory-pin-error" style="font-size:0.75rem;color:var(--sl-color-danger-600);margin:0.5rem 0 0;display:none;">Incorrect PIN</p>';

    var cancelBtn = document.createElement('sl-button');
    cancelBtn.setAttribute('slot', 'footer');
    cancelBtn.setAttribute('variant', 'default');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', function() { dialog.hide(); });

    var unlockBtn = document.createElement('sl-button');
    unlockBtn.setAttribute('slot', 'footer');
    unlockBtn.setAttribute('variant', 'warning');
    unlockBtn.setAttribute('style', 'margin-left: var(--sl-spacing-x-small);');
    unlockBtn.textContent = 'Unlock';

    function tryUnlock() {
      var input = document.getElementById('factory-pin-input');
      var errEl = document.getElementById('factory-pin-error');
      var val = input ? input.value.trim() : '';
      if (val === FACTORY_PIN) {
        _unlocked = true;
        _persistUnlock();
        dialog.hide();
        if (typeof onSuccess === 'function') onSuccess();
      } else {
        if (errEl) errEl.style.display = 'block';
        if (input) { input.value = ''; input.focus(); }
      }
    }

    unlockBtn.addEventListener('click', tryUnlock);

    // Allow Enter key to submit
    dialog.addEventListener('keydown', function(e) {
      if (e.key === 'Enter') { e.preventDefault(); tryUnlock(); }
    });

    dialog.appendChild(cancelBtn);
    dialog.appendChild(unlockBtn);
    document.body.appendChild(dialog);
    dialog.addEventListener('sl-after-hide', function() { dialog.remove(); });
    requestAnimationFrame(function() {
      dialog.show();
      // Focus the input after dialog opens
      setTimeout(function() {
        var inp = document.getElementById('factory-pin-input');
        if (inp) inp.focus();
      }, 100);
    });
  }

  /**
   * Setup the footer lock button (present on both index.html and preset-macro-manager.html).
   * - Click when locked → show PIN dialog
   * - Click when unlocked → lock again
   */
  function setupFooterLock() {
    var btn = document.getElementById('factory-lock-btn');
    var icon = document.getElementById('factory-lock-icon');
    if (!btn || !icon) return;

    function updateIcon() {
      icon.setAttribute('name', _unlocked ? 'unlock' : 'lock');
      btn.classList.toggle('unlocked', _unlocked);
      btn.title = _unlocked ? 'Factory Edit Mode (unlocked) — click to lock' : 'Factory Edit Mode — click to unlock';
    }
    // Defer initial icon update until sl-icon is defined to avoid blank icons
    if (customElements.get('sl-icon')) {
      updateIcon();
    } else {
      customElements.whenDefined('sl-icon').then(updateIcon);
    }

    btn.addEventListener('click', function() {
      if (_unlocked) {
        _unlocked = false;
        _persistUnlock();
        updateIcon();
        if (window.TBD.shared && window.TBD.shared.toast) {
          window.TBD.shared.toast('Factory edit mode locked', 'neutral', 2000);
        }
      } else {
        showPinDialog(function() {
          updateIcon();
          if (window.TBD.shared && window.TBD.shared.toast) {
            window.TBD.shared.toast('Factory edit mode unlocked — factory files are now editable', 'warning', 3000);
          }
        });
      }
    });

    // Listen for lock/unlock events from other code paths
    window.addEventListener('tbd-factory-lock-changed', function() { updateIcon(); });
  }

  window.TBD = window.TBD || {};
  window.TBD.factory = {
    isFactoryDefinition: function(id) { return defSet[id] === true; },
    isFactoryPreset: function(id) { return presetSet[id] === true; },
    isUnlocked: function() { return _unlocked; },
    showPinDialog: showPinDialog,
    setupFooterLock: setupFooterLock,
    lock: function() { _unlocked = false; _persistUnlock(); },
    FACTORY_DEFINITIONS: FACTORY_DEFINITIONS,
    FACTORY_PRESETS: FACTORY_PRESETS,
  };
})();
