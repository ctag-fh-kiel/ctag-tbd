#!/usr/bin/env python3
"""
Replace "bignum" UI type in factory macro definitions with context-appropriate types.

BIGNUM renders as a large 10x16 font number that:
- Overflows the 28px column for 14-bit values (0-16383)
- Shows no units (Hz, ms, etc.)
- Wastes screen space with no graphical feedback

This script maps each "bignum" parameter to a better UI type based on
the parameter name and value range.
"""

import json
import glob
import os
import sys
import re

# Name-based rules (checked in order, first match wins)
NAME_RULES = [
    # Time/envelope parameters
    (r"attack",                   "envattack"),
    (r"decay|release",            "envdecay"),
    (r"^time$",                   "envdecay"),
    (r"sustain",                  "envdecay"),

    # Filter parameters
    (r"lowpass|cutoff|hipass",    "filtercutoff"),
    (r"reso\b",                   "filterq"),

    # Distortion/saturation
    (r"drive|satur|dist|bit[\. ]?red|bit[\. ]?cr", "distortion"),

    # Level/amount/mix/send
    (r"level|lev\b|send|mix\b|gain|feedback|width",  "level"),
    (r"amount|amt\b",            "envamount"),

    # Frequency/pitch/tune
    (r"pitch|tune|trans",        "freq"),
    (r"speed",                   "level"),

    # Specific names
    (r"thresh|ratio|compress",   "level"),
    (r"chord|inversion",         "level"),
    (r"number of|count",         "number"),
    (r"midi ch",                 "number"),
    (r"bank\b",                  "number"),
]


def pick_ui_type(param_name, min_val, max_val):
    """Choose the best UI type for a parameter currently using bignum."""
    name_lower = param_name.lower().strip()

    # Binary toggle (0-1): always use "number" for small display
    if max_val - min_val <= 1:
        return "number"

    # Check name-based rules
    for pattern, ui_type in NAME_RULES:
        if re.search(pattern, name_lower):
            return ui_type

    # Default fallback: "level" (graphical bar, works for any range)
    return "level"


def process_file(filepath, dry_run=False):
    """Process a single macro JSON file. Returns (changes_count, details)."""
    with open(filepath, 'r') as f:
        data = json.load(f)

    changes = []
    for group in data.get('groups', []):
        for param in group.get('parameters', []):
            if param.get('ui') != 'bignum':
                continue

            name = param.get('name', '?')
            min_val = param.get('min', 0)
            max_val = param.get('max', 127)
            new_ui = pick_ui_type(name, min_val, max_val)

            changes.append({
                'idx': param['idx'],
                'name': name,
                'range': f"{min_val}-{max_val}",
                'old_ui': 'bignum',
                'new_ui': new_ui,
            })
            param['ui'] = new_ui

    if changes and not dry_run:
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
            f.write('\n')

    return changes


def main():
    dry_run = '--dry-run' in sys.argv
    macro_dir = os.path.join(os.path.dirname(__file__),
                             '..', 'sdcard_image', 'factory', 'macros')
    macro_dir = os.path.abspath(macro_dir)

    if not os.path.isdir(macro_dir):
        print(f"ERROR: Macro directory not found: {macro_dir}")
        sys.exit(1)

    files = sorted(glob.glob(os.path.join(macro_dir, '*.json')))
    total_changes = 0

    if dry_run:
        print("=== DRY RUN — no files will be modified ===\n")

    for filepath in files:
        changes = process_file(filepath, dry_run)
        if changes:
            fname = os.path.basename(filepath)
            print(f"{fname}:")
            for c in changes:
                print(f"  idx={c['idx']:2d} {c['name']:18s} {c['range']:12s} "
                      f"bignum → {c['new_ui']}")
            total_changes += len(changes)

    print(f"\n{'Would change' if dry_run else 'Changed'} {total_changes} parameters "
          f"across {len(files)} files.")
    if dry_run:
        print("\nRun without --dry-run to apply changes.")


if __name__ == '__main__':
    main()
