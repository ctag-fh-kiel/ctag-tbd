//
// Created by Robert Manzke on 04.09.20.
//

#include "SimStimulus.hpp"
#include <string>


SimStimulus::SimStimulus() {

}

SimStimulus::~SimStimulus() {

}

void SimStimulus::Process(float *cvpot, uint8_t *trig) {
    if(modeMutex.try_lock()){
        for(int i=2;i<6;i++){
            cvpot[i-2] = s[i].Process();
        }
        trig[0] = s[0].Process() >= 0.5f ? 0 : 1;
        trig[1] = s[1].Process() >= 0.5f ? 0 : 1;
        modeMutex.unlock();
    }
}

void SimStimulus::UpdateStimulus(const int *mode, const int *value) {
    modeMutex.lock();
    for(int i=0;i<6;i++){
        s[i].SetMode(mode[i]);
        s[i].SetValue(value[i]);
    }
    modeMutex.unlock();
}
