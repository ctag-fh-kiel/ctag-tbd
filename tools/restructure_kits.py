#!/usr/bin/env python3
"""
Restructure legacy kit JSON files into 32-slot banks with null padding,
and add smp_bank_meta to sample_rom.json.

This makes legacy kits (def_smp.json, a4_dub.json) compatible with
the WebUI Sample Manager's bank structure (SLICES_PER_BANK=32).
"""

import json
import os
import sys
import shutil
from collections import OrderedDict
from datetime import datetime

SLICES_PER_BANK = 32

SAMPLE_DIR = os.path.join(os.path.dirname(__file__), '..', 'sdcard_image', 'factory', 'kits')

# Bank names for the Default kit, grouped by path
DEFAULT_KIT_BANKS = [
    {"name": "DRUMS", "color": "#4CAF50", "path": "factory/drums"},
    {"name": "OTHER", "color": "#2196F3", "path": "factory/other"},
    {"name": "A4 DUB", "color": "#FFC107", "path": "factory/a4_dub"},
    {"name": "LOOPS", "color": "#FF9800", "path": "factory/loops"},
]

# Bank names for the A4 Dub kit (112 samples / 32 = 4 banks, remainder in 4th)
A4_DUB_BANKS = [
    {"name": "A4 DUB 1", "color": "#4CAF50"},
    {"name": "A4 DUB 2", "color": "#2196F3"},
    {"name": "A4 DUB 3", "color": "#FFC107"},
    {"name": "A4 DUB 4", "color": "#FF9800"},
]


def pad_bank(entries, slots=SLICES_PER_BANK):
    """Pad an entries list with nulls to fill exactly `slots` entries."""
    result = list(entries)
    while len(result) < slots:
        result.append(None)
    return result


def restructure_by_path(entries, bank_defs):
    """Group entries by path into banks of 32 slots each."""
    # Group entries by path
    groups = OrderedDict()
    for bdef in bank_defs:
        groups[bdef["path"]] = []

    for entry in entries:
        if entry is None:
            continue
        path = entry.get("path", "")
        if path in groups:
            groups[path].append(entry)
        else:
            # Entries not matching any bank go to the last bank
            last_path = list(groups.keys())[-1]
            groups[last_path].append(entry)

    # Build padded output
    result = []
    banks_meta = []
    for bdef in bank_defs:
        path = bdef["path"]
        group_entries = groups.get(path, [])
        padded = pad_bank(group_entries)
        result.extend(padded)
        banks_meta.append({"name": bdef["name"], "color": bdef["color"]})

    return result, banks_meta


def restructure_sequential(entries, bank_defs):
    """Split a flat list into sequential 32-slot banks."""
    result = []
    banks_meta = []
    idx = 0
    for bi, bdef in enumerate(bank_defs):
        chunk = entries[idx:idx + SLICES_PER_BANK]
        padded = pad_bank(chunk)
        result.extend(padded)
        banks_meta.append({"name": bdef["name"], "color": bdef["color"]})
        idx += SLICES_PER_BANK

    # If there are leftover entries beyond the defined banks, add more banks
    while idx < len(entries):
        chunk = entries[idx:idx + SLICES_PER_BANK]
        padded = pad_bank(chunk)
        result.extend(padded)
        bi = len(banks_meta)
        banks_meta.append({"name": f"BANK {bi + 1}", "color": "#607D8B"})
        idx += SLICES_PER_BANK

    return result, banks_meta


def count_non_null(entries, start, count):
    """Count non-null entries in a slice of the list."""
    c = 0
    for e in entries[start:start + count]:
        if e is not None and isinstance(e, dict) and e.get("filename"):
            c += 1
    return c


def main():
    sample_rom_path = os.path.join(SAMPLE_DIR, 'sample_rom.json')
    def_smp_path = os.path.join(SAMPLE_DIR, 'def_smp.json')
    a4_dub_path = os.path.join(SAMPLE_DIR, 'a4_dub.json')

    # Backup originals
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    for path in [sample_rom_path, def_smp_path, a4_dub_path]:
        if os.path.exists(path):
            backup = path + f'.backup_{timestamp}'
            shutil.copy2(path, backup)
            print(f"Backed up: {os.path.basename(path)} -> {os.path.basename(backup)}")

    # Load files
    with open(sample_rom_path) as f:
        sample_rom = json.load(f)
    with open(def_smp_path) as f:
        def_smp = json.load(f)
    with open(a4_dub_path) as f:
        a4_dub = json.load(f)

    # ── Restructure def_smp.json ──
    print(f"\n=== def_smp.json: {len(def_smp)} entries ===")
    new_def_smp, def_banks_meta = restructure_by_path(def_smp, DEFAULT_KIT_BANKS)
    print(f"  Result: {len(new_def_smp)} entries in {len(def_banks_meta)} banks")
    for i, bm in enumerate(def_banks_meta):
        start = i * SLICES_PER_BANK
        n = count_non_null(new_def_smp, start, SLICES_PER_BANK)
        print(f"  Bank {i}: {bm['name']} ({n} samples)")

    # ── Restructure a4_dub.json ──
    print(f"\n=== a4_dub.json: {len(a4_dub)} entries ===")
    new_a4_dub, a4_banks_meta = restructure_sequential(a4_dub, A4_DUB_BANKS)
    print(f"  Result: {len(new_a4_dub)} entries in {len(a4_banks_meta)} banks")
    for i, bm in enumerate(a4_banks_meta):
        start = i * SLICES_PER_BANK
        n = count_non_null(new_a4_dub, start, SLICES_PER_BANK)
        print(f"  Bank {i}: {bm['name']} ({n} samples)")

    # ── Update sample_rom.json with smp_bank_meta ──
    smp_bank_meta = [
        {"banks": def_banks_meta},  # index 0 = Default
        {"banks": a4_banks_meta},   # index 1 = A4 Dub
    ]
    sample_rom["smp_bank_meta"] = smp_bank_meta

    # ── Write files ──
    with open(def_smp_path, 'w') as f:
        json.dump(new_def_smp, f, indent=2)
    print(f"\nWrote: {def_smp_path}")

    with open(a4_dub_path, 'w') as f:
        json.dump(new_a4_dub, f, indent=2)
    print(f"Wrote: {a4_dub_path}")

    with open(sample_rom_path, 'w') as f:
        json.dump(sample_rom, f, indent=2)
    print(f"Wrote: {sample_rom_path}")

    # ── Summary ──
    print("\n=== DONE ===")
    print(f"Default:  {len(def_banks_meta)} banks, {len(new_def_smp)} total slots")
    print(f"A4 Dub:   {len(a4_banks_meta)} banks, {len(new_a4_dub)} total slots")
    print(f"sample_rom.json: smp_bank_meta added with {len(smp_bank_meta)} kit entries")


if __name__ == '__main__':
    main()
