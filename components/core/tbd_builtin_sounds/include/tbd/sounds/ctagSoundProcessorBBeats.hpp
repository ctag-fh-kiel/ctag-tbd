/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020 by Robert Manzke. All rights reserved.

(c) 2020 by Mathias Brüssel for this "ByteBeatsXFade"-Plugin. All rights reserved.
Includes ByteBeat algorithms by Matt Wand - make sure to have a look at his other great work at: hot-air.bandcamp.com

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <atomic>
#include <tbd/sound_processor.hpp>
#include <tbd/sound_utils/ctagADEnv.hpp>            // Needed for AD EG (Attack/Decay Enveloppe Generator)

#define BEAT_A_MAX_IDX 52     // Max Index for list of ByteBeat 1
#define BEAT_B_MAX_IDX 55     // Max Index for list of ByteBeat 2

#define BEAT_MAX_PITCH 128          // Max Index for pitch
#define BEAT_PITCH_BOUNDARY 129     // Value needed to calculate slowing-down

namespace tbd::sounds {

[[tbd(name="BBeats", description="ByteBeats with Xfade")]]
struct SoundProcessorBBeats : audio::MonoSoundProcessor {

    virtual void Process(const audio::ProcessData&) override;
    virtual void Init(std::size_t blockSize, void *blockPtr) override;
    virtual ~SoundProcessorBBeats();

    /**
     * @brief foobar
     */
    bool test_dummy;

protected:

    int process_param( const audio::ProcessData&data, int cv_myparm, int my_parm, int parm_range, int max_idx ); // rescale incoming data
    float process_param_float( const audio::ProcessData&data, int cv_myparm, int my_parm, int max_idx ); // rescale incoming data to 0.0-1.0
    bool process_param_bool( const audio::ProcessData&data, int trig_myparm, int my_parm, int prev_trig_state_id, bool direct_eg_trigger=false ); // rescale incoming data to bool
    float logic_operation_on_beat( );   // Logical operation on the bytebeats

    enum trig_states {e_stop_beatA, e_reverse_beatA, e_stop_beatB, e_reverse_beatB, e_reset_bbeats_on_stop,
                      e_activateEG_1, e_loopEG_1, e_activateEG_2, e_loopEG_2, e_activateEG_3, e_loopEG_3, e_activateEG_4, e_loopEG_4, e_bbeat_options_max };
    uint8_t prev_trig_state[e_bbeat_options_max] = {1,1,1,1,1,1,1,1,1,1,1,1,1};
    bool toggle_trig_state[e_bbeat_options_max] = {false,false,false,false,false,false,false,false,false,false,false,false,false};
    bool direct_trigger[e_bbeat_options_max] = {false,false,false,false,false,false,false,false,false,false,false,false,false};
    uint16_t cv_counter = 0;    // A counter to slow down checking of CV and controllers from GUI

    // private attributes go here
    sound_utils::ctagADEnv env_1, env_2, env_3, env_4;   // AD EG

    // --- EG member variables ---
    bool eg_loop_1=false, eg_loop_2=false, eg_loop_3=false, eg_loop_4=false;
    bool eg_was_off_1=true, eg_was_off_2=true, eg_was_off_3=true, eg_was_off_4=true;
    uint8_t eg_dest_1=0, eg_dest_2=0, eg_dest_3=0, eg_dest_4=0;    // EG is off when 0, else modulations-destinations are stored here
    float eg_amount_1=1.f, eg_amount_2=1.f, eg_amount_3=1.f, eg_amount_4=1.f;
    float attackVal_1=0.f, attackVal_2=0.f, attackVal_3=0.f, attackVal_4=0.f;
    float decayVal_1=1.f, decayVal_2=1.f, decayVal_3=1.f, decayVal_4=1.f;

    // --- ByteBeatA member variables ---
    
    uint8_t beat_byte_A = 0;  // Currently calculated or temporarily stored ByteBeatA
    uint8_t beat_byte_B = 0;  // Currently calculated or temporarily stored ByteBeatB
    float beat_val_A = 0.f;      // Currently calculated or temporarily stored audio value for ByteBeatA
    float beat_val_B = 0.f;      // Currently calculated or temporarily stored audio value for ByteBeatB
    uint32_t t1 = 0;             // Iterator for ByteBeat1
    uint32_t t2 = 0;             // Iterator for ByteBeat2

    /** this is snafoo */
    int snafoo = 12;
  
    bool stop_beatA = false;     // BeatA will not play if stopped
    bool reset_beatA = false;    // BeatB will play from start again on restart if true
    bool reverse_beatA = false;  // True if ByteBeat1 is meant to play backwards
    uint16_t slow_down_A = 0;    // Speed counter for ByteBeat1
    int slow_down_A_factor = 0;  // Speed factor for ByteBeat1

