#pragma once

//
//  watch this https://youtu.be/0oreYmOWgYE
//
//  Created by Nigel Redmon on 12/18/12.
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//
//  For a complete explanation of the ADSR envelope generator and code,
//  read the series of articles by the author, starting here:
//  http://www.earlevel.com/main/2013/06/01/envelope-generators/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code for your own purposes, free or commercial.
//
//  1.01  2016-01-02  njr   added calcCoef to SetTargetRatio functions that were in the ADSR widget but missing in this code
//  1.02  2017-01-04  njr   in calcCoef, checked for rate 0, to support non-IEEE compliant compilers
//  1.03  2020-04-08  njr   changed float to float; large target ratio and rate resulted in exp returning 1 in calcCoef
//
// used some optimization on the exp approximation, it has to be accurate for very small exponents

namespace CTAG {
    namespace SP {
        namespace HELPERS {
            class ctagADSREnv {
            public:
                ctagADSREnv(void);

                ~ctagADSREnv(void);

                float Process(void);

                float GetOutput(void);

                int GetState(void);

                void Gate(bool gate);

                void SetAttack(float rate);

                void SetDecay(float rate);

                void SetRelease(float rate);

                void SetSustain(float level);

                void SetTargetRatioA(float targetRatio);

                void SetTargetRatioDR(float targetRatio);

                void Reset(void);

                void SetSampleRate(float fs_hz);

                bool IsIdle();

                void SetModeLin();

                void SetModeExp();

                void Hold();


                enum envState {
                    env_idle = 0,
                    env_attack,
                    env_decay,
                    env_sustain,
                    env_release
                };

            protected:
                int state;
                float fs;
                float output;
                float attackRate;
                float decayRate;
                float releaseRate;
                float attackCoef;
                float decayCoef;
                float releaseCoef;
                float sustainLevel;
                float targetRatioA;
                float targetRatioDR;
                float attackBase;
                float decayBase;
                float releaseBase;

                float calcCoef(float rate, float targetRatio);
            };
        }
    }
}



