#include "ctagSoundProcessorBjorklund.hpp"
#include "helpers/ctagNumUtil.hpp"

#include "plaits/dsp/engine/engine.h"

using namespace CTAG::SP;

#define GATE_HIGH_NEW       2
#define GATE_LOW_NEW        -1

// --- Replace function-call of frequency-conversion with macro for increasing speed just a bit ---
#define noteToFreq(incoming_note) (HELPERS::fastpow2 ((incoming_note - 69.f) / 12.f) *440.f)

// --- Additional Macro for automated parameter evaluations ---
#define MK_TRIG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname);

// --- Modify sine-wave for Squarewave/PWM or various modulations (including Pitch-Mod, Filter-Mod, Z-Scan and Vector-Modulation) ---
#define SINE_TO_SQUARE(sine_val)                      sine_val = (sine_val >= 0) ? 1.f : -1.f;

// --- VULT "Library for TBD" ---
#include "../vult/vult_lib4tbd.cpp"
#include "../vult/vultin.cpp"


// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorBjorklund::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id )
{
  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    if((!data.trig[trig_myparm]) != prev_trig_state[prev_trig_state_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[prev_trig_state_id] = !data.trig[trig_myparm];       // Remember status
      if (data.trig[trig_myparm] == 0)                      // HIGH if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW_NEW);           // Trigger released
    }
  }
  else                        // We may have a trigger set by activating the button via the GUI
  {
    if (my_parm != prev_trig_state[prev_trig_state_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[prev_trig_state_id] = my_parm;       // Remember status
      if(my_parm != 0)                   // LOW if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW_NEW);           // Trigger released
    }
  }
  return(prev_trig_state[prev_trig_state_id]);            // No change (1 for active, 0 for inactive)
}


// --- ###
// https://www.musicdsp.org/en/latest/Filters/110-formant-filter.html

/*
Public source code by alex@smartelectronix.com
Simple example of implementation of formant filter
Vowelnum can be 0,1,2,3,4 <=> A,E,I,O,U
Good for spectral rich input like saw or square
*/
//-------------------------------------------------------------VOWEL COEFFICIENTS
const double coeff[5][11]= {
        { 8.11044e-06, 8.943665402,        -36.83889529,   92.01697887,    -154.337906,    181.6233289,
                -151.8651235,   89.09614114,        -35.10298511,   8.388101016,    -0.923313471  ///A
        },
        {4.36215e-06,
                8.90438318, -36.55179099,   91.05750846,    -152.422234,    179.1170248,  ///E
                -149.6496211,87.78352223,   -34.60687431,   8.282228154,    -0.914150747
        },
        { 3.33819e-06,
                8.893102966,        -36.49532826,   90.96543286,    -152.4545478,   179.4835618,
                -150.315433,        88.43409371,    -34.98612086,   8.407803364,    -0.932568035  ///I
        },
        {1.13572e-06,
                8.994734087,        -37.2084849,    93.22900521,    -156.6929844,   184.596544,   ///O
                -154.3755513,       90.49663749,    -35.58964535,   8.478996281,    -0.929252233
        },
        {4.09431e-07,
                8.997322763,        -37.20218544,   93.11385476,    -156.2530937,   183.7080141,  ///U
                -153.2631681,       89.59539726,    -35.12454591,   8.338655623,    -0.910251753
        }
};
//---------------------------------------------------------------------------------
static double memory[10]={0,0,0,0,0,0,0,0,0,0};
//---------------------------------------------------------------------------------
float formant_filter(float in, int vowelnum)
{
  // ### float res= (float) ( coeff[vowelnum][0]* (*in) +
  float res = (float) ( coeff[vowelnum][0]* in +
                 coeff[vowelnum][1]  *memory[0] +
                 coeff[vowelnum][2]  *memory[1] +
                 coeff[vowelnum][3]  *memory[2] +
                 coeff[vowelnum][4]  *memory[3] +
                 coeff[vowelnum][5]  *memory[4] +
                 coeff[vowelnum][6]  *memory[5] +
                 coeff[vowelnum][7]  *memory[6] +
                 coeff[vowelnum][8]  *memory[7] +
                 coeff[vowelnum][9]  *memory[8] +
                 coeff[vowelnum][10] *memory[9] );

  memory[9]= memory[8];
  memory[8]= memory[7];
  memory[7]= memory[6];
  memory[6]= memory[5];
  memory[5]= memory[4];
  memory[4]= memory[3];
  memory[3]= memory[2];
  memory[2]= memory[1];
  memory[1]= memory[0];
  memory[0]=(double) res;
  return res;
}
// --- ###


void ctagSoundProcessorBjorklund::Process(const ProcessData &data)
{
  float f_val_result;

  MK_TRIG_PAR(t_Saturate, Saturate);
  MK_FLT_PAR(f_Tune, Tune, 2048.f, 1.f);
  MK_FLT_PAR(f_MasterPitch, MasterPitch, 2048.f, 1.f);
  MK_FLT_PAR(f_Cutoff, Cutoff, 4095.f, 1.f);
  MK_FLT_PAR(f_Resonance, Resonance, 4095.f, 5.f);

  MK_FLT_PAR(i_CC_val, Volume, 4095.f, 128.f);
  CONSTRAIN(i_CC_val, 0, 127);

  MK_INT_PAR_ABS(i_Bank, Bank, 32.f);
  CONSTRAIN(i_Bank, 0, 31);
  MK_INT_PAR_ABS(i_Slice, Slice, 32.f);
  CONSTRAIN(i_Slice, 0, 31);

  sineOsc.SetFrequency( fabsf(f_Tune*3000.f) ); // ### experimental !!!

  static unsigned int my_x = 0;

  if( t_Saturate == GATE_HIGH_NEW )
    my_x = ++my_x % 12;

  Phasedist_controlChange(pd_data, 31, i_CC_val, 0);

  for(uint32_t i = 0; i < bufSz; i++)
  {
    f_val_result = sineOsc.Process();
    SINE_TO_SQUARE(f_val_result);
    int vowel_id = i_Bank  % 5;  // hack to get 5 vowels, to be changed! ###
    if(t_Saturate)                      // Just an on/off switch ###
    {
      Phasedist_noteOn(pd_data, 48+my_x, 110, 0);
      f_val_result = Phasedist_process(pd_data,0);
      if( i_Slice )
        f_val_result = formant_filter(f_val_result, vowel_id );
      // f_val_result = Ladder_process_euler(ladder_euler, f_val_result, f_Cutoff, f_Resonance); // Svf_process(my_val, f_val_result , f_Cutoff, f_Resonance, 0);
    }
    else
    {
      Phasedist_noteOff(pd_data, 48, 0);
      f_val_result = 0.f; // Phasedist_process(pd_data,0);
    }
    data.buf[i * 2 + processCh] = f_val_result;            // Mono channel
  }
}

ctagSoundProcessorBjorklund::ctagSoundProcessorBjorklund()
{
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    sineOsc.SetSampleRate(44100.f);
    sineOsc.SetFrequency(220.f);

  // Svf__ctx_type_4 my_val;
  // Svf__ctx_type_4_init(my_val);

  Ladder__ctx_type_6_init(ladder_euler);

  Phasedist_process_init(pd_data);
  Phasedist_default(pd_data);
}

ctagSoundProcessorBjorklund::~ctagSoundProcessorBjorklund()
{
}

void ctagSoundProcessorBjorklund::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("Run", [&](const int val){ Run = val;});
	pMapTrig.emplace("Run", [&](const int val){ trig_Run = val;});
	pMapPar.emplace("MasterPitch", [&](const int val){ MasterPitch = val;});
	pMapCv.emplace("MasterPitch", [&](const int val){ cv_MasterPitch = val;});
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("Bank", [&](const int val){ Bank = val;});
	pMapCv.emplace("Bank", [&](const int val){ cv_Bank = val;});
	pMapPar.emplace("Slice", [&](const int val){ Slice = val;});
	pMapCv.emplace("Slice", [&](const int val){ cv_Slice = val;});
	pMapPar.emplace("Speed", [&](const int val){ Speed = val;});
	pMapCv.emplace("Speed", [&](const int val){ cv_Speed = val;});
	pMapPar.emplace("Tune", [&](const int val){ Tune = val;});
	pMapCv.emplace("Tune", [&](const int val){ cv_Tune = val;});
	pMapPar.emplace("Cutoff", [&](const int val){ Cutoff = val;});
	pMapCv.emplace("Cutoff", [&](const int val){ cv_Cutoff = val;});
	pMapPar.emplace("Resonance", [&](const int val){ Resonance = val;});
	pMapCv.emplace("Resonance", [&](const int val){ cv_Resonance = val;});
	pMapPar.emplace("Saturate", [&](const int val){ Saturate = val;});
	pMapTrig.emplace("Saturate", [&](const int val){ trig_Saturate = val;});
	isStereo = false;
	id = "Bjorklund";
	// sectionCpp0
}