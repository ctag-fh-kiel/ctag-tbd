/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020, 2025 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#pragma once
#include <cstdint>
#include <vector>
#include <atomic>
#include <string>

using namespace std;

namespace CTAG::SP::HELPERS{
    class ctagSampleRom {
    public:
        static void RefreshDataStructure(); // forces refresh of data structure, not thread safe!
        static std::string GetSampleRomDescriptorJSON();
        static std::string GetSampleRomBankDescriptorFilenameByIndex(const uint32_t index);
        static void SetActiveWaveTableBank(uint8_t index);
        static void SetActiveSampleBank(uint8_t index);
        ctagSampleRom();
        explicit ctagSampleRom(const uint32_t sample_rom_size_psram);
        ~ctagSampleRom();
        uint32_t GetNumberSlices();
        uint32_t GetFirstNonWaveTableSlice();
        // return slice size in int16 samples i.e. *2 in bytes
        uint32_t GetSliceSize(const uint32_t slice);
        uint32_t GetSliceGroupSize(const uint32_t startSlice, const uint32_t endSlice);
        uint32_t GetSliceOffset(const uint32_t slice);
        bool HasSlice(const uint32_t slice);
        bool HasSliceGroup(const uint32_t startSlice, const uint32_t endSlice);
        void ReadSlice(int16_t *dst, const uint32_t slice, const uint32_t offset, const uint32_t n_samples);
        void ReadSliceAsFloat(float *dst, const uint32_t slice, const uint32_t offset, const uint32_t n_samples);
        bool IsBufferedInSPIRAM();

    private:
        static void RefreshDataStructureFromSDCard();
        static uint32_t maxPSRAMSize;
        static uint32_t totalSize;
        static uint32_t numberSlices;
        static uint32_t headerSize;
        static uint32_t *sliceSizes;
        static uint32_t *sliceOffsets;
        static uint32_t firstNonWtSlice;
        static atomic<uint32_t>  nConsumers;
        static int16_t *ptrSPIRAM;
        static uint32_t nSlicesBuffered;
        static bool readFromSD;

    };
}
