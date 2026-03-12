#!/usr/bin/env python3
"""
fix_dsp_json_alignment.py — Fix all parameter alignment issues between
C++ DSP registrations and JSON synth/macro definitions.

Fixes:
  1. Parameter swaps in synthdefinitions.json (db, ab, as, td3)
  2. Missing mixer params (ctrl 1-4) in synthdefinitions.json
  3. Parameter name/order fixes in macro definitions
  4. td3 mapping swap fix in macro definitions
  5. Missing Mix pages in macro definitions
  6. wo-allparams.json machine reference (wo → wtosc)
  7. Missing C++ params added to synthdefinitions.json (td3, wtosc)

Usage:
  python3 tests/fix_dsp_json_alignment.py
"""

import json
import os
import sys
import copy

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SYNTHDEF_PATH = os.path.join(REPO_ROOT, 'sdcard_image', 'data', 'synthdefinitions.json')
MACRODEF_DIR = os.path.join(REPO_ROOT, 'sdcard_image', 'data', 'macrodefinitions')

# Machines that should NOT get mixer params
NO_MIX_MACHINES = {'nodrum', 'nosynth', 'nofx', 'fxdelay', 'fxreverb',
                    'fxmaster', 'in', 'inp'}

# Standard mixer params to add (matching RackChannelMixer.cpp offsets 1-4)
MIXER_PARAMS = [
    {"id": "lev", "name": "Level", "type": "cc", "ctrl": 1, "def": 100},
    {"id": "pan", "name": "Pan", "type": "cc", "ctrl": 2, "def": 64},
    {"id": "fx1", "name": "FX1 Send", "type": "cc", "ctrl": 3, "def": 0},
    {"id": "fx2", "name": "FX2 Send", "type": "cc", "ctrl": 4, "def": 0},
]

# Standard Mix page for macro definitions
def make_mix_page(start_idx):
    """Create a Mix page group starting at the given parameter index."""
    return {
        "name": "Mix",
        "parameters": [
            {"idx": start_idx, "name": "Level", "def": 100, "min": 0, "max": 127, "res": 64, "ui": "bignum"},
            {"idx": start_idx + 1, "name": "Pan", "def": 64, "min": 0, "max": 127, "res": 64, "ui": "bignum"},
            {"idx": start_idx + 2, "name": "FX1 Send", "def": 0, "min": 0, "max": 127, "res": 64, "ui": "bignum"},
            {"idx": start_idx + 3, "name": "FX2 Send", "def": 0, "min": 0, "max": 127, "res": 64, "ui": "bignum"},
        ]
    }


def make_mix_mapping(start_idx):
    """Create mapping entries for Mix page params."""
    return [
        {"ctrl": 1, "start": 0, "add": [{"src": start_idx, "mul": 1, "div": 1}]},
        {"ctrl": 2, "start": 0, "add": [{"src": start_idx + 1, "mul": 1, "div": 1}]},
        {"ctrl": 3, "start": 0, "add": [{"src": start_idx + 2, "mul": 1, "div": 1}]},
        {"ctrl": 4, "start": 0, "add": [{"src": start_idx + 3, "mul": 1, "div": 1}]},
    ]


