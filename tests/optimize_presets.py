#!/usr/bin/env python3
"""
optimize_presets.py — Fix default presets & macro definitions for TBD-16
=========================================================================

Fixes THREE categories of problems:

1. PRESET VALUES:  All *-all-def presets use copy-paste generic values
   (50,70,30,0,20,...) regardless of machine type.  This script replaces
   them with DSP-aware, musically optimised defaults computed from the
   actual C++ parameter ranges.

2. MACRO DEF VALUES:  The `def` field in each macro-definition parameter
   should match the corresponding preset default.

3. DISCRETE PARAMETERS:  Booleans (Slide, Accent on/off, Loop, etc.) and
   enum parameters (filter type, timestretch mode) currently use
   min=0..max=127 but only have 2-5 meaningful values.  This wastes 90%+
   of the encoder range ("turn the knob, nothing happens").  The script
   sets appropriate min/max and adjusts the mapping multiplier so every
   encoder step produces a distinct DSP value.

Usage:
    python3 tests/optimize_presets.py              # dry-run report
    python3 tests/optimize_presets.py --apply       # write changes
"""

import json, os, sys, copy

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MACRODEF_DIR  = os.path.join(BASE, "sdcard_image", "data", "macrodefinitions")
PRESET_DIR    = os.path.join(BASE, "sdcard_image", "data", "macrosoundpresets")
SYNTHDEF_FILE = os.path.join(BASE, "sdcard_image", "data", "synthdefinitions.json")

# ──────────────────────────────────────────────────────────────────────
# DSP KNOWLEDGE BASE
#
# For every allparams macro, define:
#   "values"   — ideal preset values  (0-127 CC scale)
#   "defs"     — ideal def values in the macro definition (usually same)
#   "discrete" — dict of {idx: {"min": M, "max": X, "mul": N}}
#                for params that should use restricted encoder range
#
# Values are computed from the C++ DSP code parameter ranges.
# The formula chain: knob_value → applyCurve() → CC → cv = CC*32 → DSP
# ──────────────────────────────────────────────────────────────────────

import math

# ── Inverse curve: given the CC value the DSP should receive,
#    compute what knob position to store in the preset ──

def inverse_log(cc):
    """Inverse of piecewise log curve: 0-64→0-16, 64-100→16-64, 100-127→64-127"""
    if cc <= 0: return 0
    if cc >= 127: return 127
    if cc <= 64:
        return int(round(cc / 4.0))               # 0-64 → 0-16
    elif cc <= 100:
        return int(round((cc - 64) * 48.0 / 36.0 + 16))  # 64-100 → 16-64
    else:
        return int(round((cc - 100) * 63.0 / 27.0 + 64))  # 100-127 → 64-127

def inverse_exp(cc):
    """Inverse of exp curve: val²/127 = cc → val = sqrt(cc * 127)"""
    if cc <= 0: return 0
    if cc >= 127: return 127
    return int(round(math.sqrt(cc * 127.0)))

def inverse_curve(cc, curve_type):
    """Given desired DSP CC output, return knob position accounting for curve."""
    if curve_type == 'log':
        return inverse_log(cc)
    elif curve_type == 'exp':
        return inverse_exp(cc)
    else:
        return cc  # linear — no change


# ── Curve assignments per machine (must match add_curves_to_allparams.py) ──
from add_curves_to_allparams import CURVE_MAP


# ── Helper: compute CC for a target DSP value ──
def cc_for_linear(target, scale):
    """MK_FLT_PAR_ABS_NOCV: out = cv/4095*scale → CC = target/scale*128"""
    return int(round(target / scale * 128))

def cc_for_minmax(target, lo, hi):
    """MK_FLT_PAR_ABS_MIN_MAX_NOCV: out = cv/4095*(hi-lo)+lo"""
    return int(round((target - lo) / (hi - lo) * 128))

def cc_for_int(target, scale):
    """MK_INT_PAR_ABS_NOCV: out = cv*scale/4096 → CC = target*128/scale"""
    return int(round(target * 128 / scale))