    // --- ByteBeatB member variables ---
    bool stop_beatB = false;     // BeatB will not play if stopped
    bool reset_beatB = false;    // BeatB will play from start again on restart if true
    bool reverse_beatB = false;  // True if ByteBeat2 is meant to play backwards
    uint16_t slow_down_B = 0;    // Speed counter for ByteBeat2
    int slow_down_B_factor = 0;  // Speed factor for ByteBeat2

    int beat_index_A = 0;         // Used to decide which ByteBeat1 from the lists below has been selected by controller / CV
    int beat_index_B = 0;         // Used to decide which ByteBeat2 from the lists below has been selected by controller / CV

    // --- Mixer member variables ---
    float vca_vol = 0.f;       // This is the overall volume
    float xfade_val = 0.f;     // This is our value to Xfade between ByteBeatA and ByteBeatB

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
#pragma GCC diagnostic ignored "-Wparentheses"

    // --- List of lamdas, implementing the algorithms for Bytebeat 1 ---
    uint8_t (*beats_P1[BEAT_A_MAX_IDX+1])(uint32_t t)  // Modify or add your own ByteBeats below!
    {
      [](uint32_t t) -> uint8_t { return (uint8_t)((t&128)); },                       // This is a basic square-wave, toggelling between 0 and 128
      [](uint32_t t) -> uint8_t { return (uint8_t)(1893*t&8); },                      // Matt's waveform beats start here
      [](uint32_t t) -> uint8_t { return (uint8_t)(9893*t*(t/t*8)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*8)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*84)^990%t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*87)^990%t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t>>t|t<<245*2199); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((502%t*t|19191/t)%552&t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((9/109)-t^t<<48); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t<<5|t>>7)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t%114|t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t%119^t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<119^t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*120<<t%92); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*120<<t%90); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*1|t^119|t*99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t%8|t>>3|t&400)^t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<((t<<t)|(t>>t))<<2); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(6-t&7|t<<999^t*212/t<<2); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t^t%251); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t^t%449); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t^t%449)+22); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t^t%249)-22); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*((t>>12|t>>8)&63&t>>4)); },          // Matt's rhythmical beats start here
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*((t>>12|t>>8)&61&t>>11)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*((t>>12|t>>8)&59&t>>9)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(((t&15)*(-t&15)^!(t&16)-1)+32); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t%100>>t/200%100)*255); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(((43|4|25|66)/t>>89)+(t&2)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t/11)*(t/16|6+7)/9); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(((t*3|t>>361)+139)<<49); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(((t%16)<<6)/8|t%255|t-100); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((-t/100|(t*3))^(t*3&(t>>3))&t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((-t/100|(t&1))^(t*3&(t>>39))&t>>99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<6/(8080+t)&900+t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<2&(t*(7+(1^t>>6|9%5|4)))); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t%24^2&(t*(7+(1^t>>9|9%3|4)))); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t|(t>>9|t>>7))*t&(t>>11|t>>9)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t|(t>>11|t>>55))^t%((t>>1|t>>22)|t>>9)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*3&(t>>7)+t*9&(t*4<<1)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t>>3)|80%t*t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t>>t>>t>>t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t>>t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t&t)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t-t^t)^t|t^202&t|t<<183); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t-246|t+9*t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(((t^110/256)>>(t^956*8000/256&8)|t)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t-t%t)|t%158/112>>91|(t%(41*241<<t))^t>>8&t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t>>t+2)<<(8*t)*4); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*3|t/3|t*99|t/2); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*3|t/3|t*99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(44>>(t>>t+2)<<(8*t)*4); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t&t&129|233-244%t>>t); },
    };
    // --- List of lamdas, implementing the algorithms for Bytebeat 2 ---
    uint8_t (*beats_P2[BEAT_B_MAX_IDX+1])(uint32_t t)  // Modify or add your own ByteBeats below!
    {
      [](uint32_t t) -> uint8_t { return (uint8_t)(t&128); },                        // This is a basic square-wave, toggelling between 0 and 128
      [](uint32_t t) -> uint8_t { return (uint8_t)(1893*t&8); },                     // Matt's waveform beats start here (same as with ByteBeats1)
      [](uint32_t t) -> uint8_t { return (uint8_t)(9893*t*(t/t*8)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*8)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*84)^990%t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(9*t*(t/t*87)^990%t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t>>t|t<<245*2199); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((502%t*t|19191/t)%552&t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((9/109)-t^t<<48); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t<<5|t>>7)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t%114|t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t%119^t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<119^t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*120<<t%92); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*120<<t%90); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*1|t^119|t*99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t%8|t>>3|t&400)^t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<((t<<t)|(t>>t))<<2); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(6-t&7|t<<999^t*212/t<<2); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t^t%251); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t^t%449); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t^t%449)+22); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t^t%249)-22); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t&t/1219); },                      // Matt's rhythmical beats start here (different to ByteBeats1)
      [](uint32_t t) -> uint8_t { return (uint8_t)(4448-(t>>t^1)<<(8*t)*1); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*t+2>>t&t|(t^9)+1); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t%255^t%64); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t%(t|2+t-39)*1)>>(t&28)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*3|t^3|t*99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*333|t>>3|t*1); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*333|t>>3|t*11); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*3333|t>>3|t*11); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t^3333|t>>18|t/113); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((6622%t*484)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(7-t%7|t<<999|t*212%t<<3); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<5|t>>2&t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<(t>>t*169)|t>>t|t/20); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<5|t>>2-t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t&t/269%t|t/223); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t&t/261&t|t/225); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t&t/1269%t&t/223); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*(t<<1|t>>8)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t%100&t/200%100)*200); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(7-t%7|t<<999|t*212%t<<2); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t/111*t>>18*t/999)|t<<2); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t*t/38%216); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t^(98&t)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t^(93/t)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t+39)|t-t|t-(25*t)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)((t+39)|t&t|t-(22*t)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t+208/109-t|t^t|97%t>>t); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t^57+t%149|t%(251&103)); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t/333|t>>18*t/999); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t/666*t>>18*t/999); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t<<5|t>>2+t%99); },
      [](uint32_t t) -> uint8_t { return (uint8_t)(t-85-(98>>t)|(22*80-t)*t|t/215); },
    };
