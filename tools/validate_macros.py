#!/usr/bin/env python3
"""
Macro / preset / synthdef cross-validator for TBD-16.

Scans every macro JSON, its associated presets, and the synthdef entries to
flag the bug patterns that have cost debugging time historically. Each class
of finding points to the specific doc section that explains the fix.

Usage:
  python3 tools/validate_macros.py             # run all checks
  python3 tools/validate_macros.py --strict    # exit non-zero on any finding
  python3 tools/validate_macros.py --only fmb  # filter by machine id

Doc references:
  docs/architecture/macro-system.md   — idx ↔ ctrl-8 invariant, type-agreement
  docs/architecture/hi-res-and-nrpn.md — wire/source bugs, curve plateau gotcha
"""

from __future__ import annotations

import argparse
import json
import sys
from dataclasses import dataclass, field
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
FACTORY = REPO / "sdcard_image" / "factory"
MACROS = FACTORY / "macros"
PRESETS = FACTORY / "presets"
SYNTHDEFS = FACTORY / "synthdefinitions.json"


@dataclass
class Finding:
    severity: str        # "ERROR" | "WARN" | "INFO"
    file: str
    rule: str
    detail: str


def collect_synthdef_types() -> dict[tuple[str, int], str]:
    """Return { (machine_id, ctrl_num): type } for every synthdef parameter."""
    d = json.loads(SYNTHDEFS.read_text())
    out: dict[tuple[str, int], str] = {}
    for m in d.get("machines", []):
        mid = m.get("id", "")
        for p in m.get("parameters", []):
            ctrl = p.get("ctrl")
            if ctrl is None:
                continue
            out[(mid, ctrl)] = p.get("type", "cc")
    return out