# ──────────────────────────────────────────────────────────────────────
# MACHINE PRESETS
# ──────────────────────────────────────────────────────────────────────

MACHINE_PRESETS = {
    # ── db  (Synth Kick — Peaks BassDrum) ────────────────────────────
    # idx: 0=f0   1=tone  2=decay  3=dirty  4=fm_env  5=fm_dcy  6=accent
    # DSP: f0 0.0005-0.01, tone 0-1, decay 0-1, dirty 0-5,
    #       fm_env 0-5, fm_dcy 0-4, accent 0-1
    "db": {
        "values": [
            cc_for_minmax(0.0015, 0.0005, 0.01),  # f0 ~66 Hz punchy kick
            cc_for_linear(0.45, 1.0),              # tone — warm body
            cc_for_linear(0.20, 1.0),              # decay — tight (~200 ms)
            0,                                      # dirty — clean
            cc_for_linear(0.80, 5.0),              # fm_env — good pitch sweep
            cc_for_linear(0.50, 4.0),              # fm_dcy — medium-fast sweep
            cc_for_linear(0.08, 1.0),              # accent — subtle
        ],
    },

    # ── ab  (Analog Kick — Peaks AnalogBD) ───────────────────────────
    # idx: 0=f0  1=tone  2=decay  3=a_fm  4=s_fm  5=accent
    # DSP: f0 0.0001-0.01, tone 0-1, decay 0-1, a_fm 0-100, s_fm 0-100, accent 0-1
    "ab": {
        "values": [
            cc_for_minmax(0.0012, 0.0001, 0.01),  # f0 — deep analog kick
            cc_for_linear(0.40, 1.0),              # tone — warm
            cc_for_linear(0.25, 1.0),              # decay — tight-medium
            cc_for_minmax(15.0, 0.0, 100.0),       # a_fm — moderate attack FM
            cc_for_minmax(10.0, 0.0, 100.0),       # s_fm — subtle sustain FM
            cc_for_linear(0.08, 1.0),              # accent
        ],
    },

    # ── fmb (FM Kick — custom FM engine) ─────────────────────────────
    # idx: 0=f_b  1=d_b  2=f_m  3=d_m  4=b_m  5=A_f  6=d_f  7=I
    # DSP: f_b 20-200Hz, d_b 0.001-1.0, f_m 40-2000Hz, d_m 0.001-0.5,
    #       b_m 0-16 int, A_f 0-1000, d_f 0.001-0.1, I 0-10
    "fmb": {
        "values": [
            cc_for_minmax(60.0, 20.0, 200.0),    # carrier freq ~60 Hz
            cc_for_minmax(0.30, 0.001, 1.0),      # carrier decay ~300 ms
            cc_for_minmax(180.0, 40.0, 2000.0),   # mod freq ~180 Hz (3rd harmonic)
            cc_for_minmax(0.04, 0.001, 0.5),      # mod decay ~40 ms (fast)
            cc_for_int(6, 16),                     # body mod = 6
            cc_for_minmax(150.0, 0.0, 1000.0),    # FM attack depth
            cc_for_minmax(0.025, 0.001, 0.1),      # FM decay ~25 ms
            cc_for_minmax(3.0, 0.0, 10.0),         # intensity = 3
        ],
    },

    # ── ds  (Digital Snare — Peaks DigitalSnare) ─────────────────────
    # idx: 0=f0  1=decay  2=fm_amt  3=spy(snap)  4=accent
    # DSP: f0 0.0008-0.01, decay 0-1, fm_amt 0-1.5, spy 0-1, accent 0-1
    "ds": {
        "values": [
            cc_for_minmax(0.004, 0.0008, 0.01),   # f0 ~176 Hz snare pitch
            cc_for_linear(0.28, 1.0),              # decay — snappy
            cc_for_linear(0.35, 1.5),              # fm_amt — moderate FM body
            cc_for_linear(0.65, 1.0),              # snap — pronounced!
            cc_for_linear(0.08, 1.0),              # accent
        ],
    },

    # ── as  (Analog Snare — Peaks AnalogSnare) ───────────────────────
    # idx: 0=f0  1=tone  2=decay  3=a_spy(snap)  4=accent
    # DSP: f0 0.001-0.01, tone 0-1, decay 0-1, a_spy 0-1, accent 0-1
    "as": {
        "values": [
            cc_for_minmax(0.004, 0.001, 0.01),    # f0 ~176 Hz
            cc_for_linear(0.50, 1.0),              # tone — balanced
            cc_for_linear(0.25, 1.0),              # decay — snappy
            cc_for_linear(0.60, 1.0),              # snap
            cc_for_linear(0.08, 1.0),              # accent
        ],
    },

    # ── hh1 (Hi-Hat 1 — Peaks HiHat) ────────────────────────────────
    # idx: 0=f0  1=tone  2=decay  3=noise  4=accent
    # DSP: f0 0.0005-0.1, tone 0-1, decay 0-1, noise 0-1, accent 0-1
    "hh1": {
        "values": [
            cc_for_minmax(0.045, 0.0005, 0.1),    # f0 — classic metallic hat
            cc_for_linear(0.55, 1.0),              # tone — bright
            cc_for_linear(0.06, 1.0),              # decay — tight closed hat
            cc_for_linear(0.60, 1.0),              # noise — metallic character
            cc_for_linear(0.08, 1.0),              # accent
        ],
    },

    # ── hh2 (Hi-Hat 2 — Peaks HiHat variant) ────────────────────────
    # idx: 0=f0  1=tone  2=decay  3=noise  4=accent
    # DSP: f0 0.00001-0.1, tone 0-1, decay 0-1, noise 0-1, accent 0-1
    "hh2": {
        "values": [
            cc_for_minmax(0.035, 0.00001, 0.1),   # f0 — slightly different hat
            cc_for_linear(0.60, 1.0),              # tone — bright
            cc_for_linear(0.10, 1.0),              # decay — slightly open
            cc_for_linear(0.55, 1.0),              # noise
            cc_for_linear(0.08, 1.0),              # accent
        ],
    },

    # ── rs  (Rimshot — Peaks Rimshot) ────────────────────────────────
    # idx: 0=f0  1=tone  2=decay  3=noise  4=accent
    # DSP: f0 70-350Hz, tone 0.35-0.65 (base), decay 0.1-0.75,
    #       noise 0-0.2, accent 0.1-1.0
    "rs": {
        "values": [
            cc_for_minmax(200.0, 70.0, 350.0),    # f0 ~200 Hz
            cc_for_minmax(0.50, 0.35, 0.65),       # tone — balanced
            cc_for_minmax(0.25, 0.1, 0.75),        # decay — short-medium
            cc_for_minmax(0.08, 0.0, 0.2),         # noise — some texture
            cc_for_minmax(0.30, 0.1, 1.0),         # accent — moderate
        ],
    },

    # ── cl  (Clap — Peaks Clap) ──────────────────────────────────────
    # idx: 0=f0  1=tone  2=decay  3=scale  4=transient
    # DSP: f0 350-4000Hz(pitch1), tone 1-2.5(reso), decay 0.05-0.3,
    #       scale 0-0.1, transient 0-16
    "cl": {
        "values": [
            cc_for_minmax(1800.0, 350.0, 4000.0),  # f0 ~1800 Hz clap
            cc_for_minmax(1.6, 1.0, 2.5),           # tone — moderate resonance
            cc_for_minmax(0.15, 0.05, 0.3),          # decay — medium
            cc_for_minmax(0.04, 0.0, 0.1),           # scale — moderate attack scatter
            cc_for_int(8, 16),                        # transient — mid
        ],
    },

    # ── ro  (Rompler — sample player) ────────────────────────────────
    # idx: 0=bank 1=slice 2=start 3=end 4=fc 5=fq 6=ft 7=brr
    #      8=atk  9=dcy  10=speed 11=pitch 12=lp 13=lp_pp 14=lp_pos
    #      15=eg2fm 16=tsmode 17=tsamount
    # DSP: end 0-1, speed 0-2, pitch 0-128, dcy 0-50s, etc.
    "ro": {
        "values": [
            0,                                      # bank = 0
            0,                                      # slice = 0
            0,                                      # start = 0 (beginning)
            127,                                    # end = 1.0 (FULL SAMPLE!) ← was 0!
            127,                                    # fc = 1.0 (filter open)
            0,                                      # fq = 0 (no resonance)
            0,                                      # ft = 0 (bypass filter)
            0,                                      # brr = 0 (no bit reduction)
            0,                                      # atk = 0 (immediate)
            cc_for_linear(8.0, 50.0),              # dcy = ~8 s (long enough for samples)
            cc_for_linear(1.0, 2.0),               # speed = 1.0× (NORMAL!) ← was 0.25×!
            64,                                     # pitch = center (64)
            0,                                      # lp = off
            0,                                      # lp_pp = off
            64,                                     # lp_pos = 0.5 (mid)
            0,                                      # eg2fm = 0 (no pitch mod)
            0,                                      # tsmode = 0 (off)
            64,                                     # tsamount = 0.5 (mid)
        ],
        "discrete": {
            6:  {"min": 0, "max": 3,  "mul": 32},   # ft: 0=bypass,1=LP,2=BP,3=HP
            12: {"min": 0, "max": 1,  "mul": 1},    # lp: boolean
            13: {"min": 0, "max": 1,  "mul": 1},    # lp_pp: boolean
            16: {"min": 0, "max": 2,  "mul": 1},    # tsmode: could use mul but low values
        },
    },

    # ── td3 (Acid Bass — RackTBD03) ──────────────────────────────────
    # idx: 0=shape 1=p0 2=vca_d 3=vcf_d 4=cutoff 5=reso 6=envdec
    #      7=type 8=satur 9=drive 10=slide 11=accent 12=p1 13=p0_amt
    #      14=p1_amt 15=acc_lev
    # DSP: shape 0-47, p0/p1 0-32768, vca/vcf_d 0-5s, cutoff -5k-22kHz,
    #       reso 0-1, envelope 0-1, type 0-4, saturation 0-65535,
    #       drive 1-30, slide bool, accent bool, acc_level 0-1
    "td3": {
        "values": [
            0,                                      # shape = CSAW (waveform 0)
            64,                                     # p0 = mid timbre
            cc_for_linear(1.2, 5.0),               # vca_d = ~1.2 s (good bass note)
            cc_for_linear(1.0, 5.0),               # vcf_d = ~1.0 s
            80,                                     # cutoff — moderately open
            cc_for_linear(0.35, 1.0),              # reso — noticeable acid reso
            cc_for_linear(0.40, 1.0),              # envdec — good filter envelope
            1,                                      # type = karlson (via mul=32 → CC=32)
            0,                                      # satur = clean
            cc_for_linear(2.5, 30.0),              # drive — subtle warmth
            0,                                      # slide = off
            0,                                      # accent = off
            32,                                     # p1 = mid color
            0,                                      # p0_amt = no EG mod
            0,                                      # p1_amt = no EG mod
            cc_for_linear(0.20, 1.0),              # acc_lev = moderate
        ],
        "discrete": {
            7:  {"min": 0, "max": 3,  "mul": 32},   # type: 0-3 filter types
            10: {"min": 0, "max": 1,  "mul": 1},    # slide: boolean
            11: {"min": 0, "max": 1,  "mul": 1},    # accent: boolean
        },
    },

    # ── mo  (Macro Osc — Plaits engine) ──────────────────────────────
    # idx: 0=shape 1=p0 2=p1 3=waveshape 4=p0a 5=p1a 6=fma
    #      7=qscale 8=attack 9=decay 10=loopenv 11=decim 12=bitred
    # DSP: shape 0-47, p0/p1 0-32768, waveshape 0-65535,
    #       p0a/p1a 0-63, fma raw, qscale 0-47, attack/decay 0-5s,
    #       loopenv bool, decim 1-31, bitred 0-6
    "mo": {
        "values": [
            0,                                      # shape = CSAW
            64,                                     # p0 = mid timbre
            40,                                     # p1 = low-mid color
            0,                                      # waveshape = clean
            cc_for_int(12, 64),                    # p0a = subtle EG→timbre
            cc_for_int(8, 64),                     # p1a = subtle EG→color
            0,                                      # fma = no FM
            0,                                      # qscale = chromatic
            cc_for_linear(0.02, 5.0),              # attack = ~20 ms (plucky)
            cc_for_linear(1.5, 5.0),               # decay = ~1.5 s
            0,                                      # loopenv = off
            0,                                      # decim = off (0 → 1 in DSP)
            0,                                      # bitred = off
        ],
        "discrete": {
            10: {"min": 0, "max": 1, "mul": 1},    # loopenv: boolean
        },
    },

    # ── wtosc (Wavetable Osc) ────────────────────────────────────────
    # idx: 0=bank 1=wave 2=tune 3=fmode 4=fcut 5=freso 6=qscale
    #      7=atk 8=dcy 9=sus 10=rel 11=eg2wave 12=eg2fm 13=eg2filt
    #      14=lfospeed 15=lfosync 16=l2wave 17=l2am 18=l2fm 19=l2filt
    # DSP: bank 0-15, wave 0-1, tune ±1 (SFT, 64=center!), fmode 0-3,
    #       fcut 0-1, freso 1-20, qscale 0-47, ADSR see below
    "wtosc": {
        "values": [
            0,                                      # bank = 0
            cc_for_linear(0.50, 1.0),              # wave = mid position
            64,                                     # tune = center (SFT: 64→0.0→unison)
            0,                                      # fmode = bypass (clean wavetable)
            127,                                    # fcut = 1.0 (open)
            0,                                      # freso = min
            0,                                      # qscale = chromatic
            cc_for_linear(0.15, 10.0),             # attack = ~150 ms (gentle)
            cc_for_linear(3.0, 10.0),              # decay = ~3 s
            cc_for_linear(0.70, 1.0),              # sustain = 0.7 (pad-like)
            cc_for_linear(1.5, 10.0),              # release = ~1.5 s
            0,                                      # eg2wave = off
            0,                                      # eg2fm = off
            0,                                      # eg2filt = off
            cc_for_linear(1.5, 20.0),              # lfospeed = ~1.5 Hz
            0,                                      # lfosync = off
            0,                                      # l2wave = off
            0,                                      # l2am = off
            0,                                      # l2fm = off
            0,                                      # l2filt = off
        ],
        "discrete": {
            3:  {"min": 0, "max": 3,  "mul": 32},   # fmode: 0=bypass,1=LP,2=BP,3=HP
            15: {"min": 0, "max": 1,  "mul": 1},    # lfosync: boolean
        },
    },

    # ── pp  (Polypad — chord synth) ──────────────────────────────────
    # idx: 0=chord 1=inversion 2=detune 3=cutoff 4=reso 5=type
    #      6=qscale 7=attack 8=decay 9=sustain 10=release
    #      11=l1speed 12=l1amt 13=l2speed 14=l2amt 15=eg_filt
    #      16=l2rphase 17=nnotes
    # DSP: chord 0-N (int), inversion 0-1, detune 0-32k,
    #       cutoff 1750-16384, reso 0-32k, filter_type 0-1,
    #       attack/decay 0-2s, sustain 0-1, release 0-10s,
    #       lfo 0-5Hz, nnotes=1+cv*4/4096
    "pp": {
        "values": [
            0,                                      # chord = first chord
            0,                                      # inversion = root position
            8,                                      # detune = slight warmth
            90,                                     # cutoff = mostly open, slight warmth
            15,                                     # reso = subtle resonance
            0,                                      # type = bypass for clean pad
            0,                                      # qscale = chromatic
            cc_for_linear(0.08, 2.0),              # attack = ~80 ms (gentle)
            cc_for_linear(0.8, 2.0),               # decay = ~800 ms
            cc_for_linear(0.75, 1.0),              # sustain = 0.75 (pad-like)
            cc_for_linear(2.5, 10.0),              # release = ~2.5 s
            cc_for_linear(1.2, 5.0),               # l1speed = ~1.2 Hz vibrato
            cc_for_linear(0.3, 5.0),               # l1amt = subtle vibrato
            cc_for_linear(0.4, 5.0),               # l2speed = ~0.4 Hz filter LFO
            0,                                      # l2amt = off by default
            0,                                      # eg_filt = off by default
            127,                                    # l2rphase = on (random phase)
            64,                                     # nnotes = 3 (1+2048*4/4096=3)
        ],
        "discrete": {
            5:  {"min": 0, "max": 1,  "mul": 64},   # type: 0=bypass, 1=LP (only 2 accessible)
            16: {"min": 0, "max": 1,  "mul": 1},    # l2rphase: boolean
        },
    },

    # ── FX machines (stub implementations — set to macro def values) ─

    # fxdelay: Time, Sync, Freeze, Tapedig, Stereo width, FX2 Send,
    #          Feedback, Base, Width2, Level
    "fxdelay": {
        "values": [16, 0, 0, 0, 32, 0, 32, 0, 32, 64],
        "discrete": {
            1: {"min": 0, "max": 1, "mul": 1},   # sync: boolean
            2: {"min": 0, "max": 1, "mul": 1},   # freeze: boolean
            3: {"min": 0, "max": 1, "mul": 1},   # tapedig: boolean
        },
    },

    # fxreverb: Time, Lowpass, Level
    "fxreverb": {
        "values": [64, 96, 64],
    },

    # fxmaster: Thresh, Ratio, Attack, Release, LPF, Gain, Mix,
    #           Dly.Lev, Rev.Lev, Sum mute, Sum lev
    "fxmaster": {
        "values": [100, 32, 0, 20, 48, 0, 64, 64, 64, 0, 64],
        "discrete": {
            9: {"min": 0, "max": 1, "mul": 1},   # sum mute: boolean
        },
    },
}


