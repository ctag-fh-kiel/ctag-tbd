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

#include "tbd/helpers/ctagSampleRom.hpp"
//#include "esp_spi_flash.h"
#include <esp_flash.h>
#include "esp_heap_caps.h"
#include <cstring>

#ifdef TBD_SIM
#define CONFIG_SAMPLE_ROM_START_ADDRESS 0
#else
#include "sdkconfig.h"
#endif

namespace CTAG::SP::HELPERS {
    atomic<uint32_t> ctagSampleRom::nConsumers = 0;
    uint32_t ctagSampleRom::totalSize = 0;
    uint32_t ctagSampleRom::numberSlices = 0;
    uint32_t ctagSampleRom::headerSize = 0;
    uint32_t *ctagSampleRom::sliceSizes = nullptr;
    uint32_t *ctagSampleRom::sliceOffsets = nullptr;
    uint32_t ctagSampleRom::firstNonWtSlice = 0;
    int16_t *ctagSampleRom::ptrSPIRAM = nullptr;
    uint32_t ctagSampleRom::nSlicesBuffered = 0;

    ctagSampleRom::ctagSampleRom() {
        //TBD_LOGE("SR", "nConsumers %li", nConsumers.load());
        nConsumers++;
        if(nConsumers == 1)
            RefreshDataStructure();
    }

    uint32_t ctagSampleRom::GetNumberSlices() {
        return numberSlices;
    }

    uint32_t ctagSampleRom::GetSliceSize(const uint32_t slice) {
        if (slice >= numberSlices) return 0;
        return sliceSizes[slice];
    }

    uint32_t ctagSampleRom::GetSliceGroupSize(const uint32_t startSlice, const uint32_t endSlice) {
        if (endSlice <= startSlice) return 0;
        uint32_t totalSize = 0;
        for (uint32_t i = startSlice; i <= endSlice; i++) {
            totalSize += sliceSizes[i];
        }
        return totalSize;
    }

    uint32_t ctagSampleRom::GetSliceOffset(const uint32_t slice) {
        if (slice >= numberSlices) return 0;
        return sliceOffsets[slice];
    }

    // reads words, offset in words not bytes
    void ctagSampleRom::Read(int16_t *dst, uint32_t offset, const uint32_t n_samples) {
        assert(dst != nullptr);
        offset *= 2; // from int16 to bytes
        offset += headerSize; // add header size
        offset += CONFIG_SAMPLE_ROM_START_ADDRESS; // add start offset
        //spi_flash_read(offset, dst, n_samples * 2);
        esp_flash_read(nullptr, dst, offset, n_samples * 2);
    }

    bool ctagSampleRom::HasSlice(const uint32_t slice) {
        if (slice >= numberSlices) return false;
        return true;
    }

    bool ctagSampleRom::HasSliceGroup(const uint32_t startSlice, const uint32_t endSlice) {
        if (startSlice > numberSlices || endSlice > numberSlices) return false;
        return true;
    }

    void ctagSampleRom::ReadSlice(int16_t *dst, const uint32_t slice, const uint32_t offset, const uint32_t n_samples) {
        uint32_t start = sliceOffsets[slice] + offset;
        int32_t len = n_samples;
        if (offset + len >= sliceSizes[slice]) { // read beyond slice end ?
            len = sliceSizes[slice] - offset;
        }
        if (len <= 0) return; // nothing to read!
        if(slice >= nSlicesBuffered) // nSlicesBuffered > 0 if SPIRAM Buffer is used
            Read(dst, start, len);
        else
            memcpy(dst, &ptrSPIRAM[start], n_samples*2);
    }

    void ctagSampleRom::ReadSliceAsFloat(float *dst, const uint32_t slice, const uint32_t offset,
                                         const uint32_t n_samples) {
        uint32_t start = sliceOffsets[slice] + offset;
        int32_t len = n_samples;
        if (offset + len >= sliceSizes[slice]) { // read beyond slice end ?
            len = sliceSizes[slice] - offset;
        }
        if (len <= 0) return; // nothing to read!
        int16_t idst[len];
        int16_t *dptr = idst;
        if(slice >= nSlicesBuffered)
            Read(idst, start, len);
        else
            memcpy(idst, &ptrSPIRAM[start], n_samples*2);
        while (len--) {
            *dst++ = float(*dptr++) * 0.000030518509476f;
        }
    }

