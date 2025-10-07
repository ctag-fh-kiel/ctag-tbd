#include "ctagSampleRomModel.hpp"

CTAG::SP::ctagSampleRomModel::ctagSampleRomModel(std::string const& descriptorName){
    loadJSON(desc, descriptorName);
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberSlices(){
    totalNumberSlices = 0;
    if(desc.IsArray()){
        totalNumberSlices = desc.GetArray().Size();
    }
    return totalNumberSlices;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberSamples(){
    totalNumberSamples = 0;
    if(desc.IsArray()){
        for(auto& v : desc.GetArray()){
            if(v.HasMember("nsamples") && v["nsamples"].IsUint()){
                totalNumberSamples += v["nsamples"].GetUint();
            }
        }
    }
    return totalNumberSamples;
}

std::string CTAG::SP::ctagSampleRomModel::GetFilenameForSlice(uint32_t slice){
    std::string filename = "";
    if(desc.IsArray()){
        if(slice < desc.GetArray().Size()){
            Value& v = desc[slice];
            if(v.HasMember("filename") && v["filename"].IsString()){
                filename = v["filename"].GetString();
            }
            if(v.HasMember("path") && v["path"].IsString()){
                filename = std::string(v["path"].GetString()) + "/" + std::string(v["filename"].GetString());
            }
        }
    }
    return filename;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetDataOffsetForSlice(uint32_t slice){
    uint32_t offset = 44; // standard offset in .wav files
    if(desc.IsArray()){
        if(slice < desc.GetArray().Size()){
            Value& v = desc[slice];
            if(v.HasMember("offset") && v["offset"].IsUint()){
                offset = v["offset"].GetUint();
            }
        }
    }
    return offset;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetSliceSize(uint32_t slice){
    uint32_t size = 0;
    if(desc.IsArray()){
        if(slice < desc.GetArray().Size()){
            Value& v = desc[slice];
            if(v.HasMember("nsamples") && v["nsamples"].IsUint()){
                size = v["nsamples"].GetUint();
            }
        }
    }
    return size;
}
