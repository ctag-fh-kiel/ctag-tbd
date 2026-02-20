---
blogpost: true
date: 2026-01-15
author: dadamachines
category: Development
tags: plugin, dsp, tutorial
---

# Writing Your First TBD Plugin

One of the best things about TBD is that it is fully open-source — and that
includes the plugin system. In this post we walk through creating a simple
sine oscillator plugin from scratch.

## The plugin architecture

Every TBD plugin is a C++ class that inherits from `ctagSoundProcessor`.
The framework handles audio I/O, parameter management, and the web UI — you
just implement the DSP.

The key method to override is `Process()`, which is called once per audio
block with input/output buffer pointers.

## Scaffold a new plugin

Use the generator script to create the boilerplate:

```bash
cd generators
node generator.js --name MySineOsc
```

This creates:
- `ctagSoundProcessorMySineOsc.cpp` — your DSP code
- `ctagSoundProcessorMySineOsc.hpp` — class declaration
- `mui-MySineOsc.jsn` — web UI layout definition

## Implement the oscillator

The core DSP is surprisingly simple:

```cpp
void ctagSoundProcessorMySineOsc::Process(
    const ProcessData &data
) {
    float freq = params->GetFloat("frequency");
    float amp = params->GetFloat("amplitude");

    for (int i = 0; i < data.buf_size; i++) {
        phase += freq / 44100.f;
        if (phase >= 1.f) phase -= 1.f;
        data.buf[i * 2] = data.buf[i * 2 + 1] =
            amp * sinf(phase * 2.f * M_PI);
    }
}
```

## Next steps

Check out the [DSP Development](../../dsp/index) section for the full API
reference and more advanced examples.

Happy coding!
