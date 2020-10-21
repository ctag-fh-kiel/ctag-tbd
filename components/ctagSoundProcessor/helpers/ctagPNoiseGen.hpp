/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#pragma once

#include <cmath>
#include <cstdint>

// algorithm taken from http://www.firstpr.com.au/dsp/pink-noise/phil_burk_19990905_patest_pink.c

#define PINK_MAX_RANDOM_ROWS   (30)
#define PINK_RANDOM_BITS       (24)
#define PINK_RANDOM_SHIFT      ((sizeof(long)*8)-PINK_RANDOM_BITS)

namespace CTAG::SP::HELPERS{
    class ctagPNoiseGen {
    public:
        ctagPNoiseGen(){
            InitializePinkNoise(12);
        }
        /* Generate Pink noise values between -1.0 and +1.0 */
        float Process()
        {
            int32_t newRandom;
            int32_t sum;
            float output;

/* Increment and mask index. */
            pink.pink_Index = (pink.pink_Index + 1) & pink.pink_IndexMask;

/* If index is zero, don't update any random values. */
            if( pink.pink_Index != 0 )
            {
                /* Determine how many trailing zeros in PinkIndex. */
                /* This algorithm will hang if n==0 so test first. */
                int numZeros = 0;
                int n = pink.pink_Index;
                while( (n & 1) == 0 )
                {
                    n = n >> 1;
                    numZeros++;
                }

                /* Replace the indexed ROWS random value.
                 * Subtract and add back to RunningSum instead of adding all the random
                 * values together. Only one changes each time.
                 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
                pink.pink_RunningSum -= pink.pink_Rows[numZeros];
                newRandom = ((int32_t)GenerateRandomNumber()) >> PINK_RANDOM_SHIFT;
                pink.pink_RunningSum += newRandom;
                pink.pink_Rows[numZeros] = newRandom;
            }

/* Add extra white noise value. */
            newRandom = ((int32_t)GenerateRandomNumber()) >> PINK_RANDOM_SHIFT;
            sum = pink.pink_RunningSum + newRandom;
#pragma GCC diagnostic pop

/* Scale to range of -1.0 to 0.9999. */
            output = pink.pink_Scalar * sum;

            return output;
        }
        void ReSeed(uint32_t seed){
            randSeed = seed;
        }
    private:
        struct
        {
            int32_t      pink_Rows[PINK_MAX_RANDOM_ROWS];
            int32_t      pink_RunningSum;   /* Used to optimize summing of generators. */
            int32_t       pink_Index;        /* Incremented each sample. */
            int32_t       pink_IndexMask;    /* Index wrapped by ANDing with this mask. */
            float     pink_Scalar;       /* Used to scale within range of -1.0 to +1.0 */
        } pink;

        /* Setup PinkNoise structure for N rows of generators. */
        void InitializePinkNoise(int numRows )
        {
            int i;
            int32_t pmax;
            pink.pink_Index = 0;
            pink.pink_IndexMask = (1<<numRows) - 1;
/* Calculate maximum possible signed random value. Extra 1 for white noise always added. */
            pmax = (numRows + 1) * (1<<(PINK_RANDOM_BITS-1));
            pink.pink_Scalar = 1.0f / pmax;
/* Initialize rows. */
            for( i=0; i<numRows; i++ ) pink.pink_Rows[i] = 0;
            pink.pink_RunningSum = 0;
        }

        uint32_t randSeed = 22222;
        /* Calculate pseudo-random 32 bit number based on linear congruential method. */
        uint32_t GenerateRandomNumber()
        {
            randSeed = (randSeed * 196314165) + 907633515;
            return randSeed;
        }
    };
}

