# CTAG TBD Version Log

## V1.0.0
- Switched to ESP-IDF 5.1.1
- Swapped flash file system to LittleFS from SPIFFS
- Much improved stability of flash file system
- Fixed many bugs with backup / restore mechanism

## V0.9.21a
- ADC input channels phase issue fixed (thanks to Synthpatti for reporting)
- ADC input highpass enabled

## V0.9.21
- [VCV Rack](https://vcvrack.com/) v1 plugins available "tbd4vcv" to test and enjoy TBDs growing set of sound processors, details [here](tbd4vcv/readme.md)
- New plugin RecNPlay by M. Brüssel --> [https://soundcloud.com/taitekatto/recnplay-for-tbd](https://soundcloud.com/taitekatto/recnplay-for-tbd)
- Sample import web UI now indicates mono vs. stereo samples
- When overwriting patches, old patch name is shown in dialog and can be edited prior to overwrite
- Favorites are now contained in backup, restoring backup OVERWRITES!!! current data
- Favorite ex-/import allows exchanging patches among users, tbd4vcv and simulator, import OVERWRITES EXISTING PATCHES AND FAVORITE!!!
- Stability improvements with regard to flash storage reliability at boot up
- Minor bug fixes

## V0.9.20
- WTOsc extended with pitch quantizer, smoothed wavetable selection by CV --> significant improvement
- TBD03 slide and slide level parameter added
- Current firmware version shown in web ui configuration menu
- Calibration backup / restore dialog page

## V0.9.19a
- New plugin Freakwaves by M. Brüssel --> complex wavetable synth
- Web GUI now transmits static data in gzip compressed fashion resulting in much faster GUI loading
- Favorites menu allows to store/recall plugin + preset combinations --> convenient way to restore module setting
- Github action scripts to support [Cloud Compiler](https://fxwiegand.github.io/tbd-cloud-compiler/) which allows one to only include the best plugins e.g. to obtain more sample rom --> cloud compiled firmwares can only be flashed through serial USB connection *NOT* WIFI
- Modifications to support more upcoming hardware versions
- Fix for severe memory leak bug related to JSON data objects --> critical

## V0.9.18
- New plugin Talkbox by M. Brüssel --> Awesome Vocoder Plugin based on [mdaTalkbox](https://sourceforge.net/projects/mda-vst)
- New plugin MSxxNoise by M. Brüssel --> Models MS20 filter and [Stone Phaser](https://github.com/jpcima/stone-phaser)
- Config option to daisy chain CH0 out -> CH1 in, that is route CH0 output to CH1 input in software
- Minor bug fixes / architecture improvements

## V0.9.17
- New plugin Antique --> Generates old record / tape effect
- New plugin Subbotnik by M. Brüssel --> Huge modulations
- New plugin VctrSyn by M. Brüssel --> Complex vector synth monster
- Minor bug fixes / architecture improvements

## V0.9.16
- New plugin SpaceFX by M. Brüssel (uses [Vult Technology](https://github.com/modlfo))
- Bjorklund parameter reduction (some setting caused CPU overload)
- Numeric plugin parameters can now be reset to 0 by double clicking parameter name
- FBDlyLine plugin bug fix, dry/wet slider now working
- Limitation of patch storage to 10 patches per plugin (this is due to the limited storage resources on the platform), fixes out of mem bug, when too many patches were created
- Patch write button changes color to red to indicate overwrite of existing patch
- Web UI has now modal screen during plugin change and preset load/save
- Minor optimizations

## V0.9.15
- New plugin Retroactor by M. Brüssel (uses [Vult Technology](https://github.com/modlfo))
- Bugfixes Bjorklund by M. Brüssel

## V0.9.14
- New plugin Formantor by M. Brüssel (uses [Vult Technology](https://github.com/modlfo))
- New plugin Bjorklund by M. Brüssel (uses [Vult Technology](https://github.com/modlfo))
- Added dummy plugin Void --> No processing overhead for a single channel

## V0.9.13
- New plugin APCpp by M. Brüssel
- CDelay noise reduction / different filter model
- Rompler pitch, start position latch if slice latch on trig
- Rompler bit reduction
- Claude more grains, removed mono in as similar functionality already with mode 1 & 3 
- Added developer docs for windows (in doc/ folder)
- Added continuous integration scripts
- Added configuration option for different HW platforms including [Strämpler](https://github.com/ctag-fh-kiel/ctag-straempler)
- Bug fixes

## V0.9.12a
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
