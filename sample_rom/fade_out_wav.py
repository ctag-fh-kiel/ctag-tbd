#!/usr/bin/env python3
"""
Apply a short tail fade-out to mono 16-bit PCM WAV files to avoid clicks/transients.

Usage:
  python fade_tail_wavs.py input.wav [--samples 32] [--curve linear|log] [--in-place | --out-dir DIR]
  python fade_tail_wavs.py /path/to/dir [--samples 32] [--curve linear|log] [--in-place | --out-dir DIR]

Defaults:
  --samples 32        Number of samples to fade at the tail
  --curve linear      Fade curve: 'linear' or 'log'
  --in-place          Overwrite input files (mutually exclusive with --out-dir)

Notes:
  - Only mono 16-bit PCM WAVs are supported.
  - When processing a directory, all *.wav files inside (non-recursive) are processed.
"""

from __future__ import annotations
import argparse
from pathlib import Path
import sys
import wave
import struct
import math


def read_wav_int16_mono(path: Path):
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
        if nch != 1:
            raise ValueError(f'Only mono supported (got {nch} channels)')
        raw = w.readframes(nframes)
    return sr, raw


def write_wav_int16_mono(path: Path, sr: int, data: bytes):
    with wave.open(str(path), 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sr)
        w.writeframes(data)


def apply_fade_tail_int16(data: bytes, tail_samples: int, curve: str) -> bytes:
    if tail_samples <= 0 or not data:
        return data
    n = len(data) // 2
    if n <= 0:
        return data
    tail = min(tail_samples, n)
    start = n - tail
    out = bytearray(data)
    # For tail==1, set last sample to zero
    denom = (tail - 1) if tail > 1 else 1
    for i in range(tail):
        idx = start + i
        v = struct.unpack_from('<h', data, idx * 2)[0]
        t = i / denom  # 0..1 across the tail
        if curve == 'log':
            # more aggressive decay near the end (perceptually smoother)
            # gain goes from 1 -> 0; use squared decay
            gain = (1.0 - t)**2
        else:  # linear
            gain = 1.0 - t
        f = v * gain
        if f > 32767:
            f = 32767
        elif f < -32768:
            f = -32768
        struct.pack_into('<h', out, idx * 2, int(round(f)))
    return bytes(out)


def process_file(src: Path, dst: Path, samples: int, curve: str):
    sr, raw = read_wav_int16_mono(src)
    out = apply_fade_tail_int16(raw, samples, curve)
    dst.parent.mkdir(parents=True, exist_ok=True)
    write_wav_int16_mono(dst, sr, out)


def main():
    ap = argparse.ArgumentParser(description='Fade out the tail of mono 16-bit PCM WAV files.')
    ap.add_argument('input', help='Input .wav file or directory')
    ap.add_argument('--samples', type=int, default=32, help='Number of tail samples to fade (default: 32)')
    ap.add_argument('--curve', choices=['linear', 'log'], default='linear', help='Fade curve (default: linear)')
    g = ap.add_mutually_exclusive_group()
    g.add_argument('--in-place', action='store_true', help='Overwrite input files')
    g.add_argument('--out-dir', help='Output directory (preserve filenames)')
    args = ap.parse_args()

    src = Path(args.input)
    if not src.exists():
        print(f'Not found: {src}', file=sys.stderr)
        sys.exit(1)

    targets = []
    if src.is_dir():
        targets = [p for p in src.iterdir() if p.is_file() and p.suffix.lower() == '.wav']
    else:
        targets = [src]

    if not targets:
        print('No .wav files to process.', file=sys.stderr)
        sys.exit(1)

    out_dir = None
    if args.out_dir:
        out_dir = Path(args.out_dir)
        out_dir.mkdir(parents=True, exist_ok=True)

    processed = 0
    for f in targets:
        try:
            if args.in_place and not out_dir:
                dst = f
            else:
                dst = (out_dir or f.parent) / f.name
            process_file(f, dst, samples=max(0, int(args.samples)), curve=args.curve)
            print(f'Processed: {f.name} -> {dst}')
            processed += 1
        except Exception as e:
            print(f'WARN: failed {f}: {e}', file=sys.stderr)

    print(f'Done. {processed} file(s) processed.')


if __name__ == '__main__':
    main()
