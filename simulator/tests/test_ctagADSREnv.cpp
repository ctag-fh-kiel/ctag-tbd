//
// Created by Robert Manzke on 27.09.20.
//

#include "test_ctagADSREnv.hpp"
#include <iostream>

using namespace CTAG::TESTS;

void test_ctagADSREnv::DoTest(){
    int fs = 48000/32;
    adsr.SetSampleRate(static_cast<float>(fs));
    adsr.SetAttack(1.f);
    adsr.SetSustain(0.5f);
    adsr.SetDecay(1.f);
    adsr.SetRelease(1.f);
    adsr.SetModeExp();
    adsr.SetTargetRatioA(0.2);
    adsr.SetTargetRatioDR(0.0001);
    adsr.Gate(true);
    for(int i=0;i<fs*2 + 128;i++){
        std::cout << adsr.Process() << ", ";
    }
    adsr.Gate(false);
    for(int i=0;i<fs + 128;i++){
        std::cout << adsr.Process() << ", ";
    }
    std::cout << adsr.Process();

}