# ──────────────────────────────────────────────────────────────────────
# Map from preset ID → machine key (for all-def presets)
# ──────────────────────────────────────────────────────────────────────
PRESET_TO_MACHINE = {
    "db-all-def":       "db",
    "ab-all-def":       "ab",
    "fmb-all-def":      "fmb",
    "ds-all-def":       "ds",
    "as-all-def":       "as",
    "hh1-all-def":      "hh1",
    "hh2-all-def":      "hh2",
    "rs-all-def":       "rs",
    "cl-all-def":       "cl",
    "ro-all-def":       "ro",
    "mo-all-def":       "mo",
    "wtosc-all-def":    "wtosc",
    "fxdelay-all-def":  "fxdelay",
    "fxreverb-all-def": "fxreverb",
    "fxmaster-all-def": "fxmaster",
}

MACRODEF_TO_MACHINE = {
    "db-allparams":       "db",
    "ab-allparams":       "ab",
    "fmb-allparams":      "fmb",
    "ds-allparams":       "ds",
    "as-allparams":       "as",
    "hh1-allparams":      "hh1",
    "hh2-allparams":      "hh2",
    "rs-allparams":       "rs",
    "cl-allparams":       "cl",
    "ro-allparams":       "ro",
    "td3-allparams":      "td3",
    "mo-allparams":       "mo",
    "wtosc-allparams":    "wtosc",
    "pp-allparams":       "pp",
    "fxdelay-allparams":  "fxdelay",
    "fxreverb-allparams": "fxreverb",
    "fxmaster-allparams": "fxmaster",
}


