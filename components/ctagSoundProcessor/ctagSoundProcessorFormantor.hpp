#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "helpers/ctagADEnv.hpp"            // Needed for AD EG (Attack/Decay Envelope Generator)
#include "helpers/ctagADSREnv.hpp"          // Needed for ADSR EG (Attack/Decay/Sustain/Release Envelope Generator)
#include "helpers/ctagWNoiseGen.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagFastMath.hpp"

// --- VULT "Library for TBD" ---
#include "./vult/vult_formantor.h"
#include "./vult/vult_formantor.tables.h"

using namespace CTAG::SP::HELPERS;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorFormantor : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            ctagSoundProcessorFormantor();
            virtual ~ctagSoundProcessorFormantor();

        private:
            virtual void knowYourself() override;

            // --- Vowel/formant filter ---
            float formant_filter(float input_for_filter);   // vowel_ids are 0...4 for A, E, I, O, U -> we use seperate arrays for faster processing!
            int i_FormantSelect_save = 0;         // This is a buffer variable in case we allow switching of formants only on note-change
            const double* coeff_array[12] = { coeff_a, coeff_a2, coeff_a3, coeff_e, coeff_e2,
                                              coeff_i, coeff_i2, coeff_i3, coeff_o, coeff_o2, coeff_o3, coeff_u };
            float vowel_mem[10] = {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}; // Memory for current formant of vowel-filter
            const double coeff_a[11] = { 8.11044e-06, 8.943665402, -36.83889529, 92.01697887, -154.337906, 181.6233289, -151.8651235, 89.09614114, -35.10298511, 8.388101016, -0.923313471 };
            // Alternatively for first a-value: 3.11044e-06
            const double coeff_e[11] = { 4.36215e-06, 8.90438318, -36.55179099, 91.05750846, -152.422234, 179.1170248, -149.6496211, 87.78352223, -34.60687431, 8.282228154, -0.914150747 };
            const double coeff_i[11] = { 3.33819e-06, 8.893102966, -36.49532826, 90.96543286, -152.4545478, 179.4835618, -150.315433, 88.43409371, -34.98612086, 8.407803364, -0.932568035 };
            const double coeff_o[11] = { 1.13572e-06, 8.994734087, -37.2084849, 93.22900521, -156.6929844, 184.596544, -154.3755513, 90.49663749, -35.58964535, 8.478996281, -0.929252233 };
            const double coeff_u[11] = { 4.09431e-07, 8.997322763, -37.20218544, 93.11385476, -156.2530937, 183.7080141, -153.2631681, 89.59539726, -35.12454591, 8.338655623, -0.910251753 };
            const double* coeff_cur = (const double *)coeff_a;
            inline void formant_filter_set_formant(int formant_idx) {coeff_cur = coeff_array[formant_idx];}  // Used to set the current vowel before the main loop

            // --- Keyboard logic[s] to switch formants ---
            int formant_trigger[12] = {-1,0,-1,1,-1,-1,2,-1,3,-1,4,-1}; // Select one of 5 formants (fix or random) per key from keyboard
            int formant_selected = 0;       // We remember the most recent trigger-key of the formant here
            int i_note_save = 36;          // We remember the last note, in case we select formants via black keys...
            float f_note_save = 36.f;      // We remember the last note, in case we select formants via black keys...

            // --- Remember status of triggers / buttons ---
            inline int process_param_trig( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id, int gate_type ); // rescale incoming data to bool
            enum trig_states
            {
                e_Gate, e_EGvolActive, e_EGvolSlow, e_FormantBlendingOn,
                e_FormantRndNew, e_ResCombOn, e_SQWon, e_CrossModOn, e_ResCombBeforeFormants,
                e_FormantFilterOn, e_KeyLogic, e_FormantLock, e_ADSRon, e_Formantor_options_max
            };
            int prev_trig_state[e_Formantor_options_max] = {0};   // Initialize _all_ entries with "low value"
            bool low_reached[e_Formantor_options_max] = {false};  // We need this for look for toggle-events

            // --- Additional oscillator[s] ---
            ctagSineSource oscPWM;

            // --- LFOs ---
            ctagSineSource lfoPWM;
            ctagSineSource lfoTremolo;

            // --- VULT Stuff ---
            Phasedist_real_process_type pd_data;        // VULT PD synth voice internal datastructure, also needed for initialisation
            Svf__ctx_type_4 svf_data_x;                 // State Variable Filter (Bandpass) init (svf_data_x,y,z... for 3 bandpasses);
            Svf__ctx_type_4 svf_data_y;
            Svf__ctx_type_4 svf_data_z;

            Rescomb__ctx_type_6 rescomb_data;           // Resonator (comb-filter) data-structure
            Saw_eptr__ctx_type_0 saw_data;              // SAW Osc data-structure

            // --- Formant Parmeters for 3 BP-filters ---
            float f_CutOffXarray[12] = {0.f}; // Cutoff frequency values for 5 formants with 3 BP filters
            float f_CutOffYarray[12] = {0.f};
            float f_CutOffZarray[12] = {0.f};

            float f_ResoXarray[12] = {0.f};  // Resonance values for 5 formants with 3 BP filters
            float f_ResoYarray[12] = {0.f};
            float f_ResoZarray[12] = {0.f};

            float f_FltAmntXarray[12] = {0.f};  // Volume values for 5 formants with 3 BP filters
            float f_FltAmntYarray[12] = {0.f};
            float f_FltAmntZarray[12] = {0.f};

            // --- Find random formants by setting parameters for 5*3 BP-filters ---
            void random_bp_filter_settings(int set_num=-1);

            // --- Volume EG --
            ctagADEnv vol_eg_ad;
            ctagADSREnv vol_eg_adsr;

            // --- Random Source ---
            ctagWNoiseGen rndVal;

            // --- Based on Open Source by Autodafe to double alex@smartelectronix.com's formants by interpolation: https://github.com/antoniograzioli/Autodafe/blob/master/src/FormantFilter.cpp ---
            inline double CosineInterpolate( double y1,double y2, double mu)
            {
              double mu2;
              mu2 = (1-cos(mu*M_PI))/2;
              return(y1*(1-mu2)+y2*mu2);
            }
            const double coeff_a2[11] =  {  CosineInterpolate(3.11044e-06, 4.36215e-06,0.3333),
                                            CosineInterpolate(8.943665402, 8.90438318,0.3333),
                                            CosineInterpolate(-36.83889529, -36.55179099,0.3333),
                                            CosineInterpolate(92.01697887, 91.05750846,0.3333),
                                            CosineInterpolate(-154.337906, -152.422234,0.3333),
                                            CosineInterpolate(181.6233289, 179.1170248,0.3333),
                                            CosineInterpolate(-151.8651235, -149.6496211,0.3333),
                                            CosineInterpolate(89.09614114, 87.78352223,0.3333),
                                            CosineInterpolate(-35.10298511, -34.60687431,0.3333),
                                            CosineInterpolate(8.388101016, 8.282228154,0.3333),
                                            CosineInterpolate(-0.923313471, -0.914150747,0.3333)   };

            const double coeff_a3[11] =  {  CosineInterpolate(3.11044e-06, 4.36215e-06,0.6666),
                                            CosineInterpolate(8.943665402, 8.90438318,0.6666),
                                            CosineInterpolate(-36.83889529, -36.55179099,0.6666),
                                            CosineInterpolate(92.01697887, 91.05750846,0.6666),
                                            CosineInterpolate(-154.337906, -152.422234,0.6666),
                                            CosineInterpolate(181.6233289, 179.1170248,0.6666),
                                            CosineInterpolate(-151.8651235, -149.6496211,0.6666),
                                            CosineInterpolate(89.09614114, 87.78352223,0.6666),
                                            CosineInterpolate(-35.10298511, -34.60687431,0.6666),
                                            CosineInterpolate(8.388101016, 8.282228154,0.6666),
                                            CosineInterpolate(-0.923313471, -0.914150747,0.6666)   };

            const double coeff_e2[11] = {   CosineInterpolate(4.36215e-06, 3.33819e-06,0.5),
                                            CosineInterpolate(8.90438318, 8.893102966,0.5),
                                            CosineInterpolate(-36.55179099, -36.49532826,0.5),
                                            CosineInterpolate(91.05750846, 90.96543286,0.5),
                                            CosineInterpolate(-152.422234, -152.4545478,0.5),
                                            CosineInterpolate(179.1170248, 179.4835618,0.5),
                                            CosineInterpolate(-149.6496211, -150.315433,0.5),
                                            CosineInterpolate(87.78352223, 88.43409371,0.5),
                                            CosineInterpolate(-34.60687431, -34.98612086,0.5),
                                            CosineInterpolate(8.282228154, 8.407803364,0.5),
                                            CosineInterpolate(-0.914150747, -0.932568035,0.5)  };


            const double coeff_i2[11] = {   CosineInterpolate(3.33819e-06 , 1.13572e-06,0.3333),
                                            CosineInterpolate(8.893102966 , 8.994734087,0.3333),
                                            CosineInterpolate(-36.49532826 , -37.2084849,0.3333),
                                            CosineInterpolate(90.96543286 , 93.22900521,0.3333),
                                            CosineInterpolate(-152.4545478 , -156.6929844,0.3333),
                                            CosineInterpolate(179.4835618 , 184.596544,0.3333),
                                            CosineInterpolate(-150.315433 , -154.3755513,0.3333),
                                            CosineInterpolate(88.43409371 , 90.49663749,0.3333),
                                            CosineInterpolate(-34.98612086 , -35.58964535,0.3333),
                                            CosineInterpolate(8.407803364 , 8.478996281,0.3333),
                                            CosineInterpolate(-0.932568035 , -0.929252233,0.3333)  };

            const double coeff_i3[11] = {   CosineInterpolate(3.33819e-06 , 1.13572e-06,0.6666),
                                            CosineInterpolate(8.893102966 , 8.994734087,0.6666),
                                            CosineInterpolate(-36.49532826 , -37.2084849,0.6666),
                                            CosineInterpolate(90.96543286 , 93.22900521,0.6666),
                                            CosineInterpolate(-152.4545478 , -156.6929844,0.6666),
                                            CosineInterpolate(179.4835618 , 184.596544,0.6666),
                                            CosineInterpolate(-150.315433 , -154.3755513,0.6666),
                                            CosineInterpolate(88.43409371 , 90.49663749,0.6666),
                                            CosineInterpolate(-34.98612086 , -35.58964535,0.6666),
                                            CosineInterpolate(8.407803364 , 8.478996281,0.6666),
                                            CosineInterpolate(-0.932568035 , -0.929252233,0.6666)  };

            const double coeff_o2[11] =  {  CosineInterpolate(1.13572e-06 , 4.09431e-07,0.3333),
                                            CosineInterpolate(8.994734087 , 8.997322763,0.3333),
                                            CosineInterpolate(-37.2084849 , -37.20218544,0.3333),
                                            CosineInterpolate(93.22900521 , 93.11385476,0.3333),
                                            CosineInterpolate(-156.6929844 , -156.2530937,0.3333),
                                            CosineInterpolate(184.596544 , 183.7080141,0.3333),
                                            CosineInterpolate(-154.3755513 , -153.2631681,0.3333),
                                            CosineInterpolate(90.49663749 , 89.59539726,0.3333),
                                            CosineInterpolate(-35.58964535 , -35.12454591,0.3333),
                                            CosineInterpolate(8.478996281 , 8.338655623,0.3333),
                                            CosineInterpolate(-0.929252233 , -0.910251753,0.3333)  };

            const double coeff_o3[11] =  {  CosineInterpolate(1.13572e-06 , 4.09431e-07,0.6666),
                                            CosineInterpolate(8.994734087 , 8.997322763,0.6666),
                                            CosineInterpolate(-37.2084849 , -37.20218544,0.6666),
                                            CosineInterpolate(93.22900521 , 93.11385476,0.6666),
                                            CosineInterpolate(-156.6929844 , -156.2530937,0.6666),
                                            CosineInterpolate(184.596544 , 183.7080141,0.6666),
                                            CosineInterpolate(-154.3755513 , -153.2631681,0.6666),
                                            CosineInterpolate(90.49663749 , 89.59539726,0.6666),
                                            CosineInterpolate(-35.58964535 , -35.12454591,0.6666),
                                            CosineInterpolate(8.478996281 , 8.338655623,0.6666),
                                            CosineInterpolate(-0.929252233 , -0.910251753,0.6666)  };

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
	atomic<int32_t> Gate, trig_Gate;
	atomic<int32_t> MasterPitch, cv_MasterPitch;
	atomic<int32_t> MasterTune, cv_MasterTune;
	atomic<int32_t> QuantizePitch, trig_QuantizePitch;
	atomic<int32_t> Volume, cv_Volume;
	atomic<int32_t> VoicesDirectOut, trig_VoicesDirectOut;
	atomic<int32_t> PDPitch, cv_PDPitch;
	atomic<int32_t> PDTune, cv_PDTune;
	atomic<int32_t> PDamount, cv_PDamount;
	atomic<int32_t> SAWPitch, cv_SAWPitch;
	atomic<int32_t> SAWTune, cv_SAWTune;
	atomic<int32_t> SQWPitch, cv_SQWPitch;
	atomic<int32_t> SQWTune, cv_SQWTune;
	atomic<int32_t> PWMspeed, cv_PWMspeed;
	atomic<int32_t> PWMintensity, cv_PWMintensity;
	atomic<int32_t> CrossModOn, trig_CrossModOn;
	atomic<int32_t> PDxmod, cv_PDxmod;
	atomic<int32_t> SAWxmod, cv_SAWxmod;
	atomic<int32_t> SQWxmod, cv_SQWxmod;
	atomic<int32_t> PDvol, cv_PDvol;
	atomic<int32_t> SAWvol, cv_SAWvol;
	atomic<int32_t> SQWvol, cv_SQWvol;
	atomic<int32_t> FormantFilterOn, trig_FormantFilterOn;
	atomic<int32_t> KeyLogic, trig_KeyLogic;
	atomic<int32_t> FormantRndNew, trig_FormantRndNew;
	atomic<int32_t> FormantSelect, cv_FormantSelect;
	atomic<int32_t> FormantAmount, cv_FormantAmount;
	atomic<int32_t> ResCombOn, trig_ResCombOn;
	atomic<int32_t> ResCombBeforeFormants, trig_ResCombBeforeFormants;
	atomic<int32_t> ResFreq, cv_ResFreq;
	atomic<int32_t> ResTone, cv_ResTone;
	atomic<int32_t> ResQ, cv_ResQ;
	atomic<int32_t> ResAmount, cv_ResAmount;
	atomic<int32_t> TremoloActive, trig_TremoloActive;
	atomic<int32_t> TremoloAfterFormant, trig_TremoloAfterFormant;
	atomic<int32_t> TremoloAttack, cv_TremoloAttack;
	atomic<int32_t> TremoloSpeed, cv_TremoloSpeed;
	atomic<int32_t> TremoloAmount, cv_TremoloAmount;
	atomic<int32_t> TremoloResAmount, cv_TremoloResAmount;
	atomic<int32_t> EGvolActive, trig_EGvolActive;
	atomic<int32_t> EGvolSlow, trig_EGvolSlow;
	atomic<int32_t> Attack, cv_Attack;
	atomic<int32_t> Decay, cv_Decay;
	atomic<int32_t> ADSRon, trig_ADSRon;
	atomic<int32_t> Sustain, cv_Sustain;
	atomic<int32_t> Release, cv_Release;
	atomic<int32_t> EnvPDamount, cv_EnvPDamount;
	// sectionHpp
        };
    }
}