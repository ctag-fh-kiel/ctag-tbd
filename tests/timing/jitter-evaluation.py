#!/usr/bin/env python3
"""
jitter-evaluation.py  –  Temporal jitter analyser for sequencer audio pulses
=============================================================================

Reads a WAV file, detects pulse onsets via simple amplitude thresholding,
and reports estimated BPM and jitter statistics. Creates a plain-text report
and a vector PDF visualisation.

Usage
-----
    python3 jitter-evaluation.py <input.wav> [options]

Options
-------
    --channel {left,right}  Audio channel to analyse.  Default: left.
    --steps-per-beat N      Sequencer steps (pulses) per quarter-note beat.
                            Default: 4  (16th-note grid).
    --threshold T           Onset threshold as fraction of peak amplitude
                            (0.0–1.0).  Default: 0.10
    --min-distance-ms D     Minimum gap between successive onsets in ms.
                            Default: 50 ms.
    --output-prefix PREFIX  Prefix for output files.  Default: derived from
                            the input filename (same directory).

Outputs
-------
    <prefix>-report.txt     Plain-text statistics report.
    <prefix>-analysis.pdf   Multi-panel vector PDF visualisation.

Dependencies
------------
    pip install numpy scipy matplotlib
"""

import argparse
import os
import sys
import wave

import numpy as np
from scipy.signal import butter, filtfilt
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec


# ---------------------------------------------------------------------------
# Audio I/O
# ---------------------------------------------------------------------------

def load_wav_channel(path: str, channel: int = 0) -> tuple[np.ndarray, int]:
    """
    Load a WAV file and return (samples_float32, sample_rate) for one channel.
    channel=0 → left, channel=1 → right.
    """
    with wave.open(path, "rb") as wf:
        n_channels = wf.getnchannels()
        sampwidth  = wf.getsampwidth()
        framerate  = wf.getframerate()
        n_frames   = wf.getnframes()
        raw        = wf.readframes(n_frames)

    if sampwidth == 1:
        dtype = np.uint8
        peak  = 128.0
    elif sampwidth == 2:
        dtype = np.int16
        peak  = 32768.0
    elif sampwidth == 4:
        dtype = np.int32
        peak  = 2147483648.0
    else:
        raise ValueError(f"Unsupported sample width: {sampwidth} bytes")

    samples = np.frombuffer(raw, dtype=dtype).astype(np.float32)

    ch_idx = min(channel, n_channels - 1)
    if n_channels > 1:
        samples = samples[ch_idx::n_channels]

    samples /= peak
    return samples, framerate


# ---------------------------------------------------------------------------
# Onset detection — simple threshold on rectified + smoothed envelope
# ---------------------------------------------------------------------------

def smooth_envelope(signal: np.ndarray, sr: int, smooth_ms: float = 5.0) -> np.ndarray:
    """Full-wave rectify then low-pass filter to produce a smooth envelope."""
    rect = np.abs(signal)
    cutoff_hz = 1000.0 / smooth_ms
    nyq = sr / 2.0
    b, a = butter(2, min(cutoff_hz / nyq, 0.99), btype="low")
    return filtfilt(b, a, rect)


def detect_onsets(signal: np.ndarray, sr: int,
                  threshold_frac: float = 0.10,
                  min_distance_ms: float = 50.0) -> np.ndarray:
    """
    Detect onset sample indices.

    Strategy: find every rising-edge crossing of a fixed threshold on the
    smoothed envelope, then throw away any crossing that is closer than
    *min_distance_ms* to the previous accepted one.  This is the simplest
    approach and works well for clean percussive pulses.

    Returns an int array of sample indices.
    """
    env = smooth_envelope(signal, sr)

    threshold = threshold_frac * env.max()
    min_dist  = int(min_distance_ms * sr / 1000.0)

    above = (env >= threshold).astype(np.int8)
    # rising edge: 0→1 transition
    rising = np.flatnonzero(np.diff(above, prepend=0) == 1)

    if len(rising) == 0:
        return rising

    # Enforce minimum distance (greedy scan)
    accepted = [rising[0]]
    for r in rising[1:]:
        if r - accepted[-1] >= min_dist:
            accepted.append(r)

    return np.array(accepted, dtype=np.intp)