def load_json(path):
    with open(path) as f:
        return json.load(f)


def save_json(path, data):
    with open(path, "w") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write("\n")


def save_json_compact(path, data):
    """Save as single-line JSON (matching the preset file convention)."""
    with open(path, "w") as f:
        json.dump(data, f, separators=(",", ":"), ensure_ascii=False)
        f.write("\n")


def get_curved_preset_values(machine):
    """
    Get preset values adjusted for response curves.

    MACHINE_PRESETS stores the target DSP CC values (what the synth engine
    should receive). But with curves active, the preset stores KNOB positions
    that get curved before reaching the DSP. So we invert the curve.

    Example: DSP needs CC=64 for Freq, curve is "log"
      → log(16)=64, so we store knob=16 in the preset
    """
    if machine not in MACHINE_PRESETS:
        return []

    raw_values = MACHINE_PRESETS[machine]["values"]
    curves = CURVE_MAP.get(machine, {})

    adjusted = []
    for idx, val in enumerate(raw_values):
        curve_type = curves.get(idx, 'linear')
        knob_val = inverse_curve(val, curve_type)
        adjusted.append(knob_val)

    return adjusted


# ──────────────────────────────────────────────────────────────────────
# ANALYSIS & FIX LOGIC
# ──────────────────────────────────────────────────────────────────────