def fix_synthdefinitions(sd):
    """Fix parameter swaps and add mixer params in synthdefinitions.json."""
    changes = []

    for m in sd['machines']:
        mid = m['id']
        params = m.get('parameters', [])

        # === Fix 1: Parameter swaps ===

        if mid == 'db':
            # C++ has: f0@8, tone@9, decay@10, dirty@11, fm_env@12, fm_dcy@13, accent@14
            # JSON has: freq@8, decay@9, tone@10 — swapped!
            # Fix: swap ctrl 9 and 10
            for p in params:
                if p['ctrl'] == 9 and p['id'] == 'decay':
                    p['id'] = 'tone'
                    p['name'] = 'Tone'
                    p['def'] = 64  # tone default from C++
                    changes.append(f"db: ctrl 9 changed from decay→tone")
                elif p['ctrl'] == 10 and p['id'] == 'tone':
                    p['id'] = 'decay'
                    p['name'] = 'Decay'
                    p['def'] = 32  # decay default from C++
                    changes.append(f"db: ctrl 10 changed from tone→decay")

        elif mid == 'ab':
            # C++ has: f0@8, tone@9, decay@10, a_fm@11, s_fm@12, accent@13
            # JSON has: freq@8, decay@9, tone@10 — swapped!
            for p in params:
                if p['ctrl'] == 9 and p['id'] == 'decay':
                    p['id'] = 'tone'
                    p['name'] = 'Tone'
                    p['def'] = 32  # ab tone default
                    changes.append(f"ab: ctrl 9 changed from decay→tone")
                elif p['ctrl'] == 10 and p['id'] == 'tone':
                    p['id'] = 'decay'
                    p['name'] = 'Decay'
                    p['def'] = 64  # ab decay default
                    changes.append(f"ab: ctrl 10 changed from tone→decay")

        elif mid == 'as':
            # C++ has: f0@8, tone@9, decay@10, a_spy@11, accent@12
            # JSON has: freq@8, decay@9, fm@10 — wrong labels!
            # ASD doesn't have FM. ctrl 9 is tone, ctrl 10 is decay.
            for p in params:
                if p['ctrl'] == 9 and p['id'] == 'decay':
                    p['id'] = 'tone'
                    p['name'] = 'Tone'
                    p['def'] = 16  # keep existing default
                    changes.append(f"as: ctrl 9 changed from decay→tone")
                elif p['ctrl'] == 10 and p['id'] == 'fm':
                    p['id'] = 'decay'
                    p['name'] = 'Decay'
                    p['def'] = 16  # keep existing default
                    changes.append(f"as: ctrl 10 changed from fm→decay")

        elif mid == 'td3':
            # C++ has: envelope@14, filter_type@15
            # JSON has: type(filter_type)@14, envdec(envelope)@15 — swapped!
            for p in params:
                if p['ctrl'] == 14 and p['id'] == 'type':
                    p['id'] = 'envdec'
                    p['name'] = 'EnvDec'
                    p['def'] = 16  # envelope default
                    changes.append(f"td3: ctrl 14 changed from type→envdec (matching C++ envelope@14)")
                elif p['ctrl'] == 15 and p['id'] == 'envdec':
                    p['id'] = 'type'
                    p['name'] = 'Type'
                    p['def'] = 0  # filter_type default
                    changes.append(f"td3: ctrl 15 changed from envdec→type (matching C++ filter_type@15)")

        # === Fix 2: Add missing C++ params ===

        if mid == 'td3':
            existing_ctrls = {p['ctrl'] for p in params}
            if 24 not in existing_ctrls:
                params.append({"id": "slidelev", "name": "Slide Lev", "type": "cc", "ctrl": 24, "def": 0})
                changes.append(f"td3: added slide_level@24")
            if 25 not in existing_ctrls:
                params.append({"id": "synctrig", "name": "Sync Trig", "type": "cc", "ctrl": 25, "def": 0})
                changes.append(f"td3: added sync_trig@25")

        elif mid == 'wtosc':
            existing_ctrls = {p['ctrl'] for p in params}
            if 29 not in existing_ctrls:
                params.append({"id": "gain", "name": "Gain", "type": "cc", "ctrl": 29, "def": 64})
                changes.append(f"wtosc: added gain@29")

        elif mid == 'fmb':
            existing_ctrls = {p['ctrl'] for p in params}
            if 16 not in existing_ctrls:
                params.append({"id": "ratiomode", "name": "Ratio Mode", "type": "cc", "ctrl": 16, "def": 0})
                changes.append(f"fmb: added use_ratio_mode@16")
            if 17 not in existing_ctrls:
                params.append({"id": "envsync", "name": "Env Sync", "type": "cc", "ctrl": 17, "def": 0})
                changes.append(f"fmb: added mod_env_sync@17")

        # === Fix 3: Add mixer params ===
        if mid not in NO_MIX_MACHINES:
            existing_ctrls = {p['ctrl'] for p in params}
            for mp in MIXER_PARAMS:
                if mp['ctrl'] not in existing_ctrls:
                    params.append(copy.deepcopy(mp))
                    changes.append(f"{mid}: added mixer param {mp['id']}@{mp['ctrl']}")

        # Sort parameters by ctrl for cleanliness
        m['parameters'] = sorted(params, key=lambda p: p['ctrl'])

    return changes


