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
    DMA_ATTR static uint8_t buf0[DATA_SZ];
    DMA_ATTR static uint8_t *midi_note_trig = &buf0[N_CVS*4];
    DMA_ATTR static float *midi_data = (float*) buf0;
    DMA_ATTR static uint8_t midi_note_pitch[2] {0, 0};
    DMA_ATTR static uint8_t midi_note_vel[2] {0, 0};
    DMA_ATTR static uint8_t midi_prs[2] {0, 0};
    DMA_ATTR static uint8_t midi_pb[2] {0, 0};
    DMA_ATTR static uint8_t midi_mod[2] {0, 0};
    static midiXparser parser;
    static CTAG::DRIVERS::midiuart midi;
    // this is a low frequency thread
    static void bba_midi_thread(void* params){
        uint8_t *msgBuffer = (uint8_t *)malloc(midi.GetBufferSize());
        int len;
        // create midi_queue
        midi_queue = xQueueCreate( 10, sizeof( mididata_t ) );
        while(1){
            midi.read(msgBuffer, &len);
            uint8_t *ptr {msgBuffer};
            while(len > 0){
                if ( parser.parse( *ptr++ ) ) {
                    uint8_t *msg = parser.getMidiMsg();
                    //uint8_t mlen = parser.getMidiMsgLen();
                    /*if(mlen != 3){
                        ESP_LOGE("MIDI", "MIDI message length is not 3 bytes, but %d", mlen);
                    }*/
                    /* debug received message
                    for(int i = 0; i < mlen; i++){
                        printf("%02X ", msg[i]);
                    }
                    printf("\n");
                     */
                    // this is kind of stupid, double parsing!
                    if (parser.isMidiStatus(midiXparser::noteOnStatus)){
                        mididata_t data;
                        data.status = midiXparser::noteOnStatus;
                        memcpy(data.data, msg, 3);
                        xQueueSend(midi_queue, &data, 0);
                    }
                    else if(parser.isMidiStatus(midiXparser::noteOffStatus)){
                        mididata_t data;
                        data.status = midiXparser::noteOffStatus;
                        memcpy(data.data, msg, 3);
                        xQueueSend(midi_queue, &data, 0);
                    }
                    else if(parser.isMidiStatus(midiXparser::pitchBendStatus)){
                        mididata_t data;
                        data.status = midiXparser::pitchBendStatus;
                        memcpy(data.data, msg, 3);
                        xQueueSend(midi_queue, &data, 0);
                    }
                    else if(parser.isMidiStatus(midiXparser::controlChangeStatus)){
                        mididata_t data;
                        data.status = midiXparser::controlChangeStatus;
                        memcpy(data.data, msg, 3);
                        xQueueSend(midi_queue, &data, 0);
                    }
                    else if(parser.isMidiStatus(midiXparser::channelPressureStatus)){
                        mididata_t data;
                        data.status = midiXparser::channelPressureStatus;
                        memcpy(data.data, msg, 3);
                        xQueueSend(midi_queue, &data, 0);
                    }
                }
                len--;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        free(msgBuffer);
        // delete queue
        vQueueDelete(midi_queue);
    }
    static void bba_init(){
        memset(buf0, 0, DATA_SZ);
        memset(midi_note_trig, 1, N_TRIGS);
        parser.setMidiMsgFilter( midiXparser::channelVoiceMsgTypeMsk);
        xTaskCreatePinnedToCore(bba_midi_thread, "midi_thread", 4096, NULL, 15, NULL, 0);
    }
    IRAM_ATTR static uint8_t *bba_update(){
        mididata_t data;
        if(xQueueReceive(midi_queue, &data, 0) == pdTRUE){
            //ESP_LOGI("MIDI", "Message status %d, data %02X %02X %02X", data.status, data.data[0], data.data[1], data.data[2]);
            if(data.status == midiXparser::noteOnStatus){
                // midi note too large
                if(data.data[1] > 120) goto exit_parse;
                // midi note too small
                if(data.data[1] < 36) goto exit_parse;

                // parse channels
                if((data.data[0]&0xF) == MIDI_CHANNEL_A){
                    //ESP_LOGI("MIDI", "Note on channel A");
                    // note off through velocity 0
                    if((midi_note_pitch[0] == (data.data[1] - MIDI_NOTE_0V)) && (data.data[2] == 0)){
                        //ESP_LOGI("MIDI", "Note off thru velocity");
                        midi_note_trig[0] = 1;
                        goto exit_parse;
                    }
                    //ESP_LOGI("MIDI", "Note on channel A");
                    midi_note_trig[0] = 0;
                    midi_note_pitch[0] = data.data[1] - MIDI_NOTE_0V;
                    midi_data[0] = midi_note_pitch[0] * 0.016666666666667f;
                    midi_note_vel[0] = data.data[2];
                    midi_data[2] = midi_note_vel[0] * 0.007874015748031f;
                }
                if((data.data[0]&0xF) == MIDI_CHANNEL_B){
                    // note off through velocity 0
                    if((midi_note_pitch[1] == (data.data[1] - MIDI_NOTE_0V)) && (data.data[2] == 0)){
                        midi_note_trig[1] = 1;
                        goto exit_parse;
                    }
                    midi_note_trig[1] = 0;
                    midi_note_pitch[1] = data.data[1] - MIDI_NOTE_0V;
                    midi_data[1] = midi_note_pitch[1] * 0.016666666666667f;
                    midi_note_vel[1] = data.data[2];
                    midi_data[3] = midi_note_vel[1] * 0.007874015748031f;
                }
                goto exit_parse;
            }

            // check for note off
            if(data.status == midiXparser::noteOffStatus){
                // midi note too large
                if(data.data[1] > 120) goto exit_parse;
                // midi note too small
                if(data.data[1] < 36) goto exit_parse;
                // parse channels
                if((data.data[0]&0xF) == MIDI_CHANNEL_A){
                    //ESP_LOGI("MIDI", "Note off channel A");
                    if(midi_note_pitch[0] == data.data[1] - MIDI_NOTE_0V)
                        midi_note_trig[0] = 1;
                }
                if((data.data[0]&0xF) == MIDI_CHANNEL_B){
                    if(midi_note_pitch[1] == data.data[1] - MIDI_NOTE_0V)
                        midi_note_trig[1] = 1;
                }
                goto exit_parse;
            }

            // check for aftertouch
            if(data.status == midiXparser::channelPressureStatus){
                // parse channels
                if((data.data[0]&0xF) == MIDI_CHANNEL_A){
                    //ESP_LOGI("MIDI", "Pressure channel A");
                    midi_prs[0] = data.data[1];
                    midi_data[4] = midi_prs[0] * 0.007874015748031f;
                    midi_note_trig[2] = midi_prs[0] > 63 ? 0 : 1;
                    //ESP_LOGI("MIDI", "Pressure channel A %d %f", midi_mod[0], midi_data[6]);
                }
                if((data.data[0]&0xF) == MIDI_CHANNEL_B){
                    midi_prs[1] = data.data[1];
                    midi_data[5] = midi_prs[1] * 0.007874015748031f;
                    midi_note_trig[3] = midi_prs[1] > 63 ? 0 : 1;
                }
                goto exit_parse;
            }
            // check for pitch bend
            if(data.status == midiXparser::pitchBendStatus){
                //ESP_LOGI("MIDI", "Pitch bend");
                // parse channels
                if((data.data[0]&0xF) == MIDI_CHANNEL_A){
                    midi_pb[0] = data.data[2];
                    midi_data[6] = midi_pb[0] * 0.007874015748031f;
                    //ESP_LOGI("MIDI", "Pitch bend channel A %d %f", midi_pb[0], midi_data[4]);
                }
                if((data.data[0]&0xF) == MIDI_CHANNEL_B){
                    midi_pb[1] = data.data[2];
                    midi_data[7] = midi_pb[1] * 0.007874015748031f;
                }
                goto exit_parse;
            }

            // check for mod
            if(data.status == midiXparser::controlChangeStatus){
                // check if mod cc message, mod is controller #1
                if(data.data[1] != 1) goto exit_parse;
                // parse channels
                if((data.data[0]&0xF) == MIDI_CHANNEL_A){
                    //ESP_LOGI("MIDI", "Mod channel A");
                    midi_mod[0] = data.data[2];
                    midi_data[8] = midi_mod[0] * 0.007874015748031f;
                    midi_note_trig[4] = midi_mod[0] > 63 ? 0 : 1;
                    //ESP_LOGI("MIDI", "Mod channel A %d %f", midi_mod[0], midi_data[6]);
                }
                if((data.data[0]&0xF) == MIDI_CHANNEL_B){
                    midi_mod[1] = data.data[2];
                    midi_data[9] = midi_mod[1] * 0.007874015748031f;
                    midi_note_trig[5] = midi_mod[1] > 63 ? 0 : 1;
                }
                goto exit_parse;
            }
        }

        exit_parse:
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
