#!/usr/bin/env python3
"""
Auto-slice a WAV file into mono snippets based on silence gaps.

Usage:
  python autoslice_wav.py /path/to/file.wav [options]

Default behavior:
  - Reads 16-bit PCM WAV
  - If stereo, splits channels and slices each independently
  - Detects segments where RMS exceeds a threshold and closes when sufficient silence is observed
  - Writes mono 16-bit slices to an 'autosliced' folder alongside the input file

Options:
  --out-dir DIR            Output folder (default: autosliced)
  --threshold-db DB        Silence threshold in dBFS (default: -40.0)
  --window-ms MS           RMS window size in ms (default: 10)
  --hop-ms MS              RMS hop size in ms (default: 5)
  --min-silence-ms MS      Minimum silence to close a segment (default: 100)
  --min-sound-ms MS        Minimum segment length to keep (default: 50)
  --prepad-ms MS           Samples to include before detected start (default: 5)
  --postpad-ms MS          Samples to include after detected end (default: 10)

Notes:
  - For best results, pass a WAV where each sound ends in clear silence.
  - Only 16-bit PCM is supported for now.
"""

from __future__ import annotations
import argparse
import os
from pathlib import Path
import wave
import struct
import math
from typing import List, Tuple


def read_wav_int16(path: Path) -> Tuple[int, int, bytes]:
    """Return (sample_rate, channels, frames_bytes) for 16-bit PCM WAV.
    Raises ValueError for unsupported formats.
    """
    with wave.open(str(path), 'rb') as w:
        nch = w.getnchannels()
        sw = w.getsampwidth()
        sr = w.getframerate()
        nframes = w.getnframes()
        comptype = w.getcomptype()
        if comptype != 'NONE':
            raise ValueError(f'Unsupported compression: {comptype}')
        if sw != 2:
            raise ValueError(f'Only 16-bit PCM supported (got {sw*8}-bit)')
        raw = w.readframes(nframes)
    return sr, nch, raw


def deinterleave_int16(raw: bytes, channels: int) -> List[memoryview]:
    """Return a list of memoryviews for each mono channel as int16 little-endian bytes.
    For channels==1 returns [raw]. For channels==2 returns [L_bytes, R_bytes].
    """
    if channels == 1:
        return [memoryview(raw)]
    if channels < 1:
        return []
    frame_count = len(raw) // (2 * channels)
    views = [bytearray(frame_count * 2) for _ in range(channels)]
    # Deinterleave
    # raw is [c0s0_lo c0s0_hi, c1s0_lo c1s0_hi, ..., c0s1_lo c0s1_hi, ...]
    for i in range(frame_count):
        base = i * channels * 2
        for ch in range(channels):
            s_lo = raw[base + ch*2]
            s_hi = raw[base + ch*2 + 1]
            views[ch][i*2] = s_lo
            views[ch][i*2 + 1] = s_hi
    return [memoryview(v) for v in views]


def int16_bytes_to_float_list_le(b: bytes) -> List[float]:
    """Convert little-endian int16 bytes to float list in [-1, 1]."""
    out = []
    n = len(b) // 2
    for i in range(n):
        v = struct.unpack_from('<h', b, i*2)[0]
        out.append(max(-32768, min(32767, v)) / 32768.0)
    return out


def rms_frames(samples: List[float], sr: int, window_ms: float, hop_ms: float) -> Tuple[List[float], int, int]:
    """Compute RMS over sliding windows. Returns (rms_list, win_samples, hop_samples)."""
    win_samp = max(1, int(round(sr * window_ms / 1000.0)))
    hop_samp = max(1, int(round(sr * hop_ms / 1000.0)))
    n = len(samples)
    if n < win_samp:
        # one frame covering what's available
        if n == 0:
            return [0.0], win_samp, hop_samp
        s = sum(x*x for x in samples) / n
        return [math.sqrt(s)], win_samp, hop_samp
    rms = []
    # Initialize first window sum
    window_sum = sum(samples[i]*samples[i] for i in range(win_samp))
    rms.append(math.sqrt(window_sum / win_samp))
    # Slide by hop_samp
    start = hop_samp
    while start + win_samp <= n:
        # recompute sum for each hop to keep simple and robust
        window_sum = 0.0
        for i in range(start, start + win_samp):
            x = samples[i]
            window_sum += x * x
        rms.append(math.sqrt(window_sum / win_samp))
        start += hop_samp
    return rms, win_samp, hop_samp


