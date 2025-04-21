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
#include <tbd/audio_device/codec.hpp>

#include "freertos/FreeRTOS.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "driver/i2s.h" // legacy I2S api
#include "soc/io_mux_reg.h"
#include <string.h>


#if (TBD_AUDIO_WM8731 + TBD_AUDIO_WM8974 + TBD_AUDIO_WM8978) == 0
    #error "no WM8xxx series sound chip available"
#elif (TBD_AUDIO_WM8731 + TBD_AUDIO_WM8974 + TBD_AUDIO_WM8978) > 1
    #error "multiple WM8xxx chips selected"
#endif

#if TBD_AUDIO_WM8731
    #define CODEC_NAME "wm8713"
#elif TBD_AUDIO_WM8974
    #define CODEC_NAME "wm8974"
#elif TBD_AUDIO_WM8978
    #define CODEC_NAME "wm8978"
#endif

#if TBD_AUDIO_WM8731
    // CodecManager has noise issues with highpass on
    // https://hackaday.io/project/7936-cortex-guitar-board/log/44553-this-adc-makes-weird-noises
    #define maybe_wait_to_settle(settle_time_ms) do { \
            HighPassEnable(); \
            vTaskDelay(settle_time_ms / portTICK_PERIOD_MS); \
            HighPassDisable(); \
        } while(0)
#else
    #define maybe_wait_to_settle(settle_time_ms) do {} while(0)
#endif

#define PIN_NUM_CS_WM TBD_WM8XXX_SPI_PIN_CS
#define PIN_NUM_MOSI  TBD_WM8XXX_SPI_PIN_MOSI
#define PIN_NUM_MISO  TBD_WM8XXX_SPI_PIN_MISO
#define PIN_NUM_CLK   TBD_WM8XXX_SPI_PIN_CLK

#define I2S_BCLK_PIN   TBD_WM8XXX_I2S_PIN_BCLK
#define I2S_ADCDAT_PIN TBD_WM8XXX_I2S_PIN_DIN
#define I2S_DACDAT_PIN TBD_WM8XXX_I2S_PIN_DOUT
#define I2S_LRCLK_PIN  TBD_WM8XXX_I2S_PIN_WS

#define MAX(x, y) ((x)>(y)) ? (x) : (y)
#define MIN(x, y) ((x)<(y)) ? (x) : (y)


