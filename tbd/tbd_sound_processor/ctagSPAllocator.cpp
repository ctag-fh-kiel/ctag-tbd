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

#include "ctagSPAllocator.hpp"
#include <tbd/logging.hpp>
#include <cstdint>
#include <cassert>
#include <tbd/heaps.hpp>

namespace heaps = tbd::heaps;

using namespace CTAG::SP;

// create all definitions
void *ctagSPAllocator::internalBuffer = nullptr;
void *ctagSPAllocator::buffer1 = nullptr;
void *ctagSPAllocator::buffer2 = nullptr;
std::size_t ctagSPAllocator::totalSize = 0;
std::size_t ctagSPAllocator::size1 = 0;
std::size_t ctagSPAllocator::size2 = 0;
ctagSPAllocator::AllocationType ctagSPAllocator::allocationType = ctagSPAllocator::AllocationType::CH0;

void ctagSPAllocator::AllocateInternalBuffer(std::size_t const &size) {
    TBD_LOGI("ctagSPAllocator", "AllocateInternalBuffer: allocating %d bytes", size);
    internalBuffer = heaps::malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if(nullptr == internalBuffer){
        TBD_LOGE("ctagSPAllocator", "AllocateInternalBuffer: could not allocate memory of size %d", size);
        assert(nullptr != internalBuffer);
    }
    totalSize = size;
}

void ctagSPAllocator::ReleaseInternalBuffer() {
    TBD_LOGI("ctagSPAllocator", "ReleaseInternalBuffer: releasing memory");
    heaps::free(internalBuffer);
    internalBuffer = nullptr;
    buffer1 = nullptr;
    buffer2 = nullptr;
    totalSize = 0;
    size1 = 0;
    size2 = 0;
}

void *ctagSPAllocator::Allocate(std::size_t const &size) {
    void *ptr = nullptr;
    if(allocationType == AllocationType::CH0){
        if(size1 >= size){
            ptr = buffer1;
            buffer1 = static_cast<uint8_t *>(buffer1) + size;
            size1 -= size;
        }else{
            TBD_LOGE("ctagSPAllocator", "Allocate: not enough memory for CH0 request %d bytes, %d bytes free", size, size1);
            assert(false);
        }
    }else if(allocationType == AllocationType::CH1){
        if(size2 >= size){
            ptr = buffer2;
            buffer2 = static_cast<uint8_t *>(buffer2) + size;
            size2 -= size;
        }else{
            TBD_LOGE("ctagSPAllocator", "Allocate: not enough memory for CH1 request %d bytes, %d bytes free", size, size1);
            assert(false);
        }
    }else if(allocationType == AllocationType::STEREO){
        if(size1 >= size){
            ptr = buffer1;
            buffer1 = static_cast<uint8_t *>(buffer1) + size;
            size1 -= size;
        }else{
            TBD_LOGE("ctagSPAllocator", "Allocate: not enough memory for STEREO request %d bytes, %d bytes free", size, size1);
            assert(false);
        }
    }
    switch(allocationType){
        case AllocationType::CH0:
            TBD_LOGI("ctagSPAllocator", "Allocate: allocating CH0 %d bytes object size, ch0 %d bytes blockMem", size, size1);
            break;
        case AllocationType::CH1:
            TBD_LOGI("ctagSPAllocator", "Allocate: allocating CH1 %d bytes object size, ch1 %d bytes blockMem", size, size2);
            break;
        case AllocationType::STEREO:
            TBD_LOGI("ctagSPAllocator", "Allocate: allocating STEREO %d bytes object size, %d bytes blockMem", size, size1);
            break;
        default:
            TBD_LOGE("ctagSPAllocator", "Allocate: unknown allocation type");
            assert(false);
    }
    return ptr;
}

std::size_t ctagSPAllocator::GetRemainingBufferSize() {
    if(allocationType == AllocationType::CH0 || allocationType == AllocationType::STEREO){
        TBD_LOGD("ctagSPAllocator", "GetRemainingBuffer: CH0 or STEREO %d bytes free", size1);
        return size1;
    }else if(allocationType == AllocationType::CH1){
        TBD_LOGD("ctagSPAllocator", "GetRemainingBuffer: CH1 %d bytes free", size2);
        return size2;
    }else
        TBD_LOGE("ctagSPAllocator", "GetRemainingBufferSize: unknown allocation type");
    return 0;
}

void *ctagSPAllocator::GetRemainingBuffer() {
    void *ptr = nullptr;
    if(allocationType == AllocationType::CH0 || allocationType == AllocationType::STEREO){
        ptr = buffer1;
    }
    else if(allocationType == AllocationType::CH1){
        ptr = buffer2;
    }
    else
        TBD_LOGE("ctagSPAllocator", "GetRemainingBuffer: unknown allocation type");
    return ptr;
}


void ctagSPAllocator::PrepareAllocation(AllocationType const &type) {
    allocationType = type;
    if(allocationType == AllocationType::CH0){
        TBD_LOGI("ctagSPAllocator", "SetAllocationType: Single Channel CH0");
        size1 = totalSize / 2;
        buffer1 = internalBuffer;
    }else if(allocationType == AllocationType::CH1){
        TBD_LOGI("ctagSPAllocator", "SetAllocationType: Single Channel CH1");
        size2 = totalSize / 2;
        buffer2 = static_cast<uint8_t *>(internalBuffer) + size2;
    }else if(allocationType == AllocationType::STEREO){
        TBD_LOGI("ctagSPAllocator", "SetAllocationType: Stereo");
        size1 = totalSize;
        buffer1 = internalBuffer;
        size2 = 0;
        buffer2 = nullptr;
    }else{
        TBD_LOGE("ctagSPAllocator", "SetAllocationType: unknown allocation type");
        assert(false);
    }
}