def detect_segments(samples: List[float], sr: int,
                    threshold_db: float = -40.0,
                    window_ms: float = 10.0,
                    hop_ms: float = 5.0,
                    min_silence_ms: float = 100.0,
                    min_sound_ms: float = 50.0,
                    prepad_ms: float = 5.0,
                    postpad_ms: float = 10.0) -> List[Tuple[int, int]]:
    """Return list of (start_sample, end_sample) for detected segments.
    Uses RMS thresholding with a silence-duration requirement to close a segment.
    """
    thr = 10.0 ** (threshold_db / 20.0)
    rms, win_samp, hop_samp = rms_frames(samples, sr, window_ms, hop_ms)

    # Build a boolean activity vector
    active = [v > thr for v in rms]
    segs: List[Tuple[int, int]] = []
    in_seg = False
    seg_start_frame = 0
    silence_run_frames = 0
    min_silence_frames = max(1, int(round((min_silence_ms / 1000.0) / (hop_samp / sr))))
    min_sound_frames = max(1, int(round((min_sound_ms / 1000.0) / (hop_samp / sr))))

    for i, is_on in enumerate(active):
        if is_on:
            if not in_seg:
                # start of a new segment
                in_seg = True
                seg_start_frame = i
                silence_run_frames = 0
            else:
                # continuation
                silence_run_frames = 0
        else:
            if in_seg:
                silence_run_frames += 1
                if silence_run_frames >= min_silence_frames:
                    # Close segment at the last active frame before silence run
                    seg_end_frame = i - silence_run_frames
                    # Validate min length
                    if seg_end_frame - seg_start_frame + 1 >= min_sound_frames:
                        start_samp = max(0, seg_start_frame * hop_samp - int(round(prepad_ms * sr / 1000.0)))
                        # Include window on the end and postpad
                        end_samp = (seg_end_frame * hop_samp) + win_samp + int(round(postpad_ms * sr / 1000.0))
                        segs.append((start_samp, end_samp))
                    in_seg = False
                    silence_run_frames = 0
            else:
                # remain idle
                pass

    # If ended while in segment, close at end of audio
    if in_seg:
        seg_end_frame = len(active) - 1
        if seg_end_frame - seg_start_frame + 1 >= min_sound_frames:
            start_samp = max(0, seg_start_frame * hop_samp - int(round(prepad_ms * sr / 1000.0)))
            end_samp = (seg_end_frame * hop_samp) + win_samp + int(round(postpad_ms * sr / 1000.0))
            segs.append((start_samp, end_samp))

    # Clamp and merge overlap
    n = len(samples)
    clamped: List[Tuple[int, int]] = []
    for s, e in segs:
        s2 = max(0, min(n, s))
        e2 = max(s2, min(n, e))
        if not clamped:
            clamped.append((s2, e2))
        else:
            ps, pe = clamped[-1]
            if s2 <= pe:
                clamped[-1] = (ps, max(pe, e2))
            else:
                clamped.append((s2, e2))
    return clamped


def write_wav_int16(path: Path, sr: int, mono_bytes: bytes):
    with wave.open(str(path), 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sr)
        w.writeframes(mono_bytes)


def peak_normalize_int16(mono_bytes: bytes, target_dbfs: float = 0.0) -> bytes:
    """Normalize int16 PCM to a target peak dBFS (default 0 dBFS).
    Keeps zeros unchanged; if peak is 0, returns original bytes.
    """
    if not mono_bytes:
        return mono_bytes
    n = len(mono_bytes) // 2
    max_abs = 0
    # Find peak
    for i in range(n):
        v = struct.unpack_from('<h', mono_bytes, i*2)[0]
        av = abs(v)
        if av > max_abs:
            max_abs = av
    if max_abs == 0:
        return mono_bytes
    # Target peak linear
    target_peak = min(0.9999, 10.0 ** (target_dbfs / 20.0))
    # Input peak normalized to 1.0 is max_abs / 32767
    current_peak = max_abs / 32767.0
    if current_peak <= 0:
        return mono_bytes
    gain = target_peak / current_peak
    # Apply gain and clamp
    out = bytearray(len(mono_bytes))
    for i in range(n):
        v = struct.unpack_from('<h', mono_bytes, i*2)[0]
        f = v * gain
        if f > 32767:
            f = 32767
        elif f < -32768:
            f = -32768
        struct.pack_into('<h', out, i*2, int(round(f)))
    return bytes(out)


