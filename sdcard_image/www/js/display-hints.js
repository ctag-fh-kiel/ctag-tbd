// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Display Hints & Value Conversion Layer
// Vanilla JS · No dependencies
//
// Maps raw DSP parameter IDs to semantic display metadata:
//   units, physical ranges, scaling, format rules.
//
// Sources (priority order):
//   1. Macro Device JSON ui field (future, Phase 10)
//   2. mui schema extension physMin/physMax (future, firmware)
//   3. Built-in heuristic table (this file — works now)
//
// Aligned with MACRO-PRESET-SPEC.md §7 Display Types
// and PicoSeqRack PARAMTYPE enum from firmware.
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

  // ─── PARAMTYPE mapping ───────────────────────────────────
  // Maps PicoSeqRack PARAMTYPE enum values to display metadata.
  // These inform what OLED visualization is used on hardware.
  var PARAMTYPE = {
    PT_NONE:          0,
    PT_NUMBER:        1,
    PT_BIG_NUMBER:    2,
    PT_LEVEL:         10,
    PT_PAN:           11,
    PT_FILTER_TYPE:   20,
    PT_FILTER_CUTOFF: 21,
    PT_FILTER_Q:      22,
    PT_ENV_ATTACK:    30,
    PT_ENV_DECAY:     31,
    PT_ENV_AMOUNT:    32,
    PT_DISTORTION:    40,
    PT_SHAPE:         41,
    PT_SHAPE2:        42,
    PT_SHAPE3:        43,
    PT_FREQ:          44,
    PT_NOISE:         45,
    PT_HIDDEN:        100,
  };

  // ─── Heuristic Hint Table ────────────────────────────────
  // Pattern-based: matched against param ID suffix.
  // Each entry: { unit, scale, physMin, physMax, format?, widget? }
  //   scale: 'lin' | 'log'
  //   format: 'pan' | 'percent' | 'db' | 'ratio' | 'semitones' | 'select:...'
  //   widget: 'knob' | 'slider' | 'switch' | 'select' (default: 'knob')

  var SUFFIX_HINTS = [
    // Frequency parameters
    { pattern: /_f0$/,        hint: { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000, label: 'Frequency' } },
    { pattern: /_freq$/,      hint: { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000, label: 'Frequency' } },
    { pattern: /_cutoff$/,    hint: { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000, label: 'Cutoff' } },

    // Envelope time parameters
    { pattern: /_decay$/,     hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 5000,  label: 'Decay' } },
    { pattern: /_atk$/,       hint: { unit: 'ms',  scale: 'log', physMin: 0.5,   physMax: 5000,  label: 'Attack' } },
    { pattern: /_attack$/,    hint: { unit: 'ms',  scale: 'log', physMin: 0.5,   physMax: 5000,  label: 'Attack' } },
    { pattern: /_rel$/,       hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 10000, label: 'Release' } },
    { pattern: /_release$/,   hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 10000, label: 'Release' } },
    { pattern: /_sustain$/,   hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Sustain' } },

    // Level / volume
    { pattern: /_vol$/,       hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Volume', format: 'db' } },
    { pattern: /_level$/,     hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Level', format: 'db' } },
    { pattern: /_gain$/,      hint: { unit: 'dB',  scale: 'lin', physMin: -24,   physMax: 24,    label: 'Gain', format: 'db' } },

    // Panning
    { pattern: /_pan$/,       hint: { unit: '',    scale: 'lin', physMin: -100,  physMax: 100,   label: 'Pan', format: 'pan' } },

    // Filter
    { pattern: /_reso$/,      hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Resonance', format: 'percent' } },
    { pattern: /_q$/,         hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Resonance', format: 'percent' } },

    // Mix / send
    { pattern: /_mix$/,       hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Mix', format: 'percent' } },
    { pattern: /_send$/,      hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Send', format: 'db' } },
    { pattern: /_send1$/,     hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Send 1', format: 'db' } },
    { pattern: /_send2$/,     hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6,     label: 'Send 2', format: 'db' } },

    // Tuning
    { pattern: /_tune$/,      hint: { unit: 'st',  scale: 'lin', physMin: -24,   physMax: 24,    label: 'Tune', format: 'semitones' } },
    { pattern: /_detune$/,    hint: { unit: 'ct',  scale: 'lin', physMin: -100,  physMax: 100,   label: 'Detune' } },

    // Modulation
    { pattern: /_envmod$/,    hint: { unit: '%',   scale: 'lin', physMin: -100,  physMax: 100,   label: 'Env Mod', format: 'percent' } },
    { pattern: /_envdec$/,    hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 5000,  label: 'Env Decay' } },
    { pattern: /_accent$/,    hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Accent', format: 'percent' } },

    // Dynamics
    { pattern: /_ratio$/,     hint: { unit: ':1',  scale: 'log', physMin: 1,     physMax: 20,    label: 'Ratio', format: 'ratio' } },
    { pattern: /_threshold$/, hint: { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 0,     label: 'Threshold', format: 'db' } },

    // FX times
    { pattern: /_time$/,      hint: { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 2000,  label: 'Time' } },
    { pattern: /_feedback$/,  hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Feedback', format: 'percent' } },
    { pattern: /_fb$/,        hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Feedback', format: 'percent' } },
    { pattern: /_size$/,      hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Size', format: 'percent' } },
    { pattern: /_damp$/,      hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Damping', format: 'percent' } },

    // Shape / distortion
    { pattern: /_tone$/,      hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Tone', format: 'percent' } },
    { pattern: /_shape$/,     hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Shape', format: 'percent' } },
    { pattern: /_dirty$/,     hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Dirt', format: 'percent' } },
    { pattern: /_drive$/,     hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Drive', format: 'percent' } },
    { pattern: /_noise$/,     hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'Noise', format: 'percent' } },

    // Generic boolean-ish (mute, enable)
    { pattern: /_mute$/,      hint: { unit: '',    scale: 'lin', physMin: 0,     physMax: 1,     label: 'Mute', widget: 'switch' } },
    { pattern: /_enable$/,    hint: { unit: '',    scale: 'lin', physMin: 0,     physMax: 1,     label: 'Enable', widget: 'switch' } },

    // LFO
    { pattern: /_lfo_rate$/,  hint: { unit: 'Hz',  scale: 'log', physMin: 0.01,  physMax: 50,    label: 'LFO Rate' } },
    { pattern: /_lfo_amt$/,   hint: { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100,   label: 'LFO Amount', format: 'percent' } },
    { pattern: /_speed$/,     hint: { unit: 'Hz',  scale: 'log', physMin: 0.01,  physMax: 50,    label: 'Speed' } },
  ];

  // ─── Name-based heuristics ───────────────────────────────
  // For params where suffix matching fails, try the display name
  var NAME_HINTS = {
    'Frequency':    { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'Cutoff':       { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'Freq':         { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'FREQ':         { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'Decay':        { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 5000 },
    'DECAY':        { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 5000 },
    'Attack':       { unit: 'ms',  scale: 'log', physMin: 0.5,   physMax: 5000 },
    'ATTACK':       { unit: 'ms',  scale: 'log', physMin: 0.5,   physMax: 5000 },
    'Release':      { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 10000 },
    'RELEASE':      { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 10000 },
    'Sustain':      { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'SUSTAIN':      { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Volume':       { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Level':        { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'LEVEL':        { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Pan':          { unit: '',    scale: 'lin', physMin: -100,  physMax: 100, format: 'pan' },
    'PAN':          { unit: '',    scale: 'lin', physMin: -100,  physMax: 100, format: 'pan' },
    'Resonance':    { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'RESO':         { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Tone':         { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'TONE':         { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Mix':          { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'MIX':          { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Drive':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'DRIVE':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Feedback':     { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Noise':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'NOISE':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Accent':       { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'ACCENT':       { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Tune':         { unit: 'st',  scale: 'lin', physMin: -24,   physMax: 24, format: 'semitones' },
    'TUNE':         { unit: 'st',  scale: 'lin', physMin: -24,   physMax: 24, format: 'semitones' },
    'Detune':       { unit: 'ct',  scale: 'lin', physMin: -100,  physMax: 100 },
    'DETUNE':       { unit: 'ct',  scale: 'lin', physMin: -100,  physMax: 100 },
    'Shape':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'SHAPE':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Ratio':        { unit: ':1',  scale: 'log', physMin: 1,     physMax: 20, format: 'ratio' },
    'Threshold':    { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 0, format: 'db' },
    'Time':         { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 2000 },
    'Damping':      { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Size':         { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'FX Send 1':    { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'FX Send 2':    { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Reverb Send':  { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Reverb Level': { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Delay Level':  { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Noise Level':  { unit: 'dB',  scale: 'lin', physMin: -60,   physMax: 6, format: 'db' },
    'Accent Level': { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Slide Level':  { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Overdrive':    { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Saturation':   { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Dirtiness':    { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Pitch':        { unit: 'st',  scale: 'lin', physMin: -24,   physMax: 24, format: 'semitones' },
    'Snappy':       { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Transient':    { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Stereo Width': { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Gain':         { unit: 'dB',  scale: 'lin', physMin: -24,   physMax: 24, format: 'db' },
    'Decimation':   { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Bit Reduction':{ unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Wave Shaping': { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Self FM':      { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Speed':        { unit: 'Hz',  scale: 'log', physMin: 0.01,  physMax: 50 },
    'Low Pass':     { unit: 'Hz',  scale: 'log', physMin: 20,    physMax: 20000 },
    'Slide':        { unit: '%',   scale: 'lin', physMin: 0,     physMax: 100, format: 'percent' },
    'Delay/ms':     { unit: 'ms',  scale: 'log', physMin: 1,     physMax: 2000 },
  };

  // ─── Keyword-based substring matching ────────────────────
  // Matches if param name CONTAINS keyword (case-insensitive).
  // Ordered by specificity — first match wins.
  var KEYWORD_HINTS = [
    // FM synthesis
    { kw: 'frequency',  hint: { unit: 'Hz',  scale: 'log', physMin: 20,  physMax: 20000 } },
    { kw: 'cutoff',     hint: { unit: 'Hz',  scale: 'log', physMin: 20,  physMax: 20000 } },
    { kw: 'decay',      hint: { unit: 'ms',  scale: 'log', physMin: 1,   physMax: 5000 } },
    { kw: 'attack',     hint: { unit: 'ms',  scale: 'log', physMin: 0.5, physMax: 5000 } },
    { kw: 'release',    hint: { unit: 'ms',  scale: 'log', physMin: 1,   physMax: 10000 } },
    { kw: 'sustain',    hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'level',      hint: { unit: 'dB',  scale: 'lin', physMin: -60, physMax: 6, format: 'db' } },
    { kw: 'amount',     hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'index',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'feedback',   hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'envelope',   hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'vibrato',    hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'morph',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'send',       hint: { unit: 'dB',  scale: 'lin', physMin: -60, physMax: 6, format: 'db' } },
    { kw: 'filter',     hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'noise',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'drive',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'width',      hint: { unit: '%',   scale: 'lin', physMin: 0,   physMax: 100, format: 'percent' } },
    { kw: 'pan',        hint: { unit: '',    scale: 'lin', physMin: -100, physMax: 100, format: 'pan' } },
    { kw: 'tune',       hint: { unit: 'st',  scale: 'lin', physMin: -24,  physMax: 24, format: 'semitones' } },
    { kw: 'pitch',      hint: { unit: 'st',  scale: 'lin', physMin: -24,  physMax: 24, format: 'semitones' } },
  ];

  // ─── Resolve hint for a parameter ────────────────────────

  /**
   * Find the best display hint for a parameter.
   * @param {string} paramId - e.g. 'ch1_db_f0' or 'fx1_time'
   * @param {string} paramName - display name from schema, e.g. 'Frequency'
   * @param {object} param - full param object (may contain future physMin/physMax from mui extension)
   * @returns {object|null} hint object or null if no match
   */
  function resolveHint(paramId, paramName, param) {
    // Priority 1: Check if the param itself has physical range metadata (future mui extension)
    if (param && param.physMin !== undefined && param.physMax !== undefined) {
      return {
        unit: param.unit || '',
        scale: param.scale || 'lin',
        physMin: param.physMin,
        physMax: param.physMax,
        label: paramName,
        format: param.format || null,
      };
    }

    // Priority 2: Suffix pattern matching against param ID
    var id = paramId || '';
    for (var i = 0; i < SUFFIX_HINTS.length; i++) {
      if (SUFFIX_HINTS[i].pattern.test(id)) {
        return Object.assign({ label: paramName }, SUFFIX_HINTS[i].hint);
      }
    }

    // Priority 3: Name-based matching (exact)
    var name = (paramName || '').trim();
    if (NAME_HINTS[name]) {
      return Object.assign({ label: name }, NAME_HINTS[name]);
    }

    // Priority 4: Keyword substring matching (case-insensitive)
    var nameLower = name.toLowerCase();
    for (var k = 0; k < KEYWORD_HINTS.length; k++) {
      if (nameLower.indexOf(KEYWORD_HINTS[k].kw) !== -1) {
        return Object.assign({ label: name }, KEYWORD_HINTS[k].hint);
      }
    }

    // No match — return null (caller renders raw mode)
    return null;
  }

  // ─── Value Conversion ────────────────────────────────────

  /**
   * Convert a raw DSP value to a physical display value.
   * @param {number} rawValue - current raw value
   * @param {number} rawMin - schema min (e.g., 0)
   * @param {number} rawMax - schema max (e.g., 4095 or 127)
   * @param {object} hint - display hint from resolveHint()
   * @returns {number} physical value in display units
   */
  function rawToDisplay(rawValue, rawMin, rawMax, hint) {
    if (!hint) return rawValue;
    var range = rawMax - rawMin;
    if (range <= 0) return rawValue;
    var normalized = (rawValue - rawMin) / range;  // 0..1
    normalized = Math.max(0, Math.min(1, normalized));

    if (hint.scale === 'log' && hint.physMin > 0) {
      // Logarithmic: 0..1 → physMin..physMax log-distributed
      return hint.physMin * Math.pow(hint.physMax / hint.physMin, normalized);
    }
    // Linear: 0..1 → physMin..physMax
    return hint.physMin + normalized * (hint.physMax - hint.physMin);
  }

  /**
   * Convert a physical display value back to a raw DSP value.
   * @param {number} displayValue - value in physical units
   * @param {number} rawMin - schema min
   * @param {number} rawMax - schema max
   * @param {object} hint - display hint
   * @returns {number} raw DSP value (integer)
   */
  function displayToRaw(displayValue, rawMin, rawMax, hint) {
    if (!hint) return Math.round(displayValue);
    var range = rawMax - rawMin;
    if (range <= 0) return Math.round(displayValue);

    var normalized;
    if (hint.scale === 'log' && hint.physMin > 0) {
      normalized = Math.log(displayValue / hint.physMin) / Math.log(hint.physMax / hint.physMin);
    } else {
      var physRange = hint.physMax - hint.physMin;
      normalized = physRange !== 0 ? (displayValue - hint.physMin) / physRange : 0;
    }
    normalized = Math.max(0, Math.min(1, normalized));
    return Math.round(rawMin + normalized * range);
  }

  // ─── Value Formatting ────────────────────────────────────

  /**
   * Format a physical value for display with units.
   * @param {number} value - physical value (from rawToDisplay)
   * @param {object} hint - display hint
   * @returns {string} formatted string, e.g. "440 Hz", "L50", "+3.0 dB"
   */
  function formatDisplayValue(value, hint) {
    if (!hint) return String(Math.round(value));

    var fmt = hint.format || '';

    // Pan: L50 / C / R47
    if (fmt === 'pan') {
      if (Math.abs(value) < 1) return 'C';
      return value < 0 ? 'L' + Math.abs(Math.round(value)) : 'R' + Math.round(value);
    }

    // dB: always show sign
    if (fmt === 'db' || hint.unit === 'dB') {
      var sign = value >= 0 ? '+' : '';
      return sign + value.toFixed(1) + ' dB';
    }

    // Ratio: x:1, ∞:1 at high values
    if (fmt === 'ratio') {
      if (value > 100) return '∞:1';
      return value.toFixed(1) + ':1';
    }

    // Semitones: show sign
    if (fmt === 'semitones') {
      var st = Math.round(value);
      return (st >= 0 ? '+' : '') + st + ' st';
    }

    // Percent
    if (fmt === 'percent' || hint.unit === '%') {
      return Math.round(value) + '%';
    }

    // Hz with auto kHz
    if (hint.unit === 'Hz') {
      if (value >= 10000) return (value / 1000).toFixed(1) + ' kHz';
      if (value >= 1000) return (value / 1000).toFixed(2) + ' kHz';
      if (value >= 100) return Math.round(value) + ' Hz';
      if (value >= 10) return value.toFixed(1) + ' Hz';
      return value.toFixed(2) + ' Hz';
    }

    // ms with auto s
    if (hint.unit === 'ms') {
      if (value >= 1000) return (value / 1000).toFixed(2) + ' s';
      if (value >= 100) return Math.round(value) + ' ms';
      if (value >= 10) return value.toFixed(1) + ' ms';
      return value.toFixed(2) + ' ms';
    }

    // Cents
    if (hint.unit === 'ct') {
      var ct = Math.round(value);
      return (ct >= 0 ? '+' : '') + ct + ' ct';
    }

    // Generic with unit
    if (hint.unit) {
      return Math.round(value) + ' ' + hint.unit;
    }

    return String(Math.round(value));
  }

  // ─── Compute step for webaudio-controls ──────────────────

  /**
   * Compute a reasonable step size for a knob/slider.
   * @param {object} hint - display hint
   * @returns {number} step value
   */
  function computeStep(hint) {
    if (!hint) return 1;
    var range = Math.abs(hint.physMax - hint.physMin);
    if (range <= 1) return 0.01;
    if (range <= 10) return 0.1;
    if (range <= 100) return 1;
    if (range <= 1000) return 1;
    return Math.max(1, Math.round(range / 1000));
  }

  // ─── webaudio-controls conv expression builder ───────────
  // The `conv` attribute on <webaudio-knob> evaluates a JS expression
  // where `x` is the current value. We use it for display formatting.

  /**
   * Build the conv expression for a webaudio-controls element.
   * @param {object} hint - display hint
   * @returns {string} JS expression for conv attribute, or empty string
   */
  function buildConvExpr(hint) {
    if (!hint) return '';

    var fmt = hint.format || '';

    if (fmt === 'pan') {
      return "Math.abs(x)<1?'C':x<0?'L'+Math.abs(Math.round(x)):'R'+Math.round(x)";
    }
    if (fmt === 'db' || hint.unit === 'dB') {
      return "(x>=0?'+':'')+x.toFixed(1)+' dB'";
    }
    if (fmt === 'ratio') {
      return "x>100?'∞:1':x.toFixed(1)+':1'";
    }
    if (fmt === 'semitones') {
      return "(Math.round(x)>=0?'+':'')+Math.round(x)+' st'";
    }
    if (fmt === 'percent' || hint.unit === '%') {
      return "Math.round(x)+'%'";
    }
    if (hint.unit === 'Hz') {
      return "x>=1000?(x/1000).toFixed(1)+' kHz':x>=100?Math.round(x)+' Hz':x.toFixed(1)+' Hz'";
    }
    if (hint.unit === 'ms') {
      return "x>=1000?(x/1000).toFixed(2)+' s':x>=100?Math.round(x)+' ms':x.toFixed(1)+' ms'";
    }
    if (hint.unit === 'ct') {
      return "(Math.round(x)>=0?'+':'')+Math.round(x)+' ct'";
    }
    if (hint.unit) {
      return "Math.round(x)+' " + hint.unit + "'";
    }
    return '';
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.displayHints = {
    PARAMTYPE: PARAMTYPE,
    resolveHint: resolveHint,
    rawToDisplay: rawToDisplay,
    displayToRaw: displayToRaw,
    formatDisplayValue: formatDisplayValue,
    computeStep: computeStep,
    buildConvExpr: buildConvExpr,
  };

})();
