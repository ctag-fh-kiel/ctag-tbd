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
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <cstdint>
#include <cassert>

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
    ESP_LOGI("ctagSPAllocator", "AllocateInternalBuffer: allocating %d bytes\n", size);
    internalBuffer = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if(nullptr == internalBuffer){
        ESP_LOGE("ctagSPAllocator", "AllocateInternalBuffer: could not allocate memory of size %d\n", size);
        assert(nullptr != internalBuffer);
    }
    totalSize = size;
}

void ctagSPAllocator::ReleaseInternalBuffer() {
    ESP_LOGI("ctagSPAllocator", "ReleaseInternalBuffer: releasing memory\n");
    heap_caps_free(internalBuffer);
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
            ESP_LOGE("ctagSPAllocator", "Allocate: not enough memory for CH0\n");
            assert(false);
        }
    }else if(allocationType == AllocationType::CH1){
        if(size2 >= size){
            ptr = buffer2;
            buffer2 = static_cast<uint8_t *>(buffer2) + size;
            size2 -= size;
        }else{
            ESP_LOGE("ctagSPAllocator", "Allocate: not enough memory for CH1\n");
            assert(false);
        }
    }else if(allocationType == AllocationType::STEREO){
        if(size1 >= size){
            ptr = buffer1;
            buffer1 = static_cast<uint8_t *>(buffer1) + size;
            size1 -= size;
        }else{
            ESP_LOGE("ctagSPAllocator", "Allocate: not enough memory for STEREO\n");
            assert(false);
        }
    }
    switch(allocationType){
        case AllocationType::CH0:
            ESP_LOGI("ctagSPAllocator", "Allocate: allocating CH0 %d bytes, ch0 %d bytes remaining\n", size, size1);
            break;
        case AllocationType::CH1:
            ESP_LOGI("ctagSPAllocator", "Allocate: allocating CH1 %d bytes, ch1 %d bytes remaining\n", size, size2);
            break;
        case AllocationType::STEREO:
            ESP_LOGI("ctagSPAllocator", "Allocate: allocating STEREO %d bytes, %d bytes remaining\n", size, size1);
            break;
        default:
            ESP_LOGE("ctagSPAllocator", "Allocate: unknown allocation type\n");
            assert(false);
    }
    return ptr;
}

std::size_t ctagSPAllocator::GetRemainingBufferSize() {
    if(allocationType == AllocationType::CH0 || allocationType == AllocationType::STEREO){
        ESP_LOGI("ctagSPAllocator", "GetRemainingBuffer: CH0 or STEREO %d bytes\n", size1);
        return size1;
    }else if(allocationType == AllocationType::CH1){
        ESP_LOGI("ctagSPAllocator", "GetRemainingBuffer: CH1 %d bytes\n", size2);
        return size2;
    }else
        ESP_LOGE("ctagSPAllocator", "GetRemainingBufferSize: unknown allocation type\n");
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
        ESP_LOGE("ctagSPAllocator", "GetRemainingBuffer: unknown allocation type\n");
    return ptr;
}


void ctagSPAllocator::PrepareAllocation(AllocationType const &type) {
    allocationType = type;
    if(allocationType == AllocationType::CH0){
        ESP_LOGI("ctagSPAllocator", "SetAllocationType: Single Channel CH0\n");
        size1 = totalSize / 2;
        buffer1 = internalBuffer;
    }else if(allocationType == AllocationType::CH1){
        ESP_LOGI("ctagSPAllocator", "SetAllocationType: Single Channel CH1\n");
        size2 = totalSize / 2;
        buffer2 = static_cast<uint8_t *>(internalBuffer) + size2;
    }else if(allocationType == AllocationType::STEREO){
        ESP_LOGI("ctagSPAllocator", "SetAllocationType: Stereo\n");
        size1 = totalSize;
        buffer1 = internalBuffer;
        size2 = 0;
        buffer2 = nullptr;
    }else{
        ESP_LOGE("ctagSPAllocator", "SetAllocationType: unknown allocation type\n");
        assert(false);
    }
}