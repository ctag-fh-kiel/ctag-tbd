# TBD-16 Memory Audit

Complete audit of every memory subsystem in the TBD-16, with practical capacity
answers for firmware, sample kits, and future expansion.

---

## 1. System Overview

| Chip | Memory Type | Size | Bus / Speed | Primary Purpose |
|------|-------------|------|-------------|-----------------|
| ESP32-P4 | Internal HP SRAM | ~768 KB | On-die | Kernel, ISR stacks, plugin DSP workspace, DMA, audio buffers |
| ESP32-P4 | Internal LP SRAM | ~8 KB | RTC domain | Low-power / deep-sleep retention (currently unused) |
| ESP32-P4 | PSRAM (stacked) | **32 MB** | Octal SPI @ 200 MHz | Sample data buffer, effect delay lines, JSON, HTTP bodies |
| ESP32-P4 | Flash (SPI) | **16 MB** | Quad SPI @ 80 MHz | Firmware, bootloader, NVS, OTA, partition table |
| RP2350B | Internal SRAM | 520 KB | On-die | Sequencer state, OLED framebuffer, button/pot scan, MIDI |
| RP2350B | Flash | ~4 MB | Separate SPI | RP2350 firmware (updated via BOOTSEL USB) |
| ESP32-C6 | Internal | Small | Managed by P4 | WiFi 6 radio + Bluetooth 5 stack |
| — | **SD Card** (micro) | User-supplied | SDMMC 4-bit | Sample banks, presets, WebUI, config — no hard limit |

---

## 2. Flash Memory (16 MB)

### 2.1 Config D — TBD-16 Default (SD card + RP2350)

Source: [`partitions_example.csv`](../partitions_example.csv)

```
 Offset      End         Size        Partition        Purpose
─────────────────────────────────────────────────────────────────
 0x000000    0x001FFF       8 KB     (reserved)       ESP-IDF boot vector
 0x002000    0x007FFF      24 KB     bootloader       Second-stage bootloader (P4 = 0x2000, NOT 0x1000)
 0x008000    0x008FFF       4 KB     partition-table  Flash layout metadata
 0x009000    0x00CFFF      16 KB     nvs              WiFi credentials, device settings
 0x00D000    0x00EFFF       8 KB     otadata          OTA boot partition selector
 0x00F000    0x00FFFF       4 KB     phy_init         WiFi PHY calibration data
 0x010000    0x50FFFF       5 MB     ota_0            ★ Main firmware (dada-tbd.bin)
 0x510000    0x60FFFF       1 MB     ota_1            USB MSC helper image (too small for main FW OTA)
 0x610000    0xFFFFFF    9.94 MB     —                ★ UNALLOCATED
─────────────────────────────────────────────────────────────────
                          16 MB      Total
```

**Actual firmware binary size:** `dada-tbd.bin` = **3.2 MB** (as of current build)
→ 1.8 MB headroom in the 5 MB `ota_0` partition.

**Actual bootloader size:** `bootloader.bin` = **22 KB** (fits the 24 KB slot).

> **Note:** `ota_1` (1 MB) is intentionally small — it stores only the tiny USB Mass
> Storage firmware used during SD card deploy. It is **not** large enough for OTA
> updates of the main firmware.

### 2.2 Config A/B — Flash-Only (No SD Card)

Source: [`partitions_no_sd.csv`](../partitions_no_sd.csv)

```
 Offset      End         Size        Partition        Purpose
─────────────────────────────────────────────────────────────────
 0x000000    0x00FFFF      64 KB     (system)         Reserved + bootloader + partition table + NVS + OTA + PHY
 0x010000    0x32FFFF    3.13 MB     ota_0            Main firmware
 0x330000    0x52FFFF       2 MB     storage          LittleFS — presets, WebUI (replaces SD card)
 0x530000    0xA2FFFF       5 MB     sample_rom       Flash-based sample data (subtype 0x40)
 0xA30000    0xFFFFFF    5.81 MB     —                UNALLOCATED
─────────────────────────────────────────────────────────────────
                          16 MB      Total
```

Flash sample ROM capacity: 5 MB ≈ **59 seconds** of audio at 44.1 kHz / 16-bit mono.

### 2.3 Flash Usage Summary

