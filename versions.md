# CTAG TBD Version Log

## V0.9.12
- New plugin Karpuskl by M. Brüssel
- Sample rom can now be flashed without need for reboot
- Bug fixes

## V0.9.11a
- BIG FRESH 2021 UPDATE, TBD CAN DO SAMPLE PLAYBACK NOW, [details here](sample_rom/readme.md)
- When you update, check the new "edit sample rom" option and upload custom or stock samples (sample-rom.tbd file in release)
- New plugin Rompler (sample playback device)
- New plugin WTOsc (wavetable oscillator able to use user wavetables from WaveEdit)
- Bug fix Claude
- Extension Claude mono in option
- Rompler memory fixes

## V0.9.10
- New configuration option for output volume
- Bug fix: Modular output levels were too low

## V0.9.9a
   - New plugin Claude --> port of mutable instruments Clouds including parasites reverse option
   - Some minor optimizations

## V0.9.8
   - New plugin BBeats by M. Brüssel (credits also to M. Wand for beats) --> Byte Beat Generator

## V0.9.7a
   - Mono + Stereo CStrip effect plugin, based on airwindows design --> Channel Strip
   - Stereo Every Trim effect plugin, based on airwindows design --> Every Trim 
   - Minor bug fixes

## V0.9.6
   - Mono tape delay effect plugin, based on airwindows design --> Tape Delay

## V0.9.5
   - New ensemble / chorus plugin, based on airwindows design --> EChorus
   - Optimizations

## V0.9.4a
   - New reverb plugin Progenitor Reverb (from freesound3), essentially Griesinger Lexicon 224 topology --> with love for all ambient musicians :)
   - Reverb time of Plate Reverb can now be controlled with CV
   - Flash write issue workaround, i.e. when TBD suffers a power loss during a flash write e.g. during storing of preset data, files can get corrupted; this could only be fixed with manually re-flashing the module; now, TBD tries to recover files from a backup directory in the default firmware. Note that any settings made by the user stored in that file will lost. You should always make sure, the power supply is stable and power is only disconnected when no flash writes are in progress (LED NOT BLUE!).
   - Several smaller bug fixes and optimizations

## V0.9.3
   - Significant refactoring reducing code bloat in sound processors, improved compilation speed of sound processors
   - Improved sound processor template generation
   - Minor bug fixes
      - Some plugins previously did not work correctly with negative CV mapped to pitch
   - Otherwise no new plugins
   
## V0.9.2b
   - Bug fix, calibration

## V0.9.2
- Added CDelay delay effect (adapted and modified version of Cocoa Delay by tesselode)
- Added option of soft clipping for outputs
- Optimized Moog filters
- Changed IDF settings for 
    - Flash speed (OTA updates now faster)
    - SPIRAM speed
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