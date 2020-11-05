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


#include "codec.hpp"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define MAX(x, y) ((x)>(y)) ? (x) : (y)
#define MIN(x, y) ((x)<(y)) ? (x) : (y)

// SPI pins
#define PIN_NUM_CS_WM 4
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK 12
#define PIN_NUM_MISO 0
// I2S pins
#define I2S_BCLK_PIN 21
#define I2S_LRCLK_PIN 19
#define I2S_DACDAT_PIN 22
#define I2S_ADCDAT_PIN 27

using namespace CTAG::DRIVERS;

spi_device_handle_t Codec::codec_h;
spi_transaction_t Codec::trans;
spi_bus_config_t Codec::buscfg;
spi_device_interface_config_t Codec::codec_cfg;
bool Codec::isReady = false;


void Codec::setupSPIWM8731() {
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

    // CodecManager has noise issues with highpass on
    // https://hackaday.io/project/7936-cortex-guitar-board/log/44553-this-adc-makes-weird-noises
    HighPassEnable();
    vTaskDelay(50 / portTICK_PERIOD_MS); // wait until system is settled a bit
    HighPassDisable();

}

void Codec::setupI2SWM8731() {
    ESP_LOGI("WM8731", "Initializing CODEC I2S");
    static i2s_config_t i2s_config;
    memset(&i2s_config, 0, sizeof(i2s_config));
    i2s_config.mode = (i2s_mode_t) (I2S_MODE_SLAVE | I2S_MODE_TX | I2S_MODE_RX);
    i2s_config.sample_rate = 44100;
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
    i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    i2s_config.communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
    i2s_config.intr_alloc_flags = ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3; // default interrupt priority would be 0
    i2s_config.dma_buf_count = 4;
    i2s_config.dma_buf_len = 32;
    i2s_config.use_apll = true;

    static i2s_pin_config_t pin_config;
    memset(&pin_config, 0, sizeof(pin_config));
    pin_config.bck_io_num = I2S_BCLK_PIN;
    pin_config.ws_io_num = I2S_LRCLK_PIN;
    pin_config.data_out_num = I2S_DACDAT_PIN;
    pin_config.data_in_num = I2S_ADCDAT_PIN;


    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);   //install and start i2s driver

    i2s_set_pin(I2S_NUM_0, &pin_config);
}

void Codec::InitCodec() {
    initSPI();
#ifdef CONFIG_CODEC_IC_WM8731
    setupSPIWM8731();
    setupI2SWM8731();
    vTaskDelay(5000 / portTICK_PERIOD_MS); // wait until system is settled a bit
    // CodecManager has noise issues with highpass on
    // https://hackaday.io/project/7936-cortex-guitar-board/log/44553-this-adc-makes-weird-noises
    HighPassEnable();
    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait until system is settled a bit
    HighPassDisable();
#elif CONFIG_CODEC_IC_WM8978
    setupSPIWM8978();
    setupI2SWM8978();
#endif
    isReady = true;
}

void Codec::HighPassEnable() {
#ifdef CONFIG_CODEC_IC_WM8731
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
#ifdef CONFIG_CODEC_IC_WM8731
    unsigned char cmd;
    trans.flags = 0;
    trans.length = 8;
    trans.rxlength = 0;
    trans.tx_buffer = &cmd;
    trans.rx_buffer = NULL;
    trans.addr = 0x05 << 1;
    cmd = 0x11; // disable HPF and store last DC value
    ESP_ERROR_CHECK(spi_device_transmit(codec_h, &trans));
#endif
}

void Codec::RecalibDCOffset() {
#ifdef CONFIG_CODEC_IC_WM8731
    if(!isReady) return;
    HighPassEnable();
    vTaskDelay(50 / portTICK_PERIOD_MS); // wait until system is settled a bit
    HighPassDisable();
#endif
}