| What | Config D | Config A/B |
|------|----------|------------|
| Firmware budget | 5 MB | 3.13 MB |
| Firmware actual | 3.2 MB | 3.2 MB |
| Firmware headroom | 1.8 MB | ~0 MB (tight!) |
| Sample storage (flash) | 0 MB (uses SD) | 5 MB (59 s mono) |
| Unallocated flash | **9.94 MB** | **5.81 MB** |

---

## 3. PSRAM (32 MB)

Configured in [`sdkconfig.defaults`](../sdkconfig.defaults) lines 1477–1503.
Runs at **200 MHz** in octal (HEX) mode. DMA-capable via both AHB and AXI GDMA.

### 3.1 Budget Breakdown

| Allocation | Size | How | Source |
|------------|------|-----|--------|
| **Sample ROM buffer** | **28 MB** | Single `heap_caps_malloc(28 MB, MALLOC_CAP_SPIRAM)` | [`ctagSampleRom.cpp`](../components/ctagSoundProcessor/helpers/ctagSampleRom.cpp#L42) |
| Safety margin / dynamic | ~4 MB | Remaining PSRAM heap | `MAX_ALLOC_BYTES_SAFETY_MARGIN` |
| **Total** | **32 MB** | | |

The 28 MB figure comes from:
```c
#define TOTAL_SIZE_PSRAM_BYTES    32*1024*1024          // 32 MB hardware
#define MAX_ALLOC_BYTES_SAFETY_MARGIN  4*1024*1024      // 4 MB reserved
#define MAX_ALLOC_BYTES_PSRAM  (TOTAL_SIZE_PSRAM_BYTES - MAX_ALLOC_BYTES_SAFETY_MARGIN)  // 28 MB
```

Also set in `sdkconfig.defaults` line 784:
`CONFIG_MAX_ALLOC_BYTES_PSRAM_SAMPLE_DATA=29360128` (28 MB).

### 3.2 Sample ROM Buffer (28 MB) — What's Inside

All sample data from the active wavetable bank + active sample bank is loaded from
SD card WAV files into this contiguous PSRAM buffer at boot (or on bank switch).

| Content | Capacity | Format |
|---------|----------|--------|
| Wavetable banks | Up to 1 MB (32 banks × 64 WT × 256 samples × 2 bytes) | 16-bit integer, 44.1 kHz |
| Sample slices | Remaining ~27 MB | 16-bit integer, 44.1 kHz, mono or stereo (L then R) |
| Max slice count | 2048 WT slots + 256 sample slots = **2304 total** | `MAX_SLICES_SAMPLES=256` |

**Audio duration at 44.1 kHz / 16-bit:**

| Mode | 28 MB Budget | Per 1 MB |
|------|-------------|----------|
| Mono | **~332 seconds** (5 min 32 s) | ~11.9 s |
| Stereo | **~166 seconds** (2 min 46 s) | ~5.9 s |

### 3.3 Dynamic Allocations (~4 MB Safety Margin)

These come from the remaining PSRAM heap after the 28 MB sample buffer:

| Consumer | Typical Size | Notes |
|----------|-------------|-------|
| DSP delay lines | 88,200 × 4 = **345 KB** each | MonoDelay, DrumRack, PicoSeqRack — up to 2 per plugin |
| Reverb buffers | 32,768 × 4 = **128 KB** each | MIVerb, SpaceFX, GVerb, etc. |
| Chorus/tape delay | Variable | EChorus, CocoaDelay, TDelay — depends on sample rate × length |
| RapidJSON allocator | Variable | All JSON parsing targets SPIRAM |
| HTTP request bodies | `req->content_len` | Sample uploads, preset saves |
| SD DMA buffer | 16 KB | `MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM` |
| ZIP decompression | 2 × 64 KB = 128 KB | SD card archive extraction |
| FATFS metadata | Variable | `CONFIG_FATFS_ALLOC_PREFER_EXTRAM=y` |
| FreeRTOS task stacks | Variable | `CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY=y` |
| Wavetable rack buffers | ~34 KB | RackWTOsc: 260×64×2 + 512×4 bytes |

A plugin using two large delay lines + reverb consumes ~820 KB from this pool.
With ~4 MB available, several effect-heavy plugins can coexist.

---

## 4. Internal SRAM (~768 KB)

ESP32-P4 high-performance on-die SRAM. Used for latency-critical and DMA-requiring
allocations. All `DRAM_ATTR` and `MALLOC_CAP_INTERNAL` allocations land here.

### 4.1 Major Consumers

| Consumer | Size | Location | Notes |
|----------|------|----------|-------|
| **Plugin DSP workspace** | **300 KB** | [`ctagSPAllocator.cpp`](../components/ctagSoundProcessor/ctagSPAllocator.cpp) | Static `custom_pool[300000]` — THE critical DSP arena |
| ↳ CH0 half | 150 KB | | First plugin channel |
| ↳ CH1 half | 150 KB | | Second plugin channel (or STEREO = full 300 KB) |
| Audio task stack | 20 KB | `SPManager.cpp` | Highest FreeRTOS priority, pinned to core 1 |
| Main task stack | 8 KB | `sdkconfig.defaults` | App startup, HTTP server |
| LED task stack | 4 KB | `SPManager.cpp` | RGB LED driver, core 0 |
| Debug task stack | 2 KB | `SPManager.cpp` | Diagnostics |
| SPI API task stack | 8 KB | `SpiAPI.cpp` | RP2350 communication |
| Codec DMA buffer | 256 B | `codec_bba.cpp` | `DRAM_ATTR int32_t tmp_buffer[64]` |
| I2S DMA descriptors | ~1 KB | `codec_bba.cpp` | 4 desc × 32 frames |
| SPI stream buffer | 512 B | `rp2350_spi_stream.hpp` | Triple-buffered MIDI/control data |
| freeverb static arrays | ~20 KB | `revmodel.cc` | `DRAM_ATTR` comb/allpass filter buffers |
| FreeRTOS kernel | ~30–50 KB | ESP-IDF | Scheduler, tick, ISR stacks |
| Network stack | ~15–20 KB | lwIP + mDNS + HTTP | `CONFIG_LWIP_TCPIP_TASK_STACK_SIZE=3072` etc. |
| WiFi driver internals | ~30–50 KB | ESP-IDF | Buffers, state machines |
| NVS cache | ~5 KB | ESP-IDF | Flash page cache |
| **Estimated total used** | **~500–550 KB** | | |
| **Estimated free** | **~220–270 KB** | | General-purpose internal heap |

### 4.2 Audio Processing Path (Internal SRAM, Real-Time)

The audio hot path runs entirely from internal SRAM for minimum latency:

```
I2S RX DMA → tmp_buffer[64] (DRAM) → float conversion
  → custom_pool[] (300 KB plugin workspace, DRAM)
  → float output → tmp_buffer[64] → I2S TX DMA
```

- **Buffer size:** 32 stereo samples per block (0.73 ms at 44.1 kHz)
- **Bit depth:** 32-bit float internal processing; 32-bit I2S bus to codec
- Stack-local working buffers: `float finput[64], fbuf[64], finput2[64], fbuf2[64]` (~1 KB on audio task stack)

---

## 5. SD Card Storage

Mounted at `/sdcard/` via FATFS (4096-byte sector size, long filename support).
**No hard capacity limit** — determined by the micro SD card inserted.

### 5.1 Directory Layout

```
/sdcard/
├── data/                          ~1.2 MB total
│   ├── spm-config.json            Plugin manager cache, WiFi, output levels
│   ├── synthdefinitions.json      Synth preset definitions
│   ├── trackdefaults.json         Default track configurations
│   ├── favs.json                  User favorites
│   ├── webui-version.json         Installed WebUI version
│   ├── macrodefinitions/          Macro device configs (JSON)
│   ├── macrosoundpresets/         40+ macro sound preset files
│   └── sp/                        112 files: 56 plugins × (mp-*.json + mui-*.json)
├── www/                           ~2.7 MB total (gzipped)
│   ├── index.html.gz              Main WebUI
│   ├── webui-update.html.gz       Update interface
│   ├── preset-macro-manager.html  Preset manager
│   ├── js/, css/, img/            Assets
│   └── shoelace/                  UI component library
└── tbdsamples/                    ~30 MB factory content
    ├── sample_rom.json            Master index (active banks, names, tags)
    ├── drums/factory/             85+ drum WAV files
    ├── drums/loops/               Loop samples
    ├── a4_dub/                    112 dub samples
    ├── wavetables/                300+ wavetable WAV files
    └── [user banks]/              User-uploaded samples via WebUI
```

**Factory SD image total:** ~3.9 MB (compressed) + ~30 MB sample content.

### 5.2 Sample Bank System

Samples are organized as **banks** managed by `/sdcard/tbdsamples/sample_rom.json`:

- **Wavetable banks** and **sample banks** are independent
- Each bank has a JSON descriptor referencing WAV files by path
- Active bank loaded into PSRAM at boot or on bank switch
- Bank switching calls `ctagSampleRom::RefreshDataStructure()` — reloads all WAV data into the 28 MB PSRAM buffer

Users upload new samples via the WebUI (`POST /api/v2/samples?action=upload`),
which writes WAV files to `/sdcard/tbdsamples/{path}/` and updates bank descriptors.

---

## 6. RP2350B Co-Processor

| Resource | Size | Usage |
|----------|------|-------|
| Internal SRAM | 520 KB | Sequencer state (16 tracks), OLED framebuffer (128×64 px), button/pot/LED state, MIDI processing buffers |
| Flash | ~4 MB | RP2350 firmware (updated via BOOTSEL USB-C port #2) |

Communicates with ESP32-P4 via SPI2 slave interface (GPIO51 handshake).
Stream buffer: 512 bytes with triple buffering for MIDI + control data.

**Managed hardware:** 30 buttons, 4 endless pots, 19 RGB LEDs, 2.4" OLED,
2× MIDI TRS In, 2× MIDI TRS Out, USB Host MIDI, own micro SD slot.

---

## 7. Capacity Answers

### How much space for sample kits?

| Storage | Budget | Duration (44.1 kHz / 16-bit mono) | Duration (stereo) |
|---------|--------|-----------------------------------|-------------------|
| **PSRAM** (active at runtime) | 28 MB | **~332 s** (5 min 32 s) | ~166 s |
| **SD card** (on disk) | Card capacity | Unlimited | Unlimited |
| **Flash** sample_rom (Config A/B only) | 5 MB | ~59 s | ~30 s |

Only one bank set (WT + samples) is active in PSRAM at a time.
Bank switching reloads from SD — takes a few seconds depending on data size.

### How much space for firmware?

| Config | Partition | Budget | Current Binary | Headroom |
|--------|-----------|--------|----------------|----------|
| D (TBD-16) | ota_0 | 5 MB | 3.2 MB | **1.8 MB** |
| A/B (no SD) | ota_0 | 3.13 MB | 3.2 MB | **~0 MB** ⚠️ |

> ⚠️ **Config A/B firmware is at capacity.** The current 3.2 MB binary barely fits
> the 3.13 MB partition. Adding more plugins or features will require either
> enlarging `ota_0` (reducing `storage` or `sample_rom`) or stripping unused plugins.

### What's the unused flash?

| Config | Unallocated | Address Range | Could Be Used For |
|--------|-------------|---------------|-------------------|
| D | **9.94 MB** | 0x610000–0xFFFFFF | Larger ota_1 (enable real OTA), flash sample ROM, extra data partitions |
| A/B | **5.81 MB** | 0xA30000–0xFFFFFF | Larger sample_rom, larger storage, ota_1 for OTA |

### Plugin DSP memory budget?

Each of the two channels gets **150 KB** of internal SRAM from the static plugin pool
(`CONFIG_SP_FIXED_MEM_ALLOC_SZ=300000`). A stereo plugin gets the full 300 KB.
This is the working memory for all real-time DSP state (oscillators, filters,
envelopes, LFOs, sequencer state). Plugins needing more (delay lines, reverbs)
allocate additional buffers from PSRAM via `heap_caps_malloc(MALLOC_CAP_SPIRAM)`.

---

## 8. Memory Map Diagram

```
                    ESP32-P4 Memory Map (Config D)
                    ══════════════════════════════

 ┌─────────────────────────────────────────────────────┐
 │                INTERNAL SRAM (~768 KB)               │
 │  ┌───────────────────────────────────────────────┐  │
 │  │ Plugin DSP Pool        300 KB  (DRAM, static) │  │
 │  │ FreeRTOS kernel+ISR     50 KB  (est.)         │  │
 │  │ Task stacks (audio)     20 KB                 │  │
 │  │ Task stacks (other)     26 KB                 │  │
 │  │ Network / WiFi          50 KB  (est.)         │  │
 │  │ Codec DMA + I2S          2 KB                 │  │
 │  │ freeverb static         20 KB                 │  │
 │  │ Free heap             ~250 KB  (est.)         │  │
 │  └───────────────────────────────────────────────┘  │
 └─────────────────────────────────────────────────────┘

 ┌─────────────────────────────────────────────────────┐
 │                  PSRAM (32 MB)                       │
 │  ┌───────────────────────────────────────────────┐  │
 │  │ Sample ROM buffer      28 MB   (one malloc)   │  │
 │  │  ├─ Wavetables          1 MB   (32 banks)     │  │
 │  │  └─ Sample slices      27 MB   (up to 256)    │  │
 │  ├───────────────────────────────────────────────┤  │
 │  │ Dynamic allocations    ~4 MB                  │  │
 │  │  ├─ Delay lines       ~700 KB  (per plugin)   │  │
 │  │  ├─ Reverb buffers    ~128 KB  (per plugin)   │  │
 │  │  ├─ JSON / HTTP       variable                │  │
 │  │  ├─ SD DMA              16 KB                 │  │
 │  │  ├─ ZIP decompress     128 KB                 │  │
 │  │  └─ FATFS + misc      variable                │  │
 │  └───────────────────────────────────────────────┘  │
 └─────────────────────────────────────────────────────┘

 ┌─────────────────────────────────────────────────────┐
 │               FLASH (16 MB) — Config D               │
 │                                                      │
 │   0x000000 ┌──────────┐                              │
 │            │ Reserved │  8 KB                        │
 │   0x002000 ├──────────┤                              │
 │            │ Bootldr  │  22 KB actual                │
 │   0x008000 ├──────────┤                              │
 │            │ PartTbl  │  4 KB                        │
 │   0x009000 ├──────────┤                              │
 │            │ NVS      │  16 KB                       │
 │   0x00D000 ├──────────┤                              │
 │            │ OTA Data │  8 KB                        │
 │   0x00F000 ├──────────┤                              │
 │            │ PHY Init │  4 KB                        │
 │   0x010000 ├──────────┤                              │
 │            │          │                              │
 │            │  ota_0   │  5 MB  (3.2 MB used)         │
 │            │          │                              │
 │   0x510000 ├──────────┤                              │
 │            │  ota_1   │  1 MB  (MSC helper)          │
 │   0x610000 ├──────────┤                              │
 │            │          │                              │
 │            │  FREE    │  9.94 MB  ★ unallocated      │
 │            │          │                              │
 │   0xFFFFFF └──────────┘                              │
 └─────────────────────────────────────────────────────┘

 ┌─────────────────────────────────────────────────────┐
 │               SD CARD (user-supplied)                │
 │  /sdcard/data/         ~1.2 MB   Config + presets   │
 │  /sdcard/www/          ~2.7 MB   WebUI (gzipped)    │
 │  /sdcard/tbdsamples/   ~30 MB    Factory samples    │
 │  (+ user uploads)      no limit                     │
 └─────────────────────────────────────────────────────┘

 ┌─────────────────────────────────────────────────────┐
 │               RP2350B (independent)                  │
 │  520 KB SRAM  │  ~4 MB Flash  │  SPI2 link to P4   │
 │  Sequencer · OLED · Buttons · MIDI · LEDs           │
 └─────────────────────────────────────────────────────┘
```

---

## 9. Key Source Files

| File | What It Tells You |
|------|-------------------|
| [`partitions_example.csv`](../partitions_example.csv) | Config D/C flash partition table |
| [`partitions_no_sd.csv`](../partitions_no_sd.csv) | Config A/B flash partition table |
| [`sdkconfig.defaults`](../sdkconfig.defaults) L780–795 | Sample ROM size, plugin pool size |
| [`sdkconfig.defaults`](../sdkconfig.defaults) L1477–1503 | PSRAM speed, mode, boot init |
| [`ctagSampleRom.cpp`](../components/ctagSoundProcessor/helpers/ctagSampleRom.cpp) | 28 MB PSRAM allocation + sample loading |
| [`ctagSPAllocator.cpp`](../components/ctagSoundProcessor/ctagSPAllocator.cpp) | 300 KB plugin DSP pool (internal SRAM) |
| [`codec_bba.cpp`](../components/drivers/codec_bba.cpp) | I2S DMA config, codec buffer sizes |
| [`SPManager.cpp`](../main/SPManager.cpp) | Audio task (20 KB stack), plugin lifecycle |
| [`sample_rom/readme.md`](../sample_rom/readme.md) | Sample ROM binary format + flash addresses |
| [`HARDWARE_CONFIGURATIONS.md`](../HARDWARE_CONFIGURATIONS.md) | Config matrix (A/B/C/D) |
| [`docs/hardware/10_tbd16.rst`](hardware/10_tbd16.rst) | Hardware specs (processors, I/O, audio) |