def apply_fade_out_int16(mono_bytes: bytes, tail_samples: int) -> bytes:
    """Apply a linear fade to zero over the last tail_samples of an int16 PCM slice."""
    if not mono_bytes or tail_samples <= 0:
        return mono_bytes
    n = len(mono_bytes) // 2
    if n <= 0:
        return mono_bytes
    tail = min(tail_samples, n)
    start = n - tail
    out = bytearray(mono_bytes)
    # If tail==1, set last sample to zero
    denom = (tail - 1) if tail > 1 else 1
    for i in range(tail):
        idx = start + i
        v = struct.unpack_from('<h', mono_bytes, idx * 2)[0]
        gain = 1.0 - (i / denom)
        f = v * gain
        if f > 32767:
            f = 32767
        elif f < -32768:
            f = -32768
        struct.pack_into('<h', out, idx * 2, int(round(f)))
    return bytes(out)


def slice_and_write(channel_bytes: bytes, sr: int, out_dir: Path, base: str, tag: str,
                    threshold_db: float, window_ms: float, hop_ms: float,
                    min_silence_ms: float, min_sound_ms: float, prepad_ms: float, postpad_ms: float,
                    normalize: bool, target_dbfs: float,
                    fade_out_samples: int) -> int:
    floats = int16_bytes_to_float_list_le(channel_bytes)
    segs = detect_segments(floats, sr, threshold_db, window_ms, hop_ms, min_silence_ms, min_sound_ms, prepad_ms, postpad_ms)
    count = 0
    for idx, (s, e) in enumerate(segs, start=1):
        s_bytes = s * 2
        e_bytes = e * 2
        if s_bytes >= len(channel_bytes):
            continue
        if e_bytes <= s_bytes:
            continue
        slice_bytes = channel_bytes[s_bytes:e_bytes]
        if normalize:
            slice_bytes = peak_normalize_int16(slice_bytes, target_dbfs=target_dbfs)
        if fade_out_samples and fade_out_samples > 0:
            slice_bytes = apply_fade_out_int16(slice_bytes, fade_out_samples)
        out_name = f"{base}_{tag}_{idx:03d}.wav"
        out_path = out_dir / out_name
        write_wav_int16(out_path, sr, slice_bytes)
        count += 1
    return count


def main():
    ap = argparse.ArgumentParser(description='Auto-slice a WAV by silence into mono snippets.')
    ap.add_argument('input', help='Path to input WAV (16-bit PCM)')
    ap.add_argument('--out-dir', default='autosliced', help='Output directory (default: autosliced)')
    ap.add_argument('--threshold-db', type=float, default=-40.0, help='Silence threshold in dBFS (default: -40.0)')
    ap.add_argument('--window-ms', type=float, default=10.0, help='RMS window size in ms (default: 10)')
    ap.add_argument('--hop-ms', type=float, default=5.0, help='RMS hop size in ms (default: 5)')
    ap.add_argument('--min-silence-ms', type=float, default=100.0, help='Min silence to close a segment in ms (default: 100)')
    ap.add_argument('--min-sound-ms', type=float, default=50.0, help='Min segment length in ms to keep (default: 50)')
    ap.add_argument('--prepad-ms', type=float, default=5.0, help='Padding before start in ms (default: 5)')
    ap.add_argument('--postpad-ms', type=float, default=10.0, help='Padding after end in ms (default: 10)')
    ap.add_argument('--no-normalize', action='store_true', help='Disable peak normalization of slices (default: enabled)')
    ap.add_argument('--normalize-db', type=float, default=0.0, help='Target peak dBFS for normalization (default: 0.0)')
    ap.add_argument('--fade-out-samples', type=int, default=32, help='Apply linear fade over last N samples of each slice (default: 32; 0 disables)')
    args = ap.parse_args()

    in_path = Path(args.input)
    if not in_path.exists():
        raise SystemExit(f'Input file not found: {in_path}')

    out_dir = (in_path.parent / args.out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f'Reading: {in_path}')
    sr, nch, raw = read_wav_int16(in_path)
    print(f'Format: {sr} Hz, {nch} channel(s)')
    chans = deinterleave_int16(raw, nch)

    base = in_path.stem
    total = 0
    if not chans:
        print('No channels found.')
        return

    for ch_idx, ch_bytes in enumerate(chans):
        tag = 'L' if ch_idx == 0 else ('R' if ch_idx == 1 else f'C{ch_idx+1}')
        print(f'Analyzing channel {ch_idx+1} ({tag})...')
        n = slice_and_write(
            ch_bytes.tobytes(), sr, out_dir, base, tag,
            args.threshold_db, args.window_ms, args.hop_ms,
            args.min_silence_ms, args.min_sound_ms,
            args.prepad_ms, args.postpad_ms,
            normalize=(not args.no_normalize), target_dbfs=args.normalize_db,
            fade_out_samples=max(0, int(args.fade_out_samples))
        )
        print(f'  -> wrote {n} slice(s).')
        total += n

    print(f'Done. Total slices written: {total}. Output folder: {out_dir}')


if __name__ == '__main__':
    main()
