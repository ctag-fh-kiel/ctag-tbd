# plaits
- Set dsp/dsp.h sample rates and buffer sizes
- More initializations
# rings
- Set dsp/dsp.h sample rates and buffer sizes
- Added a few const in part.cc, part.h
- More initializations
# clouds
- Sample rate set to 44100Hz
- Initializations
- Max grains adapted to match ESP32 processing power
- Buffer allocations, changed to match available memory on ESP32
- Merge with parasites reverse mode
- Spectral mode not working, would require own FFT due to performance
# general
- stmlib remove TEST ifdef make default standard c api, remove STM special op codes
- stmlib DelayLine with heap buffer