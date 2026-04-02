# Session Notes — 2026-04-02 Memory Audit

Research notes captured during the TBD-16 memory audit session.
Complements `docs/MEMORY_AUDIT.md` with implementation-level details.

---

## PSRAM Allocation Sites (50+ call sites)

Every `heap_caps_malloc(..., MALLOC_CAP_SPIRAM)` in the codebase:

### Sample ROM (dominant consumer)
- `ctagSampleRom.cpp:264` — **28 MB** contiguous block for all sample data
- `ctagSampleRom.cpp:249,251` — slice offset/size arrays (`maxSlices × 4 bytes` each, up to 2304 slots)

### Per-Plugin Effect Buffers (allocated on plugin load, freed on unload)
| Plugin | File | Buffer | Size |
|--------|------|--------|------|
| MonoDelay | `ctagSoundProcessorMonoDelay.cpp:151` | Delay line | 88,200 × 4 = 345 KB |
| PicoSeqRack | `ctagSoundProcessorPicoSeqRack.cpp:904,909` | 2× delay lines | 2 × 345 KB = 690 KB |
| PicoSeqRack | `ctagSoundProcessorPicoSeqRack.cpp:915` | Reverb | 32,768 × 4 = 128 KB |
| DrumRack | `ctagSoundProcessorDrumRack.cpp:1007,1010` | 2× delay lines | 2 × 345 KB = 690 KB |
| MIVerb | `ctagSoundProcessorMIVerb.cpp:36-37` | Reverb | 128 KB |
| MIVerb2 | `ctagSoundProcessorMIVerb2.cpp:36-37` | Reverb | 128 KB |
| TBDings | `ctagSoundProcessorTBDings.cpp:42` | Reverb (16-bit) | 32,768 × 2 = 64 KB |
| SpaceFX | `ctagSoundProcessorSpaceFX.cpp:512` | Reverb | 128 KB |
| CocoaDelay | `tesselode/CocoaDelay.cpp:145,151` | 2× tape buffers | sampleRate × tapeLength × 4 each |
| StrampDly | `ctagSoundProcessorStrampDly.cpp:43,47` | 2× stereo delay | bufLen × 4 each |
| EChorus | `airwindows/EChorus.cpp:143-144` | 2× chorus delay | totalsamples × 4 each |
| TDelay | `airwindows/TDelay.cpp:209` | Tape delay | length × 4 |
| GVerb | `gverb/gverbdsp.c:103` | Reverb (calloc) | size × 4 |
| RackWTOsc | `rack/RackWTOsc.cpp:34-35` | WT + float buffer | ~34 KB |
| FBDelayLine | `helpers/ctagFBDelayLine.cpp:50` | Feedback delay | maxLength × 4 |
| freeverb3 | `freeverb3/utils.cpp:252` | Prefers internal, falls back to SPIRAM | variable |

### Infrastructure Buffers
| Component | File | Size |
|-----------|------|------|
| SD DMA | `custom_sdmmc_cmd.cpp:41` | 16 KB (DMA+SPIRAM+32BIT) |
| File copy | `fs.cpp:151` | BUFSIZ |
| ZIP search | `fs.cpp:239` | variable |
| ZIP central dir | `fs.cpp:282` | cd_size |
| ZIP decompress | `fs.cpp:292-293` | 2 × 64 KB (DMA-capable) |
| RapidJSON | `rapidjson/allocators.h:86,103` | All JSON → SPIRAM |
| Data models | `ctagDataModelBase.cpp:106` | MB_BUF_SZ |
| Synth defs | `SynthDefinitionDataModel.cpp:35-36` | MAX_SYNTHS + MAX_TRACKS arrays |
| Macro defs | `MacroDeviceDefinitionDataModel.cpp:36` | MaxMacroDeviceDefinitions |
| Sound presets | `MacroSoundPresetDataModel.cpp:40-41` | MaxSoundPresets + groups |
| Device API | `DeviceAPI.cpp:169,201` | req->content_len |
| Sample API | `SampleAPI.cpp:103+` | Multiple: req bodies + 4 KB chunks |
| Macro API | `MacroAPI.cpp:82+` | 8 KB + request buffers |
| Plugin API | `PluginAPI.cpp:296-297` | req->content_len |

---

## Config A/B Firmware Size Problem

The `partitions_no_sd.csv` allocates only **3.125 MB** (0x320000) for `ota_0`.
The current build produces a **3.2 MB** binary. This means Config A/B builds
may fail to flash or will require stripping plugins to fit.

**Options to fix:**
1. Shrink `storage` from 2 MB to 1.5 MB, give 0.5 MB to `ota_0`
2. Shrink `sample_rom` from 5 MB to 4.5 MB
3. Strip unused plugins from Config A/B builds via CMake conditional compilation
4. Use `-Os` optimization (if not already) for Config A/B

---

## Unused Flash Expansion Opportunities (Config D)

9.94 MB of flash (0x610000–0xFFFFFF) is completely unallocated in Config D.
Potential uses:

1. **Enlarge ota_1 to 5 MB** — enables true over-the-air firmware updates
   (requires changing `partitions_example.csv` and updating the web flasher's
   hardcoded `ota1Addr = 0x510000`)
2. **Add flash-based sample_rom** — like Config A/B's 5 MB partition, for
   samples that survive SD card removal
3. **Add a SPIFFS/LittleFS partition** — for factory-reset presets that
   survive SD wipe
4. **Larger ota_0** — future-proof against firmware growth beyond 5 MB

---

## Memory Monitoring at Runtime

The codebase logs both internal and SPIRAM free heap at multiple points.
To check live memory usage, look for these log tags in serial output:

```
ESP_LOGI("SR", "Max Bytes free in PSRAM: %li", maxSizeBytes);
ESP_LOGI("HEAP", "Internal free: %d, largest: %d, SPIRAM free: %d, largest: %d", ...);
```

Key monitoring call sites:
- `SPManager.cpp:500-792` — multiple logging points during plugin load
- `MacroSPManager.cpp:41-210` — macro plugin lifecycle
- `PluginAPI.cpp:339-361` — REST API reports memory after plugin changes
- `SynthDefinitionDataModel.cpp:100-103` — after synth def allocation
- `MacroSoundPresetDataModel.cpp:60-169` — after preset allocation

---

## SD Card Sample System Architecture

The sample system uses a **two-level bank architecture**:

1. **Master index:** `/sdcard/tbdsamples/sample_rom.json`
   - Lists all available WT banks and sample banks by name
   - Stores `active_wt_bank` and `active_smp_bank` indices
   - Bank names and tags for UI display

2. **Bank descriptors:** e.g., `def_smp.json`, `a4_dub.json`
   - JSON arrays of `{filename, path, nsamples, offset, sname}`
   - Reference WAV files by relative path under `tbdsamples/`

3. **WAV files:** Standard PCM format, any sample rate/bit depth
   - WebUI resamples to 44.1 kHz / 16-bit before upload
   - Stored in organized directories (drums/factory, wavetables, etc.)

**Loading flow:**
`sample_rom.json` → active bank descriptor → file list → `open()/read()` each WAV
→ copy PCM data into contiguous 28 MB PSRAM buffer → build offset/size index

**Bank switching:** `ctagSampleRom::SetActiveSampleBank(index)` updates the
active index in `sample_rom.json`, then `RefreshDataStructure()` re-reads all
files from SD into PSRAM. Takes a few seconds depending on total data size.

**Factory content:** 427 WAV files totaling ~30 MB on disk.