# ---------------------------------------------------------------------------
# Statistics helper
# ---------------------------------------------------------------------------

def stats(arr: np.ndarray) -> dict:
    return {
        "mean":   float(np.mean(arr)),
        "std":    float(np.std(arr, ddof=1)) if len(arr) > 1 else 0.0,
        "min":    float(np.min(arr)),
        "max":    float(np.max(arr)),
        "median": float(np.median(arr)),
        "n":      len(arr),
    }


# ---------------------------------------------------------------------------
# Main analysis
# ---------------------------------------------------------------------------

def analyse(wav_path: str,
            channel: int           = 0,
            steps_per_beat: int    = 4,
            threshold_frac: float  = 0.10,
            min_distance_ms: float = 50.0,
            outlier_threshold_pct: float = 0.3,
            output_prefix: str     = None) -> None:

    if output_prefix is None:
        output_prefix = os.path.splitext(wav_path)[0]

    report_path = output_prefix + "-report.txt"
    image_path  = output_prefix + "-analysis.pdf"

    ch_name = "left" if channel == 0 else "right"
    print(f"Loading: {wav_path}  (channel: {ch_name})")
    signal, sr = load_wav_channel(wav_path, channel=channel)
    duration_s = len(signal) / sr
    peak_amp   = float(np.abs(signal).max())
    print(f"  Sample rate : {sr} Hz")
    print(f"  Duration    : {duration_s:.2f} s")
    print(f"  Peak amp    : {peak_amp:.5f}  ({peak_amp * 100:.2f}% of full scale)")

    if peak_amp < 0.01:
        print(f"\n  WARNING: The {ch_name} channel looks nearly silent "
              f"(peak = {peak_amp:.5f}).")
        print(f"  Consider --channel right if the signal is on the right channel.\n")

    print(f"Detecting onsets  (threshold={threshold_frac:.2f}, "
          f"min_distance={min_distance_ms:.0f} ms) …")
    onset_samples = detect_onsets(signal, sr,
                                  threshold_frac=threshold_frac,
                                  min_distance_ms=min_distance_ms)
    n_onsets = len(onset_samples)
    print(f"  Found {n_onsets} onsets")

    if n_onsets < 4:
        print("ERROR: too few onsets detected.  "
              "Try --threshold lower, --min-distance-ms smaller, or "
              "check --channel.", file=sys.stderr)
        sys.exit(1)

    onset_times_s = onset_samples / float(sr)
    ioi_s         = np.diff(onset_times_s)

    # Reference is the median IOI — robust against outliers
    ref_ioi_s = float(np.median(ioi_s))
    ref_bpm   = 60.0 / (ref_ioi_s * steps_per_beat)

    ioi_bpm   = 60.0 / (ioi_s * steps_per_beat)
    jitter_ms = (ioi_s - ref_ioi_s) * 1000.0

    # ── Timing quantisation detection ───────────────────────────────────────
    # The sequencer fires on audio DMA block boundaries, so IOI values cluster
    # at multiples of the block size.  Find it from the step between unique
    # IOI sample counts (ignoring 1-sample threshold-crossing noise).
    ioi_samp        = np.diff(onset_samples).astype(np.int64)
    unique_ioi_samp = np.unique(ioi_samp)
    samp_diffs      = np.diff(unique_ioi_samp)
    samp_diffs_filt = samp_diffs[samp_diffs > 1]   # drop ±1-sample noise
    if len(samp_diffs_filt) >= 3:
        quant_block = int(np.median(samp_diffs_filt))
    else:
        quant_block = 0   # could not detect
    quant_ms = quant_block / sr * 1000.0 if quant_block else 0.0

    # ── Outlier detection (Tukey IQR method) ────────────────────────────────
    q1, q3   = np.percentile(jitter_ms, [25, 75])
    iqr      = q3 - q1
    out_lo   = q1 - 1.5 * iqr
    out_hi   = q3 + 1.5 * iqr
    out_mask = (jitter_ms < out_lo) | (jitter_ms > out_hi)   # length = n_onsets-1
    n_out    = int(out_mask.sum())
    out_pct  = 100.0 * n_out / len(ioi_s)

    # Use filtered stats for highlights when outlier % <= configurable threshold
    use_filtered_highlight = out_pct <= outlier_threshold_pct

    # Indices into onset_times_s for the "arriving" onset of each outlier IOI
    out_onset_idx   = np.where(out_mask)[0] + 1
    out_onset_times = onset_times_s[out_onset_idx]

    # Filtered arrays (outliers removed)
    ioi_bpm_f   = ioi_bpm[~out_mask]
    jitter_ms_f = jitter_ms[~out_mask]

    st_bpm    = stats(ioi_bpm)
    st_jitter = stats(jitter_ms)
    st_bpm_f  = stats(ioi_bpm_f)
    st_jit_f  = stats(jitter_ms_f)

    # ── Report ──────────────────────────────────────────────────────────────
    sep  = "=" * 62
    sep2 = "-" * 62
    lines = [
        sep,
        "  SEQUENCER JITTER ANALYSIS REPORT",
        sep,
        f"  Input file      : {os.path.basename(wav_path)}",
        f"  Channel         : {ch_name}",
        f"  Duration        : {duration_s:.2f} s",
        f"  Sample rate     : {sr} Hz",
        f"  Steps per beat  : {steps_per_beat}  (e.g. 4 = 16th-note grid)",
        f"  Onsets detected : {n_onsets}  ({n_onsets - 1} intervals)",
        f"  Outliers (IQR)  : {n_out}  ({100*n_out/(n_onsets-1):.1f} %)",
        f"  Timing quantis. : {quant_block} samples  ({quant_ms:.4f} ms) — likely audio block size" if quant_block else
        f"  Timing quantis. : not detected",
        sep2,
        "  ★  KEY RESULTS  ★",
        sep2,
        f"  ► Mean BPM              : {st_bpm['mean']:.3f}  ±  {st_bpm['std']:.3f}",
        f"  ► Mean jitter           : {st_jitter['mean']:+.3f}  ±  {st_jitter['std']:.3f} ms",
        f"  ► Mean BPM  (filtered)  : {st_bpm_f['mean']:.3f}  ±  {st_bpm_f['std']:.3f}",
        f"  ► Mean jitter (filtered): {st_jit_f['mean']:+.3f}  ±  {st_jit_f['std']:.3f} ms",
        sep2,
        "  BPM  —  all intervals",
        sep2,
        f"  Reference BPM   : {ref_bpm:.3f}  (median IOI = {ref_ioi_s*1000:.3f} ms)",
        f"  Mean  ± Std     : {st_bpm['mean']:.4f}  ±  {st_bpm['std']:.4f} BPM",
        f"  Min              : {st_bpm['min']:.4f} BPM",
        f"  Max              : {st_bpm['max']:.4f} BPM",
        f"  Median           : {st_bpm['median']:.4f} BPM",
        sep2,
        f"  BPM  —  outliers removed  (n={len(ioi_bpm_f)})",
        sep2,
        f"  Mean  ± Std     : {st_bpm_f['mean']:.4f}  ±  {st_bpm_f['std']:.4f} BPM",
        f"  Min              : {st_bpm_f['min']:.4f} BPM",
        f"  Max              : {st_bpm_f['max']:.4f} BPM",
        f"  Median           : {st_bpm_f['median']:.4f} BPM",
        sep2,
        "  JITTER  —  all intervals",
        sep2,
        f"  Mean  ± Std     : {st_jitter['mean']:+.4f}  ±  {st_jitter['std']:.4f} ms",
        f"  Min              : {st_jitter['min']:+.4f} ms",
        f"  Max              : {st_jitter['max']:+.4f} ms",
        f"  Median           : {st_jitter['median']:+.4f} ms",
        f"  Jitter range     : {st_jitter['max'] - st_jitter['min']:.4f} ms  (max − min)",
        sep2,
        f"  JITTER  —  outliers removed  (n={len(jitter_ms_f)})",
        sep2,
        f"  Mean  ± Std     : {st_jit_f['mean']:+.4f}  ±  {st_jit_f['std']:.4f} ms",
        f"  Min              : {st_jit_f['min']:+.4f} ms",
        f"  Max              : {st_jit_f['max']:+.4f} ms",
        f"  Median           : {st_jit_f['median']:+.4f} ms",
        f"  Jitter range     : {st_jit_f['max'] - st_jit_f['min']:.4f} ms  (max − min)",
        sep,
    ]
    report_text = "\n".join(lines)

    print()
    print(report_text)

    with open(report_path, "w", encoding="utf-8") as f:
        f.write(report_text + "\n")
    print(f"\nReport saved : {report_path}")

    _make_plot(signal, sr, onset_samples, onset_times_s, ioi_s, ioi_bpm, jitter_ms,
               out_mask, out_onset_times, out_lo, out_hi,
               st_bpm, st_jitter, st_bpm_f, st_jit_f,
               ref_bpm, ref_ioi_s, steps_per_beat, n_out, out_pct,
               use_filtered_highlight, outlier_threshold_pct,
               quant_block, quant_ms,
               os.path.basename(wav_path), ch_name, image_path)
    print(f"Image  saved : {image_path}")


