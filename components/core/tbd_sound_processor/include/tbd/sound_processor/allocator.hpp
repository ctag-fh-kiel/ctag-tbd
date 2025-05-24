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
// steps to use:
// 1. call AllocateInternalBuffer with size of large buffer at program start
// 2. call PrepareAllocation with specific AllocationType before creating new sound processor,
// note that the state is set in "allocationType" for subsequent allocations
// 3. Allocate is called in overloaded new operator of sound processor
// 4. GetRemainingBufferSize is called by sound processor factory in "Init()" to pass remaining memory size available to sound processor
// 5. GetRemainingBuffer is called by sound processor factory in "Init()" to pass remaining memory available to sound processor
// 6. ReleaseInternalBuffer is called at program end to release large buffer, or when large memory is needed somewhere else

#pragma once

#include <cstddef>

namespace tbd::sound_processor {

struct SoundProcessorAllocator final {
    // allocator can allocate memory for different type of sound processor creation
    enum AllocationType {
        CH0,
        CH1,
        STEREO
    };
    SoundProcessorAllocator() = delete;

    // allocate large block of memory which is used by the sound processors
    static void allocate_buffers(std::size_t const &size);
    // release large block of memory
    static void release_buffers();
    // called by new operator of sound processors
    static void *allocate(std::size_t const &size);
    // called to determine remaining size after new allocation for other heap allocations of sound processor
    static std::size_t get_remaining_buffer_size();
    // called to pass heap available to sound processor
    static void *get_remaining_buffer();
    // prepare allocation type, must be called before creating new sound processor
    static void prepare_allocation(AllocationType const &type);
};

}