namespace {

//// codec driver data ////

bool is_ready = false;
spi_device_handle_t codec_h = NULL;
spi_transaction_t trans;
spi_bus_config_t buscfg;
spi_device_interface_config_t codec_cfg;

//// core codec driver implementation ////

void WM8978_ADDA_Cfg(uint8_t dacen, uint8_t adcen);
void WM8978_Input_Cfg(uint8_t micen, uint8_t lineinen, uint8_t auxen);
void WM8978_Output_Cfg(uint8_t dacen, uint8_t bpsen);
void WM8978_MIC_Gain(uint8_t gain);
void WM8978_LINEIN_Gain(uint8_t gain);
void WM8978_AUX_Gain(uint8_t gain);
uint8_t WM8978_Write_Reg(uint8_t reg, uint16_t val);
uint16_t WM8978_Read_Reg(uint8_t reg);
void WM8978_HPvol_Set(uint8_t voll, uint8_t volr);
void WM8978_SPKvol_Set(uint8_t volx);
void WM8978_I2S_Cfg(uint8_t fmt, uint8_t len);
void WM8978_3D_Set(uint8_t depth);
void WM8978_EQ_3D_Dir(uint8_t dir);
void WM8978_EQ1_Set(uint8_t cfreq, uint8_t gain);
void WM8978_EQ2_Set(uint8_t cfreq, uint8_t gain);
void WM8978_EQ3_Set(uint8_t cfreq, uint8_t gain);
void WM8978_EQ4_Set(uint8_t cfreq, uint8_t gain);
void WM8978_EQ5_Set(uint8_t cfreq, uint8_t gain);
void WM8978_Noise_Set(uint8_t enable, uint8_t gain);
void WM8978_ALC_Set(uint8_t enable, uint8_t maxgain, uint8_t mingain);

// WM8978 register value buffer zone (total 58 registers 0 to 57), occupies 116 bytes of memory
// Because the IIC WM8978 operation does not support read operations, so save all the register values in the local
// Write WM8978 register, synchronized to the local register values, register read, register directly back locally stored value.
// Note: WM8978 register value is 9, so use uint16_t storage.
uint16_t WM8978_REGVAL_TBL[58] = {
    0X0000, 0X0000, 0X0000, 0X0000, 0X0050, 0X0000, 0X0140, 0X0000,
    0X0000, 0X0000, 0X0000, 0X00FF, 0X00FF, 0X0000, 0X0100, 0X00FF,
    0X00FF, 0X0000, 0X012C, 0X002C, 0X002C, 0X002C, 0X002C, 0X0000,
    0X0032, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000,
    0X0038, 0X000B, 0X0032, 0X0000, 0X0008, 0X000C, 0X0093, 0X00E9,
    0X0000, 0X0000, 0X0000, 0X0000, 0X0003, 0X0010, 0X0010, 0X0100,
    0X0100, 0X0002, 0X0001, 0X0001, 0X0039, 0X0039, 0X0039, 0X0039,
    0X0001, 0X0001
};

uint8_t WM8978_Write_Reg(uint8_t reg, uint16_t val) {
    unsigned char buf[2];
    buf[0] = (reg << 1) | ((val >> 8) & 0X01);
    buf[1] = val & 0XFF;

    WM8978_REGVAL_TBL[reg] = val;

    trans.flags = 0;
    trans.length = 8;
    trans.rxlength = 0;
    trans.tx_buffer = &buf[1];
    trans.rx_buffer = NULL;
    trans.addr = buf[0];
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    return 0;
}

uint16_t WM8978_Read_Reg(uint8_t reg) {
    return WM8978_REGVAL_TBL[reg];
}

void WM8978_ADDA_Cfg(uint8_t dacen, uint8_t adcen) {
    uint16_t regval;
    regval = WM8978_Read_Reg(3);
    if (dacen)
        regval |= 3 << 0;
    else
        regval &= ~(3 << 0);
    WM8978_Write_Reg(3, regval);
    regval = WM8978_Read_Reg(2);
    if (adcen)
        regval |= 3 << 0;
    else
        regval &= ~(3 << 0);
    WM8978_Write_Reg(2, regval);
}

void WM8978_Input_Cfg(uint8_t micen, uint8_t lineinen, uint8_t auxen) {
    uint16_t regval;
    regval = WM8978_Read_Reg(2);
    if (micen)
        regval |= 3 << 2;
    else
        regval &= ~(3 << 2); 
    WM8978_Write_Reg(2, regval); 

    regval = WM8978_Read_Reg(44);
    if (micen)
        regval |= 3 << 4 | 3 << 0; 
    else
        regval &= ~(3 << 4 | 3 << 0); 
    WM8978_Write_Reg(44, regval);

    if (lineinen)
        WM8978_LINEIN_Gain(5);
    else
        WM8978_LINEIN_Gain(0);
    if (auxen)
        WM8978_AUX_Gain(7);
    else
        WM8978_AUX_Gain(0); 

    regval = WM8978_Read_Reg(44); // input polarity
}

void WM8978_Output_Cfg(uint8_t dacen, uint8_t bpsen) {
    uint16_t regval = 0;
    if (dacen)
        regval |= 1 << 0;
    if (bpsen) {
        regval |= 1 << 1;
        regval |= 5 << 2;
    }
    WM8978_Write_Reg(50, regval);
    WM8978_Write_Reg(51, regval);
}

void WM8978_MIC_Gain(uint8_t gain) {
    gain &= 0X3F;
    WM8978_Write_Reg(45, gain);    
    WM8978_Write_Reg(46, gain | 1 << 8);
}

void WM8978_LINEIN_Gain(uint8_t gain) {
    uint16_t regval;
    gain &= 0X07;
    regval = WM8978_Read_Reg(47);          
    regval &= ~(7 << 4);                   
    WM8978_Write_Reg(47, regval | gain << 4);
    regval = WM8978_Read_Reg(48);           
    regval &= ~(7 << 4);                  
    WM8978_Write_Reg(48, regval | gain << 4);
}

void WM8978_AUX_Gain(uint8_t gain) {
    uint16_t regval;
    gain &= 0X07;
    regval = WM8978_Read_Reg(47);         
    regval &= ~(7 << 0);                   
    WM8978_Write_Reg(47, regval | gain << 0);
    regval = WM8978_Read_Reg(48);             
    regval &= ~(7 << 0);                     
    WM8978_Write_Reg(48, regval | gain << 0); 
}

void WM8978_I2S_Cfg(uint8_t fmt, uint8_t len) {
    fmt &= 0X03;
    len &= 0X03;                                
    WM8978_Write_Reg(4, (fmt << 3) | (len << 5) | 2 | 4); 
}

void WM8978_HPvol_Set(uint8_t voll, uint8_t volr) {
    voll &= 0X3F;
    volr &= 0X3F; 
    if (voll == 0)
        voll |= 1 << 6; 
    if (volr == 0)
        volr |= 1 << 6;                    
    WM8978_Write_Reg(52, voll);             
    WM8978_Write_Reg(53, volr | (1 << 8)); 
}

void WM8978_SPKvol_Set(uint8_t volx) {
    volx &= 0X3F; 
    if (volx == 0)
        volx |= 1 << 6;                  
    WM8978_Write_Reg(54, volx);          
    WM8978_Write_Reg(55, volx | (1 << 8));
}

void WM8978_3D_Set(uint8_t depth) {
    depth &= 0XF;             
    WM8978_Write_Reg(41, depth);
}

void WM8978_EQ_3D_Dir(uint8_t dir) {
    uint16_t regval;
    regval = WM8978_Read_Reg(0X12);
    if (dir)
        regval |= 1 << 8;
    else
        regval &= ~(1 << 8);
    WM8978_Write_Reg(18, regval); 
}

void WM8978_EQ1_Set(uint8_t cfreq, uint8_t gain) {
    uint16_t regval;
    cfreq &= 0X3;
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval = WM8978_Read_Reg(18);
    regval &= 0X100;
    regval |= cfreq << 5;        
    regval |= gain;                
    WM8978_Write_Reg(18, regval); 
}

void WM8978_EQ2_Set(uint8_t cfreq, uint8_t gain) {
    uint16_t regval = 0;
    cfreq &= 0X3; 
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval |= cfreq << 5;        
    regval |= gain;                 
    WM8978_Write_Reg(19, regval);
}

void WM8978_EQ3_Set(uint8_t cfreq, uint8_t gain) {
    uint16_t regval = 0;
    cfreq &= 0X3; 
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval |= cfreq << 5;     
    regval |= gain;               
    WM8978_Write_Reg(20, regval); 
}

void WM8978_EQ4_Set(uint8_t cfreq, uint8_t gain) {
    uint16_t regval = 0;
    cfreq &= 0X3; 
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval |= cfreq << 5;     
    regval |= gain;           
    WM8978_Write_Reg(21, regval);
}

void WM8978_EQ5_Set(uint8_t cfreq, uint8_t gain) {
    uint16_t regval = 0;
    cfreq &= 0X3; 
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval |= cfreq << 5;      
    regval |= gain;                
    WM8978_Write_Reg(22, regval);
}

void WM8978_ALC_Set(uint8_t enable, uint8_t maxgain, uint8_t mingain) {
    uint16_t regval;

    if (maxgain > 7)
        maxgain = 7;
    if (mingain > 7)
        mingain = 7;

    regval = WM8978_Read_Reg(32);
    if (enable)
        regval |= (3 << 7);
    regval |= (maxgain << 3) | (mingain << 0);
    WM8978_Write_Reg(32, regval);
}

void WM8978_Noise_Set(uint8_t enable, uint8_t gain) {
    uint16_t regval;

    if (gain > 7)
        gain = 7;

    regval = WM8978_Read_Reg(35);
    regval = (enable << 3);
    regval |= gain;            
    WM8978_Write_Reg(35, regval); 
}

uint8_t WM8978_Init(void) {
    uint8_t res;

    ESP_LOGI("WM8978", "Init");
    res = WM8978_Write_Reg(0, 0); //soft reset WM8978

    if (res)
        ESP_LOGE("WM8978", "Soft reset failed: %d", res);
    else
        ESP_LOGI("WM8978", "Soft reset successfull");

    WM8978_Write_Reg(1, 0b000001111);
    WM8978_Write_Reg(2, 0b110111111);
    WM8978_Write_Reg(3, 0b000001111);
    WM8978_Write_Reg(4, 0b000010000);
    WM8978_Write_Reg(6, 0);
    WM8978_Write_Reg(10, 1 << 3);
    WM8978_Write_Reg(14, 0b100001000);

    ESP_LOGI("WM8978", "Init finished");
    return 0;
}

//// core codec driver setup ////

void initialize_codec() {
#if TBD_AUDIO_WM8731
    // configure CodecManager
    unsigned char cmd;
    trans.flags = 0;
    trans.length = 8;
    trans.rxlength = 0;
    trans.tx_buffer = &cmd;
    trans.rx_buffer = NULL;
    // reset command
    trans.addr = 0x0f << 1;
    cmd = 0x00;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    // set master mode
    trans.addr = 0x07 << 1;
    cmd = 0x4E;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    // set sampling control register
    trans.addr = 0x08 << 1;
    cmd = 0x20;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    // power on
    trans.addr = 0x06 << 1;
    cmd = 0x00;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    // activate interface
    trans.addr = 0x09 << 1;
    cmd = 0x01;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    // activate line ins
    trans.addr = 0x00;
    cmd = 0x17;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    trans.addr = 0x01 << 1;
    cmd = 0x17;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    // select channels
    trans.addr = 0x04 << 1;
    cmd = 0x10;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    // set headphones levels
    trans.addr = 0x02 << 1;
    cmd = 0x68;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
    trans.addr = 0x03 << 1;
    cmd = 0x68;
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
#elif TBD_AUDIO_WM8974
    ESP_LOGI("WM8974", "Init");
    WM8978_Write_Reg(0, 0); //soft reset WM8978
    vTaskDelay(100 / portTICK_PERIOD_MS); // wait until system is settled a bit
    WM8978_Write_Reg(1, 0b001001111); // enable AUX input buffer
    WM8978_Write_Reg(2, 0b000010101); // enable ADC, PGA and boost section
    WM8978_Write_Reg(3, 0b010001001); // enable DAC, Mono out, mono mixer
    WM8978_Write_Reg(4, 0b000010000); // i2s mode
    WM8978_Write_Reg(5, 0);
    WM8978_Write_Reg(6, 0);
    WM8978_Write_Reg(7, 0);
    WM8978_Write_Reg(8, 0);
    WM8978_Write_Reg(10, 0b000001000); // DAC 128x Oversampling
    WM8978_Write_Reg(11, 0b011110011); // DAC level
    WM8978_Write_Reg(14, 0b100001000); // enable ADC HP + 128x Oversampling
    WM8978_Write_Reg(15, 0b011111111); // ADC level
    WM8978_Write_Reg(40, 0b000000000); // no attenuation
    WM8978_Write_Reg(44, 0b000000100); // aux to PGA
    WM8978_Write_Reg(45, 0b000010011); // PGA level
    WM8978_Write_Reg(56, 0b000000001); // DAC to Mono Mixer
#elif TBD_AUDIO_WM8978
    WM8978_Init();
    WM8978_ADDA_Cfg(1, 1);
    WM8978_Input_Cfg(0, 1, 0);
    WM8978_Output_Cfg(1, 0);
    WM8978_LINEIN_Gain(5); // 0db
    WM8978_HPvol_Set(58, 58); //0-63 // 0db is 57 // 38
#endif
}


/** @brief initialize the SPI channel used by all WMxxx chips
 * 
 */
void init_config_bus() {
    memset(&buscfg, 0, sizeof(buscfg));
    buscfg.miso_io_num = PIN_NUM_MISO;
    buscfg.mosi_io_num = PIN_NUM_MOSI;
    buscfg.sclk_io_num = PIN_NUM_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.intr_flags = 0;
    buscfg.flags = 0;
    buscfg.max_transfer_sz = 0;


    memset(&codec_cfg, 0, sizeof(codec_cfg));
    codec_cfg.address_bits = 8;
    codec_cfg.command_bits = 0;
    codec_cfg.dummy_bits = 0;
    codec_cfg.duty_cycle_pos = 0;
    codec_cfg.cs_ena_posttrans = 0;
    codec_cfg.cs_ena_pretrans = 0;
    codec_cfg.flags = 0;
    codec_cfg.pre_cb = 0;
    codec_cfg.post_cb = 0;
    codec_cfg.clock_speed_hz = 1000000;       //Clock out at 1 MHz
    codec_cfg.mode = 0;                           //SPI mode 0
    codec_cfg.spics_io_num = PIN_NUM_CS_WM;              //CS pin
    codec_cfg.queue_size = 1;

    esp_err_t ret;
    //Initialize the SPI bus
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    assert(ret == ESP_OK);
    ret = spi_bus_add_device(VSPI_HOST, &codec_cfg, &codec_h);
    assert(ret == ESP_OK);
}


void release_config_bus() {
    esp_err_t ret;
    ret = spi_bus_remove_device(codec_h);
    assert(ret == ESP_OK);
    ret = spi_bus_free(VSPI_HOST);
    assert(ret == ESP_OK);
}


void init_data_bus() {
    static i2s_config_t i2s_config;
    memset(&i2s_config, 0, sizeof(i2s_config));
    i2s_config.sample_rate = 44100;
    i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    i2s_config.communication_format = (i2s_comm_format_t) I2S_COMM_FORMAT_STAND_I2S;
    i2s_config.intr_alloc_flags = ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3; // default interrupt priority would be 0
    i2s_config.dma_buf_count = 4;
    i2s_config.dma_buf_len = 32;
    i2s_config.use_apll = true;

#if TBD_AUDIO_WM8731
    i2s_config.mode = (i2s_mode_t) (I2S_MODE_SLAVE | I2S_MODE_TX | I2S_MODE_RX);
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
#elif TBD_AUDIO_WM8974 || TBD_AUDIO_WM9878
    i2s_config.mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
#endif

    static i2s_pin_config_t pin_config;
    memset(&pin_config, 0, sizeof(pin_config));
    pin_config.bck_io_num = I2S_BCLK_PIN;
    pin_config.ws_io_num = I2S_LRCLK_PIN;
    pin_config.data_out_num = I2S_DACDAT_PIN;
    pin_config.data_in_num = I2S_ADCDAT_PIN;

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);   //install and start i2s driver
    i2s_set_pin(I2S_NUM_0, &pin_config);   
}


