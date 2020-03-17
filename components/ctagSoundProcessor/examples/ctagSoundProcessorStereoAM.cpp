#include "ctagSoundProcessorStereoAM.hpp"
#include <iostream>

using namespace CTAG::SP;

ctagSoundProcessorStereoAM::ctagSoundProcessorStereoAM()
{
    isStereo = true;
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    model->LoadPreset(0);
}

void ctagSoundProcessorStereoAM::Process(const ProcessData &data){
}

void ctagSoundProcessorStereoAM::setParamValueInternal(const string& id, const int val) {

}

ctagSoundProcessorStereoAM::~ctagSoundProcessorStereoAM(){
}

const char * ctagSoundProcessorStereoAM::GetCStrID() const{
    return id.c_str();
}
