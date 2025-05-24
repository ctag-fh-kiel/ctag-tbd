#include <tbd/sound_processor/allocator.hpp>

#include <tbd/logging.hpp>
#include <cstdint>
#include <cassert>
#include <tbd/heaps.hpp>
#include <tbd/sound_processor/module.hpp>


namespace heaps = tbd::heaps;
using tbd::sound_processor::tag;

namespace tbd::sound_processor {

namespace {

void* internalBuffer = nullptr;
void* buffer1 = nullptr;
void* buffer2 = nullptr;
std::size_t totalSize = 0;
std::size_t size1 = 0;
std::size_t size2 = 0;
SoundProcessorAllocator::AllocationType allocationType = SoundProcessorAllocator::AllocationType::CH0;

}

void SoundProcessorAllocator::allocate_buffers(std::size_t const &size) {
    if (internalBuffer != nullptr) {
        TBD_LOGE(tag, "plugin buffers have already been allocated");
        return;
    }
    TBD_LOGI(tag, "AllocateInternalBuffer: allocating %zd bytes", size);
    internalBuffer = heaps::malloc(size, TBD_HEAPS_INTERNAL | TBD_HEAPS_8BIT);
    if(nullptr == internalBuffer){
        TBD_LOGE(tag, "AllocateInternalBuffer: could not allocate memory of size %zd", size);
        assert(nullptr != internalBuffer);
    }
    totalSize = size;
}

void SoundProcessorAllocator::release_buffers() {
    TBD_LOGI(tag, "ReleaseInternalBuffer: releasing memory");
    heaps::free(internalBuffer);
    internalBuffer = nullptr;
    buffer1 = nullptr;
    buffer2 = nullptr;
    totalSize = 0;
    size1 = 0;
    size2 = 0;
}

void *SoundProcessorAllocator::allocate(std::size_t const &size) {
    void *ptr = nullptr;
    if(allocationType == AllocationType::CH0){
        if(size1 >= size){
            ptr = buffer1;
            buffer1 = static_cast<uint8_t *>(buffer1) + size;
            size1 -= size;
        }else{
            TBD_LOGE(tag, "Allocate: not enough memory for CH0 request %zd bytes, %zd bytes free", size, size1);
            assert(false);
        }
    }else if(allocationType == AllocationType::CH1){
        if(size2 >= size){
            ptr = buffer2;
            buffer2 = static_cast<uint8_t *>(buffer2) + size;
            size2 -= size;
        }else{
            TBD_LOGE(tag, "Allocate: not enough memory for CH1 request %zd bytes, %zd bytes free", size, size1);
            assert(false);
        }
    }else if(allocationType == AllocationType::STEREO){
        if(size1 >= size){
            ptr = buffer1;
            buffer1 = static_cast<uint8_t *>(buffer1) + size;
            size1 -= size;
        }else{
            TBD_LOGE(tag, "Allocate: not enough memory for STEREO request %zd bytes, %zd bytes free", size, size1);
            assert(false);
        }
    }
    switch(allocationType){
    case AllocationType::CH0:
        TBD_LOGI(tag, "Allocate: allocating CH0 %zd bytes object size, ch0 %zd bytes blockMem", size, size1);
        break;
    case AllocationType::CH1:
        TBD_LOGI(tag, "Allocate: allocating CH1 %zd bytes object size, ch1 %zd bytes blockMem", size, size2);
        break;
    case AllocationType::STEREO:
        TBD_LOGI(tag, "Allocate: allocating STEREO %zd bytes object size, %zd bytes blockMem", size, size1);
        break;
    default:
        TBD_LOGE(tag, "Allocate: unknown allocation type");
        assert(false);
    }
    return ptr;
}

std::size_t SoundProcessorAllocator::get_remaining_buffer_size() {
    if(allocationType == AllocationType::CH0 || allocationType == AllocationType::STEREO){
        TBD_LOGD(tag, "GetRemainingBuffer: CH0 or STEREO %zd bytes free", size1);
        return size1;
    }else if(allocationType == AllocationType::CH1){
        TBD_LOGD(tag, "GetRemainingBuffer: CH1 %zd bytes free", size2);
        return size2;
    }else
        TBD_LOGE(tag, "GetRemainingBufferSize: unknown allocation type");
    return 0;
}

void* SoundProcessorAllocator::get_remaining_buffer() {
    if(allocationType == AllocationType::CH0 || allocationType == AllocationType::STEREO) {
        return buffer1;
    } else if(allocationType == AllocationType::CH1){
        return buffer2;
    } else {
        TBD_LOGE(tag, "GetRemainingBuffer: unknown allocation type");
        return nullptr;
    }
}


void SoundProcessorAllocator::prepare_allocation(AllocationType const &type) {
    allocationType = type;
    if(allocationType == AllocationType::CH0){
        TBD_LOGI(tag, "SetAllocationType: Single Channel CH0");
        size1 = totalSize / 2;
        buffer1 = internalBuffer;
    }else if(allocationType == AllocationType::CH1){
        TBD_LOGI(tag, "SetAllocationType: Single Channel CH1");
        size2 = totalSize / 2;
        buffer2 = static_cast<uint8_t *>(internalBuffer) + size2;
    }else if(allocationType == AllocationType::STEREO){
        TBD_LOGI(tag, "SetAllocationType: Stereo");
        size1 = totalSize;
        buffer1 = internalBuffer;
        size2 = 0;
        buffer2 = nullptr;
    }else{
        TBD_LOGE(tag, "SetAllocationType: unknown allocation type");
        assert(false);
    }
}

}