def fix_macro_definition(filepath, macro_data, machine_id):
    """Fix parameter swaps and add Mix page to a macro definition."""
    changes = []

    groups = macro_data.get('groups', [])
    mapping = macro_data.get('mapping', [])

    # === Fix parameter name swaps in groups ===

    if machine_id == 'db':
        # Swap idx 1 (Decay) and idx 2 (Tone) names/defaults
        for g in groups:
            for p in g.get('parameters', []):
                if p['idx'] == 1 and p['name'] == 'Decay':
                    p['name'] = 'Tone'
                    p['def'] = 64
                    changes.append(f"  idx 1: Decay→Tone")
                elif p['idx'] == 2 and p['name'] == 'Tone':
                    p['name'] = 'Decay'
                    p['def'] = 32
                    changes.append(f"  idx 2: Tone→Decay")

    elif machine_id == 'ab':
        for g in groups:
            for p in g.get('parameters', []):
                if p['idx'] == 1 and p['name'] == 'Decay':
                    p['name'] = 'Tone'
                    p['def'] = 32
                    changes.append(f"  idx 1: Decay→Tone")
                elif p['idx'] == 2 and p['name'] == 'Tone':
                    p['name'] = 'Decay'
                    p['def'] = 64
                    changes.append(f"  idx 2: Tone→Decay")

    elif machine_id == 'as':
        for g in groups:
            for p in g.get('parameters', []):
                if p['idx'] == 1 and p['name'] == 'Decay':
                    p['name'] = 'Tone'
                    p['def'] = 16
                    changes.append(f"  idx 1: Decay→Tone")
                elif p['idx'] == 2 and p['name'] == 'FM':
                    p['name'] = 'Decay'
                    p['def'] = 16
                    changes.append(f"  idx 2: FM→Decay")

    elif machine_id == 'td3':
        # Fix the EnvDec/Type swap in groups AND mapping
        for g in groups:
            for p in g.get('parameters', []):
                if p['idx'] == 6 and p['name'] == 'EnvDec':
                    # Keep name EnvDec at idx 6, but fix mapping below
                    pass
                elif p['idx'] == 7 and p['name'] == 'Type':
                    # Keep name Type at idx 7, but fix mapping below
                    pass

        # Fix the mapping: currently idx 6→ctrl 15, idx 7→ctrl 14 (swapped)
        # Should be: idx 6→ctrl 14 (EnvDec→envelope), idx 7→ctrl 15 (Type→filter_type)
        for m_entry in mapping:
            if m_entry.get('ctrl') == 15:
                for add in m_entry.get('add', []):
                    if add.get('src') == 6:
                        m_entry['ctrl'] = 14
                        changes.append(f"  mapping: idx 6 (EnvDec) ctrl 15→14")
            elif m_entry.get('ctrl') == 14:
                for add in m_entry.get('add', []):
                    if add.get('src') == 7:
                        m_entry['ctrl'] = 15
                        changes.append(f"  mapping: idx 7 (Type) ctrl 14→15")

    # === Fix machine reference ===
    if machine_id == 'wo':
        macro_data['machine'] = 'wtosc'
        changes.append(f"  machine: wo→wtosc")

    # === Add Mix page if missing ===
    if machine_id not in NO_MIX_MACHINES and machine_id not in ('nodrum', 'nosynth', 'nofx',
                                                                  'fxdelay', 'fxreverb',
                                                                  'fxmaster', 'extsynth',
                                                                  'extdrum'):
        has_mix = any(g.get('name') == 'Mix' for g in groups if g.get('parameters'))
        if not has_mix:
            # Find max idx currently used
            max_idx = -1
            for g in groups:
                for p in g.get('parameters', []):
                    if p['idx'] > max_idx:
                        max_idx = p['idx']
            mix_start_idx = max_idx + 1

            # Add Mix page
            groups.append(make_mix_page(mix_start_idx))
            macro_data['groups'] = groups

            # Add Mix mapping entries
            mapping.extend(make_mix_mapping(mix_start_idx))
            macro_data['mapping'] = mapping

            changes.append(f"  Added Mix page (idx {mix_start_idx}-{mix_start_idx + 3})")

    return changes


def main():
    print("=" * 60)
    print("  TBD-16 DSP/JSON Alignment Fix Script")
    print("=" * 60)

    # === Fix synthdefinitions.json ===
    print("\n── Fixing synthdefinitions.json ──")
    with open(SYNTHDEF_PATH) as f:
        sd = json.load(f)

    synthdef_changes = fix_synthdefinitions(sd)
    for c in synthdef_changes:
        print(f"  • {c}")

    with open(SYNTHDEF_PATH, 'w') as f:
        json.dump(sd, f, indent=2)
        f.write('\n')
    print(f"\n  Wrote {SYNTHDEF_PATH}")

    # === Fix macro definitions ===
    print("\n── Fixing macro definitions ──")
    total_macro_changes = 0

    for filename in sorted(os.listdir(MACRODEF_DIR)):
        if not filename.endswith('.json'):
            continue

        filepath = os.path.join(MACRODEF_DIR, filename)
        with open(filepath) as f:
            macro = json.load(f)

        machine_id = macro.get('machine', '')
        macro_changes = fix_macro_definition(filepath, macro, machine_id)

        if macro_changes:
            print(f"\n  {filename} (machine={machine_id}):")
            for c in macro_changes:
                print(f"    {c}")
            total_macro_changes += len(macro_changes)

            with open(filepath, 'w') as f:
                json.dump(macro, f, indent=2)
                f.write('\n')

    print(f"\n  Total macro definition changes: {total_macro_changes}")

    # === Summary ===
    print(f"\n{'=' * 60}")
    print(f"  DONE: {len(synthdef_changes)} synthdef changes, {total_macro_changes} macrodef changes")
    print(f"  Run test_dsp_json_alignment.py to verify.")
    print(f"{'=' * 60}")


if __name__ == '__main__':
    main()
