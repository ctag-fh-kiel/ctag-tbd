# CTAG TBD Version Log

## V0.9.2
- Added CDelay delay effect (adapted and modified version of Cocoa Delay by tesselode)
- Added option of soft clipping for outputs
- Optimized Moog filters
- Changed IDF settings for 
    - Memory allocations, std::vector can now allocate from PSRAM pool
    - Flash speed (OTA updates now faster)
    - PSRAM speed
- Fixed many compiler warnings
- Minor bug fixes and optimizations

## V0.9.1
- Added TBD03 emulation of TB303, essentially braids oscillator with various ladder filter models and some envelopes, filter model 1 is least computationally expensive and allows for TBD03 running on two channels
- Some minor bugs fixes and adaptations
- Updated IDF Release 4.1

## V0.9.0
- Added version log
- Added Poly Pad synth
- Several internal improvements such as ADC value constraints and better trig + ADC synchronisation
- Better ADSR envelope generator
- Bug fix in sine generator
- fast-math gcc flag added

## Pre V0.9.0
- No log present, see commit history