# ---------------------------------------------------------------------------
# Waveform decimation for sharp vector output
# ---------------------------------------------------------------------------

def _minmax_decimate(signal: np.ndarray, sr: int,
                     target_points: int = 8000) -> tuple[np.ndarray, np.ndarray]:
    """
    Reduce a long waveform to *target_points* samples using min-max decimation.

    For each block of N consecutive samples the function preserves both the
    minimum and the maximum value (interleaved), so that amplitude peaks are
    never lost.  The returned (t, y) arrays are suitable for a vector plot
    that stays sharp at any zoom level without rasterisation.
    """
    n = len(signal)
    if n <= target_points:
        return np.arange(n) / sr, signal

    # Each block contributes 2 output points (min + max)
    n_blocks = target_points // 2
    block    = n // n_blocks
    trimmed  = signal[:n_blocks * block].reshape(n_blocks, block)

    mins = trimmed.min(axis=1)
    maxs = trimmed.max(axis=1)

    # Centre time of each block
    block_centres = (np.arange(n_blocks) * block + block / 2) / sr

    # Interleave: for each block emit (t, min) then (t, max)
    t_out = np.empty(2 * n_blocks)
    y_out = np.empty(2 * n_blocks)
    t_out[0::2] = block_centres
    t_out[1::2] = block_centres
    y_out[0::2] = mins
    y_out[1::2] = maxs

    return t_out, y_out