    void ctagSampleRom::RefreshDataStructure() {
        if(nConsumers == 0) return;
        if (sliceOffsets != nullptr) {
            heaps::free(sliceOffsets);
            sliceOffsets = nullptr;
        }
        if (sliceSizes != nullptr) {
            heaps::free(sliceSizes);
            sliceSizes = nullptr;
        }
        uint32_t deadface = 0;
        totalSize = 0;
        numberSlices = 0;
        headerSize = 0;
        //spi_flash_read(CONFIG_SAMPLE_ROM_START_ADDRESS, &deadface, 4);
        esp_flash_read(nullptr, &deadface, CONFIG_SAMPLE_ROM_START_ADDRESS, 4);
        if (deadface != 0xdeadface) {
            TBD_LOGE("SROM", "Magic number wrong!");
            return;
        }
        headerSize += 4;
        //spi_flash_read(CONFIG_SAMPLE_ROM_START_ADDRESS + 4, &totalSize, 4);
        esp_flash_read(nullptr,&totalSize, CONFIG_SAMPLE_ROM_START_ADDRESS + 4, 4);
        headerSize += 4;
        TBD_LOGD("SROM", "Total sample data size %li bytes", totalSize);
        //spi_flash_read(CONFIG_SAMPLE_ROM_START_ADDRESS + 8, &numberSlices, 4);
        esp_flash_read(nullptr, &numberSlices, CONFIG_SAMPLE_ROM_START_ADDRESS + 8, 4);
        headerSize += 4;
        TBD_LOGD("SROM", "Number slices %li", numberSlices);
        // alloc memory
        sliceOffsets = (uint32_t *) heaps::malloc(numberSlices * sizeof(uint32_t), MALLOC_CAP_SPIRAM);
        assert(sliceOffsets != nullptr);
        sliceSizes = (uint32_t *) heaps::malloc(numberSlices * sizeof(uint32_t), MALLOC_CAP_SPIRAM);
        assert(sliceSizes != nullptr);
        //spi_flash_read(CONFIG_SAMPLE_ROM_START_ADDRESS + 12, &sliceOffsets[0], 4 * numberSlices);
        esp_flash_read(nullptr, &sliceOffsets[0], CONFIG_SAMPLE_ROM_START_ADDRESS + 12, 4 * numberSlices);
        headerSize += 4 * numberSlices;
        int lastOffset = 0;
        for (uint32_t i = 0; i < numberSlices; i++) {
            sliceSizes[i] = sliceOffsets[i] - lastOffset;
            lastOffset = sliceOffsets[i];
            sliceOffsets[i] -= sliceSizes[i];
            TBD_LOGD("SROM", "Slice size %li, offset %li", sliceSizes[i], sliceOffsets[i]);
        }
        // get first non Wt Slice
        for (int i = 0; i < numberSlices; i++) {
            if (sliceSizes[i] > 256){
                firstNonWtSlice = i;
                break;
            }
        }
    }

    ctagSampleRom::~ctagSampleRom() {
        //TBD_LOGE("SR", "nConsumers %li", nConsumers.load());
        nConsumers--;

        if (nConsumers > 0) return;
        //TBD_LOGE("SR", "freeing up SR data structure");
        if (sliceOffsets != nullptr) {
            heaps::free(sliceOffsets);
        }
        if (sliceSizes != nullptr) {
            heaps::free(sliceSizes);
        }
        totalSize = 0;
        numberSlices = 0;
        headerSize = 0;
        sliceSizes = nullptr;
        sliceOffsets = nullptr;
        firstNonWtSlice = 0;

        if(ptrSPIRAM != nullptr){
            heaps::free(ptrSPIRAM);
            ptrSPIRAM = nullptr;
            nSlicesBuffered = 0;
        }
    }

    uint32_t ctagSampleRom::GetFirstNonWaveTableSlice() {
        return firstNonWtSlice;
    }

    bool ctagSampleRom::IsBufferedInSPIRAM() {
        if(ptrSPIRAM == nullptr) return false;
        return true;
    }

    void ctagSampleRom::BufferInSPIRAM() {
        if(ptrSPIRAM != nullptr) return; // already buffered
        size_t maxSizeBytes = heaps::get_largest_free_block(MALLOC_CAP_SPIRAM);
        maxSizeBytes -= 128*1024; // reserve 128k for other stuff
        if(maxSizeBytes < 1024*1024) return; // not enough memory for this to make sense
        ptrSPIRAM = (int16_t *)heaps::malloc(maxSizeBytes, MALLOC_CAP_SPIRAM);
        if(ptrSPIRAM == nullptr) return;
        TBD_LOGI("SR", "Buffering %d bytes in SPIRAM", maxSizeBytes);
        // figure out how many slices can be buffered
        uint32_t maxSizeWords = maxSizeBytes / 2;
        nSlicesBuffered = 0;
        uint32_t totalSizeWords = 0;
        for(uint32_t i=0;i<numberSlices;i++){
            if(totalSizeWords + sliceSizes[i] > maxSizeWords) break;
            totalSizeWords += sliceSizes[i];
            nSlicesBuffered++;
        }
        TBD_LOGI("SR", "Buffering %li slices of %li, consuming %li bytes", nSlicesBuffered, numberSlices, totalSizeWords*2);
        Read(ptrSPIRAM, 0, totalSizeWords);
    }
}
