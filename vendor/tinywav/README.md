# TinyWav

A minimal C library for reading and writing (32-bit float) WAV audio files.

## Code Example
### Writing
```C
#include "tinywav.h"

#define NUM_CHANNELS 1
#define SAMPLE_RATE 48000

tinywav tw;
tinywav_open_write(&tw,
    NUM_CHANNELS,
    SAMPLE_RATE,
    TW_FLOAT32, // the output samples will be 32-bit floats. TW_INT16 is also supported
    TW_INLINE,  // the samples will be presented inlined in a single buffer.
                // Other options include TW_INTERLEAVED and TW_SPLIT
    "path/to/output.wav" // the output path
);

for (int i = 0; i < 100; i++) {
  float samples[480]; // samples are always presented in float32 format
  tinywav_write_f(&tw, samples, sizeof(samples));
}

tinywav_close_write(&tw);
```

### Reading
```C
#include "tinywav.h"

#define NUM_CHANNELS 1
#define SAMPLE_RATE 48000
#define BLOCK_SIZE 480

TinyWav tw;
tinywav_open_read(&tw, "path/to/input.wav", TW_SPLIT, TW_FLOAT32);

for (int i = 0; i < 100; i++) {
  // samples are always presented in float32 format
  float samples[NUM_CHANNELS][BLOCK_SIZE];

  tinywav_read_f(&tw, samples, BLOCK_SIZE);
}

tinywav_close_read(&tw);
```

## License
TinyWav is published under the [ISC license](http://opensource.org/licenses/ISC). Please see the `LICENSE` file included in this repository, also reproduced below. In short, you are welcome to use this code for any purpose, including commercial and closed-source use.

```
Copyright (c) 2015-2017, Martin Roth <mhroth@gmail.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```
