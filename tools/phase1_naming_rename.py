#!/usr/bin/env python3
"""
Phase 1 of the TBD-16 naming-audit coordinated rename pass.

See tbd-pico-seq3/docs/architecture/naming-audit-april-2026.md for the full plan.

This script applies only the no-risk, data-only JSON edits:
  1a  factory-default preset names  → "Default"
  1b  placeholder preset groups     → "FX" / "None"
  1c  preset groups aligned to OLED machine label
  1d  macro display names           → drop "All param" suffix

Does NOT touch:
  - macro `id` fields (would cascade into trackdefaults + presets)
  - Phase 1e firmware edit (DigSnr → DigSnare in commonrender.cpp) — done by hand
  - Phase 2 items

Idempotent: running twice is a no-op.

Usage:
  python3 tools/phase1_naming_rename.py            # apply edits
  python3 tools/phase1_naming_rename.py --dry-run  # show what would change
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
PRESETS = REPO / "sdcard_image" / "factory" / "presets"
MACROS = REPO / "sdcard_image" / "factory" / "macros"
SYNTHDEFS = REPO / "sdcard_image" / "factory" / "synthdefinitions.json"
TRACKDEFAULTS = REPO / "sdcard_image" / "factory" / "trackdefaults" / "default.json"


# ---- 1a. Preset display names ------------------------------------------------
# Each *-def.json is the boot default for its machine. Its .name reads
# "Default" so the OLED never shows "<MachineLabel> / <SameMachineLabel>".
# Rompler is the only exception: it ships two variants, so we differentiate
# with "Basic" and "Extended" (matching ro-basic and ro-full macros).
PRESET_NAME_MAP = {
    "ro-all-def.json":      "Basic",
    "ro-full-def.json":     "Extended",
    "pp-all-def.json":      "Default",
    "db-all-def.json":      "Default",
    "fmb-all-def.json":     "Default",
    "cl-all-def.json":      "Default",
    "rs-all-def.json":      "Default",
    "ds-all-def.json":      "Default",
    "as-all-def.json":      "Default",
    "ab-all-def.json":      "Default",
    "hh1-all-def.json":     "Default",
    "hh2-all-def.json":     "Default",
    "mo-all-def.json":      "Default",
    "td3-all-def.json":     "Default",
    "wtosc-all-def.json":   "Default",
}

# ---- 1b. Placeholder group fixes ---------------------------------------------
PRESET_GROUP_PLACEHOLDER_FIXES = {
    "fxdelay-all-def.json": "FX",
    "fxreverb-def.json": "FX",
    "fxmaster-def.json": "FX",
    "nofx-all-def.json": "None",
}

# ---- 1c. Preset groups aligned to OLED machine label -------------------------
# All presets whose "macro" prefix matches get the canonical short label.
# Prefixes checked against the first '-' component of preset["macro"].
MACHINE_LABEL_BY_PREFIX = {
    "mo":       "MonoSyn",
    "ro":       "Rompler",
    "pp":       "PolyPad",
    "td3":      "TBD03",
    "wtosc":    "WT Osc",
    "db":       "SynKick",
    "ab":       "AnaKick",
    "fmb":      "FM Kick",
    "ds":       "DigSnare",
    "as":       "AnaSnare",
    "hh1":      "Hat 1",
    "hh2":      "Hat 2",
    "cl":       "Clap",
    "rs":       "Rimshot",
    "inp":      "Audio In",
    "extdrum":  "ExtDrum",
    "extsynth": "ExtSyn",
    # fx* presets get "FX" group via 1b placeholder fix above, not machine-label
    # nodrum/nosynth/nofx: skip — not real machines
}

# Presets that already have correct machine-label groups and should not be
# forced if the file also matches a 1b rule. 1b wins (FX / None).
SKIP_GROUP_MACHINE_ALIGN = set(PRESET_GROUP_PLACEHOLDER_FIXES.keys())

# ---- 1e. synthdefinitions.json machines[].name alignment -------------------
# These are canonical machine display names surfaced by the WebUI machine picker
# and other contexts. Align to the manual's machine page titles (which are the
# authoritative user-facing names).
SYNTHDEF_MACHINE_NAME_UPDATES = {
    "mo":       "Mono Synth",        # was "Macro osc"
    "td3":      "TBD03",             # was "TBD03" → briefly "Acid Bass" → reverted per Rule 1 (OLED label = canonical)
    "ab":       "Analog Bass Drum",  # was "Analog Kick"
    "pp":       "PolyPad",           # was "Polypad"
    "wtosc":    "Wavetable Osc",     # was "Wavetable osc"
    "hh1":      "Hi-Hat 1",          # was "Hihat 1"
    "hh2":      "Hi-Hat 2",          # was "Hihat 2"
    "inp":      "Audio Input",       # was "External input"
    "extsynth": "External Synth",    # was "MIDI Synth"
    "extdrum":  "External Drum",     # was "MIDI Drum"
    # unchanged: db, fmb, ds, as, rs, cl, ro, fxdelay, fxreverb, fxmaster,
    #            nodrum, nosynth, nofx
}

# ---- 1f. trackdefaults/default.json _name comment updates -------------------
# _name is a human-readable comment (underscore-prefixed, ignored by loaders).
# Keep it honest as machine names shift.
TRACKDEFAULTS_NAME_UPDATES = {
    # (index, new _name). Only entries that currently drift.
    10: "Lead     — Mono Synth (mo)",       # was "Mono Osc (mo)"
    11: "Lead2    — Mono Synth Bass (mo)",  # now boots with mo-bass preset; was "Mono Osc (mo/wtosc)"
}


# ---- 1d. Macro display name cleanup -----------------------------------------
MACRO_NAME_UPDATES = {
    "ab-allparams.json":      "Analog Bass Drum",
    "as-allparams.json":      "Analog Snare",
    "cl-allparams.json":      "Clap",
    "db-allparams.json":      "Synth Kick",
    "ds-allparams.json":      "Digital Snare",
    "fmb-allparams.json":     "FM Kick",
    "hh1-allparams.json":     "Hi-Hat 1",
    "hh2-allparams.json":     "Hi-Hat 2",
    "pp-allparams.json":      "PolyPad",
    "rs-allparams.json":      "Rimshot",
    "mo-allparams.json":      "Mono Synth",
    "fxdelay-all.json":  "Delay FX",
    "fxreverb-all.json": "Reverb FX",
    "fxmaster-all.json": "Master FX",
}


def _load(path: Path) -> dict:
    return json.loads(path.read_text())


def _save(path: Path, data: dict, *, dry_run: bool) -> None:
    new_text = json.dumps(data, indent=2, ensure_ascii=False) + "\n"
    if dry_run:
        return
    if path.read_text() != new_text:
        path.write_text(new_text)


def _macro_prefix(macro_id: str) -> str:
    """Return the prefix before the first '-' (e.g. 'ro-basic' → 'ro')."""
    return macro_id.split("-", 1)[0] if "-" in macro_id else macro_id


def apply(dry_run: bool) -> list[tuple[str, str, str, str]]:
    """Apply all Phase-1 edits. Returns list of (file, field, before, after)."""
    changes: list[tuple[str, str, str, str]] = []

    # 1a
    for fname, new_name in PRESET_NAME_MAP.items():
        path = PRESETS / fname
        if not path.exists():
            print(f"WARN: {fname} missing", file=sys.stderr)
            continue
        data = _load(path)
        before = data.get("name", "")
        if before != new_name:
            data["name"] = new_name
            changes.append((f"presets/{fname}", "name", before, new_name))
            _save(path, data, dry_run=dry_run)

    # 1b
    for fname, new_group in PRESET_GROUP_PLACEHOLDER_FIXES.items():
        path = PRESETS / fname
        if not path.exists():
            print(f"WARN: {fname} missing", file=sys.stderr)
            continue
        data = _load(path)
        before = data.get("group", "")
        if before != new_group:
            data["group"] = new_group
            changes.append((f"presets/{fname}", "group", before, new_group))
            _save(path, data, dry_run=dry_run)

    # 1c — iterate every preset file, align group to its machine prefix
    for path in sorted(PRESETS.glob("*.json")):
        fname = path.name
        if fname in SKIP_GROUP_MACHINE_ALIGN:
            continue
        data = _load(path)
        macro_id = data.get("macro", "")
        if not macro_id:
            continue
        prefix = _macro_prefix(macro_id)
        target = MACHINE_LABEL_BY_PREFIX.get(prefix)
        if target is None:
            continue
        before = data.get("group", "")
        if before != target:
            data["group"] = target
            changes.append((f"presets/{fname}", "group", before, target))
            _save(path, data, dry_run=dry_run)

    # 1d
    for fname, new_name in MACRO_NAME_UPDATES.items():
        path = MACROS / fname
        if not path.exists():
            print(f"WARN: {fname} missing", file=sys.stderr)
            continue
        data = _load(path)
        before = data.get("name", "")
        if before != new_name:
            data["name"] = new_name
            changes.append((f"macros/{fname}", "name", before, new_name))
            _save(path, data, dry_run=dry_run)

    # 1e — synthdefinitions.json machines[].name
    if SYNTHDEFS.exists():
        sd = _load(SYNTHDEFS)
        touched = False
        for m in sd.get("machines", []):
            mid = m.get("id", "")
            new_name = SYNTHDEF_MACHINE_NAME_UPDATES.get(mid)
            if new_name is None:
                continue
            before = m.get("name", "")
            if before != new_name:
                m["name"] = new_name
                changes.append((f"synthdefinitions.json [machines/{mid}]",
                                "name", before, new_name))
                touched = True
        if touched:
            _save(SYNTHDEFS, sd, dry_run=dry_run)
    else:
        print("WARN: synthdefinitions.json missing", file=sys.stderr)

    # 1f — trackdefaults/default.json _name comments
    if TRACKDEFAULTS.exists():
        td = _load(TRACKDEFAULTS)
        touched = False
        for entry in td.get("tracks", []):
            idx = entry.get("index")
            new_name = TRACKDEFAULTS_NAME_UPDATES.get(idx)
            if new_name is None:
                continue
            before = entry.get("_name", "")
            if before != new_name:
                entry["_name"] = new_name
                changes.append((f"trackdefaults/default.json [index {idx}]",
                                "_name", before, new_name))
                touched = True
        if touched:
            _save(TRACKDEFAULTS, td, dry_run=dry_run)
    else:
        print("WARN: trackdefaults/default.json missing", file=sys.stderr)

    return changes


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true",
                    help="report changes without writing files")
    args = ap.parse_args()

    changes = apply(dry_run=args.dry_run)

    if not changes:
        print("No changes — already up to date.")
        return 0

    # Group by file section for readable diff
    print(f"{'File':<48} {'Field':<6} {'Before':<24} → After")
    print("-" * 96)
    for rel, field, before, after in changes:
        print(f"{rel:<48} {field:<6} {before!r:<24} → {after!r}")

    print(f"\n{len(changes)} field(s) {'would change' if args.dry_run else 'changed'}.")
    if args.dry_run:
        print("Run without --dry-run to apply.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
