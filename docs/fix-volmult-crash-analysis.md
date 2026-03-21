# Fix Analysis: P4 Crash When Saving Volume Multiplier

## Problem

The ESP32-P4 sometimes crashes when saving a new Volume Multiplier value
from the WebUI (designer page → save definition → firmware reload).

## Root Cause: Stack Overflow on HTTP Task

The WebUI save path triggers this call chain on the **HTTP server task** (8 KB stack):

```
POST /api/v2/macros?action=reload
  → handle_reload()                          [MacroAPI.cpp]
    → DisablePluginProcessing()              [SPManager.cpp – currently no-op]
    → RefreshMacros()                        [SPManager.cpp]
      → xSemaphoreTake(processMutex)
      → ReloadSynthDefinitions()             [SynthDefinitionDataModel.cpp]
        → Document d                         (stack: ~80 B struct + 1 KB heap alloc)
        → loadJSON()                         (FileReadStream on stack, parses 30 KB file)
      → ReloadMachineDefinitions()           [MacroDeviceDefinitionDataModel.cpp]
        → Document d (UNUSED outer decl!)    (stack: ~80 B wasted)
        → while-loop over 34+ JSON files:
          → Document d (inner)               (stack: ~80 B struct + 1 KB heap alloc)
          → loadJSON()                       (FileReadStream on stack)
          → DeserializeJSON()
      → RefreshActiveDefinitions()           [MacroTranslator.cpp]
        → loops 16 tracks
        → LoadMacroDeviceDefinition() → copy()
      → xSemaphoreGive(processMutex)
    → EnablePluginProcessing()               [SPManager.cpp – currently no-op]
```

Each `Document` object, `FileReadStream`, `std::string` temporaries, and the
`loadJSON()` stack frame (which also uses `std::regex_replace` on parse
errors) accumulate on the 8 KB HTTP task stack.  This is borderline and crashes
intermittently depending on JSON file sizes, heap fragmentation, and compiler
optimisation choices.

## Failed Fix: Increasing HTTP Stack to 16 KB

**What was done:**  Changed `config.stack_size` from 8192 to 16384 in
`RestServer.cpp`.

**Why it broke audio:**  The ESP-IDF HTTP server allocates its task stack from
**internal RAM** (`MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT`).  Internal RAM on
this P4 configuration is extremely constrained:

| Metric                         | 8 KB stack (audio OK) | 16 KB stack (audio broken) |
|--------------------------------|----------------------:|---------------------------:|
| Free internal after HTTP start |              40,860 B |                   32,620 B |
| **Largest contiguous block**   |          **31,744 B** |               **13,312 B** |
| Largest block after plugins    |          **29,696 B** |               **11,264 B** |

The extra 8 KB from internal RAM **fragmented the heap**, halving the largest
available contiguous block.  Audio DMA descriptors, I2S buffers, and synth
engine allocations that need contiguous internal memory either fail silently or
get placed in suboptimal regions, producing corrupted/broken audio output.

## Correct Fix: Offload RefreshMacros to a Temporary SPIRAM-Stack Task

Instead of giving the HTTP task more internal RAM, we keep the HTTP stack at
8 KB and spawn a **short-lived FreeRTOS task** for the heavy reload work:

1. `handle_reload()` creates a one-shot task with a **16 KB SPIRAM stack**
   (via `xTaskCreatePinnedToCoreWithCaps` + `MALLOC_CAP_SPIRAM`).
2. The task runs `RefreshMacros()` (which already holds `processMutex`
   internally, so audio is muted during the reload).
3. The HTTP handler waits on an `xEventGroupWaitBits()` until the task
   finishes, then returns `send_ok()`.  This keeps the HTTP response
   synchronous — the WebUI gets a 200 only after reload is complete.
4. The task deletes itself when done.

**Benefits:**
- HTTP task stack stays at 8 KB internal RAM → audio unaffected
- RefreshMacros gets 16 KB from SPIRAM → no stack overflow
- Behaviour is identical from the WebUI's perspective (synchronous response)
- SPIRAM is abundant (2.6+ MB free at runtime)

## Secondary Fixes (already applied)

1. **Removed unused `Document d` in `ReloadMachineDefinitions()`** — saved
   ~80 B of stack.
2. **Fixed `handle_set_track_macro()` bugs:**
   - Added null check after `heap_caps_malloc` (prevents crash on SPIRAM
     exhaustion)
   - Fixed memory leak on error path (was returning `ESP_FAIL` without
     freeing `content`)
   - Changed `free()` to `heap_caps_free()` to match allocator
