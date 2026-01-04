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

#include "ctagSampleRom.hpp"
#include "ctagSampleRomModel.hpp"
//#include "esp_spi_flash.h"
#include <esp_flash.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include <cstring>
#include <filesystem>

#ifdef TBD_SIM
#define CONFIG_SAMPLE_ROM_START_ADDRESS 0
#else
#include "sdkconfig.h"
#endif

namespace CTAG::SP::HELPERS {
    atomic<uint32_t> ctagSampleRom::nConsumers = 0;
    uint32_t ctagSampleRom::maxPSRAMSize = CONFIG_MAX_ALLOC_BYTES_PSRAM_SAMPLE_DATA;
    uint32_t ctagSampleRom::totalSize = 0;
    uint32_t ctagSampleRom::numberSlices = 0;
    uint32_t ctagSampleRom::headerSize = 0;
    uint32_t *ctagSampleRom::sliceSizes = nullptr;
    uint32_t *ctagSampleRom::sliceOffsets = nullptr;
    uint32_t ctagSampleRom::firstNonWtSlice = 0;
    int16_t *ctagSampleRom::ptrSPIRAM = nullptr;
    uint32_t ctagSampleRom::nSlicesBuffered = 0;
    bool ctagSampleRom::readFromSD = false;

    ctagSampleRom::ctagSampleRom() : ctagSampleRom{CONFIG_MAX_ALLOC_BYTES_PSRAM_SAMPLE_DATA} {}

    ctagSampleRom::ctagSampleRom(const uint32_t sample_rom_size_psram){
        assert(sample_rom_size_psram <= CONFIG_MAX_ALLOC_BYTES_PSRAM_SAMPLE_DATA);
        nConsumers++;
        if(nConsumers == 1){
            maxPSRAMSize = sample_rom_size_psram;
            RefreshDataStructure();
        }
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
        uint32_t totalSizeBytes = 0;
        for (uint32_t i = startSlice; i <= endSlice; i++) {
            totalSizeBytes += sliceSizes[i];
        }
        return totalSizeBytes;
    }

    uint32_t ctagSampleRom::GetSliceOffset(const uint32_t slice) {
        if (slice >= numberSlices) return 0;
        return sliceOffsets[slice];
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
            memset(dst, 0, len*2);
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
            memset(idst, 0, len*2);
        else
            memcpy(idst, &ptrSPIRAM[start], n_samples*2);
        while (len--) {
            *dst++ = float(*dptr++) * 0.000030518509476f;
        }
    }

    void ctagSampleRom::RefreshDataStructure() {
        // check if sample folder exists and contains json descriptor file
        if(CTAG::SP::ctagSampleRomModel::IsSampleRomSDValid()){
            readFromSD = true;
            ESP_LOGI("SROM", "Reading sample data structure from SD card");
            RefreshDataStructureFromSDCard();
            return;
        }
        ESP_LOGI("SROM", "Could not read sample data structure from SD card");
        readFromSD = false;
    }

    std::string ctagSampleRom::GetSampleRomDescriptorJSON(){
        if (!CTAG::SP::ctagSampleRomModel::IsSampleRomSDValid()) return "{}";
        ctagSampleRomModel sample_rom_model;
        return sample_rom_model.GetSampleRomDescriptorJSON();
    }

    std::string ctagSampleRom::GetSampleRomBankDescriptorFilenameByIndex(const uint32_t index){
        if (!CTAG::SP::ctagSampleRomModel::IsSampleRomSDValid()) return "";
        ctagSampleRomModel sample_rom_model;
        if (index < sample_rom_model.GetTotalNumberSampleBanks()){
            return sample_rom_model.GetFilenameSampleBankByIndex(index);
        }
        return "";
    }

