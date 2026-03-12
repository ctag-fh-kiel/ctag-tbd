#!/usr/bin/env python3
"""
add_curves_to_allparams.py
────────────────────────────────────────────────────────────────
Adds DSP-aware response curve fields to all allparams macro definitions.

Curve types:
  linear  — default, 1:1 knob-to-CC mapping (most params)
  log     — for frequency/cutoff params (pitch perception is logarithmic)
  exp     — for decay/envelope time params (more resolution for short times)

The curve field is added in TWO places:
  1. mapping.add[].curve  — used by C++ MacroTranslator engine
  2. groups.parameters[].curve — used by WebUI for badge display

Usage:
  python3 add_curves_to_allparams.py          # dry-run: show changes
  python3 add_curves_to_allparams.py --apply  # write changes to files
"""

import json
import os
import sys

MACRO_DIR = os.path.join(os.path.dirname(__file__), '..', 'sdcard_image', 'data', 'macrodefinitions')

# ──────────────────────────────────────────────────────────────
# CURVE ASSIGNMENTS: machine → { param_idx: curve_type }
# Only non-linear curves are listed; unlisted params stay "linear".
# ──────────────────────────────────────────────────────────────

CURVE_MAP = {
    # ═══════════════════════════════════════════════════════════
    # CONSERVATIVE curve assignments.
    # Philosophy: LINEAR by default.  Only add curves where
    # they solve a real control problem:
    #   LOG — frequency / cutoff / filter tone (pitch is logarithmic)
    #   EXP — decay / envelope time (more resolution for short times)
    # Everything else stays LINEAR.
    # No SCURVE — it's non-standard for musical instruments.
    # ═══════════════════════════════════════════════════════════

    # ── DRUM MACHINES ──

    "ab": {  # Analog Kick
        0: "log",    # Freq — oscillator frequency
        2: "exp",    # Decay — envelope time
    },

    "as": {  # Analog Snare
        0: "log",    # Freq
        2: "exp",    # Decay
    },

    "cl": {  # Clap
        0: "log",    # Freq
        2: "exp",    # Decay
    },

    "db": {  # Synth Kick
        0: "log",    # Freq
        2: "exp",    # Decay
        5: "exp",    # Fm Decay — envelope time
    },

    "ds": {  # Digital Snare
        0: "log",    # Freq
        1: "exp",    # Decay
    },

    "fmb": {  # FM Kick
        1: "exp",    # DB — body decay time
        3: "exp",    # DM — decay modulation time
        6: "exp",    # DF — FM decay time
    },

    "hh1": {  # Hihat 1
        0: "log",    # Freq
        2: "exp",    # Decay
    },

    "hh2": {  # Hihat 2
        0: "log",    # Freq
        2: "exp",    # Decay
    },

    "rs": {  # Rimshot
        0: "log",    # Freq
        2: "exp",    # Decay
    },

    # ── SYNTH MACHINES ──

    "mo": {  # Macro Osc
        8: "exp",     # Attack — envelope time
        9: "exp",     # Decay — envelope time
    },

    "pp": {  # Polypad
        3: "log",      # Cutoff — filter cutoff
        7: "exp",      # Attack
        8: "exp",      # Decay
        10: "exp",     # Release
    },

    "ro": {  # Rompler
        4: "log",      # Cutoff — filter cutoff
        8: "exp",      # Attack
        9: "exp",      # Decay
    },

    "td3": {  # TBD03
        2: "exp",     # VCA D — VCA decay
        3: "exp",     # VCF D — VCF decay
        4: "log",     # Cutoff
        6: "exp",     # EnvDec — envelope decay
    },

    "wtosc": {  # Wavetable Osc
        4: "log",      # Cutoff — filter cutoff
        7: "exp",      # Attack
        8: "exp",      # Decay
        10: "exp",     # Release
    },

    # ── FX MACHINES ──

    "fxreverb": {
        0: "log",   # Time — reverb time
        1: "log",   # Lowpass — filter
    },

    "fxmaster": {
        2: "exp",   # Attack — envelope time
        3: "exp",   # Release — envelope time
        4: "log",   # LPF — filter cutoff
    },
}


def process_file(filepath, apply=False):
    """Process a single allparams JSON file, adding curve fields."""
    with open(filepath, 'r') as f:
        data = json.load(f)

    machine = data.get('machine', '')
    file_id = data.get('id', '')

    # Only process allparams files
    if 'allparams' not in file_id:
        return None

    if machine not in CURVE_MAP:
        return None  # No curve assignments for this machine

    curves = CURVE_MAP[machine]
    changes = []

    # Build a lookup: param_idx → mapping_index and add_index
    # For allparams, it's always 1:1: each mapping has one source
    src_to_mapping = {}
    for mi, m in enumerate(data.get('mapping', [])):
        for ai, a in enumerate(m.get('add', [])):
            src_to_mapping[a['src']] = (mi, ai)

    for param_idx, curve_type in sorted(curves.items()):
        param_name = "?"

        # 1. Add curve to parameter definition (for WebUI badge)
        for group in data.get('groups', []):
            for p in group.get('parameters', []):
                if p['idx'] == param_idx:
                    param_name = p.get('name', '?')
                    old_curve = p.get('curve')
                    if old_curve != curve_type:
                        p['curve'] = curve_type
                        changes.append(f"  param [{param_idx}] {param_name}: curve → {curve_type}")
                    break

        # 2. Add curve to mapping add entry (for C++ engine)
        if param_idx in src_to_mapping:
            mi, ai = src_to_mapping[param_idx]
            add_entry = data['mapping'][mi]['add'][ai]
            old_curve = add_entry.get('curve')
            if old_curve != curve_type:
                add_entry['curve'] = curve_type
                ctrl = data['mapping'][mi].get('ctrl', '?')
                changes.append(f"  mapping ctrl={ctrl} src={param_idx}: curve → {curve_type}")

    if not changes:
        return None

    if apply:
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
            f.write('\n')

    return changes


def main():
    apply = '--apply' in sys.argv
    mode = "APPLYING" if apply else "DRY-RUN"
    print(f"═══ add_curves_to_allparams.py ({mode}) ═══\n")

    total_changes = 0
    files_changed = 0

    for filename in sorted(os.listdir(MACRO_DIR)):
        if not filename.endswith('.json'):
            continue
        filepath = os.path.join(MACRO_DIR, filename)
        changes = process_file(filepath, apply=apply)
        if changes:
            files_changed += 1
            total_changes += len(changes)
            print(f"📄 {filename}:")
            for c in changes:
                print(c)
            print()

    print(f"────────────────────────────────────")
    print(f"Files modified: {files_changed}")
    print(f"Total curve fields added: {total_changes}")

    if not apply:
        print(f"\nRe-run with --apply to write changes.")


if __name__ == '__main__':
    main()