void Codec::initSPI() {
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

void IRAM_ATTR Codec::ReadBuffer(float *buf, uint32_t sz) {
#ifdef CONFIG_CODEC_IC_WM8731
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
#elif CONFIG_CODEC_IC_WM8978
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

void IRAM_ATTR Codec::WriteBuffer(float *buf, uint32_t sz) {
#ifdef CONFIG_CODEC_IC_WM8731
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
#elif CONFIG_CODEC_IC_WM8978
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


void Codec::setupSPIWM8978() {
    WM8978_Init();
    WM8978_ADDA_Cfg(1, 1);
    WM8978_Input_Cfg(0, 1, 0);
    WM8978_Output_Cfg(1, 0);
    //WM8978_MIC_Gain(0);
    //WM8978_AUX_Gain(0);
    WM8978_LINEIN_Gain(5); // 0db
    //WM8978_SPKvol_Set(0);
    WM8978_HPvol_Set(50, 50); //0-63 // 0db is 57 // 38
    /*
    WM8978_EQ_3D_Dir(0);
    WM8978_EQ1_Set(0, 12); // 0 db is 12
    WM8978_EQ2_Set(0, 12);
    WM8978_EQ3_Set(0, 12);
    WM8978_EQ4_Set(0, 12);
    WM8978_EQ5_Set(0, 12);
     */
    WM8978_I2S_Cfg(2, 0);
}

void Codec::setupI2SWM8978() {
    ESP_LOGI("WM8978", "Initializing CODEC I2S");
    // allow GPIO0 to be clock out
    gpio_config_t io_conf;
    io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1 << GPIO_NUM_0;
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    io_conf.pull_up_en = (gpio_pullup_t) 0;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_0, 0);

    static i2s_config_t i2s_config;
    memset(&i2s_config, 0, sizeof(i2s_config));
    i2s_config.mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
    i2s_config.sample_rate = 44100;
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    i2s_config.communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
    i2s_config.intr_alloc_flags = ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3; // default interrupt priority would be 0
    i2s_config.dma_buf_count = 4;
    i2s_config.dma_buf_len = 32;
    i2s_config.use_apll = true;

    static i2s_pin_config_t pin_config;
    memset(&pin_config, 0, sizeof(pin_config));
    pin_config.bck_io_num = I2S_BCLK_PIN;
    pin_config.ws_io_num = I2S_LRCLK_PIN;
    pin_config.data_out_num = I2S_DACDAT_PIN;
    pin_config.data_in_num = I2S_ADCDAT_PIN;


    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);   //install and start i2s driver

    i2s_set_pin(I2S_NUM_0, &pin_config);

    // output master clock on GPIO0
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
    REG_WRITE(PIN_CTRL, 0xFFFFFFF0);
}

/* this code is by an unknown author */

// WM8978 register value buffer zone (total 58 registers 0 to 57), occupies 116 bytes of memory
// Because the IIC WM8978 operation does not support read operations, so save all the register values in the local
// Write WM8978 register, synchronized to the local register values, register read, register directly back locally stored value.
// Note: WM8978 register value is 9, so use u16 storage.

static u16 WM8978_REGVAL_TBL[58] =
        {
                0X0000, 0X0000, 0X0000, 0X0000, 0X0050, 0X0000, 0X0140, 0X0000,
                0X0000, 0X0000, 0X0000, 0X00FF, 0X00FF, 0X0000, 0X0100, 0X00FF,
                0X00FF, 0X0000, 0X012C, 0X002C, 0X002C, 0X002C, 0X002C, 0X0000,
                0X0032, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000,
                0X0038, 0X000B, 0X0032, 0X0000, 0X0008, 0X000C, 0X0093, 0X00E9,
                0X0000, 0X0000, 0X0000, 0X0000, 0X0003, 0X0010, 0X0010, 0X0100,
                0X0100, 0X0002, 0X0001, 0X0001, 0X0039, 0X0039, 0X0039, 0X0039,
                0X0001, 0X0001};

//WM8978 init
//返回值:0,初始化正常
//    其他,错误代码
u8 Codec::WM8978_Init(void) {
    u8 res;

    ESP_LOGI("WM8978", "Init");
    res = WM8978_Write_Reg(0, 0); //soft reset WM8978

    if (res)
        ESP_LOGE("WM8978", "Soft reset failed: %d", res);
    else
        ESP_LOGI("WM8978", "Soft reset successfull");

    //以下为通用设置
    WM8978_Write_Reg(1, 0b000001111); // 0x1011
    //WM8978_Write_Reg(1, 0X1B);	// R1,MICEN设置为1(MIC使能),BIASEN设置为1(模拟器工作),VMIDSEL[1:0]设置为:11(5K)
    WM8978_Write_Reg(2, 0b110111111);
    //WM8978_Write_Reg(2, 0X1B0);   //R2,ROUT1,LOUT1输出使能(耳机可以工作),BOOSTENR,BOOSTENL使能
    WM8978_Write_Reg(3, 0b000001111);
    //WM8978_Write_Reg(3, 0X6C);	//R3,LOUT2,ROUT2输出使能(喇叭工作),RMIX,LMIX使能
    WM8978_Write_Reg(6, 0);          //R6,MCLK由外部提供
    //WM8978_Write_Reg(43, 1 << 4); //R43,INVROUT2反向,驱动喇叭
    //WM8978_Write_Reg(47, 1 << 8); //R47设置,PGABOOSTL,左通道MIC获得20倍增益
    //WM8978_Write_Reg(48, 1 << 8); //R48设置,PGABOOSTR,右通道MIC获得20倍增益
    //WM8978_Write_Reg(49, 1 << 1); //R49,TSDEN,开启过热保护
    WM8978_Write_Reg(10, 1 << 3); //R10,SOFTMUTE关闭,128x采样,最佳SNR
    WM8978_Write_Reg(14, 1 << 3 | 1); //R14,ADC 128x采样率 HPF Enable
    //WM8978_Write_Reg(50, 0b11);

    ESP_LOGI("WM8978", "Init finished");
    return 0;
}
//WM8978写寄存器
//reg:寄存器地址
//val:要写入寄存器的值
//返回值:0,成功;
//其他,错误代码

u8 Codec::WM8978_Write_Reg(u8 reg, u16 val) {
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
/*
u8 Codec::WM8978_Write_Reg(u8 reg, u16 val)
{
	char buf[2];
	buf[0] = (reg << 1) | ((val >> 8) & 0X01);
	buf[1] = val & 0XFF;

	WM8978_REGVAL_TBL[reg] = val; //保存寄存器值到本地

	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	i2c_master_driver_initialize();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, WM8978_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN); //发送数据到设备号为4的从机

	i2c_master_write_byte(cmd, buf[0], ACK_CHECK_EN);
	i2c_master_write_byte(cmd, buf[1], ACK_CHECK_EN);

	i2c_master_stop(cmd); // 停止发送
	esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	if (ret == ESP_OK)
	{
		ESP_LOGV(TAG, "Write OK bytes %x %x", buf[0], buf[1]);
	}
	else if (ret == ESP_ERR_TIMEOUT)
	{
		ESP_LOGE(TAG, "Bus is busy");
	}
	else
	{
		ESP_LOGE(TAG, "Write Failed");
	}
	i2c_driver_delete(I2C_NUM_0);

	return 0;
}*/

// WM8978 read register
// Reads the value  of the local register buffer zone
// reg: Register Address
// Return Value: Register value
u16 Codec::WM8978_Read_Reg(u8 reg) {
    return WM8978_REGVAL_TBL[reg];
}

//WM8978 DAC/ADC配置
//adcen:adc使能(1)/关闭(0)
//dacen:dac使能(1)/关闭(0)
void Codec::WM8978_ADDA_Cfg(u8 dacen, u8 adcen) {
    u16 regval;
    regval = WM8978_Read_Reg(3); //读取R3
    if (dacen)
        regval |= 3 << 0; //R3最低2个位设置为1,开启DACR&DACL
    else
        regval &= ~(3 << 0);     //R3最低2个位清零,关闭DACR&DACL.
    WM8978_Write_Reg(3, regval); //设置R3
    regval = WM8978_Read_Reg(2); //读取R2
    if (adcen)
        regval |= 3 << 0; //R2最低2个位设置为1,开启ADCR&ADCL
    else
        regval &= ~(3 << 0);     //R2最低2个位清零,关闭ADCR&ADCL.
    WM8978_Write_Reg(2, regval); //设置R2
}

//WM8978 输入通道配置
//micen:MIC开启(1)/关闭(0)
//lineinen:Line In开启(1)/关闭(0)
//auxen:aux开启(1)/关闭(0)
void Codec::WM8978_Input_Cfg(u8 micen, u8 lineinen, u8 auxen) {
    u16 regval;
    regval = WM8978_Read_Reg(2); //读取R2
    if (micen)
        regval |= 3 << 2; //开启INPPGAENR,INPPGAENL(MIC的PGA放大)
    else
        regval &= ~(3 << 2);     //关闭INPPGAENR,INPPGAENL.
    WM8978_Write_Reg(2, regval); //设置R2

    regval = WM8978_Read_Reg(44); //读取R44
    if (micen)
        regval |= 3 << 4 | 3 << 0; //开启LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
    else
        regval &= ~(3 << 4 | 3 << 0); //关闭LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA.
    WM8978_Write_Reg(44, regval);     //设置R44

    if (lineinen)
        WM8978_LINEIN_Gain(5); //LINE IN 0dB增益
    else
        WM8978_LINEIN_Gain(0); //关闭LINE IN
    if (auxen)
        WM8978_AUX_Gain(7); //AUX 6dB增益
    else
        WM8978_AUX_Gain(0); //关闭AUX输入
}

//WM8978 输出配置
//dacen:DAC输出(放音)开启(1)/关闭(0)
//bpsen:Bypass输出(录音,包括MIC,LINE IN,AUX等)开启(1)/关闭(0)
void Codec::WM8978_Output_Cfg(u8 dacen, u8 bpsen) {
    u16 regval = 0;
    if (dacen)
        regval |= 1 << 0; //DAC输出使能
    if (bpsen) {
        regval |= 1 << 1; //BYPASS使能
        regval |= 5 << 2; //0dB增益
    }
    WM8978_Write_Reg(50, regval); //R50设置
    WM8978_Write_Reg(51, regval); //R51设置
}

//WM8978 MIC增益设置(不包括BOOST的20dB,MIC-->ADC输入部分的增益)
//gain:0~63,对应-12dB~35.25dB,0.75dB/Step
void Codec::WM8978_MIC_Gain(u8 gain) {
    gain &= 0X3F;
    WM8978_Write_Reg(45, gain);             //R45,左通道PGA设置
    WM8978_Write_Reg(46, gain | 1 << 8); //R46,右通道PGA设置
}

//WM8978 L2/R2(也就是Line In)增益设置(L2/R2-->ADC输入部分的增益)
//gain:0~7,0表示通道禁止,1~7,对应-12dB~6dB,3dB/Step
void Codec::WM8978_LINEIN_Gain(u8 gain) {
    u16 regval;
    gain &= 0X07;
    regval = WM8978_Read_Reg(47);              //读取R47
    regval &= ~(7 << 4);                      //清除原来的设置
    WM8978_Write_Reg(47, regval | gain << 4); //设置R47
    regval = WM8978_Read_Reg(48);              //读取R48
    regval &= ~(7 << 4);                      //清除原来的设置
    WM8978_Write_Reg(48, regval | gain << 4); //设置R48
}

//WM8978 AUXR,AUXL(PWM音频部分)增益设置(AUXR/L-->ADC输入部分的增益)
//gain:0~7,0表示通道禁止,1~7,对应-12dB~6dB,3dB/Step
void Codec::WM8978_AUX_Gain(u8 gain) {
    u16 regval;
    gain &= 0X07;
    regval = WM8978_Read_Reg(47);              //读取R47
    regval &= ~(7 << 0);                      //清除原来的设置
    WM8978_Write_Reg(47, regval | gain << 0); //设置R47
    regval = WM8978_Read_Reg(48);              //读取R48
    regval &= ~(7 << 0);                      //清除原来的设置
    WM8978_Write_Reg(48, regval | gain << 0); //设置R48
}

//设置I2S工作模式
//fmt:0,LSB(右对齐);1,MSB(左对齐);2,飞利浦标准I2S;3,PCM/DSP;
//len:0,16位;1,20位;2,24位;3,32位;
void Codec::WM8978_I2S_Cfg(u8 fmt, u8 len) {
    fmt &= 0X03;
    len &= 0X03;                                  //限定范围
    WM8978_Write_Reg(4, (fmt << 3) | (len << 5) | 2 | 4); //R4,WM8978工作模式设置
}

//设置耳机左右声道音量
//voll:左声道音量(0~63)
//volr:右声道音量(0~63)
void Codec::WM8978_HPvol_Set(u8 voll, u8 volr) {
    voll &= 0X3F;
    volr &= 0X3F; //限定范围
    if (voll == 0)
        voll |= 1 << 6; //音量为0时,直接mute
    if (volr == 0)
        volr |= 1 << 6;                       //音量为0时,直接mute
    WM8978_Write_Reg(52, voll);               //R52,耳机左声道音量设置
    WM8978_Write_Reg(53, volr | (1 << 8)); //R53,耳机右声道音量设置,同步更新(HPVU=1)
}

//设置喇叭音量
//voll:左声道音量(0~63)
void Codec::WM8978_SPKvol_Set(u8 volx) {
    volx &= 0X3F; //限定范围
    if (volx == 0)
        volx |= 1 << 6;                       //音量为0时,直接mute
    WM8978_Write_Reg(54, volx);               //R54,喇叭左声道音量设置
    WM8978_Write_Reg(55, volx | (1 << 8)); //R55,喇叭右声道音量设置,同步更新(SPKVU=1)
}

//设置3D环绕声
//depth:0~15(3D强度,0最弱,15最强)
void Codec::WM8978_3D_Set(u8 depth) {
    depth &= 0XF;                 //限定范围
    WM8978_Write_Reg(41, depth); //R41,3D环绕设置
}

//设置EQ/3D作用方向
//dir:0,在ADC起作用
//    1,在DAC起作用(默认)
void Codec::WM8978_EQ_3D_Dir(u8 dir) {
    u16 regval;
    regval = WM8978_Read_Reg(0X12);
    if (dir)
        regval |= 1 << 8;
    else
        regval &= ~(1 << 8);
    WM8978_Write_Reg(18, regval); //R18,EQ1的第9位控制EQ/3D方向
}

//设置EQ1
//cfreq:截止频率,0~3,分别对应:80/105/135/175Hz
//gain:增益,0~24,对应-12~+12dB
void Codec::WM8978_EQ1_Set(u8 cfreq, u8 gain) {
    u16 regval;
    cfreq &= 0X3; //限定范围
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval = WM8978_Read_Reg(18);
    regval &= 0X100;
    regval |= cfreq << 5;          //设置截止频率
    regval |= gain;                  //设置增益
    WM8978_Write_Reg(18, regval); //R18,EQ1设置
}

//设置EQ2
//cfreq:中心频率,0~3,分别对应:230/300/385/500Hz
//gain:增益,0~24,对应-12~+12dB
void Codec::WM8978_EQ2_Set(u8 cfreq, u8 gain) {
    u16 regval = 0;
    cfreq &= 0X3; //限定范围
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval |= cfreq << 5;          //设置截止频率
    regval |= gain;                  //设置增益
    WM8978_Write_Reg(19, regval); //R19,EQ2设置
}

//设置EQ3
//cfreq:中心频率,0~3,分别对应:650/850/1100/1400Hz
//gain:增益,0~24,对应-12~+12dB
void Codec::WM8978_EQ3_Set(u8 cfreq, u8 gain) {
    u16 regval = 0;
    cfreq &= 0X3; //限定范围
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval |= cfreq << 5;          //设置截止频率
    regval |= gain;                  //设置增益
    WM8978_Write_Reg(20, regval); //R20,EQ3设置
}

//设置EQ4
//cfreq:中心频率,0~3,分别对应:1800/2400/3200/4100Hz
//gain:增益,0~24,对应-12~+12dB
void Codec::WM8978_EQ4_Set(u8 cfreq, u8 gain) {
    u16 regval = 0;
    cfreq &= 0X3; //限定范围
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval |= cfreq << 5;          //设置截止频率
    regval |= gain;                  //设置增益
    WM8978_Write_Reg(21, regval); //R21,EQ4设置
}

//设置EQ5
//cfreq:中心频率,0~3,分别对应:5300/6900/9000/11700Hz
//gain:增益,0~24,对应-12~+12dB
void Codec::WM8978_EQ5_Set(u8 cfreq, u8 gain) {
    u16 regval = 0;
    cfreq &= 0X3; //限定范围
    if (gain > 24)
        gain = 24;
    gain = 24 - gain;
    regval |= cfreq << 5;          //设置截止频率
    regval |= gain;                  //设置增益
    WM8978_Write_Reg(22, regval); //R22,EQ5设置
}

void Codec::WM8978_ALC_Set(u8 enable, u8 maxgain, u8 mingain) {
    u16 regval;

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

void Codec::WM8978_Noise_Set(u8 enable, u8 gain) {
    u16 regval;

    if (gain > 7)
        gain = 7;

    regval = WM8978_Read_Reg(35);
    regval = (enable << 3);
    regval |= gain;                  //设置增益
    WM8978_Write_Reg(35, regval); //R18,EQ1设置
}

