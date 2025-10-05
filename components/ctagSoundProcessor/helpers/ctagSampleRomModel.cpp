#include "ctagSampleRomModel.hpp"
#include <filesystem>

#include "rapidjson/writer.h"

#ifdef TBD_SIM
#define SD_CARD_SAMPLE_FOLDER "../../sample_rom/tbdsamples"
#else
#define SD_CARD_SAMPLE_FOLDER "/spiffs/tbdsamples"
#endif

#define SAMPLE_ROM_DEFINITION_FILE "sample_rom.jsn"

CTAG::SP::ctagSampleRomModel::ctagSampleRomModel(){
    sampleRomDescFileName_ = std::string(SD_CARD_SAMPLE_FOLDER) + "/" + std::string(SAMPLE_ROM_DEFINITION_FILE);
    loadJSON(sample_rom, sampleRomDescFileName_);
    LoadActiveWTBankDescriptor();
    LoadActiveSampleBankDescriptor();
}

bool CTAG::SP::ctagSampleRomModel::IsSampleRomSDValid(){
    if (!std::filesystem::exists(SD_CARD_SAMPLE_FOLDER)) return false;
    if (!std::filesystem::is_directory(SD_CARD_SAMPLE_FOLDER)) return false;
    if (!std::filesystem::exists(std::string(SD_CARD_SAMPLE_FOLDER) + "/" + SAMPLE_ROM_DEFINITION_FILE)) return false;
    return true;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberWTBanks(){
    if (!sample_rom.IsObject()) return 0;
    if (!sample_rom.HasMember("wt_banks")) return 0;
    if (!sample_rom["wt_banks"].IsArray()) return 0;
    return sample_rom["wt_banks"].GetArray().Size();
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberSampleBanks(){
    if (!sample_rom.IsObject()) return 0;
    if (!sample_rom.HasMember("smp_banks")) return 0;
    if (!sample_rom["smp_banks"].IsArray()) return 0;
    return sample_rom["smp_banks"].GetArray().Size();
}

std::string CTAG::SP::ctagSampleRomModel::GetFilenameWTBankByIndex(const uint32_t index){
    if (!sample_rom.IsObject()) return "";
    if (!sample_rom.HasMember("wt_banks")) return "";
    if (!sample_rom["wt_banks"].IsArray()) return "";
    if (index >= sample_rom["wt_banks"].GetArray().Size()) return "";
    return std::string(SD_CARD_SAMPLE_FOLDER) + "/" + std::string(sample_rom["wt_banks"].GetArray()[index].GetString());
}

std::string CTAG::SP::ctagSampleRomModel::GetFilenameSampleBankByIndex(const uint32_t index){
    if (!sample_rom.IsObject()) return "";
    if (!sample_rom.HasMember("smp_banks")) return "";
    if (!sample_rom["smp_banks"].IsArray()) return "";
    if (index >= sample_rom["smp_banks"].GetArray().Size()) return "";
    return std::string(SD_CARD_SAMPLE_FOLDER) + "/" + std::string(sample_rom["smp_banks"].GetArray()[index].GetString());
}

std::string CTAG::SP::ctagSampleRomModel::GetSampleRomDescriptorJSON(){
    json.Clear();
    Writer<StringBuffer> writer(json);
    sample_rom.Accept(writer);
    return json.GetString();
}

void CTAG::SP::ctagSampleRomModel::SetActiveWTBankIndex(const uint32_t index){
    if (!sample_rom.IsObject()) return;
    if (!sample_rom.HasMember("active_wt_bank")) return;
    if (!sample_rom["active_wt_bank"].IsUint()) return;
    if (index >= GetTotalNumberWTBanks()) return;
    sample_rom["active_wt_bank"].SetUint(index);
    storeJSON(sample_rom, sampleRomDescFileName_);
    loadJSON(desc_wt, GetFilenameWTBankByIndex(index));
}

void CTAG::SP::ctagSampleRomModel::SetActiveSampleBankIndex(const uint32_t index){
    if (!sample_rom.IsObject()) return;
    if (!sample_rom.HasMember("active_smp_bank")) return;
    if (!sample_rom["active_smp_bank"].IsUint()) return;
    if (index >= GetTotalNumberSampleBanks()) return;
    sample_rom["active_smp_bank"].SetUint(index);
    storeJSON(sample_rom, sampleRomDescFileName_);
    loadJSON(desc_smp, GetFilenameSampleBankByIndex(index));
}

void CTAG::SP::ctagSampleRomModel::LoadActiveWTBankDescriptor(){
    loadJSON(desc_wt, GetFilenameWTBankByIndex(GetActiveWTBankIndex()));
}

void CTAG::SP::ctagSampleRomModel::LoadActiveSampleBankDescriptor(){
    loadJSON(desc_smp, GetFilenameSampleBankByIndex(GetActiveSampleBankIndex()));
}

uint32_t CTAG::SP::ctagSampleRomModel::GetActiveWTBankIndex(){
    if (!sample_rom.IsObject()) return 0;
    if (!sample_rom.HasMember("active_wt_bank")) return 0;
    if (!sample_rom["active_wt_bank"].IsUint()) return 0;
    return sample_rom["active_wt_bank"].GetUint();
}

uint32_t CTAG::SP::ctagSampleRomModel::GetActiveSampleBankIndex(){
    if (!sample_rom.IsObject()) return 0;
    if (!sample_rom.HasMember("active_smp_bank")) return 0;
    if (!sample_rom["active_smp_bank"].IsUint()) return 0;
    return sample_rom["active_smp_bank"].GetUint();
}

void CTAG::SP::ctagSampleRomModel::AddWTBank(const std::string& filename){
    // add filename to wt bank array
    if (!sample_rom.IsObject()) return;
    if (!sample_rom.HasMember("wt_banks")) return;
    if (!sample_rom["wt_banks"].IsArray()) return;
    Value fn(kStringType);
    fn.SetString(filename, sample_rom.GetAllocator());
    sample_rom["wt_banks"].PushBack(fn, sample_rom.GetAllocator());
    storeJSON(sample_rom, sampleRomDescFileName_);
}

void CTAG::SP::ctagSampleRomModel::AddSampleBank(const std::string& filename){
    // add filename to smp bank array
    if (!sample_rom.IsObject()) return;
    if (!sample_rom.HasMember("smp_banks")) return;
    if (!sample_rom["smp_banks"].IsArray()) return;
    Value fn(kStringType);
    fn.SetString(filename, sample_rom.GetAllocator());
    sample_rom["smp_banks"].PushBack(fn, sample_rom.GetAllocator());
}

void CTAG::SP::ctagSampleRomModel::RemoveWTBank(const uint32_t index){
    // remove filename from wt bank array
    if (!sample_rom.IsObject()) return;
    if (!sample_rom.HasMember("wt_banks")) return;
    if (!sample_rom["wt_banks"].IsArray()) return;
    if (index >= sample_rom["wt_banks"].GetArray().Size()) return;
    // TODO: check if active bank is the bank to be removed and change active index accordingly
    // TODO: do not allow to remove all banks!
    sample_rom["wt_banks"].Erase(sample_rom["wt_banks"].Begin() + index);
    storeJSON(sample_rom, sampleRomDescFileName_);
}

void CTAG::SP::ctagSampleRomModel::RemoveSampleBank(const uint32_t index){
    // remove filename from smp bank array
    if (!sample_rom.IsObject()) return;
    if (!sample_rom.HasMember("smp_banks")) return;
    if (!sample_rom["smp_banks"].IsArray()) return;
    if (index >= sample_rom["smp_banks"].GetArray().Size()) return;
    // TODO: check if active bank is the bank to be removed and change active index accordingly
    // TODO: do not allow to remove all banks!
    sample_rom["smp_banks"].Erase(sample_rom["smp_banks"].Begin() + index);
    storeJSON(sample_rom, sampleRomDescFileName_);
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberWTSlices(){
    if(!desc_wt.IsArray()) return 0;
    return desc_wt.GetArray().Size();
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberSampleSlices(){
    if(!desc_smp.IsArray()) return 0;
    return desc_smp.GetArray().Size();
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberWTSamples(){
    uint32_t totalNumberSamples = 0;
    if(desc_wt.IsArray()){
        for(auto& v : desc_wt.GetArray()){
            if(v.HasMember("nsamples") && v["nsamples"].IsUint()){
                totalNumberSamples += v["nsamples"].GetUint();
            }
        }
    }
    return totalNumberSamples;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetTotalNumberSampleSamples(){
    uint32_t totalNumberSamples = 0;
    if(desc_smp.IsArray()){
        for(auto& v : desc_smp.GetArray()){
            if(v.HasMember("nsamples") && v["nsamples"].IsUint()){
                totalNumberSamples += v["nsamples"].GetUint();
            }
        }
    }
    return totalNumberSamples;
}

std::string CTAG::SP::ctagSampleRomModel::GetFilenameForWTSlice(uint32_t slice){
    std::string filename = "";
    if(desc_wt.IsArray()){
        if(slice < desc_wt.GetArray().Size()){
            Value& v = desc_wt[slice];
            if(v.HasMember("filename") && v["filename"].IsString()){
                filename = v["filename"].GetString();
            }
            if(v.HasMember("path") && v["path"].IsString()){
                filename = std::string(SD_CARD_SAMPLE_FOLDER) + "/" + std::string(v["path"].GetString()) + "/" + std::string(v["filename"].GetString()) + ".wav";
            }
        }
    }
    return filename;
}

std::string CTAG::SP::ctagSampleRomModel::GetFilenameForSampleSlice(uint32_t slice){
    std::string filename = "";
    if(desc_smp.IsArray()){
        if(slice < desc_smp.GetArray().Size()){
            Value& v = desc_smp[slice];
            if(v.HasMember("filename") && v["filename"].IsString()){
                filename = v["filename"].GetString();
            }
            if(v.HasMember("path") && v["path"].IsString()){
                filename = std::string(SD_CARD_SAMPLE_FOLDER) + "/" + std::string(v["path"].GetString()) + "/"  + std::string(v["filename"].GetString()) + ".wav";
            }
        }
    }
    return filename;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetDataOffsetForWTSlice(uint32_t slice){
    uint32_t offset = 44; // standard offset in .wav files
    if(desc_wt.IsArray()){
        if(slice < desc_wt.GetArray().Size()){
            Value& v = desc_wt[slice];
            if(v.HasMember("offset") && v["offset"].IsUint()){
                offset = v["offset"].GetUint();
            }
        }
    }
    return offset;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetDataOffsetForSampleSlice(uint32_t slice){
    uint32_t offset = 44; // standard offset in .wav files
    if(desc_smp.IsArray()){
        if(slice < desc_smp.GetArray().Size()){
            Value& v = desc_smp[slice];
            if(v.HasMember("offset") && v["offset"].IsUint()){
                offset = v["offset"].GetUint();
            }
        }
    }
    return offset;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetWTSliceSize(uint32_t slice){
    uint32_t size = 0;
    if(desc_wt.IsArray()){
        if(slice < desc_wt.GetArray().Size()){
            Value& v = desc_wt[slice];
            if(v.HasMember("nsamples") && v["nsamples"].IsUint()){
                size = v["nsamples"].GetUint();
            }
        }
    }
    return size;
}

uint32_t CTAG::SP::ctagSampleRomModel::GetSampleSliceSize(uint32_t slice){
    uint32_t size = 0;
    if(desc_smp.IsArray()){
        if(slice < desc_smp.GetArray().Size()){
            Value& v = desc_smp[slice];
            if(v.HasMember("nsamples") && v["nsamples"].IsUint()){
                size = v["nsamples"].GetUint();
            }
        }
    }
    return size;
}