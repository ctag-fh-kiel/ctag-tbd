#pragma once

#include "../ctagDataModelBase.hpp"

namespace CTAG{
    namespace SP{
        class ctagSampleRomModel final : public  ctagDataModelBase{
        public:
            ctagSampleRomModel();
            static bool IsSampleRomSDValid();
            uint32_t GetTotalNumberWTBanks();
            uint32_t GetTotalNumberSampleBanks();
            std::string GetFilenameWTBankByIndex(const uint32_t index);
            std::string GetFilenameSampleBankByIndex(const uint32_t index);
            std::string GetSampleRomDescriptorJSON();
            void SetActiveWTBankIndex(const uint32_t index);
            void SetActiveSampleBankIndex(const uint32_t index);
            void LoadActiveWTBankDescriptor();
            void LoadActiveSampleBankDescriptor();
            uint32_t GetActiveWTBankIndex();
            uint32_t GetActiveSampleBankIndex();
            void AddWTBank(const std::string &filename);
            void AddSampleBank(const std::string &filename);
            void RemoveWTBank(const uint32_t index);
            void RemoveSampleBank(const uint32_t index);

            // returns total amount of slices
            uint32_t GetTotalNumberWTSlices();
            uint32_t GetTotalNumberSampleSlices();
            // returns total amount of samples, i.e. accumulated slice sizes
            uint32_t GetTotalNumberWTSamples();
            uint32_t GetTotalNumberSampleSamples();
            // returns file name for slice including path and case-sensitive ending (.wav, .WAV etc.)
            std::string GetFilenameForWTSlice(uint32_t slice);
            std::string GetFilenameForSampleSlice(uint32_t slice);
            // returns offset, where sample data begins in .wav file (default 44 bytes)
            uint32_t GetDataOffsetForWTSlice(uint32_t slice);
            uint32_t GetDataOffsetForSampleSlice(uint32_t slice);
            // returns number of samples contained in slice
            uint32_t GetWTSliceSize(uint32_t slice);
            uint32_t GetSampleSliceSize(uint32_t slice);

        private:
            std::string sampleRomDescFileName_;
            Document sample_rom;
            Document desc_wt;
            Document desc_smp;
        };
    }
}
