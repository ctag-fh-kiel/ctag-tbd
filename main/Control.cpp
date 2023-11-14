#include "Control.hpp"
#include "Calibration.hpp"
#include "gpio.hpp"
#include "adc.hpp"
#include "mk2.hpp"

#if CONFIG_TBD_PLATFORM_MK2
    // nothing to do here
#elif CONFIG_TBD_PLATFORM_BBA
    #include "midiuart.hpp"
    #define byte uint8_t
    #include "midiXparser/midiXparser.h"
    #define DATA_SZ  (N_CVS * 4 + N_TRIGS + 2)
    #define MIDI_CHANNEL_A 0
    #define MIDI_CHANNEL_B 1
    #define MIDI_NOTE_0V 48 // is C3

    typedef struct{
        midiXparser::midiStatusValue status;
        uint8_t data[3]; // one too much
    }mididata_t;

    // queue handle
    static QueueHandle_t midi_queue;
    DRAM_ATTR static uint8_t buf0[DATA_SZ];
    DRAM_ATTR static uint8_t *midi_note_trig = &buf0[N_CVS*4];
    DRAM_ATTR static float *midi_data = (float*) buf0;
    DRAM_ATTR static uint8_t midi_note_pitch[2] {0, 0};
    DRAM_ATTR static uint8_t midi_note_vel[2] {0, 0};
    DRAM_ATTR static uint8_t midi_prs[2] {0, 0};
    DRAM_ATTR static uint8_t midi_pb[2] {0, 0};
    DRAM_ATTR static uint8_t midi_mod[2*10] {0, 0};
    DRAM_ATTR static uint8_t msgBuffer[128];
    static midiXparser parser;
    static CTAG::DRIVERS::midiuart midi;

    static void bba_init(){
        memset(buf0, 0, DATA_SZ);
        memset(midi_note_trig, 1, N_TRIGS);
        parser.setMidiMsgFilter( midiXparser::channelVoiceMsgTypeMsk);
    }

    IRAM_ATTR static uint8_t *bba_update(){
        int len;
        midi.read(msgBuffer, &len);
        uint8_t *ptr {msgBuffer};
        while(len > 0){
            len--;
            if ( parser.parse( *ptr++ ) ) {
                uint8_t *msg = parser.getMidiMsg();
                //ESP_LOGI("MIDI", "Message status %d, data %02X %02X %02X", data.status, data.data[0], data.data[1], data.data[2]);
                if (parser.isMidiStatus(midiXparser::noteOnStatus)) {
                    // midi note too large
                    if (msg[1] > 120) continue;
                    // midi note too small
                    if (msg[1] < 36) continue;

                    // parse channels
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_A) {
                        //ESP_LOGI("MIDI", "Note on channel A");
                        // note off through velocity 0
                        if ((midi_note_pitch[0] == (msg[1] - MIDI_NOTE_0V)) && (msg[2] == 0)) {
                            //ESP_LOGI("MIDI", "Note off thru velocity");
                            midi_note_trig[0] = 1;
                            continue;
                        }
                        //ESP_LOGI("MIDI", "Note on channel A");
                        midi_note_trig[0] = 0;
                        midi_note_pitch[0] = msg[1] - MIDI_NOTE_0V;
                        midi_data[0] = midi_note_pitch[0] * 0.016666666666667f;
                        midi_note_vel[0] = msg[2];
                        midi_data[2] = midi_note_vel[0] * 0.007874015748031f;
                    }
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_B) {
                        // note off through velocity 0
                        if ((midi_note_pitch[1] == (msg[1] - MIDI_NOTE_0V)) && (msg[2] == 0)) {
                            midi_note_trig[1] = 1;
                            continue;
                        }
                        midi_note_trig[1] = 0;
                        midi_note_pitch[1] = msg[1] - MIDI_NOTE_0V;
                        midi_data[1] = midi_note_pitch[1] * 0.016666666666667f;
                        midi_note_vel[1] = msg[2];
                        midi_data[3] = midi_note_vel[1] * 0.007874015748031f;
                    }
                    continue;
                }

                // check for note off
                if (parser.isMidiStatus(midiXparser::noteOffStatus)) {
                    // midi note too large
                    if (msg[1] > 120) continue;
                    // midi note too small
                    if (msg[1] < 36) continue;
                    // parse channels
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_A) {
                        //ESP_LOGI("MIDI", "Note off channel A");
                        if (midi_note_pitch[0] == msg[1] - MIDI_NOTE_0V)
                            midi_note_trig[0] = 1;
                    }
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_B) {
                        if (midi_note_pitch[1] == msg[1] - MIDI_NOTE_0V)
                            midi_note_trig[1] = 1;
                    }
                    continue;
                }

                // check for aftertouch
                if (parser.isMidiStatus(midiXparser::channelPressureStatus)) {
                    // parse channels
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_A) {
                        //ESP_LOGI("MIDI", "Pressure channel A");
                        midi_prs[0] = msg[1];
                        midi_data[4] = midi_prs[0] * 0.007874015748031f;
                        midi_note_trig[2] = midi_prs[0] > 63 ? 0 : 1;
                        //ESP_LOGI("MIDI", "Pressure channel A %d %f", midi_mod[0], midi_data[6]);
                    }
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_B) {
                        midi_prs[1] = msg[1];
                        midi_data[5] = midi_prs[1] * 0.007874015748031f;
                        midi_note_trig[3] = midi_prs[1] > 63 ? 0 : 1;
                    }
                    continue;
                }
                // check for pitch bend
                if (parser.isMidiStatus(midiXparser::pitchBendStatus)) {
                    //ESP_LOGI("MIDI", "Pitch bend");
                    // parse channels
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_A) {
                        midi_pb[0] = msg[2];
                        midi_data[6] = midi_pb[0] * 0.007874015748031f;
                        //ESP_LOGI("MIDI", "Pitch bend channel A %d %f", midi_pb[0], midi_data[4]);
                    }
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_B) {
                        midi_pb[1] = msg[2];
                        midi_data[7] = midi_pb[1] * 0.007874015748031f;
                    }
                    continue;
                }

                // check for mod
                if (parser.isMidiStatus(midiXparser::controlChangeStatus)) {
                    // check if mod cc message, mod is controller #1
                    if (msg[1] != 1) continue;
                    // parse channels
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_A) {
                        //ESP_LOGI("MIDI", "Mod channel A");
                        midi_mod[0] = msg[2];
                        midi_data[8] = midi_mod[0] * 0.007874015748031f;
                        midi_note_trig[4] = midi_mod[0] > 63 ? 0 : 1;
                        //ESP_LOGI("MIDI", "Mod channel A %d %f", midi_mod[0], midi_data[6]);
                    }
                    if ((msg[0] & 0xF) == MIDI_CHANNEL_B) {
                        midi_mod[1] = msg[2];
                        midi_data[9] = midi_mod[1] * 0.007874015748031f;
                        midi_note_trig[5] = midi_mod[1] > 63 ? 0 : 1;
                    }
                    continue;
                }
            }
        }

        return buf0;
    }
