import os
import json
from pathlib import Path
import struct

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
    }
    meta = {}

    with open(filepath, 'rb') as f:
        # RIFF header: 'RIFF'(4) + size(4) + 'WAVE'(4)
        header = f.read(12)
        if len(header) < 12 or header[0:4] not in (b'RIFF', b'RIFX') or header[8:12] != b'WAVE':
            return fmt, meta  # Not a standard RIFF/WAVE
        little = header[0:4] == b'RIFF'
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
                                # KSDATAFORMAT_SUBTYPE_PCM GUID in little-endian byte order
                                PCM_GUID_LE = b'\x01\x00\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71'
                                fmt['pcm'] = (subformat == PCM_GUID_LE)
                    fmt['sample_resolution'] = bits_per_sample if bits_per_sample != 0 else None
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


def main():
    base_dir = Path(__file__).parent
    # Find .wav files case-insensitively (e.g., .wav, .WAV, .WaV)
    wav_files = [p for p in base_dir.rglob('*.[Ww][Aa][Vv]')]
    results = []
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
    out_path = base_dir / 'wav_info.json'
    with open(out_path, 'w') as f:
        json.dump(results, f, indent=2)
    print(f"Processed {len(results)} wav files. Output: {out_path.name}")

if __name__ == '__main__':
    main()
