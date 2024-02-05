
// some parts are taken from TI Linux Driver for TLV320AIC3254 and https://github.com/gluonsandquarks/digicrusher-boilerplate/blob/main/src/AIC3254.cpp

#include "aic3254.hpp"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"

#define I2C_PORT_NUM I2C_NUM_1 // 0 is used for OLED
#define AIC3254_ADDR 0x18 // 0b0011000 (7-bit address)
#define ACK_CHECK_EN 1
/* tlv320aic32x4 register space (in decimal to match datasheet) */
#define AIC32X4_REG(page, reg)    ((page * 128) + reg)
#define    AIC32X4_PSEL        AIC32X4_REG(0, 0)
#define    AIC32X4_RESET        AIC32X4_REG(0, 1)
#define    AIC32X4_CLKMUX        AIC32X4_REG(0, 4)
#define    AIC32X4_PLLPR        AIC32X4_REG(0, 5)
#define    AIC32X4_PLLJ        AIC32X4_REG(0, 6)
#define    AIC32X4_PLLDMSB        AIC32X4_REG(0, 7)
#define    AIC32X4_PLLDLSB        AIC32X4_REG(0, 8)
#define    AIC32X4_NDAC        AIC32X4_REG(0, 11)
#define    AIC32X4_MDAC        AIC32X4_REG(0, 12)
#define AIC32X4_DOSRMSB        AIC32X4_REG(0, 13)
#define AIC32X4_DOSRLSB        AIC32X4_REG(0, 14)
#define    AIC32X4_NADC        AIC32X4_REG(0, 18)
#define    AIC32X4_MADC        AIC32X4_REG(0, 19)
#define AIC32X4_AOSR        AIC32X4_REG(0, 20)
#define AIC32X4_CLKMUX2        AIC32X4_REG(0, 25)
#define AIC32X4_CLKOUTM        AIC32X4_REG(0, 26)
#define AIC32X4_IFACE1        AIC32X4_REG(0, 27)
#define AIC32X4_IFACE2        AIC32X4_REG(0, 28)
#define AIC32X4_IFACE3        AIC32X4_REG(0, 29)
#define AIC32X4_BCLKN        AIC32X4_REG(0, 30)
#define AIC32X4_IFACE4        AIC32X4_REG(0, 31)
#define AIC32X4_IFACE5        AIC32X4_REG(0, 32)
#define AIC32X4_IFACE6        AIC32X4_REG(0, 33)
#define AIC32X4_GPIOCTL        AIC32X4_REG(0, 52)
#define AIC32X4_DOUTCTL        AIC32X4_REG(0, 53)
#define AIC32X4_DINCTL        AIC32X4_REG(0, 54)
#define AIC32X4_MISOCTL        AIC32X4_REG(0, 55)
#define AIC32X4_SCLKCTL        AIC32X4_REG(0, 56)
#define AIC32X4_DACSPB        AIC32X4_REG(0, 60)
#define AIC32X4_ADCSPB        AIC32X4_REG(0, 61)
#define AIC32X4_DACSETUP    AIC32X4_REG(0, 63)
#define AIC32X4_DACMUTE        AIC32X4_REG(0, 64)
#define AIC32X4_LDACVOL        AIC32X4_REG(0, 65)
#define AIC32X4_RDACVOL        AIC32X4_REG(0, 66)
#define AIC3254_BEEPCTL_L        AIC32X4_REG(0, 71)
#define AIC3254_BEEPCTL_R        AIC32X4_REG(0, 72)
#define AIC32X4_ADCSETUP    AIC32X4_REG(0, 81)
#define    AIC32X4_ADCFGA        AIC32X4_REG(0, 82)
#define AIC32X4_LADCVOL        AIC32X4_REG(0, 83)
#define AIC32X4_RADCVOL        AIC32X4_REG(0, 84)
#define AIC32X4_LAGC1        AIC32X4_REG(0, 86)
#define AIC32X4_LAGC2        AIC32X4_REG(0, 87)
#define AIC32X4_LAGC3        AIC32X4_REG(0, 88)
#define AIC32X4_LAGC4        AIC32X4_REG(0, 89)
#define AIC32X4_LAGC5        AIC32X4_REG(0, 90)
#define AIC32X4_LAGC6        AIC32X4_REG(0, 91)
#define AIC32X4_LAGC7        AIC32X4_REG(0, 92)
#define AIC32X4_RAGC1        AIC32X4_REG(0, 94)
#define AIC32X4_RAGC2        AIC32X4_REG(0, 95)
#define AIC32X4_RAGC3        AIC32X4_REG(0, 96)
#define AIC32X4_RAGC4        AIC32X4_REG(0, 97)
#define AIC32X4_RAGC5        AIC32X4_REG(0, 98)
#define AIC32X4_RAGC6        AIC32X4_REG(0, 99)
#define AIC32X4_RAGC7        AIC32X4_REG(0, 100)
#define AIC32X4_PWRCFG        AIC32X4_REG(1, 1)
#define AIC32X4_LDOCTL        AIC32X4_REG(1, 2)
#define AIC32X4_LPLAYBACK    AIC32X4_REG(1, 3)
#define AIC32X4_RPLAYBACK    AIC32X4_REG(1, 4)
#define AIC32X4_OUTPWRCTL    AIC32X4_REG(1, 9)
#define AIC32X4_CMMODE        AIC32X4_REG(1, 10)
#define AIC32X4_HPLROUTE    AIC32X4_REG(1, 12)
#define AIC32X4_HPRROUTE    AIC32X4_REG(1, 13)
#define AIC32X4_LOLROUTE    AIC32X4_REG(1, 14)
#define AIC32X4_LORROUTE    AIC32X4_REG(1, 15)
#define    AIC32X4_HPLGAIN        AIC32X4_REG(1, 16)
#define    AIC32X4_HPRGAIN        AIC32X4_REG(1, 17)
#define    AIC32X4_LOLGAIN        AIC32X4_REG(1, 18)
#define    AIC32X4_LORGAIN        AIC32X4_REG(1, 19)
#define AIC32X4_HEADSTART    AIC32X4_REG(1, 20)
#define AIC32X4_MICBIAS        AIC32X4_REG(1, 51)
#define AIC32X4_LMICPGAPIN    AIC32X4_REG(1, 52)
#define AIC32X4_LMICPGANIN    AIC32X4_REG(1, 54)
#define AIC32X4_RMICPGAPIN    AIC32X4_REG(1, 55)
#define AIC32X4_RMICPGANIN    AIC32X4_REG(1, 57)
#define AIC32X4_FLOATINGINPUT    AIC32X4_REG(1, 58)
#define AIC32X4_LMICPGAVOL    AIC32X4_REG(1, 59)
#define AIC32X4_RMICPGAVOL    AIC32X4_REG(1, 60)
#define AIC32X4_REFPOWERUP    AIC32X4_REG(1, 123)

