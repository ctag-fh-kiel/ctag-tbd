/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

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

using namespace std;

namespace CTAG::SP::HELPERS{
    class ctagSampleRom {
    public:
        static void RefreshDataStructure(); // forces refresh of data structure, not thread safe!
        ctagSampleRom();
        uint32_t GetNumberSlices();
        uint32_t GetFirstNonWaveTableSlice();
        // return slice size in int16 samples i.e. *2 in bytes
        uint32_t GetSliceSize(const uint32_t slice);
        uint32_t GetSliceGroupSize(const uint32_t startSlice, const uint32_t endSlice);
        uint32_t GetSliceOffset(const uint32_t slice);
        bool HasSlice(const uint32_t slice);
        bool HasSliceGroup(const uint32_t startSlice, const uint32_t endSlice);
        void Read(int16_t *dst, uint32_t offset, const uint32_t n_samples);
        void ReadSlice(int16_t *dst, const uint32_t slice, const uint32_t offset, const uint32_t n_samples);
        void ReadSliceAsFloat(float *dst, const uint32_t slice, const uint32_t offset, const uint32_t n_samples);
    private:
        static uint32_t totalSize;
        static uint32_t numberSlices;
        static uint32_t headerSize;
        static vector<uint32_t> v_sliceSizes;
        static vector<uint32_t> v_sliceOffsets;
        static bool isInitialized;
    };
}
