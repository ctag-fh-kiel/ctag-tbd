#include "ctagSoundProcessorMSxxNoise.hpp"
using namespace CTAG::SP;

// --- Trigger/Gate values ---
#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

#define MAX_CUTOFF_FREQ     14000.f
#define MIN_LFO_SPEED       0.05f
#define MAX_LFO_SPEED       20.f

// --- Additional macros for oscillator and GUI-parameter processing ---
#define MK_TRIG_PAR(outname, inname)            int outname = process_param_trig(data, trig_##inname, inname, e_##inname);

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorMSxxNoise::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int enum_trigger_id, int gate_type = 0 )
{
  int trig_status = 0;

  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    trig_status = (data.trig[trig_myparm]==0); // HIGH is 0, so we negate for boolean logic
    if( gate_type == 1 )
      return(trig_status);

    if(trig_status)    // Statuschange from HIGH to LOW or LOW to HIGH? Startup-Status for prev_trig_state is -1, so first change is always new
    {
      if( low_reached[enum_trigger_id] )    // We had a trigger low before the new trigger high
      {
        if (prev_trig_state[enum_trigger_id] == GATE_LOW || gate_type==2 )   // Toggle or AD EG Trigger...
        {
          prev_trig_state[enum_trigger_id] = GATE_HIGH;       // Remember status for next round
          low_reached[enum_trigger_id] = false;
          return (GATE_HIGH_NEW);           // New trigger
        }
        else        // previous status was high!
        {
          prev_trig_state[enum_trigger_id] = GATE_LOW;       // Remember status for next round
          low_reached[enum_trigger_id] = false;
          return (GATE_LOW);           // New trigger
        }
      }
    }
    else
      low_reached[enum_trigger_id] = true;
  }
  else                        // We may have a trigger set by activating the button via the GUI
  {
    if (my_parm != prev_trig_state[enum_trigger_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[enum_trigger_id] = my_parm;       // Remember status
      if(my_parm != 0)                   // LOW if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW);              // Trigger released
    }
  }
  return(prev_trig_state[enum_trigger_id]);            // No change (1 for active, 0 for inactive)
}


