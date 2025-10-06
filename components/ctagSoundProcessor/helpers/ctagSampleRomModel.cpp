#include "ctagSampleRomModel.hpp"

CTAG::SP::ctagSampleRomModel::ctagSampleRomModel(std::string const& descriptorName){
    loadJSON(desc, descriptorName);
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberSlices(){
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberSamples(){
}
