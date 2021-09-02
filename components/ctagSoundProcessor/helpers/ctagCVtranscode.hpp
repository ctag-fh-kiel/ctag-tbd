#pragma once
#ifndef CV_TRANSCODER
#define CV_TRANSCODER

#include "ctagFastMath.hpp"
#define USING_CTAG_FAST_MATH  true
using namespace CTAG::SP::HELPERS;

// =========================================================================================
// Transcode linear CV-input to exponential or logarithmic curves with variable strength,
// different input-ranges for such activities and variable output values.
// For simplicity and speed input-values must be scaled to 0..1 when calling the function
// Found and modified from here: https://playground.arduino.cc/Main/Fscale and here:
// https://forum.arduino.cc/t/scaling-logarithmic-signal-for-analogread/504238/7
// Floating Point Autoscale Function Paul Badger V0.1 2007 Modified from code by Greg Shakar
// =========================================================================================

#ifndef CONSTRAIN
#define CONSTRAIN(var, min, max) \
  if (var < (min)) { \
    var = (min); \
  } else if (var > (max)) { \
    var = (max); \
  }
#endif

class CVtranscoder
{
public:
  static void Init() {};

  // --- Input parameters: Curve: -10 to +10, 0 is linear, negative values are exponential, positive values are logarithmic ---
  // --- Please note: input values must be scaled to 0..1 already on input! If outMax < outMin or startPos > endPos, the result will be inverted ---
  // --- When the start-pos and end-pos are used for input: output will be set to 'outMin' or 'outMax' when outside of that value-range ---
  static float Process(float inputValue, float curve, float startPos = 0.f, float endPos = 1.f, float outMin = 0.f, float outMax = 1.f)
  {
    float NewRange = 0.f;
    float inputRange = 1.f;
    float rangedValue = 0.f;
    bool invFlag = false;

    if( startPos != 0.f || endPos != 1.f )   // MB 20210819: Check if we have a range of input to process
    {
      if( startPos > endPos )     // MB 20210819: if we have reversed start and end of incoming range: reverse output
      {
        float tmp_val = startPos;
        startPos = endPos;
        endPos = tmp_val;
        invFlag = true;
      }
      if (inputValue >= endPos)  // MB 20210819: if incoming value is 'behind' the range to be mapped give out max. value for output
        return (outMax);
      if (inputValue <= startPos)  // MB 20210819: if incoming value is 'before' the range to be mapped give out min. value for output
        return (outMin);
      inputValue = (inputValue - startPos) / (endPos - startPos); // MB 20210819: rescale input within range to standard-range of 0...1
    }
    CONSTRAIN(inputValue, 0.f, 1.f);
    CONSTRAIN(curve, -10.f, 10.f);
    curve = (curve * -.1) ; // invert and scale - this seems more intuitive - positive numbers give more weight to high end on output
    curve = powf_fast_precise(10, curve); // convert linear scale into logarithmic exponent for other pow function

    // Zero Reference the values
    if (outMax > outMin)
      NewRange = outMax - outMin;
    else
    {
      NewRange = outMin - outMax;
      invFlag = true;
    }
    if(!invFlag)
      rangedValue =  (powf_fast_precise(inputValue, curve) * NewRange) + outMin;
    else     // invert the ranges
      rangedValue =  outMin - (powf_fast_precise(inputValue, curve) * NewRange);

    CONSTRAIN(rangedValue, 0.f, 1.f);
    return rangedValue;
  }

private:
#ifndef USING_CTAG_FAST_MATH
  static float powf_fast_precise(float a, float b)      // Taken from ctagFastMath.cpp
  {
    int flipped = 0;
    if (b < 0)
    {
      flipped = 1;
      b = -b;
    }
    /* calculate approximation with fraction of the exponent */
    int e = (int) b;
    union { float f; int x; } u = { a };
    u.x = (int)((b - e) * (u.x - 1065353216) + 1065353216);

    float r = 1.0f;
    while (e)
    {
      if (e & 1)
        r *= a;
      a *= a;
      e >>= 1;
    }
    r *= u.f;
    return flipped ? 1.0f/r : r;
  }
#endif
};

#endif