#else
    uint8_t CTAG::CTRL::Control::trig_data[N_TRIGS];
    float CTAG::CTRL::Control::cv_data[N_CVS];
#endif

IRAM_ATTR void CTAG::CTRL::Control::Update(uint8_t **trigs, float **cvs) {
#if defined(CONFIG_TBD_PLATFORM_MK2)
    uint8_t *data = (uint8_t *) DRIVERS::mk2::Update();
    *cvs = (float*) data;
    *trigs = &data[N_CVS*4];
    /* for debug purposes
    uint16_t *magic_number = (uint16_t*) &data[98];
    if(*magic_number != 0xcafe){
        // Debug transmission
        printf("%5d ", *magic_number);
        for(int i=0;i<8;i++){
            printf("%d ", (*trigs)[i]);
        }
        for(int i=0;i<18;i++){
            printf("%.3f ", (*cvs)[i]);
        }
        printf("\n");
    }
     */
#elif defined(CONFIG_TBD_PLATFORM_BBA)
    uint8_t *data = bba_update();
    *cvs = (float*) data;
    *trigs = &data[N_CVS*4];
#else
// update CVs
    CTAG::DRIVERS::ADC::Update();
    CTAG::CAL::Calibration::MapCVData(CTAG::DRIVERS::ADC::data, cv_data);
    *cvs = cv_data;

// update trig data
    trig_data[0] = CTAG::DRIVERS::GPIO::GetTrig0();
    trig_data[1] = CTAG::DRIVERS::GPIO::GetTrig1();
    *trigs = trig_data;
#endif
}

void CTAG::CTRL::Control::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {
    // ifdefs to exclude this from BBA and MK2 are in Calibration.hpp
    CTAG::CAL::Calibration::ConfigCVChannels(v0 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v1 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v2 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v3 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar);
}

void CTAG::CTRL::Control::Init() {
    ESP_LOGI("Control", "Initializing control!");
#if CONFIG_TBD_PLATFORM_MK2
    DRIVERS::mk2::Init();
#elif CONFIG_TBD_PLATFORM_BBA
    bba_init();
#else
    DRIVERS::ADC::InitADCSystem();
    DRIVERS::GPIO::InitGPIO();
    CAL::Calibration::Init();
#endif
}