# ---------------------------------------------------------------------------
# Visualisation
# ---------------------------------------------------------------------------

def _make_plot(signal, sr, onset_samples, onset_times_s, ioi_s, ioi_bpm, jitter_ms,
               out_mask, out_onset_times, out_lo, out_hi,
               st_bpm, st_jitter, st_bpm_f, st_jit_f,
               ref_bpm, ref_ioi_s, steps_per_beat, n_out, out_pct,
               use_filtered_highlight, outlier_threshold_pct,
               quant_block, quant_ms,
               title_name, ch_name, image_path):

    mid_t     = (onset_times_s[:-1] + onset_times_s[1:]) / 2.0
    n_total   = len(ioi_s)

    # Normal / outlier split for scatter plots
    norm_t  = mid_t[~out_mask];  norm_bpm = ioi_bpm[~out_mask];  norm_j = jitter_ms[~out_mask]
    out_t   = mid_t[out_mask];   out_bpm  = ioi_bpm[out_mask];   out_j  = jitter_ms[out_mask]

    # When outliers are <= threshold %, use filtered stats for highlights (decided in stats box)
    # Jitter range (= max jitter minus min jitter = worst-case timing spread)
    jrange   = st_jitter['max'] - st_jitter['min']
    jrange_f = st_jit_f['max']  - st_jit_f['min']

    fig = plt.figure(figsize=(14, 13))
    fig.suptitle(
        f"Sequencer Jitter Analysis  –  {title_name}  [{ch_name} ch]\n"
        f"Reference: {ref_bpm:.2f} BPM  "
        f"({steps_per_beat} steps/beat,  ref interval = {ref_ioi_s*1000:.2f} ms)  |  "
        f"Outliers (IQR): {n_out}/{n_total}",
        fontsize=11, fontweight="bold",
    )

    gs = gridspec.GridSpec(3, 2, figure=fig,
                           height_ratios=[1.1, 1.0, 1.0],
                           hspace=0.50, wspace=0.38,
                           left=0.07, right=0.96,
                           top=0.92, bottom=0.05)

    # ── 1. Waveform + onset markers (full width) ────────────────────────────
    ax_wave = fig.add_subplot(gs[0, :])

    # Decimate to ~8000 vector points — stays crisp at any zoom
    t_dec, y_dec = _minmax_decimate(signal, sr, target_points=8000)
    ax_wave.plot(t_dec, y_dec, color="#3377bb", linewidth=0.4, alpha=0.6,
                 label=f"{ch_name} ch")

    # Signal amplitude at each detected onset (for x-marker placement)
    samp_idx    = np.clip(onset_samples, 0, len(signal) - 1)
    onset_amps  = signal[samp_idx].astype(float)

    # Normal onsets — 'x' marker placed at the signal value, crisp at any zoom
    normal_onset_mask = np.ones(len(onset_times_s), dtype=bool)
    normal_onset_mask[np.where(out_mask)[0] + 1] = False
    ax_wave.plot(onset_times_s[normal_onset_mask],
                 onset_amps[normal_onset_mask],
                 marker="x", markersize=4, linewidth=0,
                 color="#22aa44", alpha=0.8, label="Onsets",
                 markeredgewidth=0.8, zorder=4)

    # Outlier onsets — full-height orange line + diamond marker at signal value
    if len(out_onset_times):
        out_samp_idx = np.clip(onset_samples[np.where(out_mask)[0] + 1],
                               0, len(signal) - 1)
        out_amps = signal[out_samp_idx].astype(float)
        ax_wave.vlines(out_onset_times, -1.05, 1.05,
                       color="#ff7700", linewidth=1.0, alpha=0.8,
                       label="Outlier onsets")
        ax_wave.plot(out_onset_times, out_amps,
                     marker="D", markersize=6, linewidth=0,
                     color="#ff7700", alpha=0.95, markeredgewidth=0.5,
                     markeredgecolor="#cc4400", zorder=5)

    ax_wave.set_xlim(onset_times_s[0] - 0.5, onset_times_s[-1] + 0.5)
    ax_wave.set_ylim(-1.1, 1.1)
    ax_wave.set_xlabel("Time (s)")
    ax_wave.set_ylabel("Amplitude")
    ax_wave.set_title(f"Waveform  –  {len(onset_times_s)} onsets  "
                      f"({n_out} outliers marked in orange)")
    ax_wave.legend(fontsize=7, loc="upper right", ncol=3)

    # ── 2. BPM over time ────────────────────────────────────────────────────
    ax_bpm = fig.add_subplot(gs[1, 0])
    ax_bpm.scatter(norm_t, norm_bpm, color="#2266aa", s=3, alpha=0.6,
                   label="Normal", zorder=3)
    if len(out_t):
        ax_bpm.scatter(out_t, out_bpm, color="#ff7700", s=20, marker="D",
                       alpha=0.9, label="Outlier", zorder=4)
    ax_bpm.axhline(st_bpm["mean"],   color="#cc3300", lw=1.3, ls="--",
                   label=f"Mean  {st_bpm['mean']:.2f}")
    ax_bpm.axhline(st_bpm_f["mean"], color="#008855", lw=1.1, ls="-.",
                   label=f"Mean* {st_bpm_f['mean']:.2f}")
    ax_bpm.fill_between(mid_t,
                        st_bpm["mean"] - st_bpm["std"],
                        st_bpm["mean"] + st_bpm["std"],
                        alpha=0.10, color="#cc3300")
    ax_bpm.set_xlabel("Time (s)")
    ax_bpm.set_ylabel("BPM")
    ax_bpm.set_title("BPM over time  (* = outliers removed)")
    ax_bpm.legend(fontsize=7)

    # ── 3. Jitter over time ─────────────────────────────────────────────────
    ax_jit = fig.add_subplot(gs[1, 1])
    ax_jit.scatter(norm_t, norm_j, color="#2266aa", s=3, alpha=0.6,
                   label="Normal", zorder=3)
    if len(out_t):
        ax_jit.scatter(out_t, out_j, color="#ff7700", s=20, marker="D",
                       alpha=0.9, label="Outlier", zorder=4)
    ax_jit.axhline(0,                  color="black",   lw=0.7, ls="--")
    ax_jit.axhline(out_hi,             color="#ff7700", lw=0.8, ls=":",
                   label=f"IQR fence  {out_hi:+.2f} ms")
    ax_jit.axhline(out_lo,             color="#ff7700", lw=0.8, ls=":")
    ax_jit.axhline(st_jitter["mean"],  color="#cc3300", lw=1.3, ls="--",
                   label=f"Mean  {st_jitter['mean']:+.3f} ms")
    ax_jit.axhline(st_jit_f["mean"],   color="#008855", lw=1.1, ls="-.",
                   label=f"Mean* {st_jit_f['mean']:+.3f} ms")
    ax_jit.fill_between(mid_t,
                        st_jitter["mean"] - st_jitter["std"],
                        st_jitter["mean"] + st_jitter["std"],
                        alpha=0.10, color="#cc3300")
    ax_jit.set_xlabel("Time (s)")
    ax_jit.set_ylabel("Jitter (ms)")
    ax_jit.set_title("Jitter over time  (* = outliers removed)")
    ax_jit.legend(fontsize=7)

    # ── 4. Jitter histogram ─────────────────────────────────────────────────
    ax_hist = fig.add_subplot(gs[2, 0])
    n_bins = max(20, min(80, n_total // 10))
    ax_hist.hist(norm_j, bins=n_bins, color="#4488cc",
                 edgecolor="white", linewidth=0.3, label="Normal")
    ax_hist.hist(out_j,  bins=n_bins, color="#ff7700",
                 edgecolor="white", linewidth=0.3, alpha=0.8, label="Outlier")
    ax_hist.axvline(st_jitter["mean"], color="#cc3300", lw=1.4, ls="--",
                    label=f"Mean  {st_jitter['mean']:+.3f} ms")
    ax_hist.axvline(st_jit_f["mean"],  color="#008855", lw=1.2, ls="-.",
                    label=f"Mean* {st_jit_f['mean']:+.3f} ms")
    ax_hist.axvline(0, color="black", lw=0.7, ls="--")
    ax_hist.set_xlabel("Jitter (ms)")
    ax_hist.set_ylabel("Count")
    ax_hist.set_title("Jitter distribution  (* = outliers removed)")
    ax_hist.legend(fontsize=7)

    # ── 5. Stats box ────────────────────────────────────────────────────────
    ax_txt = fig.add_subplot(gs[2, 1])
    ax_txt.axis("off")

    # Key highlight boxes — use filtered stats when outlier % <= threshold
    hi_bpm = st_bpm_f if use_filtered_highlight else st_bpm
    hi_jit = st_jit_f if use_filtered_highlight else st_jitter
    hi_tag = "*" if use_filtered_highlight else ""

    ax_txt.text(0.5, 0.99,
                f"Mean BPM{hi_tag}\n{hi_bpm['mean']:.3f} ± {hi_bpm['std']:.3f}",
                transform=ax_txt.transAxes, ha="center", va="top",
                fontsize=13, fontweight="bold", color="#cc3300",
                bbox=dict(boxstyle="round,pad=0.3", facecolor="#fff0ee",
                          edgecolor="#cc3300", linewidth=1.2))
    ax_txt.text(0.5, 0.72,
                f"Mean Jitter{hi_tag}\n{hi_jit['mean']:+.3f} ± {hi_jit['std']:.3f} ms",
                transform=ax_txt.transAxes, ha="center", va="top",
                fontsize=13, fontweight="bold", color="#2266aa",
                bbox=dict(boxstyle="round,pad=0.3", facecolor="#eef4ff",
                          edgecolor="#2266aa", linewidth=1.2))

    # ── Side-by-side table + footer notes (single text block, no overlap) ──
    jrange   = st_jitter['max'] - st_jitter['min']
    jrange_f = st_jit_f['max']  - st_jit_f['min']
    W = 10   # column width

    if use_filtered_highlight:
        policy_sym = f"*  {n_out}/{len(ioi_s)} outliers ({out_pct:.2f}%) ≤ {outlier_threshold_pct:.1f}% → * uses filtered"
        policy_clr = "#005500"
    else:
        policy_sym = f"   {n_out}/{len(ioi_s)} outliers ({out_pct:.2f}%) > {outlier_threshold_pct:.1f}% → * NOT applied"
        policy_clr = "#884400"

    if quant_block:
        quant_note = f"   Block quantisation: {quant_block} smp = {quant_ms:.3f} ms (explains discrete bins)"
    else:
        quant_note = "   Block quantisation: not detected"

    divider = "─" * (16 + 2 * W + 2)
    rows = [
        f"{'':16s}{'All':>{W}s}  {'Filtered*':>{W}s}",
        divider,
        f"{'Mean BPM':16s}{st_bpm['mean']:>{W}.3f}  {st_bpm_f['mean']:>{W}.3f}",
        f"{'± Std':16s}{st_bpm['std']:>{W}.3f}  {st_bpm_f['std']:>{W}.3f}",
        f"{'Min BPM':16s}{st_bpm['min']:>{W}.3f}  {st_bpm_f['min']:>{W}.3f}",
        f"{'Max BPM':16s}{st_bpm['max']:>{W}.3f}  {st_bpm_f['max']:>{W}.3f}",
        divider,
        f"{'Mean Jitter':16s}{st_jitter['mean']:>+{W}.3f}  {st_jit_f['mean']:>+{W}.3f}",
        f"{'± Std (ms)':16s}{st_jitter['std']:>{W}.3f}  {st_jit_f['std']:>{W}.3f}",
        f"{'Min (ms)':16s}{st_jitter['min']:>+{W}.3f}  {st_jit_f['min']:>+{W}.3f}",
        f"{'Max (ms)':16s}{st_jitter['max']:>+{W}.3f}  {st_jit_f['max']:>+{W}.3f}",
        f"{'Range (ms)':16s}{jrange:>{W}.3f}  {jrange_f:>{W}.3f}",
        divider,
        policy_sym,
        quant_note,
    ]
    ax_txt.text(0.04, 0.45, "\n".join(rows),
                transform=ax_txt.transAxes, va="top",
                fontfamily="monospace", fontsize=7.5,
                color="#222222")


    fig.savefig(image_path)
    plt.close(fig)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Evaluate temporal jitter of audio pulses from a sequencer.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("wav",
                        help="Input WAV file.")
    parser.add_argument("--channel", choices=["left", "right"], default="left",
                        help="Audio channel to analyse (default: left).")
    parser.add_argument("--steps-per-beat", type=int, default=4, metavar="N",
                        help="Steps per quarter-note beat (default: 4 = 16th notes).")
    parser.add_argument("--threshold", type=float, default=0.10, metavar="T",
                        help="Onset threshold as fraction of peak amplitude (default: 0.10).")
    parser.add_argument("--min-distance-ms", type=float, default=50.0, metavar="D",
                        help="Minimum gap between onsets in ms (default: 50).")
    parser.add_argument("--outlier-threshold", type=float, default=0.3, metavar="PCT",
                        help="If outliers are <= this %% of intervals, highlighted results "
                             "use outlier-filtered values (default: 0.3).")
    parser.add_argument("--output-prefix", default=None, metavar="PREFIX",
                        help="Prefix for output files (default: input basename without extension).")

    args = parser.parse_args()

    if not os.path.isfile(args.wav):
        print(f"ERROR: file not found: {args.wav}", file=sys.stderr)
        sys.exit(1)

    analyse(
        wav_path               = args.wav,
        channel                = 0 if args.channel == "left" else 1,
        steps_per_beat         = args.steps_per_beat,
        threshold_frac         = args.threshold,
        min_distance_ms        = args.min_distance_ms,
        outlier_threshold_pct  = args.outlier_threshold,
        output_prefix          = args.output_prefix,
    )


if __name__ == "__main__":
    main()

