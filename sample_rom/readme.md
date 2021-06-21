# Information about sample rom
TBD allows to use a configurable amount of flash memory for storage of sample data. 
This sample data can be used for wavetable storage and / or playback of mono and stereo samples.
Sample data can be uploaded by users through a web-based UI or a serial connection.
The UI allows one to drop .wav files, pre-listen and order files to define the memory layout.
Once files are selected and ordered, users can download the data to TBD's flash memory or to the host PC to create backups or exchange sample packs with other users.
## Layout and structure of sample data
TBD's stock configuration reserves 5MiB of flash memory (highest 5MiB of overall 16MiB system flash) for sample rom.
This corresponds to approx. 59s of sample memory (TBD operates at 44.1kHz, data format in flash is 16 bit integer).
The first 1MiB of storage is dedicated to wavetable data (though it does not have to be). Wav files containing wavetable data exported 
from [WaveEdit online](https://waveeditonline.com/) or [WaveEdit](https://synthtech.com/waveedit) can be imported through the web UI. 
Wavetable .wav files must be 256 samples per wavetable times 64 wavetables int16 format. 
If an input file corresponds to wavetable data, the user must indicate so by activating a check box in the UI.
Generally .wav files can be of any browser supported format (i.e. 48kHz float), the browser UI uses the WebAudio API to 
resample and convert the data (44.1kHz 16 bit integer on the TBD).
Once all samples are in the right order, one large linear (mono) data structure can be compiled containing all samples including their start and length information
(more details on the structure layout below in the developer section).
Stereo .wav files are separated into left and right channel, where first the left and then the right channel is stored in the data structure.
This blob can be downloaded to the host PC or to the TBD.
## Basic stock plugins
TBD's stock firmware offers two basic plugins which make use of the sample rom:
### WTOsc
    A basic wave-table oscillator using the 1MiB of wavetable rom at the beginning of the sample rom section.
    This oscillator makes use of the plaits wavetable oscillator implementation (again, kudos to E. Gillet!!!), but using user wavetable data from TBD's sample rom.
    It allows for Z-morphing within on wavetable bank (one bank consists of 64 wavetables each 256 samples large).
    The 1MiB of wavetable rom allows to store 32 banks of wavetable data, i.e. 32 banks * 64 wavetables * 256 wavetable size * 2 bytes (int16 format) = 1MiB.
    WTOsc allows to select any of those 32 banks, whereas Z-morphing is only implemented for within a bank.
    Features of WTOsc include:
    - User wavetables
    - Wavetable (CV) 
    - Wavetable bank switching (not per CV as pre slower pre-calculations are required)
    - Z-morphing within a bank (EG, LFO, CV)
    - Pitch / pitch modulation (EG, LFO, CV)
    - ADSR EG
    - Amplitude modulated with EG/LFO/CV
    - SVF filter modulated with EG/LFO/CV
### MRompler
    A basic mono sample playback device, which allows to playback sample sections from sample rom.
    Stereo playback can be achieved by allocating one MRompler instance to each channel. 
    When a stereo sample is in sample rom, one can assign either the left or the right slice to one instance and trigger playback at the same time.
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
    - Mono / duophonic voice allocation (note that duo may cause frame dropps if pitch is high, as number of calculations is increased)
## How-to flash without wifi in serial communication mode
In order to flash the sample rom without wifi you can use the following steps. First create a valid sample rom file using the sample rom dialog from TBDs UI.
Then download the compiled file.
Perform the following commands (note, you will have to have [esptool.py](https://github.com/espressif/esptool) installed on your system):

    esptool.py -p serial_port -b 460800 erase_region 0xB00000 0x500000
    esptool.py -p serial_port -b 460800 write_flash --flash_mode dio -fs detect 0xB00000 sample-rom.tbd 
    
Replace "serial_port" with your serial port e.g. COM3 or /dev/cu.usbserial-1420, an example can be found in flash-sample-rom.sh which can be the base for your own.

Replace 0xB00000 with the start address of your sample rom flash start address in case of custom size configs.
Same for 0x500000 if you change the default size.
Replace sample-rom.tbd with the correct file name you have downloaded from TBDs UI.
## Remarks for developers
### Config and layout of flash memory:
The size of the flash rom allocated for sample rom can be configured in the TBD configuration section when executing idf.py menuconfig.
The compiled data is structured as follows (note that downloaded .tbd sample rom files follow the same structure):
- uint32 magicnumber, that is 0xdeadface
- uint32 overall sample data size
- uint32 total number of sample slices
- uint32 end offset of slice 0
- ...
- uint32 end offset of slice N-1
- int16 array, i.e. sample data blob
### Convenience class for sample rom access
Use helpers/ctagSampleRom as a convenience class to access TBD's sample rom.
### Simulator access
Sample rom access is fully modelled in TBD's simulator application. One can access it the same way.
The sample rom data is read from an external .tbd binary file (configurable by command line option of tbd-sim).
## Stock samples and credits
- Wavetables are from [WaveTable online](https://waveeditonline.com/) with [CC0 1.0 Universal (CC0 1.0) Public Domain Dedication License](https://creativecommons.org/publicdomain/zero/1.0/)
- Samples are from CTAG and Freesound (all CC0)