#pragma GCC diagnostic pop
    // autogenerated code here
    // sectionHpp
    std::atomic<int32_t> beatA_stop, trig_beatA_stop;
    std::atomic<int32_t> beatA_backwards, trig_beatA_backwards;
    std::atomic<int32_t> beatA_select, cv_beatA_select;
    std::atomic<int32_t> beatA_pitch, cv_beatA_pitch;
    std::atomic<int32_t> beatB_stop, trig_beatB_stop;
    std::atomic<int32_t> beatB_backwards, trig_beatB_backwards;
    std::atomic<int32_t> beatB_select, cv_beatB_select;
    std::atomic<int32_t> beatB_pitch, cv_beatB_pitch;
    std::atomic<int32_t> reset_bbeats_on_stop, trig_reset_bbeats_on_stop;
    std::atomic<int32_t> volume, cv_volume;
    std::atomic<int32_t> xFadeA_B, cv_xFadeA_B;
    std::atomic<int32_t> destinationEG_1, cv_destinationEG_1;
    std::atomic<int32_t> activateEG_1, trig_activateEG_1;
    std::atomic<int32_t> loopEG_1, trig_loopEG_1;
    std::atomic<int32_t> amountEG_1, cv_amountEG_1;
    std::atomic<int32_t> attackEG_1, cv_attackEG_1;
    std::atomic<int32_t> decayEG_1, cv_decayEG_1;
    std::atomic<int32_t> destinationEG_2, cv_destinationEG_2;
    std::atomic<int32_t> activateEG_2, trig_activateEG_2;
    std::atomic<int32_t> loopEG_2, trig_loopEG_2;
    std::atomic<int32_t> amountEG_2, cv_amountEG_2;
    std::atomic<int32_t> attackEG_2, cv_attackEG_2;
    std::atomic<int32_t> decayEG_2, cv_decayEG_2;
    std::atomic<int32_t> destinationEG_3, cv_destinationEG_3;
    std::atomic<int32_t> activateEG_3, trig_activateEG_3;
    std::atomic<int32_t> loopEG_3, trig_loopEG_3;
    std::atomic<int32_t> amountEG_3, cv_amountEG_3;
    std::atomic<int32_t> attackEG_3, cv_attackEG_3;
    std::atomic<int32_t> decayEG_3, cv_decayEG_3;
    std::atomic<int32_t> destinationEG_4, cv_destinationEG_4;
    std::atomic<int32_t> activateEG_4, trig_activateEG_4;
    std::atomic<int32_t> loopEG_4, trig_loopEG_4;
    std::atomic<int32_t> amountEG_4, cv_amountEG_4;
    std::atomic<int32_t> attackEG_4, cv_attackEG_4;
    std::atomic<int32_t> decayEG_4, cv_decayEG_4;
    // sectionHpp
};

}