void enable_master_clock_on_pin0() {
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
    REG_WRITE(PIN_CTRL, 0xFFFFFFF0);
}

}

namespace tbd::drivers {

//// public codec interface implementation ////

void Codec::init() {
    init_config_bus();
    initialize_codec();
    maybe_wait_to_settle(50);
    init_data_bus();
    maybe_wait_to_settle(1000);

// release SPI on devices with no volume control
#if !TBD_VOLUME_CONTROL
    release_config_bus();
#endif
    is_ready = true;
}


void Codec::deinit() {
    // FIXME: release drivers for updates etc
}


void Codec::HighPassEnable() {
// #if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
#if TBD_AUDIO_WM8731
    unsigned char cmd;
    trans.flags = 0;
    trans.length = 8;
    trans.rxlength = 0;
    trans.tx_buffer = &cmd;
    trans.rx_buffer = NULL;
    trans.addr = 0x05 << 1;
    cmd = 0x00; // disable HPF and store last DC value
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
#endif
}


void Codec::HighPassDisable() {
// #if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
#if TBD_AUDIO_WM8731
    unsigned char cmd;
    trans.flags = 0;
    trans.length = 8;
    trans.rxlength = 0;
    trans.tx_buffer = &cmd;
    trans.rx_buffer = NULL;
    trans.addr = 0x05 << 1;
    cmd = 0x11; // disable HPF and store last DC value`
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
#endif
}


void Codec::RecalibDCOffset() {
// #if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
#if TBD_AUDIO_WM8731
    if(!is_ready) return;
    HighPassEnable();
    vTaskDelay(50 / portTICK_PERIOD_MS); // wait until system is settled a bit
    HighPassDisable();
#endif
}


void Codec::SetOutputLevels(const uint32_t left, const uint32_t right) {
// #if defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_MK2)
#if TBD_AUDIO_WM8978
    if(!is_ready){
        ESP_LOGD("CODEC", "Codec not initialized");
    }
    ESP_LOGD("CODEC", "Setting levels to %li, %li", left, right);
    WM8978_HPvol_Set(static_cast<uint8_t>(left), static_cast<uint8_t>(right));
#endif
}


void IRAM_ATTR Codec::WriteBuffer(float *buf, uint32_t sz) {
// #if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
#if TBD_AUDIO_WM8731
    int32_t tmp[sz * 2];
    int32_t tmp2;
    size_t nb;
    const float mult = 8388608.f;
    // 32 bit word config
    for (int i = 0; i < sz; i++) {
        tmp2 = (int32_t) (mult * buf[i * 2]);
        tmp2 = MAX(tmp2, -8388608);
        tmp2 = MIN(tmp2, 8388607);
        tmp[i * 2] = tmp2 << 8;
        tmp2 = (int32_t) (mult * buf[i * 2 + 1]);
        tmp2 = MAX(tmp2, -8388608);
        tmp2 = MIN(tmp2, 8388607);
        tmp[i * 2 + 1] = tmp2 << 8;
    }
    i2s_write(I2S_NUM_0, tmp, sz * 4 * 2, &nb, portMAX_DELAY);
    // #elif CONFIG_TBD_PLATFORM_AEM
#elif TBD_AUDIO_WM8974
    int16_t tmp[sz * 2];
    int16_t tmp2;
    size_t nb;
    const float mult = 32767.f;
    // 16 bit word config
    for (int i = 0; i < sz; i++) {
        tmp2 = (int32_t) (mult * buf[i * 2]);
        tmp2 = MAX(tmp2, -32767);
        tmp2 = MIN(tmp2, 32767);
        tmp[i * 2] = tmp2;
        //tmp[i * 2 + 1] = 0; // not used in mono codec
    }
    i2s_write(I2S_NUM_0, tmp, sz * 2 * 2, &nb, portMAX_DELAY);
// #elif defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_MK2)
#elif TBD_AUDIO_WM8978
    int16_t tmp[sz * 2];
    int16_t tmp2;
    size_t nb;
    const float mult = 32767.f;
    // 16 bit word config
    for (int i = 0; i < sz; i++) {
        tmp2 = (int32_t) (mult * buf[i * 2]);
        tmp2 = MAX(tmp2, -32767);
        tmp2 = MIN(tmp2, 32767);
        tmp[i * 2] = tmp2;
        tmp2 = (int32_t) (mult * buf[i * 2 + 1]);
        tmp2 = MAX(tmp2, -32767);
        tmp2 = MIN(tmp2, 32767);
        tmp[i * 2 + 1] = tmp2;
    }
    i2s_write(I2S_NUM_0, tmp, sz * 2 * 2, &nb, portMAX_DELAY);
#endif
}


void IRAM_ATTR Codec::ReadBuffer(float *buf, uint32_t sz) {
// #if defined(CONFIG_TBD_PLATFORM_V1) || defined(CONFIG_TBD_PLATFORM_STR)
#if TBD_AUDIO_WM8731
    int32_t tmp[sz * 2];
    int32_t *ptrTmp = tmp;
    size_t nb;
    const float div = 0.0000000004656612873077392578125f;
    // 32 bit word config
    i2s_read(I2S_NUM_0, tmp, sz * 4 * 2, &nb, portMAX_DELAY);
    while(sz > 0){
        *buf++ = div * (float) *ptrTmp++;
        *buf++ = div * (float) *ptrTmp++;
        sz--;
    }
// #elif CONFIG_TBD_PLATFORM_AEM
#elif TBD_AUDIO_WM8974
    int16_t tmp[sz * 2];
    int16_t *ptrTmp = tmp;
    size_t nb;
    const float div = 3.0518509476E-5f;
    i2s_read(I2S_NUM_0, tmp, sz * 2 * 2, &nb, portMAX_DELAY);
    while (sz > 0) {
        float s = div * (float) *ptrTmp++;
        ptrTmp++;
        *buf++ = s;
        *buf++ = s;
        sz--;
    }
// #elif defined(CONFIG_TBD_PLATFORM_V2) || defined(CONFIG_TBD_PLATFORM_MK2)
#elif TBD_AUDIO_WM8978
    int16_t tmp[sz * 2];
    int16_t *ptrTmp = tmp;
    size_t nb;
    const float div = 3.0518509476E-5f;
    // 16 bit word config stereo
    i2s_read(I2S_NUM_0, tmp, sz * 2 * 2, &nb, portMAX_DELAY);
    while (sz > 0) {
        *buf++ = div * (float) *ptrTmp++;
        *buf++ = div * (float) *ptrTmp++;
        sz--;
    }
#endif
}

}