void ctagSoundProcessorMSxxNoise::Process(const ProcessData &data)
{
  // === Process all parameters from the GUI ===
  // --- Soundsources ---
  MK_TRIG_PAR(t_ExternalActive, ExternalActive);
  MK_FLT_PAR_ABS(f_NoiseExternalBalance, NoiseExternalBalance, 4095.f, 1.f);
  MK_TRIG_PAR(t_PinkActive, PinkActive);
  MK_TRIG_PAR(t_WhiteActive, WhiteActive);
  MK_FLT_PAR_ABS(f_PinkWhiteBalance, PinkWhiteBalance, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 4.f);   // We use a high value to have headroom, so lower the volume normally

  // --- Soundmodifiers ---
  MK_TRIG_PAR(t_FilterBypass, FilterBypass);
  MK_TRIG_PAR(t_SaturationActive, SaturationActive);
  MK_FLT_PAR_ABS_MIN_MAX(f_Saturation, Saturation, 4095.f, 0.899f, 8.99f);
  MK_FLT_PAR_ABS(f_Cutoff, Cutoff, 4095.f, MAX_CUTOFF_FREQ);
  MK_FLT_PAR_ABS_MIN_MAX(f_Resonance, Resonance, 4095.f, 0.0002f, 1.99f);
  MK_TRIG_PAR(t_MGactive, MGactive);
  MK_TRIG_PAR(t_MGisSquare, MGisSquare);
  MK_FLT_PAR_ABS_MIN_MAX(f_MGspeed, MGspeed, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS(f_MGamnt, MGamnt, 4095.f, MAX_CUTOFF_FREQ);
  MK_TRIG_PAR(t_PhaserBypass, PhaserBypass);
  MK_FLT_PAR_ABS_MIN_MAX(f_PhaserDryWet, PhaserDryWet, 4095.f, 0.f, 100.f);

  // --- Phaser settings ---
  MK_TRIG_PAR(t_PhaserPreset, PhaserPreset);
  MK_FLT_PAR_ABS(f_PhaserColor, PhaserColor, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_PhaserLFOfrequency, PhaserLFOfrequency, 4095.f, 0.00999999978f,5.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_PhaserFeedbackDepth, PhaserFeedbackDepth, 4095.f, 0.f, 99.f );
  MK_FLT_PAR_ABS_MIN_MAX(f_PhaserFeedbackBassCut, PhaserFeedbackBassCut, 4095.f, 10.f, 5000.f);

  // === Precalculate values for DSP-processing ===
  // --- Precalculate internal/external sound and noise-mix ---
  float f_internal_sound = t_ExternalActive ? 1.f-f_NoiseExternalBalance : 1.f;
  float f_external_sound = t_ExternalActive ? f_NoiseExternalBalance : 0.f;
  float f_pinkAmnt = t_PinkActive ? (1.f-f_PinkWhiteBalance)*f_internal_sound : 0.f;
  float f_whiteAmnt = t_WhiteActive ? f_PinkWhiteBalance*f_internal_sound : 0.f;

  // --- LFO modulation of the filter cutoff ---
  if(!t_MGactive)
    f_MGamnt = 0.f;
  mg.SetFrequency(f_MGspeed);
  float mg_wave = mg.Process();
  if(t_MGisSquare)
    mg_wave = (mg_wave >= 0.f) ? 1.f : -1.f;
  f_Cutoff += mg_wave*f_MGamnt;
  CONSTRAIN( f_Cutoff, 0.f, MAX_CUTOFF_FREQ );

  // --- Apply filter-parameters ---
  if(!t_SaturationActive)
    f_Saturation = 0.f;     // Switch off saturator!
  wpKorg35.SetSaturation(f_Saturation);
  wpKorg35.SetCutoff(f_Cutoff);
  wpKorg35.SetResonance(f_Resonance);

  // --- Phaser ---
  if( t_PhaserPreset == GATE_HIGH_NEW )
    phaser.Init();      // Set to preset-defaults

  if(!t_PhaserPreset)   // Use individual settings instead
  {
    phaser.SetDryWet(f_PhaserDryWet);
    phaser.SetColor(f_PhaserColor);
    phaser.SetLFOfrequency(f_PhaserLFOfrequency);
    phaser.SetFeedbackDepth(f_PhaserFeedbackDepth);
    phaser.SetFeedbackBassCut(f_PhaserFeedbackBassCut);
  }
  // === Main DSP loops (depending on Bypass-settings) ===
  if(!t_FilterBypass && !t_PhaserBypass)
  {
    for (int i = 0; i < bufSz; i++)
      data.buf[i*2+processCh] = phaser.Process(wpKorg35.Process(data.buf[i*2+processCh]*f_external_sound + pNoise.Process()*f_pinkAmnt + wNoise.Process()*f_whiteAmnt))*f_Volume;
  }
  else if(!t_FilterBypass && t_PhaserBypass)
  {
    for (int i = 0; i < bufSz; i++)
      data.buf[i*2+processCh] = wpKorg35.Process(data.buf[i*2+processCh]*f_external_sound + pNoise.Process()*f_pinkAmnt + wNoise.Process()*f_whiteAmnt)*f_Volume;
  }
  else if(t_FilterBypass && !t_PhaserBypass)
  {
    for (int i = 0; i < bufSz; i++)
      data.buf[i*2+processCh] = phaser.Process(data.buf[i*2+processCh]*f_external_sound + pNoise.Process()*f_pinkAmnt + wNoise.Process()*f_whiteAmnt)*f_Volume;
  }
  else if(t_FilterBypass && t_PhaserBypass)
  {
    for (int i = 0; i < bufSz; i++)
      data.buf[i*2+processCh] = (data.buf[i*2+processCh]*f_external_sound + pNoise.Process()*f_pinkAmnt + wNoise.Process()*f_whiteAmnt)*f_Volume;
  }
}

void ctagSoundProcessorMSxxNoise::Init(std::size_t const &blockSize, void *const blockPtr)
{
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

  // --- Init LFO ---
  mg.SetSampleRate(44100.f / bufSz);   // Please note: because the LFO is applied already outside the DSP-loop we reduce it's frequency in a manner to fit
  mg.SetFrequency(1.f);
}

ctagSoundProcessorMSxxNoise::~ctagSoundProcessorMSxxNoise()
{
}

void ctagSoundProcessorMSxxNoise::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("ExternalActive", [&](const int val){ ExternalActive = val;});
	pMapTrig.emplace("ExternalActive", [&](const int val){ trig_ExternalActive = val;});
	pMapPar.emplace("NoiseExternalBalance", [&](const int val){ NoiseExternalBalance = val;});
	pMapCv.emplace("NoiseExternalBalance", [&](const int val){ cv_NoiseExternalBalance = val;});
	pMapPar.emplace("PinkActive", [&](const int val){ PinkActive = val;});
	pMapTrig.emplace("PinkActive", [&](const int val){ trig_PinkActive = val;});
	pMapPar.emplace("WhiteActive", [&](const int val){ WhiteActive = val;});
	pMapTrig.emplace("WhiteActive", [&](const int val){ trig_WhiteActive = val;});
	pMapPar.emplace("PinkWhiteBalance", [&](const int val){ PinkWhiteBalance = val;});
	pMapCv.emplace("PinkWhiteBalance", [&](const int val){ cv_PinkWhiteBalance = val;});
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("FilterBypass", [&](const int val){ FilterBypass = val;});
	pMapTrig.emplace("FilterBypass", [&](const int val){ trig_FilterBypass = val;});
	pMapPar.emplace("SaturationActive", [&](const int val){ SaturationActive = val;});
	pMapTrig.emplace("SaturationActive", [&](const int val){ trig_SaturationActive = val;});
	pMapPar.emplace("Saturation", [&](const int val){ Saturation = val;});
	pMapCv.emplace("Saturation", [&](const int val){ cv_Saturation = val;});
	pMapPar.emplace("Cutoff", [&](const int val){ Cutoff = val;});
	pMapCv.emplace("Cutoff", [&](const int val){ cv_Cutoff = val;});
	pMapPar.emplace("Resonance", [&](const int val){ Resonance = val;});
	pMapCv.emplace("Resonance", [&](const int val){ cv_Resonance = val;});
	pMapPar.emplace("MGactive", [&](const int val){ MGactive = val;});
	pMapTrig.emplace("MGactive", [&](const int val){ trig_MGactive = val;});
	pMapPar.emplace("MGisSquare", [&](const int val){ MGisSquare = val;});
	pMapTrig.emplace("MGisSquare", [&](const int val){ trig_MGisSquare = val;});
	pMapPar.emplace("MGspeed", [&](const int val){ MGspeed = val;});
	pMapCv.emplace("MGspeed", [&](const int val){ cv_MGspeed = val;});
	pMapPar.emplace("MGamnt", [&](const int val){ MGamnt = val;});
	pMapCv.emplace("MGamnt", [&](const int val){ cv_MGamnt = val;});
	pMapPar.emplace("PhaserBypass", [&](const int val){ PhaserBypass = val;});
	pMapTrig.emplace("PhaserBypass", [&](const int val){ trig_PhaserBypass = val;});
	pMapPar.emplace("PhaserDryWet", [&](const int val){ PhaserDryWet = val;});
	pMapCv.emplace("PhaserDryWet", [&](const int val){ cv_PhaserDryWet = val;});
	pMapPar.emplace("PhaserPreset", [&](const int val){ PhaserPreset = val;});
	pMapTrig.emplace("PhaserPreset", [&](const int val){ trig_PhaserPreset = val;});
	pMapPar.emplace("PhaserColor", [&](const int val){ PhaserColor = val;});
	pMapCv.emplace("PhaserColor", [&](const int val){ cv_PhaserColor = val;});
	pMapPar.emplace("PhaserLFOfrequency", [&](const int val){ PhaserLFOfrequency = val;});
	pMapCv.emplace("PhaserLFOfrequency", [&](const int val){ cv_PhaserLFOfrequency = val;});
	pMapPar.emplace("PhaserFeedbackDepth", [&](const int val){ PhaserFeedbackDepth = val;});
	pMapCv.emplace("PhaserFeedbackDepth", [&](const int val){ cv_PhaserFeedbackDepth = val;});
	pMapPar.emplace("PhaserFeedbackBassCut", [&](const int val){ PhaserFeedbackBassCut = val;});
	pMapCv.emplace("PhaserFeedbackBassCut", [&](const int val){ cv_PhaserFeedbackBassCut = val;});
	isStereo = false;
	id = "MSxxNoise";
	// sectionCpp0
}