/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2024 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

// this is an arena style allocator for the sound processors
// implemented to reduce memory fragmentation

#pragma once

#include <cstddef>

namespace CTAG::SP {
    class ctagSPAllocator final {
        // allocator can allocate memory for different type of sound processor creation
        enum AllocationType {
            CH0,
            CH1,
            STEREO
        };
    public:
        ctagSPAllocator() = delete;

        // allocate large block of memory which is used by the sound processors
        static void AllocateInternalBuffer(std::size_t const &size);
        // release large block of memory
        static void ReleaseInternalBuffer();
        // called by new operator of sound processors
        static void *Allocate(std::size_t const &size);
        // called to determine remaining size after new allocation for other heap allocations of sound processor
        static std::size_t GetRemainingBufferSize();
        // called to pass heap available to sound processor
        static void *GetRemainingBuffer();
        // prepare allocation type, must be called before creating new sound processor
        static void PrepareAllocation(AllocationType const &type);

    private:
        static void *internalBuffer; // main ptr to large buffer
        static void *buffer1, *buffer2; // ptrs pointing at memory available for sound processor
        static std::size_t totalSize, size1, size2; // size of large buffer and size of memory available for sound processor
        static AllocationType allocationType; // type of sound processor to create
    };
}