    ctagSampleRom::~ctagSampleRom() {
        //ESP_LOGE("SR", "nConsumers %li", nConsumers.load());
        nConsumers--;

        if (nConsumers > 0) return;
        //ESP_LOGE("SR", "freeing up SR data structure");
        if (sliceOffsets != nullptr) {
            heap_caps_free(sliceOffsets);
        }
        if (sliceSizes != nullptr) {
            heap_caps_free(sliceSizes);
        }
        totalSize = 0;
        numberSlices = 0;
        headerSize = 0;
        sliceSizes = nullptr;
        sliceOffsets = nullptr;
        firstNonWtSlice = 0;

        if(ptrSPIRAM != nullptr){
            heap_caps_free(ptrSPIRAM);
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

    void ctagSampleRom::SetActiveWaveTableBank(uint8_t index){
        if (!ctagSampleRomModel::IsSampleRomSDValid()) return;
        ctagSampleRomModel sample_rom_model;
        if (index >= sample_rom_model.GetTotalNumberWTBanks()) return;
        sample_rom_model.SetActiveWTBankIndex(index);
    }

    void ctagSampleRom::SetActiveSampleBank(uint8_t index){
        if (!ctagSampleRomModel::IsSampleRomSDValid()) return;
        ctagSampleRomModel sample_rom_model;
        if (index >= sample_rom_model.GetTotalNumberSampleBanks()) return;
        sample_rom_model.SetActiveSampleBankIndex(index);
    }

    void ctagSampleRom::RefreshDataStructureFromSDCard(){
        ctagSampleRomModel sample_rom_model;

        // get total number of slices and samples
        uint32_t size_wt_bytes = sample_rom_model.GetTotalNumberWTSamples() * 2;
        uint32_t size_samples_bytes = sample_rom_model.GetTotalNumberSampleSamples() * 2;
        uint32_t slices_wt = sample_rom_model.GetTotalNumberWTSlices(); // each wt slice contains 64 wavetables each 256 bytes large
        uint32_t slices_samples = sample_rom_model.GetTotalNumberSampleSlices();

        totalSize = size_wt_bytes + size_samples_bytes;
        numberSlices = slices_wt * 64 + slices_samples;
        headerSize = 0;

        const uint32_t maxSlices = 32 * 64 + CONFIG_MAX_SAMPLES_IN_SAMPLE_ROM; // wavetable max 32 slices

        // alloc memory for data structure in memory
        if (sliceOffsets == nullptr) sliceOffsets = (uint32_t *) heap_caps_malloc(maxSlices * sizeof(uint32_t), MALLOC_CAP_SPIRAM);
        assert(sliceOffsets != nullptr);
        if (sliceSizes == nullptr) sliceSizes = (uint32_t *) heap_caps_malloc(maxSlices * sizeof(uint32_t), MALLOC_CAP_SPIRAM);
        assert(sliceSizes != nullptr);

        // allocate large block of PSRAM
        assert(maxPSRAMSize >= totalSize);
        if(ptrSPIRAM == nullptr){
            size_t maxSizeBytes = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
            ESP_LOGI("SR", "Max Bytes free in PSRAM: %li", maxSizeBytes);
            assert(maxSizeBytes >= CONFIG_MAX_ALLOC_BYTES_PSRAM_SAMPLE_DATA); // check if sample data fits in PSRAM
            ptrSPIRAM = (int16_t *)heap_caps_malloc(maxPSRAMSize, MALLOC_CAP_SPIRAM);
            assert(ptrSPIRAM != nullptr);
            // init with zeros
            memset(ptrSPIRAM, 0, maxPSRAMSize);
            maxSizeBytes = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
            ESP_LOGI("SR", "Overall bytes for wt+sample %li, allocated in PSRAM %li, largest block free in PSRAM %li", totalSize, maxPSRAMSize, maxSizeBytes);
        }

        // generate offsets and sizes, start with wavetables, sizes and offsets are words (due to 16 bit samples)
        uint32_t slice_in_mem_index = 0;
        uint32_t slice_in_mem_offset = 0;
        for (uint32_t i = 0; i < slices_wt; i++) {
            uint32_t sliceSize = sample_rom_model.GetWTSliceSize(i);
            for (uint32_t j = 0; j < 64; j++) {
                sliceSizes[slice_in_mem_index] = sliceSize / 64; // should be 256
                sliceOffsets[slice_in_mem_index] = slice_in_mem_offset;
                slice_in_mem_offset += sliceSizes[slice_in_mem_index];
                slice_in_mem_index++;
            }
        }
        // add samples
        for (uint32_t i = 0; i < slices_samples; i++) {
            uint32_t sliceSize = sample_rom_model.GetSampleSliceSize(i);
            sliceSizes[slice_in_mem_index] = sliceSize;
            sliceOffsets[slice_in_mem_index] = slice_in_mem_offset;
            slice_in_mem_offset += sliceSizes[slice_in_mem_index];
            slice_in_mem_index++;
        }

        // first non Wt Slice
        firstNonWtSlice = slices_wt * 64;

        // read slices from files and buffer in PSRAM
        // load all wavetable files
        for (uint32_t i = 0; i < slices_wt; i++) {
            std::string filename = sample_rom_model.GetFilenameForWTSlice(i);
            if (filename == "") {
                ESP_LOGE("SROM", "No filename for slice %li", i);
                continue;
            }
            if (!std::filesystem::exists(filename)) {
                ESP_LOGE("SROM", "File %s does not exist!", filename.c_str());
                continue;
            }
            uint32_t dataOffset = sample_rom_model.GetDataOffsetForWTSlice(i);
            uint32_t nSamples = sample_rom_model.GetWTSliceSize(i);
            FILE *f = fopen(filename.c_str(), "rb");
            if (f == nullptr) {
                ESP_LOGE("SROM", "Could not open file %s", filename.c_str());
                continue;
            }
            fseek(f, dataOffset, SEEK_SET);
            uint32_t ofs = sliceOffsets[i*64];
            size_t nRead = fread(&ptrSPIRAM[ofs], 2, nSamples, f);
            if (nRead != nSamples) {
                ESP_LOGE("SROM", "Could not read all samples from file %s, read %li of %li", filename.c_str(), nRead,
                         nSamples);
            } else {
                ESP_LOGI("SROM", "Loaded file %s, read %li samples", filename.c_str(), nRead);
            }
            fclose(f);
        }

        // load all sample files
        for (uint32_t i = 0; i < slices_samples; i++) {
            uint32_t j = i + firstNonWtSlice;
            std::string filename = sample_rom_model.GetFilenameForSampleSlice(i);
            if (filename == "") {
                ESP_LOGE("SROM", "No filename for slice %li", j);
                continue;
            }
            if (!std::filesystem::exists(filename)) {
                ESP_LOGE("SROM", "File %s does not exist!", filename.c_str());
                continue;
            }
            uint32_t dataOffset = sample_rom_model.GetDataOffsetForSampleSlice(i);
            uint32_t nSamples = sample_rom_model.GetSampleSliceSize(i);
            FILE *f = fopen(filename.c_str(), "rb");
            if (f == nullptr) {
                ESP_LOGE("SROM", "Could not open file %s", filename.c_str());
                continue;
            }
            fseek(f, dataOffset, SEEK_SET);
            uint32_t ofs = sliceOffsets[j];
            size_t nRead = fread(&ptrSPIRAM[ofs], 2, nSamples, f);
            if (nRead != nSamples) {
                ESP_LOGE("SROM", "Could not read all samples from file %s, read %li of %li", filename.c_str(), nRead,
                         nSamples);
            } else {
                ESP_LOGI("SROM", "Loaded file %s, read %li samples", filename.c_str(), nRead);
            }
            fclose(f);
        }

        // everything is buffered
        nSlicesBuffered = numberSlices;
    }
}
