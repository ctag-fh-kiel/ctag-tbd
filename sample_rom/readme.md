# Sample Bank Manager

A web-based tool for building and managing sample banks for the CTAG-TBD audio platform. Supports up to 128 samples with intelligent preview generation, tagging, and capacity management.

## Features

- **128 Sample Bank Management** - Build custom sample banks with capacity tracking
- **Intelligent Preview Generation** - Adaptive algorithms for different sample types:
  - Peak detection for transients (drums, percussion)
  - RMS-based for AM-modulated sounds (tremolo, pads)
  - Spectral-weighted for frequency-varying sounds (filter sweeps)
  - Hybrid approaches for complex evolving sounds
- **Tag System** - Classify samples with predefined or custom tags
- **OLED Preview** - Real-time 128×64 monochrome display simulation (2.42" display)
- **Audio Preview** - Integrated waveform display with playback
- **Export/Import** - Save and load sample banks with metadata

---

## Input File Formats

### 1. wav_info_short.json

**Location:** Must be in the tbdsamples folder
**Format:** JSON array describing all available WAV files

```json
[
  {
    "filename": "kick_01.wav",
    "path": "drums",
    "nsamples": 44100,
    "offset": 44
  },
  {
    "filename": "pad_ambient.wav",
    "path": "synth",
    "nsamples": 220500,
    "offset": 44
  }
]
```

**Fields:**
- `filename` (string, required) - WAV filename
- `path` (string, optional) - Subdirectory path within tbdsamples
- `nsamples` (number, required) - Number of PCM samples (not bytes)
- `offset` (number, optional) - Byte offset to PCM data (typically 44 for standard WAV)

**Notes:**
- All WAV files must be **16-bit mono PCM**
- Sample rate: typically 44.1kHz
- Total capacity limit: **8,388,608 bytes** (4,194,304 samples × 2 bytes)

---

### 2. WAV Audio Files

**Location:** tbdsamples folder (with optional subdirectories)
**Format:** Standard WAV files

**Requirements:**
- Format: 16-bit PCM
- Channels: Mono (1 channel)
- Sample rate: Any (typically 44.1kHz)
- Byte order: Little-endian (standard WAV)

**File Structure:**
```
tbdsamples/
  ├── wav_info_short.json
  ├── kick_01.wav
  ├── snare_02.wav
  ├── drums/
  │   ├── hat_closed.wav
  │   └── hat_open.wav
  └── synth/
      └── pad_ambient.wav
```

---

## Output File Formats

### 1. sample_bank.jsn

**Format:** JSON array with sample metadata and tags

```json
[
  {
    "filename": "kick_01.wav",
    "path": "drums",
    "nsamples": 44100,
    "offset": 44,
    "sname": "kick1",
    "tags": ["kick", "percussive", "short", "techno"]
  },
  {
    "filename": "pad_ambient.wav",
    "path": "synth",
    "nsamples": 220500,
    "sname": "ambpad",
    "tags": ["pad", "ambient", "long", "wet", "reverb"]
  }
]
```

**Fields:**
- `filename` (string) - Original WAV filename
- `path` (string) - Subdirectory path
- `nsamples` (number) - Number of samples
- `offset` (number, optional) - Byte offset to PCM data
- `sname` (string) - Sample name (max 8 characters) for device display
- `tags` (array, optional) - Array of tag strings for classification

**Purpose:** Device-independent metadata file for loading sample banks

---

### 2. sample_bank_preview.bin

**Format:** Binary file with 4-bit compressed preview data

**Structure:**
```
For N samples in bank:
  Total size = N × 64 bytes
  
Each sample preview:
  - 128 amplitude values (0-15 range, 4-bit)
  - Compressed to 64 bytes (2 samples per byte)
  - Each sample uses 4 bits (0-15)
  
Byte packing:
  byte[i] = (sample[2i] << 4) | sample[2i+1]
  
  High nibble (bits 7-4): First sample (0-15)
  Low nibble (bits 3-0): Second sample (0-15)
```

**Example:**
```
Sample bank with 3 entries:
  - Total size: 3 × 64 = 192 bytes
  - Samples 0-127: bytes 0-63 (first preview)
  - Samples 0-127: bytes 64-127 (second preview)
  - Samples 0-127: bytes 128-191 (third preview)
```

**Decompression Algorithm:**
```c
// Read compressed preview
uint8_t compressed[64];
uint8_t preview[128];

for (int i = 0; i < 64; i++) {
    uint8_t byte = compressed[i];
    // Extract high nibble (first sample)
    preview[i * 2] = (byte >> 4) & 0x0F;
    // Extract low nibble (second sample)
    preview[i * 2 + 1] = byte & 0x0F;
}

// Scale to display height (optional, for 24-pixel display):
for (int i = 0; i < 128; i++) {
    // Scale 0-15 to 0-23 for 24-pixel height display
    preview[i] = (preview[i] * 24) / 15;
}
```

**Purpose:** Efficient storage of OLED preview waveforms (50% size reduction)

---

## Usage Guide

### 1. Loading Samples

1. **Prepare your sample folder:**
   ```
   tbdsamples/
     ├── wav_info_short.json
     └── [your .wav files]
   ```

2. **Click "Pick tbdsamples folder…"**
   - Browser will prompt for folder selection
   - Select the folder containing wav_info_short.json
   - Tool will load all available samples

3. **Browse available samples:**
   - Files are organized by directory structure
   - Hover over samples for auto-preview
   - Click samples to preview manually

### 2. Building a Sample Bank

1. **Add samples to bank:**
   - Click "Add" button next to desired samples
   - Samples appear in "Selected bank" list
   - Capacity indicator shows remaining space

2. **Manage sample order:**
   - Samples are numbered 1-128 (slot order)
   - Remove unwanted samples with "Remove" button
   - Order determines device slot assignment

3. **Add metadata:**
   - Click ✏️ (Edit) button or right-click any sample
   - Edit panel opens with:
     - **Sample Name:** Max 8 characters for device display
     - **Tags:** Predefined + custom tags for classification
   - Changes save automatically
   - Sample name and tags visible inline

### 3. Preview Generation

**Automatic Algorithm Selection:**

The tool analyzes each sample and chooses the optimal preview algorithm:

- **Peak** - Transient-heavy sounds (drums, percussion)
  - Preserves attack transients
  - Shows clear envelope shape
  
- **RMS-AM** - Amplitude-modulated sounds (tremolo, LFO)
  - Displays smooth envelope curves
  - Good for pads with volume modulation
  
- **Spectral-Weighted** - Filter sweeps, FM synthesis
  - Tracks frequency content changes
  - Visualizes spectral movement
  
- **Hybrid-Dynamic** - Complex evolving sounds
  - Combines AM and spectral analysis
  - Best for layered textures

- **Special Cases:**
  - Short percussive samples (< 500ms)
  - Sparse samples with concentrated energy
  - Time-stretching for better visibility

**OLED Display:**
- Resolution: 128×64 pixels (2.42" display)
- Active area: 55.01 × 27.49 mm
- Preview height: 24 pixels (bottom third)
- Physical size calibration slider available
- Real-time playback position indicator

### 4. Sample Tagging

**Predefined Tag Categories:**

- **Instruments:** kick, snare, clap, hihat, cymbal, tom, percussion, bass, lead, pad, pluck, chord, stab
- **Characteristics:** short, long, percussive, melodic, atonal, tonal, bright, dark, warm, cold, clean, distorted, wet, dry, ambient, aggressive, soft, hard
- **Effects:** reverb, delay, filtered, modulated, compressed
- **Genres:** techno, house, dub, industrial, ambient, experimental

**Workflow:**
1. Select sample in bank
2. Click ✏️ or right-click row
3. Toggle quick tags or add custom tags
4. Tags visible immediately in bank list

### 5. Exporting

**Click "Export JSN":**
- Generates `sample_bank.jsn` (metadata + tags + sample names)
- Generates `sample_bank_preview.bin` (compressed preview waveforms)
- Both files download automatically

**Export includes:**
- All sample metadata (filename, path, nsamples, offset)
- Sample names (sname field, max 8 chars)
- Tags (array of strings)
- Compressed preview data (64 bytes per sample)

### 6. Importing

**Click "Load JSN":**
1. Select a previously exported `sample_bank.jsn` file
2. Tool matches samples to available files
3. Restores:
   - Sample selection
   - Sample order
   - Sample names
   - Tags
   - Preview data (if included)

**Notes:**
- Must load tbdsamples folder first
- Tool attempts to match by filename + nsamples + offset
- Falls back to filename-only matching if unique
- Warnings shown for missing files

### 7. Save to Folder (File System API)

**Available in Chromium-based browsers:**
- Saves directly to tbdsamples folder
- Writes `sample_bank.jsn`
- Writes `sample_bank_preview.bin`
- Requires write permission

---

## Capacity Management

**Limits:**
- Maximum samples: 128
- Total capacity: 8,388,608 bytes (8 MiB)
- Per sample: 2 bytes per PCM sample

**Visual Indicators:**
- **Green:** < 80% capacity
- **Yellow:** 80-95% capacity
- **Red:** > 95% capacity or exceeds limit

**File List Color Coding:**
- Files too large for remaining space highlighted in red
- Prevents adding samples that would exceed capacity

---

## Keyboard Shortcuts

- **Space:** Play/Pause audio preview
- **Click waveform:** Seek to position
- **Right-click sample row:** Open edit panel
- **Enter (in custom tag input):** Add custom tag
- **ESC:** Close floating panels (planned)

---

## Browser Compatibility

**Recommended:** Chrome, Edge, or other Chromium-based browsers

**Requirements:**
- Web Audio API (waveform playback)
- File System Access API (directory picker)
- Canvas API (waveform and OLED rendering)
- ES6+ JavaScript support

**File System API Features:**
- Directory picker: ✅ Chromium-based only
- Save to folder: ✅ Chromium-based only
- File downloads: ✅ All modern browsers

---

## Technical Details

### Preview Generation Algorithm

**Process:**
1. **DC Offset Removal** - Removes average bias from signal
2. **Waveform Analysis** - Detects AM, transients, spectral changes, silence
3. **Algorithm Selection** - Chooses optimal rendering method
4. **Block Processing** - 128 blocks, adaptive processing per block
5. **Normalization** - Optional auto-normalization for quiet samples
6. **Perceptual Curve** - Applies power curve for better visual perception
7. **Quantization** - Converts to 0-15 range (4-bit) for efficient storage

**Spectral Analysis:**
- Zero-crossing rate detection
- Perceptual frequency weighting (A-weighting inspired)
- Bass/mid/treble separation
- Brightness calculation
- Human hearing optimized

**Compression:**
- Preview values: 0-15 (4-bit, 16 levels)
- Storage: 64 bytes per preview (2 samples per byte)
- Display: Direct mapping, 0-15 values = 0-15 pixel heights
- Space savings: 50% vs 8-bit encoding
- Quality: Native 4-bit quantization, true representation

### Data Flow

```
WAV Files + wav_info_short.json
    ↓
[Load Folder]
    ↓
Available Samples List
    ↓
[Add to Bank] → [Edit Name/Tags]
    ↓
Selected Bank (128 max)
    ↓
[Export]
    ↓
sample_bank.jsn + sample_bank_preview.bin
```

---

## Troubleshooting

**"No folder loaded" message:**
- Click "Pick tbdsamples folder…"
- Ensure folder contains `wav_info_short.json`

**"Failed to parse wav_info_short.json":**
- Check JSON syntax (use validator)
- Ensure array format: `[{...}, {...}]`

**Preview generation fails:**
- Check WAV format (16-bit mono PCM required)
- Verify offset value in wav_info_short.json
- Ensure nsamples matches actual file

**Capacity exceeded:**
- Remove samples to free space
- Check sample length (nsamples × 2 bytes)
- Maximum: 8,388,608 bytes total

**Tags/names not showing:**
- Ensure sample is in Selected Bank (right side)
- Check browser console for errors
- Refresh page and reload folder

**Export not working:**
- Check browser downloads folder
- Enable downloads in browser settings
- Try different browser (Chromium recommended)

---

## File Size Reference

**Typical Sample Sizes:**
- 1 second @ 44.1kHz: 88,200 bytes (88.2 KB)
- 5 seconds: 441,000 bytes (441 KB)
- 10 seconds: 882,000 bytes (882 KB)

**Bank Capacity Examples:**
- 128 × 1-second samples: 11.3 MB (exceeds limit)
- 95 × 1-second samples: 8.4 MB (at limit)
- 47 × 2-second samples: 8.3 MB (at limit)
- 19 × 5-second samples: 8.4 MB (at limit)

**Preview Sizes:**
- Uncompressed: 128 samples × 128 bytes = 16 KB
- Compressed: 128 samples × 64 bytes = 8 KB

---

## Version History

**v1.0** - Initial release
- 128 sample bank management
- Intelligent preview generation
- Tag system with 40+ predefined tags
- OLED preview simulation
- Export/Import with metadata

**v1.1** - Preview compression
- 4-bit compressed preview format
- 50% size reduction
- Backward compatible loading

---

## Credits

**CTAG-TBD Sample Bank Manager**
- Developed for CTAG-TBD audio platform
- Preview algorithms based on DAW waveform rendering techniques
- Perceptual audio analysis with A-weighting principles
- OLED display simulation for 128×64 monochrome screens

---

## License

See LICENSE file in parent directory.

---

## Support

For issues, feature requests, or questions:
- GitHub Issues: [repository link]
- Documentation: This README
- Examples: Included sample banks