def analyze_and_fix():
    changes = []
    errors = []

    # ── 1. Fix preset values ──────────────────────────────────────────
    print("=" * 70)
    print("  PRESET VALUE FIXES")
    print("=" * 70)

    for preset_file in sorted(os.listdir(PRESET_DIR)):
        if not preset_file.endswith(".json"):
            continue
        preset_path = os.path.join(PRESET_DIR, preset_file)
        preset = load_json(preset_path)
        preset_id = preset.get("id", "")

        if preset_id not in PRESET_TO_MACHINE:
            continue

        machine = PRESET_TO_MACHINE[preset_id]
        if machine not in MACHINE_PRESETS:
            continue

        new_values = get_curved_preset_values(machine)
        old_values = preset.get("values", [])

        if len(new_values) != len(old_values):
            errors.append(f"  ⚠ {preset_id}: value count mismatch "
                          f"(have {len(old_values)}, need {len(new_values)})")
            # Fix the count anyway
            print(f"\n  {preset_id}: adjusting value count "
                  f"{len(old_values)} → {len(new_values)}")

        if old_values == new_values:
            print(f"\n  {preset_id}: already optimal ✓")
            continue

        print(f"\n  {preset_id} ({machine}):")
        max_len = max(len(old_values), len(new_values))
        for i in range(max_len):
            old = old_values[i] if i < len(old_values) else "—"
            new = new_values[i] if i < len(new_values) else "—"
            if old != new:
                marker = " ← CRITICAL" if (
                    (preset_id == "ro-all-def" and i in (3, 10)) or
                    (preset_id == "wtosc-all-def" and i in (7, 8, 9, 10))
                ) else ""
                print(f"    [{i:2d}] {old:>4} → {new:>4}{marker}")

        preset["values"] = new_values
        changes.append(("preset", preset_path, preset))

    # ── 2. Fix macro definition `def` values and discrete params ──────
    print("\n" + "=" * 70)
    print("  MACRO DEFINITION FIXES")
    print("=" * 70)

    for macrodef_file in sorted(os.listdir(MACRODEF_DIR)):
        if not macrodef_file.endswith(".json"):
            continue
        macrodef_path = os.path.join(MACRODEF_DIR, macrodef_file)
        macrodef = load_json(macrodef_path)
        macrodef_id = macrodef.get("id", "")

        if macrodef_id not in MACRODEF_TO_MACHINE:
            continue

        machine = MACRODEF_TO_MACHINE[macrodef_id]
        if machine not in MACHINE_PRESETS:
            continue

        preset_info = MACHINE_PRESETS[machine]
        new_values = get_curved_preset_values(machine)
        discrete_fixes = preset_info.get("discrete", {})
        modified = False
        param_changes = []
        discrete_changes = []
        mapping_changes = []

        # Update `def` values for each parameter in groups
        for group in macrodef.get("groups", []):
            for param in group.get("parameters", []):
                idx = param["idx"]
                if idx < len(new_values):
                    new_def = new_values[idx]

                    # For discrete params with mul > 1, the def value should
                    # be in the RESTRICTED range (the mapping will scale it)
                    if idx in discrete_fixes:
                        df = discrete_fixes[idx]
                        # The preset value IS the restricted value (before mapping)
                        # But we need to make sure the def is in [min, max]
                        new_def = min(new_def, df["max"])
                        new_def = max(new_def, df["min"])

                    old_def = param.get("def", 0)
                    if old_def != new_def:
                        param["def"] = new_def
                        param_changes.append(f"    [{idx:2d}] {param['name']:12s} "
                                             f"def: {old_def:>4} → {new_def:>4}")
                        modified = True

                    # Fix min/max for discrete params
                    if idx in discrete_fixes:
                        df = discrete_fixes[idx]
                        old_min = param.get("min", 0)
                        old_max = param.get("max", 127)
                        if old_min != df["min"] or old_max != df["max"]:
                            param["min"] = df["min"]
                            param["max"] = df["max"]
                            discrete_changes.append(
                                f"    [{idx:2d}] {param['name']:12s} "
                                f"range: {old_min}-{old_max} → {df['min']}-{df['max']}")
                            modified = True

        # Fix mapping mul for discrete params
        for mapping in macrodef.get("mapping", []):
            # Find which idx this maps from
            add_entries = mapping.get("add", [])
            if len(add_entries) == 1:
                src_idx = add_entries[0].get("src")
                if src_idx in discrete_fixes:
                    df = discrete_fixes[src_idx]
                    old_mul = add_entries[0].get("mul", 1)
                    new_mul = df["mul"]
                    if old_mul != new_mul:
                        add_entries[0]["mul"] = new_mul
                        mapping_changes.append(
                            f"    ctrl {mapping['ctrl']:2d} (idx {src_idx}): "
                            f"mul: {old_mul} → {new_mul}")
                        modified = True

        if modified:
            print(f"\n  {macrodef_id} ({machine}):")
            for c in param_changes:
                print(c)
            for c in discrete_changes:
                print(c)
            for c in mapping_changes:
                print(c)
            changes.append(("macrodef", macrodef_path, macrodef))
        else:
            print(f"\n  {macrodef_id}: no changes needed ✓")

    # ── 3. Summary ────────────────────────────────────────────────────
    print("\n" + "=" * 70)
    print("  SUMMARY")
    print("=" * 70)

    preset_changes = [c for c in changes if c[0] == "preset"]
    macrodef_changes = [c for c in changes if c[0] == "macrodef"]
    print(f"  Preset changes:   {len(preset_changes):3d} files")
    print(f"  Macrodef changes: {len(macrodef_changes):3d} files")
    print(f"  Total:            {len(changes):3d} files")

    if errors:
        print(f"\n  Warnings:")
        for e in errors:
            print(e)

    # ── Critical fixes highlighted ────────────────────────────────────
    print(f"\n  CRITICAL FIXES:")
    print(f"    • ro-all-def [3] End: 0→127    (was producing ZERO-LENGTH samples!)")
    print(f"    • ro-all-def [10] Speed: 16→64 (was playing at 0.25× speed!)")
    print(f"    • wtosc-all-def ADSR: all 0→proper (was SILENT — no envelope!)")
    print(f"    • All drums: copy-paste [50,70,30,0,20] → DSP-tuned values")
    print(f"    • All synths: generic values → musically optimised defaults")

    return changes


def apply_changes(changes):
    for kind, path, data in changes:
        if kind == "preset":
            save_json_compact(path, data)
        else:
            save_json(path, data)


def main():
    apply = "--apply" in sys.argv

    print()
    print("╔══════════════════════════════════════════════════════════════╗")
    print("║  TBD-16 Preset & Mapping Optimizer                         ║")
    print("║  Replaces copy-paste defaults with DSP-tuned values        ║")
    print("╚══════════════════════════════════════════════════════════════╝")
    print()

    changes = analyze_and_fix()

    if not changes:
        print("\n  No changes needed. All presets are already optimal.")
        return

    if apply:
        print(f"\n  Applying {len(changes)} changes...")
        apply_changes(changes)
        print("  Done! ✓")
    else:
        print(f"\n  DRY RUN — use --apply to write changes.")
        print(f"  ({len(changes)} files would be modified)")


if __name__ == "__main__":
    main()
