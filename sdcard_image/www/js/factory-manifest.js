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

  window.TBD = window.TBD || {};
  window.TBD.factory = {
    isFactoryDefinition: function(id) { return defSet[id] === true; },
    isFactoryPreset: function(id) { return presetSet[id] === true; },
    FACTORY_DEFINITIONS: FACTORY_DEFINITIONS,
    FACTORY_PRESETS: FACTORY_PRESETS,
  };
})();