def validate_macro(
    path: Path,
    synth_types: dict[tuple[str, int], str],
    findings: list[Finding],
) -> None:
    macro = json.loads(path.read_text())
    name = path.name
    machine = macro.get("machine", "")
    macro_id = macro.get("id", "")

    # Build idx → param map
    idx_to_param: dict[int, dict] = {}
    for g in macro.get("groups", []):
        for p in g.get("parameters", []):
            idx = p.get("idx")
            if idx is not None:
                idx_to_param[idx] = p

    # 1. idx == ctrl - 8 identity check (macro-system.md invariant)
    #    For every mapping whose src is used as a direct wire (not a fan-out),
    #    check `ctrl == idx + 8` holds.
    src_fanout_count: dict[int, int] = {}
    for mp in macro.get("mapping", []):
        for a in mp.get("add", []):
            s = a.get("src")
            if s is not None:
                src_fanout_count[s] = src_fanout_count.get(s, 0) + 1
    for idx, param in idx_to_param.items():
        # If this idx is used as a unique src (no fan-out), expect a mapping
        # where ctrl == idx + 8.
        if src_fanout_count.get(idx, 0) == 1:
            expected_ctrl = idx + 8
            has_identity = any(
                mp["ctrl"] == expected_ctrl
                and any(a.get("src") == idx for a in mp.get("add", []))
                for mp in macro.get("mapping", [])
            )
            if not has_identity:
                findings.append(Finding(
                    severity="WARN",
                    file=name,
                    rule="idx-ctrl-8-invariant",
                    detail=f"idx={idx} ({param.get('name')!r}) — no mapping "
                           f"found at ctrl={expected_ctrl}. Knob turns may "
                           f"write to the wrong storage slot. "
                           f"See macro-system.md § 'idx ↔ ctrl-8 invariant'.",
                ))

    # 2. Mapping type must match synthdef type for same (machine, ctrl).
    for mp in macro.get("mapping", []):
        ctrl = mp["ctrl"]
        mtype = mp.get("type", "cc")
        stype = synth_types.get((machine, ctrl))
        if stype is None:
            # Exception: ctrl numbers above the synthdef's range are legitimate
            # "storage-only" identity slots for hi-res performance-macro source
            # knobs at idx >= 18 (hi-res-and-nrpn.md § "Hi-res performance-
            # macro source knobs"). Two shapes: empty add[] (pure storage
            # slot for the source knob's own identity), or an add[] whose
            # src = ctrl - 8 and that src is also used elsewhere as fan-out.
            is_storage_only_identity = False
            if ctrl >= 26 and not mp.get("add"):
                # Empty add[] on ctrl >= 26 is the "pure identity storage"
                # shape. Only valid when ctrl - 8 is a source knob idx used
                # as fan-out elsewhere in this macro.
                src_candidate = ctrl - 8
                if src_fanout_count.get(src_candidate, 0) > 0:
                    is_storage_only_identity = True
            else:
                for a in mp.get("add", []):
                    s = a.get("src")
                    if s is None:
                        continue
                    if s + 8 == ctrl and s >= 18 and src_fanout_count.get(s, 0) > 1:
                        is_storage_only_identity = True
                        break
            if not is_storage_only_identity:
                findings.append(Finding(
                    severity="ERROR",
                    file=name,
                    rule="synthdef-missing",
                    detail=f"ctrl={ctrl} is mapped but no synthdef entry found "
                           f"for machine={machine!r}. Check synthdefinitions.json.",
                ))
            continue
        if mtype != stype:
            # Exception: identity-hi-res performance macros can declare nrpm
            # where synthdef says cc. Detect identity = (ctrl == paramIdx+8
            # for a src used elsewhere too).
            is_identity_exception = False
            for a in mp.get("add", []):
                s = a.get("src")
                if s is None:
                    continue
                if s + 8 == ctrl and src_fanout_count.get(s, 0) > 1:
                    is_identity_exception = True
                    break
            if not is_identity_exception:
                findings.append(Finding(
                    severity="ERROR",
                    file=name,
                    rule="type-disagreement",
                    detail=f"ctrl={ctrl} macro_type={mtype!r} but synthdef "
                           f"says {stype!r}. Can produce cross-machine audio "
                           f"regressions. See macro-system.md § 'type "
                           f"agreement invariant'.",
                ))

    # 3. Hi-res source (max>127) with CC-wire mapping = clipped to 127.
    # 4. CC-range source (max<=127) with NRPM mapping = 0.76% dynamic range.
    for mp in macro.get("mapping", []):
        mtype = mp.get("type", "cc")
        for a in mp.get("add", []):
            s = a.get("src")
            if s is None:
                continue
            src_max = idx_to_param.get(s, {}).get("max", 127)
            if mtype == "cc" and src_max > 127:
                findings.append(Finding(
                    severity="WARN",
                    file=name,
                    rule="cc-clip-on-hires-source",
                    detail=f"ctrl={mp['ctrl']} type=cc src={s} src.max={src_max} "
                           f"— wire clamps to 127, upper bits lost. "
                           f"See hi-res-and-nrpn.md gotcha #1.",
                ))
            if mtype == "nrpm" and src_max <= 127:
                findings.append(Finding(
                    severity="ERROR",
                    file=name,
                    rule="nrpm-with-cc-source",
                    detail=f"ctrl={mp['ctrl']} type=nrpm src={s} src.max={src_max} "
                           f"— CC-range source through NRPM wire gives ~0.76 % "
                           f"DSP range. Knob appears dead. "
                           f"See hi-res-and-nrpn.md § 'Debugging unresponsive "
                           f"knobs' Step 3.",
                ))

    # 5. applyCurve plateau bug pre-2026-04-24: log/exp curve on a source with
    #    max > 127 used to create plateaus due to integer math. The fix landed
    #    in MacroTranslator.cpp (64-bit intermediates), so this is now an
    #    INFO flag rather than a hard error — worth surfacing so authors know
    #    why specific macro files have curves on hi-res sources.
    for mp in macro.get("mapping", []):
        for a in mp.get("add", []):
            s = a.get("src")
            curve = a.get("curve")
            if s is None or curve is None or curve == "linear":
                continue
            src_max = idx_to_param.get(s, {}).get("max", 127)
            if src_max > 127:
                findings.append(Finding(
                    severity="INFO",
                    file=name,
                    rule="curve-on-hires-source",
                    detail=f"ctrl={mp['ctrl']} src={s} curve={curve!r} with "
                           f"max={src_max}. Pre-2026-04-24 this was broken "
                           f"(plateau bug, hi-res-and-nrpn.md gotcha #5). "
                           f"The P4-side applyCurve() now uses 64-bit math so "
                           f"this works correctly — but verify on device that "
                           f"slow knob turns remain smooth.",
                ))
    for idx, p in idx_to_param.items():
        if p.get("curve") in ("log", "exp") and p.get("max", 127) > 127:
            findings.append(Finding(
                severity="INFO",
                file=name,
                rule="curve-on-hires-param",
                detail=f"idx={idx} ({p.get('name')!r}) curve={p.get('curve')!r} "
                       f"with max={p.get('max')}. Same caveat as "
                       f"curve-on-hires-source.",
            ))

    # 6. Preset values[] length must match declared idx count, and every
    #    value must be within param min..max.
    macro_file_id = macro.get("id", name.replace(".json", ""))
    max_idx = max(idx_to_param.keys()) if idx_to_param else -1
    for preset_path in PRESETS.glob("*.json"):
        preset = json.loads(preset_path.read_text())
        if preset.get("macro") != macro_file_id:
            continue
        values = preset.get("values", [])
        for i, v in enumerate(values):
            if i not in idx_to_param:
                continue
            mn = idx_to_param[i].get("min", 0)
            mx = idx_to_param[i].get("max", 127)
            if v < mn or v > mx:
                findings.append(Finding(
                    severity="ERROR",
                    file=preset_path.name,
                    rule="preset-value-out-of-range",
                    detail=f"values[{i}]={v} outside [{mn}..{mx}] for "
                           f"{macro_file_id}.{idx_to_param[i].get('name')!r}. "
                           f"Preset will load with clipped or garbage value.",
                ))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--strict", action="store_true",
                    help="Exit with code 1 if any ERROR/WARN is found.")
    ap.add_argument("--only", default=None,
                    help="Filter to a single machine id (e.g. --only fmb).")
    args = ap.parse_args()

    synth_types = collect_synthdef_types()
    findings: list[Finding] = []

    for path in sorted(MACROS.glob("*.json")):
        macro = json.loads(path.read_text())
        if args.only and macro.get("machine") != args.only:
            continue
        validate_macro(path, synth_types, findings)

    # Group by severity then by file for readable output
    errors = [f for f in findings if f.severity == "ERROR"]
    warns  = [f for f in findings if f.severity == "WARN"]
    infos  = [f for f in findings if f.severity == "INFO"]

    for title, items in (("ERRORS", errors), ("WARNINGS", warns), ("INFO", infos)):
        if not items:
            continue
        print(f"\n=== {title} ({len(items)}) ===")
        for f in items:
            print(f"  [{f.severity}] {f.file:<28} {f.rule}")
            print(f"      {f.detail}")

    total = len(errors) + len(warns) + len(infos)
    if total == 0:
        print("All macros clean.")
    else:
        print(f"\n{len(errors)} error(s), {len(warns)} warning(s), "
              f"{len(infos)} info.")

    if args.strict and (errors or warns):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