aic3254::aic3254() : _pinsda{GPIO_NUM_41}, _pinscl{GPIO_NUM_40}, _pinreset{GPIO_NUM_42}, _i2cspeed{400000} {
    esp_err_t err = ESP_OK;
    i2c_config_t conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = _pinsda,
            .scl_io_num = _pinscl,
            .sda_pullup_en = false,
            .scl_pullup_en = false,
            .master = {
                    .clk_speed = _i2cspeed,
            },
            .clk_flags = 0,
    };

    err |= i2c_param_config(i2c_port_t(I2C_PORT_NUM), &conf);
    err |= i2c_driver_install(i2c_port_t(I2C_PORT_NUM), conf.mode, 0, 0, 0);

    gpio_reset_pin(_pinreset);
    gpio_set_direction(_pinreset, GPIO_MODE_OUTPUT);
    gpio_set_level(_pinreset, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(_pinreset, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

aic3254::~aic3254() {
    // TODO destroy
    i2c_driver_delete(i2c_port_t(I2C_PORT_NUM));
}

void aic3254::write_reg(uint8_t reg_add, uint8_t data) {
    esp_err_t err = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    err |= i2c_master_start(cmd);
    ESP_ERROR_CHECK(err);// send start bit
    err |= i2c_master_write_byte(cmd, (AIC3254_ADDR << 1) | I2C_MASTER_WRITE,
                          ACK_CHECK_EN); // aic3254 7-bit address + write bit
    ESP_ERROR_CHECK(err);
    err |= i2c_master_write_byte(cmd, reg_add, ACK_CHECK_EN);                // target register
    ESP_ERROR_CHECK(err);
    err |= i2c_master_write_byte(cmd, data, ACK_CHECK_EN);                       // target value
    ESP_ERROR_CHECK(err);
    err |= i2c_master_stop(cmd);                                                      // send stop bit
    ESP_ERROR_CHECK(err);
    err |= i2c_master_cmd_begin((i2c_port_t) I2C_PORT_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    ESP_ERROR_CHECK(err);
}

uint8_t aic3254::read_reg(uint8_t reg_add) {
    uint8_t data = 0xFF; // dummy
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);                                                     // send start bit
    i2c_master_write_byte(cmd, (AIC3254_ADDR << 1) | I2C_MASTER_WRITE,
                          ACK_CHECK_EN); // aic3254 7-bit address + write bit
    i2c_master_write_byte(cmd, reg_add, ACK_CHECK_EN);                // register to be read
    i2c_master_start(
            cmd);                                                     // resend start bit (see application reference guide p.80 for more info on the i2c transaction)
    i2c_master_write_byte(cmd, (AIC3254_ADDR << 1) | I2C_MASTER_READ,
                          ACK_CHECK_EN);  // aic3254 7-bit address + read bit
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);                         // read into data buffer
    i2c_master_stop(cmd);                                                      // send stop bit
    i2c_master_cmd_begin((i2c_port_t) I2C_PORT_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return data;
}

bool aic3254::test_reg(uint8_t register_address, uint8_t expected_value) {
    uint8_t read_value = read_reg(register_address);

    if (read_value == expected_value) {
        //printf("[PASS] R_0x%X -> EXPECTED: 0x%X. ACTUAL: 0x%X.\n", register_address, expected_value, read_value);
        return true;
    } else {
        //printf("[ERROR] R_0x%X -> EXPECTED: 0x%X. ACTUAL: 0x%X.\n", register_address, expected_value, read_value);
        return false;
    }
}

bool aic3254::identify() {
    write_AIC32X4_reg(AIC32X4_PSEL, 0);
    return test_reg(0, 0);
}

void aic3254::init() {
    // issue a soft reset
    write_AIC32X4_reg(AIC32X4_RESET, 0x01);            // (P0_R1) issue a software reset to the codec
    vTaskDelay(10 / portTICK_PERIOD_MS); // wait for device to initialize registers
    // configure power
    write_AIC32X4_reg(AIC32X4_PWRCFG, 0b00001000); // (P1_R1) disable crude AVDD generation from DVDD [0b00001000]
    write_AIC32X4_reg(AIC32X4_LDOCTL, 0x01); // (P1_R2) enable analog block, AVDD LDO powered up [0b00000001]
    write_AIC32X4_reg(AIC32X4_CMMODE, 0x08); // (P1_R10) output common mode for LOL and LOR is 1.65 from LDOIN (= Vcc / 2) [0b00001000]
    write_AIC32X4_reg(AIC32X4_IFACE1, 0b00110000); // I2S, 32bit depth, BCLK+WCLK as input, DOUT enabled
    write_AIC32X4_reg(AIC32X4_IFACE2, 0x00); // no BCLK offset
    write_AIC32X4_reg(AIC32X4_CLKMUX, 0b00000000); // MCLK is codec clock in, TODO: test maybe PLL clock if issues
    write_AIC32X4_reg(AIC32X4_PLLPR, 0x00); // PLL power down
    write_AIC32X4_reg(AIC32X4_DOSRMSB, 0x00);
    write_AIC32X4_reg(AIC32X4_DOSRLSB, 0x80); // (P0_R14) set DOSR = 128 decimal or 0x0080 hex
    write_AIC32X4_reg(AIC32X4_AOSR, 0x80); // (P0_R20) set AOSR = 128 decimal or 0x0080 hex, for decimation filters 1 to 6, ADC Oversampling
    write_AIC32X4_reg(AIC32X4_NDAC, 0x81); // (P0_R11) power up and set NDAC = 1
    write_AIC32X4_reg(AIC32X4_MDAC, 0x82); // (P0_R12) power up and set MDAC = 2
    write_AIC32X4_reg(AIC32X4_NADC, 0x81); // (P0_R18) power up and set NADC = 1
    write_AIC32X4_reg(AIC32X4_MADC, 0x82); // (P0_R19) power up and set MADC = 2
    // TODO: DACSPB -> using default
    // TODO: ADCSPB -> using default
    //write_16bit_reg(AIC32X4_DACSPB, 0x08); // PRB_P8

    // DAC routing and power up
    write_AIC32X4_reg(AIC32X4_LOLROUTE, 0x08);
    write_AIC32X4_reg(AIC32X4_LORROUTE, 0x08);
    write_AIC32X4_reg(AIC32X4_HPLROUTE, 0x08);
    write_AIC32X4_reg(AIC32X4_HPRROUTE, 0x08);

    write_AIC32X4_reg(AIC32X4_DACMUTE, 0x00); // individual volume control for L/R
    write_AIC32X4_reg(AIC32X4_LDACVOL, 0x00); // (P0_R65) set left DAC gain to 0dB DIGITAL VOL
    write_AIC32X4_reg(AIC32X4_RDACVOL, 0x00); // (P0_R66) set right DAC gain to 0dB DIGITAL VOL
    write_AIC32X4_reg(AIC32X4_DACSETUP, 0b11010100); // (P0_R63) Power up left and right DAC data paths and set channel [0b11010100]
    write_AIC32X4_reg(AIC32X4_OUTPWRCTL, 0b00111100); // (P1_R9) power up HPL, HPR, LOL and LOR [0b00111100]
    write_AIC32X4_reg(AIC32X4_HPLGAIN, 0x00); // (P1_R16) unmute HPL, set 0dB gain
    write_AIC32X4_reg(AIC32X4_HPRGAIN, 0x00); // (P1_R16) unmute HPL, set 0dB gain
    write_AIC32X4_reg(AIC32X4_LOLGAIN, 0x06); // (P1_R18) unmute LOL, set 6dB gain
    write_AIC32X4_reg(AIC32X4_LORGAIN, 0x06); // (P1_R18) unmute LOL, set 6dB gain

    // ADC routing and power up
    write_AIC32X4_reg(AIC32X4_LMICPGAPIN, 0b01000000); // IN1L routed to left MICPGA with 10k
    write_AIC32X4_reg(AIC32X4_RMICPGAPIN, 0b01000000); // IN1L routed to left MICPGA with 10k
    write_AIC32X4_reg(AIC32X4_LMICPGANIN, 0b01000000); // (P1_R54) CM is routed to Left MICPGA via CM1L with 10kohm resistance
    write_AIC32X4_reg(AIC32X4_RMICPGANIN, 0b01000000); // (P1_R57) CM is routed to Right MICPGA via CM1R with 10kohm resistance
    write_AIC32X4_reg(AIC32X4_LMICPGAVOL, 0x80); // 0dB gain for left MICPGA
    write_AIC32X4_reg(AIC32X4_RMICPGAVOL, 0x80); // 0dB gain for left MICPGA
    write_AIC32X4_reg(AIC32X4_ADCSETUP, 0b11000000); // (P0_R81) power up left and right ADCs
    write_AIC32X4_reg(AIC32X4_ADCFGA, 0x00); // (P0_R82) unmute left and right ADCs
    vTaskDelay(10 / portTICK_PERIOD_MS); // wait for device to initialize registers
}

void aic3254::write_AIC32X4_reg(uint8_t reg_add, uint8_t data) {
    if((reg_add >> 7) != page) {
        page = reg_add >> 7;
        write_reg(AIC32X4_PSEL, page);
//        ESP_LOGE("AIC3254", "WRITE: Switched to page %d", page);
    }
    uint8_t reg_add1 = reg_add & 0x7F;
    write_reg(reg_add1, data);
//    uint8_t val = read_16bit_reg(reg_add);
//    ESP_LOGE("AIC3254", "addr: 0x%02X val: 0x%02X,0x%02X", reg_add, data, val);
}

uint8_t aic3254::read_16bit_reg(uint8_t reg_add) {
    if((reg_add >> 7) != page) {
        page = reg_add >> 7;
        write_reg(AIC32X4_PSEL, page);
        ESP_LOGE("AIC3254", "READ: Switched to page %d", page);
    }
    reg_add = reg_add & 0x7F;
    return read_reg(reg_add);
}

void aic3254::beep() {
    write_AIC32X4_reg(AIC3254_BEEPCTL_L, 0x80);
    write_AIC32X4_reg(AIC3254_BEEPCTL_R, 0x80);
}

bool aic3254::setOutputVolume(uint8_t lvol, uint8_t rvol) {
    // range 0b00110000 = 0x30 (+24dB) to 0b10000001 = 0x81 (-63.5dB), if msb is set the attenuation
    // incoming range 0 to 63 for lvol and rvol, default 0dB is 58
    lvol = lvol > 63 ? 63 : lvol;
    rvol = rvol > 63 ? 63 : rvol;
    ESP_LOGE("AIC3254", "Setting volume to %d, %d", lvol, rvol);
    lvol = volume_mapping[lvol];
    rvol = volume_mapping[rvol];
    ESP_LOGE("AIC3254", "Setting volume to 0x%02X, 0x%02X", lvol, rvol);
    write_AIC32X4_reg(AIC32X4_LDACVOL, lvol);
    write_AIC32X4_reg(AIC32X4_RDACVOL, rvol);
    return true;
}
