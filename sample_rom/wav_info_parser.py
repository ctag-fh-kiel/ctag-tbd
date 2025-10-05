import os
import json
from pathlib import Path
import struct
import shutil
import re
import unicodedata
# import audioop  # removed: Python 3.13 no longer includes audioop
import wave
import sys

# Try optional numpy for faster conversion/resampling
try:
    import numpy as _np
except Exception:
    _np = None

# Parse WAV chunks in a streaming way to extract format info, data size, and RIFF INFO metadata

def parse_wav(filepath):
    fmt = {
        'sample_rate': None,
        'sample_resolution': None,
        'channels': None,
        'block_align': None,
        'format_tag': None,
        'data_bytes': 0,
        'fact_samples': None,
        'data_offset': None,
        'pcm': None,
        'ieee_float': None,
        'sampwidth': None,
        'data_chunks': [],  # list of (payload_offset, size)
        'little_endian': True,
    }
    meta = {}

    with open(filepath, 'rb') as f:
        # RIFF header: 'RIFF'(4) + size(4) + 'WAVE'(4)
        header = f.read(12)
        if len(header) < 12 or header[0:4] not in (b'RIFF', b'RIFX') or header[8:12] != b'WAVE':
            return fmt, meta  # Not a standard RIFF/WAVE
        little = header[0:4] == b'RIFF'
        fmt['little_endian'] = little
        endian = '<' if little else '>'

        # Iterate over chunks
        while True:
            chunk_hdr = f.read(8)
            if len(chunk_hdr) < 8:
                break
            chunk_id = chunk_hdr[0:4]
            chunk_size = struct.unpack(endian + 'I', chunk_hdr[4:8])[0]
            chunk_start = f.tell()

            if chunk_id == b'fmt ':
                # Read base fmt fields
                to_read = min(chunk_size, 64)
                fmt_data = f.read(to_read)
                if len(fmt_data) >= 16:
                    wFormatTag, nChannels, nSamplesPerSec, nAvgBytesPerSec, nBlockAlign, wBitsPerSample = struct.unpack(endian + 'HHIIHH', fmt_data[:16])
                    fmt['format_tag'] = int(wFormatTag)
                    fmt['channels'] = int(nChannels)
                    fmt['sample_rate'] = int(nSamplesPerSec)
                    fmt['block_align'] = int(nBlockAlign)
                    bits_per_sample = int(wBitsPerSample)
                    # Default PCM detection
                    fmt['pcm'] = (fmt['format_tag'] == 0x0001)
                    fmt['ieee_float'] = (fmt['format_tag'] == 0x0003)
                    # If extensible and we have enough bytes, attempt to read wValidBitsPerSample and SubFormat GUID
                    if fmt['format_tag'] == 0xFFFE:
                        if len(fmt_data) >= 20:
                            cbSize = struct.unpack(endian + 'H', fmt_data[16:18])[0]
                            if cbSize >= 2:
                                wValidBitsPerSample = struct.unpack(endian + 'H', fmt_data[18:20])[0]
                                if wValidBitsPerSample:
                                    bits_per_sample = int(wValidBitsPerSample)
                            # Check SubFormat GUID if available (requires 22 bytes: 2+4+16)
                            if cbSize >= 22 and len(fmt_data) >= 40:
                                subformat = fmt_data[24:40]
                                # GUIDs in little-endian byte order
                                PCM_GUID_LE = b'\x01\x00\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71'
                                FLOAT_GUID_LE = b'\x03\x00\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71'
                                fmt['pcm'] = (subformat == PCM_GUID_LE)
                                fmt['ieee_float'] = (subformat == FLOAT_GUID_LE)
                    fmt['sample_resolution'] = bits_per_sample if bits_per_sample != 0 else None
                    if fmt['sample_resolution']:
                        fmt['sampwidth'] = max(1, fmt['sample_resolution'] // 8)
                # Seek to end of chunk (consider padding byte for odd sizes)
                f.seek(chunk_start + chunk_size + (chunk_size % 2))
                continue

            if chunk_id == b'LIST':
                # Peek the list type
                list_type = f.read(4)
                remaining = chunk_size - 4 if chunk_size >= 4 else 0
                if list_type == b'INFO' and remaining > 0:
                    data = f.read(remaining)
                    sub_idx = 0
                    while sub_idx + 8 <= len(data):
                        sub_id = data[sub_idx:sub_idx+4]
                        sub_size = struct.unpack(endian + 'I', data[sub_idx+4:sub_idx+8])[0]
                        sub_data_start = sub_idx + 8
                        sub_data_end = sub_data_start + sub_size
                        if sub_data_end > len(data):
                            break
                        value = data[sub_data_start:sub_data_end]
                        value_str = value.rstrip(b'\x00').decode('utf-8', errors='ignore')
                        key = sub_id.decode('ascii', errors='ignore')
                        if key:
                            meta[key] = value_str
                        # Subchunks are padded to even size
                        sub_idx = sub_data_end + (sub_size % 2)
                else:
                    # Not an INFO list; skip payload
                    f.seek(f.tell() + remaining)
                # Ensure file position is at end of chunk including padding
                after = chunk_start + chunk_size + (chunk_size % 2)
                f.seek(after)
                continue

            if chunk_id == b'data':
                # record the payload offset
                fmt['data_chunks'].append((chunk_start, chunk_size))
                # Record the start of sample data (first data chunk)
                if fmt['data_offset'] is None:
                    fmt['data_offset'] = chunk_start
                # Accumulate data bytes; skip payload
                fmt['data_bytes'] += chunk_size
                f.seek(chunk_start + chunk_size + (chunk_size % 2))
                continue

            if chunk_id == b'fact':
                # For non-PCM, first 4 bytes often contain total sample frames
                fact_data = f.read(min(chunk_size, 4))
                if len(fact_data) >= 4:
                    fmt['fact_samples'] = struct.unpack(endian + 'I', fact_data[:4])[0]
                f.seek(chunk_start + chunk_size + (chunk_size % 2))
                continue

            # Skip other chunks
            f.seek(chunk_start + chunk_size + (chunk_size % 2))

    # fallback sampwidth if missing
    if fmt['sampwidth'] is None and fmt['block_align'] and fmt['channels']:
        fmt['sampwidth'] = max(1, fmt['block_align'] // fmt['channels'])
    return fmt, meta


def get_wav_info(filepath, base_dir):
    info = {}
    p = Path(filepath)
    # Remove extension
    info['filename'] = p.stem
    # Only the directory path, relative to base
    rel_parent = '' if p.parent == base_dir else str(p.parent.relative_to(base_dir))
    info['path'] = rel_parent

    fmt, meta = parse_wav(filepath)
    info['sample_rate'] = fmt['sample_rate']
    info['sample_resolution'] = fmt['sample_resolution']
    info['channels'] = fmt['channels']
    # mono: True if exactly 1 channel, else False
    info['mono'] = True if fmt['channels'] == 1 else False

    # nsamples: prefer FACT chunk if present, else derive from data_bytes and block_align
    nsamples = None
    if fmt.get('fact_samples') is not None:
        nsamples = int(fmt['fact_samples'])
    elif fmt.get('data_bytes') and fmt.get('block_align'):
        try:
            nsamples = int(fmt['data_bytes'] // fmt['block_align'])
        except Exception:
            nsamples = None
    info['nsamples'] = nsamples

    # offset: byte offset to the beginning of sample data
    info['offset'] = fmt.get('data_offset')
    # Add format_ok: 44.1kHz, mono, 16-bit, PCM
    info['format_ok'] = (
            (fmt.get('sample_rate') == 44100)
            and (fmt.get('channels') == 1)
            and (fmt.get('sample_resolution') == 16)
            and (fmt.get('pcm') is True)
    )
    info['meta_data'] = meta
    return info


def _normalize_stem(name: str) -> str:
    # Normalize unicode to ASCII where possible
    n = unicodedata.normalize('NFKD', name)
    n = n.encode('ascii', 'ignore').decode('ascii')
    # Replace separators with underscore
    n = re.sub(r'[\s\-]+', '_', n)
    # Remove any character not alnum or underscore
    n = re.sub(r'[^A-Za-z0-9_]', '', n)
    # Collapse multiple underscores and trim
    n = re.sub(r'_+', '_', n).strip('_')
    # If empty after sanitization, fallback to 'SAMPLE'
    return n if n else 'SAMPLE'


def _shorten_stem(name: str, max_len: int = 32) -> str:
    n = _normalize_stem(name)
    if len(n) <= max_len:
        return n
    # Preserve start and end around an underscore separator
    keep = max_len - 1  # space for the underscore
    first = keep // 2
    last = keep - first
    return f"{n[:first]}_{n[-last:]}"


def _unique_dest_stem(dest_dir: Path, stem: str, ext: str, max_len: int = 32) -> str:
    base = _shorten_stem(stem, max_len)
    candidate = base
    i = 2
    # If file exists, append _2, _3, ... while keeping <= max_len
    while (dest_dir / f"{candidate}{ext}").exists():
        suffix = f"_{i}"
        # Truncate base to allow suffix
        allowed = max_len - len(suffix)
        truncated = base[:allowed]
        # Avoid trailing underscore
        truncated = truncated.rstrip('_')
        candidate = f"{truncated}{suffix}"
        i += 1
    return candidate


def _multi_to_mono_first_channel(buf: bytes, width: int, channels: int) -> bytes:
    if channels <= 1:
        return buf
    frame_size = width * channels
    out = bytearray()
    for i in range(0, len(buf), frame_size):
        out += buf[i:i+width]
    return bytes(out)


def _bytes_to_mono_float(data: bytes, sampwidth: int, channels: int) -> list:
    """Convert interleaved PCM bytes to mono float samples [-1.0, 1.0].
    If channels > 1, average channels. Pure-Python fallback (no numpy).
    """
    if sampwidth not in (1, 2, 3, 4):
        raise ValueError(f"Unsupported sample width: {sampwidth}")
    frame_size = sampwidth * channels
    n_frames = len(data) // frame_size
    floats = []
    # Precompute divisors and unpack formats
    if sampwidth == 1:
        # 8-bit PCM WAV is unsigned [0..255]
        for i in range(n_frames):
            acc = 0
            for ch in range(channels):
                v = data[i*frame_size + ch]  # unsigned byte
                acc += (v - 128) / 128.0
            floats.append(acc / channels)
    elif sampwidth == 2:
        for i in range(n_frames):
            acc = 0
            base = i*frame_size
            for ch in range(channels):
                lo = data[base + ch*2]
                hi = data[base + ch*2 + 1]
                val = int.from_bytes(bytes((lo, hi)), 'little', signed=True)
                acc += max(-32768, min(32767, val)) / 32768.0
            floats.append(acc / channels)
    elif sampwidth == 3:
        for i in range(n_frames):
            acc = 0
            base = i*frame_size
            for ch in range(channels):
                b0 = data[base + ch*3]
                b1 = data[base + ch*3 + 1]
                b2 = data[base + ch*3 + 2]
                val = int.from_bytes(bytes((b0, b1, b2)), 'little', signed=True)
                # int.from_bytes with signed=True for 3 bytes sign-extends correctly
                acc += max(-8388608, min(8388607, val)) / 8388608.0
            floats.append(acc / channels)
    else:  # sampwidth == 4
        for i in range(n_frames):
            acc = 0
            base = i*frame_size
            for ch in range(channels):
                b = data[base + ch*4: base + ch*4 + 4]
                val = int.from_bytes(b, 'little', signed=True)
                acc += max(-2147483648, min(2147483647, val)) / 2147483648.0
            floats.append(acc / channels)
    return floats


def _bytes_to_mono_float_ieee(data: bytes, sampwidth: int, channels: int) -> list:
    if sampwidth not in (4, 8):
        raise ValueError(f"Unsupported IEEE float width: {sampwidth}")
    frame_size = sampwidth * channels
    n_frames = len(data) // frame_size
    floats = []
    for i in range(n_frames):
        acc = 0.0
        base = i * frame_size
        for ch in range(channels):
            off = base + ch * sampwidth
            if sampwidth == 4:
                # little-endian float32
                v = struct.unpack_from('<f', data, off)[0]
            else:
                v = struct.unpack_from('<d', data, off)[0]
            acc += v
        floats.append(acc / channels)
    return floats


def _resample_linear_py(samples: list, in_rate: int, out_rate: int) -> list:
    """Simple linear interpolation resampler in pure Python.
    Input: list of float samples; Output: list of float samples.
    """
    if in_rate == out_rate or len(samples) == 0:
        return samples[:]
    ratio = out_rate / in_rate
    out_len = int(round(len(samples) * ratio))
    if out_len <= 1:
        return samples[:1]
    res = [0.0] * out_len
    for i in range(out_len):
        # position in input
        pos = i / ratio
        i0 = int(pos)
        i1 = min(i0 + 1, len(samples) - 1)
        frac = pos - i0
        res[i] = samples[i0] * (1.0 - frac) + samples[i1] * frac
    return res


def _float_to_int16_bytes(samples) -> bytes:
    out = bytearray()
    for x in samples:
        if x > 1.0:
            x = 1.0
        elif x < -1.0:
            x = -1.0
        v = int(round(x * 32767.0))
        out += int(v).to_bytes(2, 'little', signed=True)
    return bytes(out)


def _read_data_chunks(filepath: Path, data_chunks):
    # Concatenate all data chunk payloads in order
    out = bytearray()
    with open(filepath, 'rb') as f:
        for payload_off, size in data_chunks:
            f.seek(payload_off)
            # skip 0: we are already at payload offset immediately after 8-byte header
            out += f.read(size)
    return bytes(out)


def copy_with_progress(src: Path, dest: Path, progress_cb=None):
    """Copy file src -> dest in chunks and call progress_cb(percent) if provided."""
    total = 0
    try:
        total = src.stat().st_size
    except Exception:
        total = 0
    copied = 0
    # Stream copy to allow progress reporting
    with open(src, 'rb') as fsrc, open(dest, 'wb') as fdst:
        while True:
            buf = fsrc.read(1024 * 1024)
            if not buf:
                break
            fdst.write(buf)
            copied += len(buf)
            if progress_cb and total:
                pct = int((copied / total) * 100)
                if pct > 100:
                    pct = 100
                progress_cb(pct)
    # Preserve metadata where possible
    try:
        shutil.copystat(src, dest)
    except Exception:
        pass


def convert_to_pcm16_mono_44100(src_path: Path, dest_path: Path, progress_cb=None) -> int:
    """Convert a WAV file to 44.1kHz, mono, 16-bit PCM and write to dest_path.
    progress_cb: optional callable(percent:int) to receive progress updates 0..100.
    Returns the number of output samples (frames). Raises on failure.
    """
    fmt_info, _ = parse_wav(src_path)
    if not fmt_info.get('little_endian'):
        raise ValueError('Big-endian RIFX WAV not supported for conversion')
    is_float = bool(fmt_info.get('ieee_float'))
    in_rate = int(fmt_info.get('sample_rate') or 0)
    in_channels = int(fmt_info.get('channels') or 0)
    in_width = int(fmt_info.get('sampwidth') or 0)
    if not in_rate or not in_channels or not in_width:
        raise ValueError('Missing essential format fields for conversion')
    if not fmt_info.get('data_chunks'):
        raise ValueError('No data chunks found')

    # Stream-read raw bytes with progress (map read progress to 0..90)
    total_bytes = sum(sz for _, sz in fmt_info['data_chunks'])
    read_bytes = 0
    raw_buf = bytearray()
    with open(src_path, 'rb') as f:
        for payload_off, size in fmt_info['data_chunks']:
            remaining = size
            pos = 0
            while remaining > 0:
                to_read = 1024 * 1024 if remaining >= 1024 * 1024 else remaining
                f.seek(payload_off + pos)
                chunk = f.read(to_read)
                if not chunk:
                    break
                raw_buf += chunk
                pos += len(chunk)
                remaining -= len(chunk)
                read_bytes += len(chunk)
                if progress_cb and total_bytes:
                    pct = int((read_bytes / total_bytes) * 90)
                    if pct > 90:
                        pct = 90
                    progress_cb(pct)
    raw = bytes(raw_buf)

    out_rate = 44100
    # Indicate conversion phase start (90..95)
    if progress_cb:
        progress_cb(92)
    # Fast path with numpy if available
    if _np is not None:
        if is_float:
            if in_width == 4:
                arr = _np.frombuffer(raw, dtype='<f4').astype(_np.float64)
            elif in_width == 8:
                arr = _np.frombuffer(raw, dtype='<f8').astype(_np.float64)
            else:
                raise ValueError(f'Unsupported IEEE float width: {in_width}')
        else:
            if in_width == 1:
                arr = _np.frombuffer(raw, dtype=_np.uint8)
                arr = (arr.astype(_np.float64) - 128.0) / 128.0
            elif in_width == 2:
                arr = _np.frombuffer(raw, dtype='<i2').astype(_np.float64) / 32768.0
            elif in_width == 3:
                b = _np.frombuffer(raw, dtype=_np.uint8).reshape(-1, 3)
                sign = (b[:, 2] >= 128).astype(_np.uint8) * 255
                b4 = _np.column_stack((b, sign))
                arr = b4.view('<i4').reshape(-1).astype(_np.float64) / 8388608.0
            elif in_width == 4:
                arr = _np.frombuffer(raw, dtype='<i4').astype(_np.float64) / 2147483648.0
            else:
                raise ValueError(f'Unsupported sample width: {in_width}')
        if in_channels > 1:
            arr = arr.reshape(-1, in_channels).mean(axis=1)
        if in_rate != out_rate:
            in_len = arr.shape[0]
            out_len = int(round(in_len * (out_rate / in_rate)))
            if out_len > 1 and in_len > 1:
                x = _np.linspace(0, 1, in_len, endpoint=True)
                xi = _np.linspace(0, 1, out_len, endpoint=True)
                arr = _np.interp(xi, x, arr)
        arr = _np.clip(arr, -1.0, 1.0)
        int16 = (arr * 32767.0).round().astype('<i2')
        out_bytes = int16.tobytes()
        nsamples = len(int16)
    else:
        if is_float:
            mono = _bytes_to_mono_float_ieee(raw, in_width, in_channels)
        else:
            mono = _bytes_to_mono_float(raw, in_width, in_channels)
        res = _resample_linear_py(mono, in_rate, out_rate) if in_rate != out_rate else mono
        out_bytes = _float_to_int16_bytes(res)
        nsamples = len(res)
    # Indicate nearing write (95)
    if progress_cb:
        progress_cb(95)

    with wave.open(str(dest_path), 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(out_rate)
        # Write in chunks to avoid large syscalls and update final progress to 100
        mv = memoryview(out_bytes)
        offset = 0
        total = len(out_bytes)
        while offset < total:
            end = offset + 1024 * 1024
            if end > total:
                end = total
            w.writeframes(mv[offset:end])
            offset = end
    if progress_cb:
        progress_cb(100)
    return nsamples


def _is_under(p: Path, root: Path) -> bool:
    try:
        return p.resolve().relative_to(root.resolve()) is not None
    except Exception:
        return False


def main():
    base_dir = Path(__file__).parent
    tbds_dir = base_dir / 'tbdsamples'
    # Find .wav files case-insensitively, excluding tbdsamples (output) directory
    wav_files = [p for p in base_dir.rglob('*.[Ww][Aa][Vv]') if not _is_under(p, tbds_dir)]
    results = []
    srcs = []
    for wav_path in wav_files:
        try:
            info = get_wav_info(wav_path, base_dir)
        except Exception as e:
            info = {
                'filename': wav_path.stem,
                'path': '' if wav_path.parent == base_dir else str(wav_path.parent.relative_to(base_dir)),
                'sample_rate': None,
                'sample_resolution': None,
                'channels': None,
                'mono': False,
                'nsamples': None,
                'offset': None,
                'format_ok': False,
                'meta_data': {},
                'error': str(e),
            }
        results.append(info)
        srcs.append(wav_path)
    out_path = base_dir / 'wav_info.json'
    with open(out_path, 'w') as f:
        json.dump(results, f, indent=2)

    # Ensure destination base folder exists
    tbds_dir.mkdir(parents=True, exist_ok=True)

    # Build short entries and copy/convert into tbdsamples preserving structure and shortening names
    short_entries = []
    warnings = []
    for info, src in zip(results, srcs):
        dest_subdir = tbds_dir if info['path'] == '' else tbds_dir / info['path']
        dest_subdir.mkdir(parents=True, exist_ok=True)
        ext = src.suffix  # preserve original extension casing
        new_stem = _unique_dest_stem(dest_subdir, src.stem, ext, 32)
        dest_path = dest_subdir / f"{new_stem}{ext}"
        if info.get('format_ok') is True:
            # Copy as-is
            try:
                # Show single-line progress for this copy
                def _print_progress(pct: int):
                    msg = f"Copying   {src.name[:40]:<40} {pct:3d}%"
                    print("\r" + msg, end='', flush=True)
                copy_with_progress(src, dest_path, progress_cb=_print_progress)
                # Ensure final line ends with 100%
                print("\r" + f"Copying   {src.name[:40]:<40} 100%", end='\n', flush=True)
                ns = info.get('nsamples')
                off = info.get('offset')
            except Exception as e:
                # Ensure we end the in-place line with a newline for clarity
                print("\r" + f"Copying   {src.name[:40]:<40} ERR", end='\n', flush=True)
                warnings.append(f"Copy failed: {src} -> {dest_path}: {e}")
                continue
        else:
            # Show single-line progress for this conversion
            def _print_progress(pct: int):
                msg = f"Converting {src.name[:40]:<40} {pct:3d}%"
                print("\r" + msg, end='', flush=True)
            try:
                ns = convert_to_pcm16_mono_44100(src, dest_path, progress_cb=_print_progress)
                off = 44  # standard header for files we write
                # Finish line
                print("\r" + f"Converting {src.name[:40]:<40} 100%", end='\n', flush=True)
            except Exception as e:
                # Ensure we end the in-place line with a newline for clarity
                print("\r" + f"Converting {src.name[:40]:<40} ERR", end='\n', flush=True)
                warnings.append(f"Convert failed: {src} -> {dest_path}: {e}")
                continue
        entry = {
            'filename': new_stem,
            'path': info['path'],
            'nsamples': ns,
        }
        if off is not None and off != 44:
            entry['offset'] = off
        short_entries.append(entry)

    # Write shortened JSON into tbdsamples folder
    short_path = tbds_dir / 'wav_info_short.json'
    with open(short_path, 'w') as f:
        json.dump(short_entries, f, indent=2)

    # Write warnings (if any)
    if warnings:
        warn_path = tbds_dir / 'conversion_warnings.txt'
        with open(warn_path, 'w') as wf:
            for line in warnings:
                wf.write(line + '\n')
        for line in warnings:
            print(f"WARN: {line}")

    total = len(results)
    success = len(short_entries)
    failed = total - success
    print(f"Processed {total} wav files. Success: {success}, Failed: {failed}. Output: {out_path.name}. Placed {success} files into {tbds_dir.name} and wrote {short_path.name}.")

if __name__ == '__main__':
    main()
