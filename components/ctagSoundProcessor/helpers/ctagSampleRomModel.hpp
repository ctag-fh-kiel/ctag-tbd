#pragma once

#include "../ctagDataModelBase.hpp"

namespace CTAG{
    namespace SP{
        class ctagSampleRomModel final : public  ctagDataModelBase{
        public:
            ctagSampleRomModel(std::string const &descriptorName);

            // returns total amount of slices
            uint32_t GetTotalNumberSlices();
            // returns total amount of samples, i.e. accumulated slice sizes
            uint32_t GetTotalNumberSamples();
            // returns file name for slice including path and case-sensitive ending (.wav, .WAV etc.)
            std::string GetFilenameForSlice(uint32_t slice);
            // returns offset, where sample data begins in .wav file (default 44 bytes)
            uint32_t GetDataOffsetForSlice(uint32_t slice);
            // returns number of samples contained in slice
            uint32_t GetSliceSize(uint32_t slice);
        private:
            Document desc;
            uint32_t totalNumberSlices = 0;
            uint32_t totalNumberSamples = 0;
        };
    }
}
