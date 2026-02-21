**************
Rompler (Mono)
**************


Description
~~~~~~~~~~~

A mono sample playback device with comprehensive functionality,
which allows to playback sample sections from sample rom. Stereo
playback can be achieved by allocating one MRompler instance to each
channel. When a stereo sample is in sample rom, one can assign either
the left or the right slice to one instance and trigger playback at the
same time.

Features of MRompler include:

- User samples

- Sample selection (CV)

- Variable playback speed +/-200% (EG/LFO/CV)

- Pitch / pitch modulation (EG/LFO/CV)

- Play modes single-shot, loop, ping-pong, latch

- Loop markers within selected samples

- ADSR EG

- Sine wave LFO

- Amplitude modulated with EG/LFO/CV

- SVF filter modulated with EG/LFO/CV

- Mono / duophonic voice allocation

Parameters:

-  Gain: Sets overall loudness

-  Gate: Gate signal

-  Latch: If latch is enabled gate signal is switched upon
      change

-  Bank: Bank selection, each bank contains 32 sample slices

-  Slice: Selection of 32 slices, slices are stored sequentially
      in sample rom

-  Slice Lock: If enabled slice and slice pitch is locked each
      time note is triggered

-  Skip Wavetables: Skips sample rom area containing wavetables,
      affects bank and slice selection

-  Speed: Playback speed -200% … 200%

-  Pitch: Pitch in halftone steps -48 … 48

-  Tune: Tuning +/- one octave

-  Start: Start sample position (relative to slice length) of
      slice playback

-  Length: Playback length (relative to slice length)

-  Duo: Monophonic or duophonic mode

-  Loop: Loops sample between loop position end end position

-  Ping pong: Sample loop ping pong mode

-  Loop Position: Sample loop position relative to length
      parameter

-  Filter mode: Disable, low pass, band pass, high pass

-  Filter Cutoff

-  Filter Resonance

-  LFO AM: Lfo modulation level on amplitude

-  LFO FM: Lfo modulation level on pitch

-  LFO Filter FM: Lfo modulation level on filter cutoff

-  EG AM: Envelope modulation level on amplitude

-  EG FM: Envelope modulation level on pitch

-  EG Filter FM: Envelope modulation level on filter cutoff

-  LFO Speed

-  EG Speed: Slow / Fast

-  EG ADSR

-  EG Stop: Stops sample playback, when ADSR is idle (beyond
      release phase)
