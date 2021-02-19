#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "helpers/ctagADEnv.hpp"            // Needed for AD EG (Attack/Decay Envelope Generator)
#include "helpers/ctagADSREnv.hpp"          // Needed for ADSR EG (Attack/Decay/Sustain/Release Envelope Generator)


// --- VULT "Library for TBD" ---
#include "../vult/vult_lib4tbd.h"
#include "../vult/vult_lib4tbd.tables.h"
#include "../vult/vultin.h"


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
            int i_FormantSelect_save = 0;
            float formant_filter_l(float input_for_filter, int vowel_id);   // vowel_ids are 0...4 for A, E, I, O, U
            float formant_filter_c(float input_for_filter, int vowel_id);   // vowel_ids are 0...4 for A, E, I, O, U
            float formant_filter_h(float input_for_filter, int vowel_id);   // vowel_ids are 0...4 for A, E, I, O, U
            const double coeff[5][11]=
            { // --- A, E, I, O, U ---
              { 8.11044e-06, 8.943665402, -36.83889529, 92.01697887, -154.337906, 181.6233289, -151.8651235, 89.09614114, -35.10298511, 8.388101016, -0.923313471 },
              { 4.36215e-06, 8.90438318, -36.55179099, 91.05750846, -152.422234, 179.1170248, -149.6496211, 87.78352223, -34.60687431, 8.282228154, -0.914150747 },
              { 3.33819e-06, 8.893102966, -36.49532826, 90.96543286, -152.4545478, 179.4835618, -150.315433, 88.43409371, -34.98612086, 8.407803364, -0.932568035 },
              { 1.13572e-06, 8.994734087, -37.2084849, 93.22900521, -156.6929844, 184.596544, -154.3755513, 90.49663749, -35.58964535, 8.478996281, -0.929252233 },
              { 4.09431e-07, 8.997322763, -37.20218544, 93.11385476, -156.2530937, 183.7080141, -153.2631681, 89.59539726, -35.12454591, 8.338655623, -0.910251753 }
            };
            double vowel_m_l[10] = {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}; // Formant one lower than current formant
            double vowel_m_c[10] = {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}; // Current Formant (blending e.g. A<->E<->I or e.g. O<->U<->A or e.g. U<->A<->E)
            double vowel_m_h[10] = {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}; // Formant one higher than current formant

            // --- Keyboard logic[s] to switch formants ---
            int formant_trigger[12] = {-1,0,-1,1,-1,-1,2,-1,3,-1,4,-1};   // Black keys on a keyboard
            int formant_selected = 0;   // We remember the most recent trigger-key of the formant here
            int note_save = 0;          // We remember the last note, in case we select formants via black keys...

            // --- Remember status of triggers / buttons ---
            inline int process_param_trig( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id ); // rescale incoming data to bool
            enum trig_states
            {
                e_Gate, e_EGvolActive, e_EGvolSlow, e_FormantBlendingOn, e_FormantFilterOn, e_BlackKeyLogic, e_FormantLock, e_ADSRon, e_Formantor_options_max
            };
            int prev_trig_state[e_Formantor_options_max] = {0};   // Initialize _all_ entries with "low value"

            // --- VULT Stuff ---
            Phasedist_process_type pd_data;        // VULT PD synth voice

            // --- Volume EG --
            ctagADEnv vol_eg_ad;
            ctagADSREnv vol_eg_adsr;

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
	atomic<int32_t> Gate, trig_Gate;
	atomic<int32_t> MasterPitch, cv_MasterPitch;
	atomic<int32_t> Volume, cv_Volume;
	atomic<int32_t> PDamount, cv_PDamount;
	atomic<int32_t> FormantFilterOn, trig_FormantFilterOn;
	atomic<int32_t> BlackKeyLogic, trig_BlackKeyLogic;
	atomic<int32_t> FormantLock, trig_FormantLock;
	atomic<int32_t> FormantSelect, cv_FormantSelect;
	atomic<int32_t> FormantBlendingOn, trig_FormantBlendingOn;
	atomic<int32_t> FormantBlend, cv_FormantBlend;
	atomic<int32_t> TremoloActive, trig_TremoloActive;
	atomic<int32_t> TremoloAfterFormant, trig_TremoloAfterFormant;
	atomic<int32_t> TremoloAttack, cv_TremoloAttack;
	atomic<int32_t> TremoloSpeed, cv_TremoloSpeed;
	atomic<int32_t> TremoloAmount, cv_TremoloAmount;
	atomic<int32_t> EGvolActive, trig_EGvolActive;
	atomic<int32_t> EGvolSlow, trig_EGvolSlow;
	atomic<int32_t> Attack, cv_Attack;
	atomic<int32_t> Decay, cv_Decay;
	atomic<int32_t> ADSRon, trig_ADSRon;
	atomic<int32_t> Sustain, cv_Sustain;
	atomic<int32_t> Release, cv_Release;
	// sectionHpp
        };
    }
}