/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2025 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "codec_bba.hpp"

#include <cmath>
#include <driver/i2s_std.h>
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"


using namespace CTAG::DRIVERS;

// ── Boot timing instrumentation ─────────────────────────────
// Stores microsecond timestamps at each codec init step.
// Printed as a summary AFTER boot (when USB serial is available to host).
#define CODEC_MAX_STEPS 24
static struct {
    int64_t timestamps[CODEC_MAX_STEPS];
    const char *labels[CODEC_MAX_STEPS];
    int count;
    bool reported;
} boot_timing = {.count = 0, .reported = false};

static void boot_mark(int step, const char *label) {
    if (boot_timing.count < CODEC_MAX_STEPS) {
        boot_timing.timestamps[boot_timing.count] = esp_timer_get_time();
        boot_timing.labels[boot_timing.count] = label;
        boot_timing.count++;
    }
}

void Codec::PrintBootTiming() {
    if (boot_timing.reported || boot_timing.count == 0) return;
    boot_timing.reported = true;
    ESP_LOGI("CODEC", "========== CODEC BOOT TIMING REPORT ==========");
    int64_t t0 = boot_timing.timestamps[0];
    for (int i = 0; i < boot_timing.count; i++) {
        int64_t abs_us = boot_timing.timestamps[i];
        int64_t rel_us = boot_timing.timestamps[i] - t0;
        ESP_LOGI("CODEC", "  [step %2d] %8lld us (boot+%lld us) %s",
                 i + 1, abs_us, rel_us, boot_timing.labels[i]);
    }
    ESP_LOGI("CODEC", "  Total codec init: %lld us",
             boot_timing.timestamps[boot_timing.count - 1] - t0);
    ESP_LOGI("CODEC", "========== END CODEC BOOT TIMING ==============");
}

static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;

static const char *TAG = "CODEC";

static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_dev_handle_t i2c_dev_handle = NULL;

#define I2C_PORT_NUM I2C_NUM_1 // 0 is used for OLED
#define I2C_SDA GPIO_NUM_7
#define I2C_SCL GPIO_NUM_8
#define I2C_CLK_SPEED 400000

#define I2S_PORT_NUM I2S_NUM_0
#define I2S_MCLK GPIO_NUM_13
#define I2S_BCLK GPIO_NUM_12
#define I2S_WS GPIO_NUM_10
#define I2S_DOUT GPIO_NUM_11
#define I2S_DIN GPIO_NUM_9

uint8_t page = 255;

#define AIC3254_ADDR 0x18 // 0b0011000 (7-bit address)
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
#define AIC32X4_DACPRB        AIC32X4_REG(0, 60)
#define AIC32X4_ADCPRB        AIC32X4_REG(0, 61)
#define AIC32X4_HPF_COEFF_N0_MSB  AIC32X4_REG(8, 1)
#define AIC32X4_HPF_COEFF_N0_LSB  AIC32X4_REG(8, 2)
#define AIC32X4_HPF_COEFF_N1_MSB  AIC32X4_REG(8, 3)
#define AIC32X4_HPF_COEFF_N1_LSB  AIC32X4_REG(8, 4)
#define AIC32X4_HPF_COEFF_D1_MSB  AIC32X4_REG(8, 5)
#define AIC32X4_HPF_COEFF_D1_LSB  AIC32X4_REG(8, 6)

static void cfg_i2c(){
    ESP_LOGI(TAG, "cfg codec i2c");
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT_NUM,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AIC3254_ADDR,
        .scl_speed_hz = I2C_CLK_SPEED,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_config, &i2c_dev_handle));
}

static void write_reg(uint8_t reg_add, uint8_t data) {
    uint8_t write_buf[2] = {reg_add, data};
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_dev_handle, write_buf, sizeof(write_buf), 1000));
}

static uint8_t read_reg(uint8_t reg_add) {
    uint8_t data = 0xFF;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev_handle, &reg_add, 1, &data, 1, 1000));
    return data;
}

static esp_err_t test_reg(uint8_t register_address, uint8_t expected_value) {
    uint8_t read_value = read_reg(register_address);

    if (read_value == expected_value) {
        //printf("[PASS] R_0x%X -> EXPECTED: 0x%X. ACTUAL: 0x%X.\n", register_address, expected_value, read_value);
        return ESP_OK;
    } else {
        //printf("[ERROR] R_0x%X -> EXPECTED: 0x%X. ACTUAL: 0x%X.\n", register_address, expected_value, read_value);
        return ESP_ERR_INVALID_STATE;
    }
}

static void write_AIC32X4_reg(uint8_t reg_add, uint8_t data) {
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


static void identify() {
    write_AIC32X4_reg(AIC32X4_PSEL, 0);
    esp_err_t err =  test_reg(0, 0);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "AIC3254 found");
        // NOTE: No muting here — soft reset in cfg_codec() wipes all registers anyway
    } else {
        ESP_LOGE(TAG, "AIC3254 not found");
    }
}

void Codec::cfg_codec() {
    boot_mark(0, "cfg_codec() entry");

    // ══════════════════════════════════════════════════════════════════
    //  POP-FREE INIT v9 (FINAL)
    // ══════════════════════════════════════════════════════════════════
    //
    // TBD-16 Standalone codec power topology (from schematics):
    //   DVDD, AVDD, LDOIN, LDO_SELECT all → +3.3V_A (TPS7A2033)
    //   LDO_SELECT = HIGH → internal LDO BYPASSED
    //   HPL/HPR → NC (not connected)
    //   LOL/LOR → MCP6004 buffer → CN13 (DC-coupled, no blocking caps!)
    //
    // Root cause: Output driver power-on steps LOL/LOR from 0V→CM.
    //   HEADSTART de-pop is ineffective on this board: output sees
    //   ~5pF MCP6004 input, so RC τ ≈ 50ps — ramp completes in ns
    //   regardless of register setting (designed for headphone loads).
    //   Staggering LOL/LOR causes TWO pops — worse than simultaneous.
    //
    // v9 = the best software can do on DC-coupled outputs.
    //   HEADSTART 0x66 = 8TC + medium step (0x6C slowest step was -13.7, worse)
    //
    // Measurement history:
    //   baseline: -11.3 dBFS (original code, CMMODE+PWRCFG)
    //   v9:  -17.3 dBFS — hw-aware, no PWRCFG/CMMODE
    //   v9x-stagger: -10.8/-18.1 dBFS — WORSE (two pops!)
    //   v9x-0x6C: -13.7 dBFS — slowest step worse than medium
    //   v12: -21.1 to -29.9 dBFS (avg -24.5, 7 runs) — REFPOWERUP=0x04 (force+slow) ← BEST
    //   v13: -8.7/-9.8/-16.7 dBFS — 0x07(force+120ms) + drivers during ramp — WORSE
    //   v14: -12.7/-8.9/-11.7 dBFS — 0x07(force+120ms) + drivers after 200ms — WORSE
    //   → Slow ramp speed is the dominant factor; driver timing irrelevant
    // ══════════════════════════════════════════════════════════════════
    ESP_LOGI(TAG, "AIC3254 pop-free init v12");

    // ── 1. CONFIRM CLEAN STATE ──
    boot_mark(1, "ensure outputs off");
    write_AIC32X4_reg(AIC32X4_OUTPWRCTL, 0x00);        // power down ALL output drivers
    write_AIC32X4_reg(AIC32X4_DACSETUP, 0x00);         // power down DAC
    write_AIC32X4_reg(AIC32X4_DACMUTE, 0b00001100);    // mute L+R DAC
    write_AIC32X4_reg(AIC32X4_LOLGAIN, 0b01000000);    // mute LOL
    write_AIC32X4_reg(AIC32X4_LORGAIN, 0b01000000);    // mute LOR
    write_AIC32X4_reg(AIC32X4_LOLROUTE, 0x00);         // disconnect routing
    write_AIC32X4_reg(AIC32X4_LORROUTE, 0x00);
    write_AIC32X4_reg(AIC32X4_NDAC, 0x01);             // NDAC off
    write_AIC32X4_reg(AIC32X4_MDAC, 0x02);             // MDAC off
    write_AIC32X4_reg(AIC32X4_NADC, 0x01);             // NADC off
    write_AIC32X4_reg(AIC32X4_MADC, 0x02);             // MADC off

    // ── 2. DE-POP + REFERENCE ──
    // NO PWRCFG write — crude AVDD = clean +3.3V_A on this board
    // NO CMMODE write — use default bandgap CM (1.35V)
    boot_mark(2, "HEADSTART(0x66) + LDOCTL + REFPOWERUP(force)");
    write_AIC32X4_reg(AIC32X4_HEADSTART, 0x66);        // (P1_R20) 8TC + medium step (proven best)
    write_AIC32X4_reg(AIC32X4_LDOCTL, 0x01);           // (P1_R2) enable analog supply path
    write_AIC32X4_reg(AIC32X4_REFPOWERUP, 0x04);       // (P1_R123) FORCE ref power-up NOW, slow
    // v9 bug: 0x01 = "power up when analog blocks powered" → ref only
    // started at OUTPWRCTL write, so 200ms wait was wasted!
    // 0x04 = "force power up NOW, slow" → ref settles during our wait

    boot_mark(3, "waiting 200ms for forced reference settle");
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // ── 3. POWER OUTPUT DRIVERS (simultaneous — staggering causes 2 pops!) ──
    boot_mark(4, "output driver power on (LOL+LOR simultaneous)");
    write_AIC32X4_reg(AIC32X4_OUTPWRCTL, 0b00001100);  // (P1_R9) LOL + LOR on
    // LOL/LOR step 0V→1.35V (softened by HEADSTART de-pop)

    boot_mark(5, "waiting 500ms for output CM settle");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // ── 4. CLOCKING ──
    boot_mark(6, "clocking config");
    write_AIC32X4_reg(AIC32X4_IFACE1, 0b00110000);     // I2S, 32bit
    write_AIC32X4_reg(AIC32X4_IFACE2, 0x00);
    write_AIC32X4_reg(AIC32X4_CLKMUX, 0b00000000);     // MCLK
    write_AIC32X4_reg(AIC32X4_PLLPR, 0x00);            // PLL off
    write_AIC32X4_reg(AIC32X4_DOSRMSB, 0x00);
    write_AIC32X4_reg(AIC32X4_DOSRLSB, 0x80);          // DOSR = 128
    write_AIC32X4_reg(AIC32X4_AOSR, 0x80);             // AOSR = 128
    write_AIC32X4_reg(AIC32X4_NDAC, 0x81);             // NDAC = 1, on
    write_AIC32X4_reg(AIC32X4_MDAC, 0x82);             // MDAC = 2, on
    write_AIC32X4_reg(AIC32X4_NADC, 0x81);             // NADC = 1, on
    write_AIC32X4_reg(AIC32X4_MADC, 0x82);             // MADC = 2, on

    // ── 5. DAC MUTED AT MINIMUM VOLUME ──
    write_AIC32X4_reg(AIC32X4_DACMUTE, 0b00001100);
    write_AIC32X4_reg(AIC32X4_LDACVOL, 0x81);          // -63.5 dB (minimum)
    write_AIC32X4_reg(AIC32X4_RDACVOL, 0x81);
    write_AIC32X4_reg(AIC32X4_DACSETUP, 0b11010100);   // power up DAC (muted)

    // ── 6. CONNECT DAC ROUTING ──
    boot_mark(7, "connecting DAC routing");
    write_AIC32X4_reg(AIC32X4_LOLROUTE, 0x08);         // left DAC → LOL
    write_AIC32X4_reg(AIC32X4_LORROUTE, 0x08);         // right DAC → LOR

    // ── 7. ADC PATH ──
    write_AIC32X4_reg(AIC32X4_LMICPGAPIN, 0b01000000);
    write_AIC32X4_reg(AIC32X4_RMICPGAPIN, 0b01000000);
    write_AIC32X4_reg(AIC32X4_LMICPGANIN, 0b01000000);
    write_AIC32X4_reg(AIC32X4_RMICPGANIN, 0b01000000);
    write_AIC32X4_reg(AIC32X4_LMICPGAVOL, 0x80);
    write_AIC32X4_reg(AIC32X4_RMICPGAVOL, 0x80);
    write_AIC32X4_reg(AIC32X4_ADCPRB, 0x01);
    ADCHighPassEnable();
    write_AIC32X4_reg(AIC32X4_ADCSETUP, 0b11000000);
    write_AIC32X4_reg(AIC32X4_ADCFGA, 0x00);

    // ── 8. UNMUTE ANALOG OUTPUTS ──
    boot_mark(8, "unmuting analog outputs");
    write_AIC32X4_reg(AIC32X4_LOLGAIN, 0x00);          // unmute LOL, 0dB
    write_AIC32X4_reg(AIC32X4_LORGAIN, 0x00);          // unmute LOR, 0dB

    // ── 9. GRADUAL DAC VOLUME FADE-IN ──
    write_AIC32X4_reg(AIC32X4_DACMUTE, 0x00);          // unmute DAC
    boot_mark(9, "DAC volume fade-in");
    static const uint8_t fade_ramp[] = {
        0x89, 0x91, 0x99, 0xA1, 0xA9, 0xB1, 0xB9, 0xC1,
        0xC9, 0xD1, 0xD9, 0xE1, 0xE9, 0xF1, 0xF9, 0x00
    };
    for (int i = 0; i < 16; i++) {
        write_AIC32X4_reg(AIC32X4_LDACVOL, fade_ramp[i]);
        write_AIC32X4_reg(AIC32X4_RDACVOL, fade_ramp[i]);
        vTaskDelay(1);
    }

    boot_mark(10, "init complete");
    ESP_LOGI(TAG, "AIC3254 pop-free init v12 complete");
}

void Codec::SetOutputLevels(const uint32_t left, const uint32_t right) {
    uint8_t lvol = left;
    uint8_t rvol = right;
    const uint8_t volume_mapping[64] = {
        0x81, 0x83, 0x85, 0x87, 0x89, 0x8B, 0x8D, 0x8F, 0x91, 0x93, 0x95, 0x97, 0x99, 0x9B, 0x9D, 0x9F,
        0xA1, 0xA3, 0xA5, 0xA7, 0xA9, 0xAB, 0xAD, 0xAF, 0xB1, 0xB3, 0xB5, 0xB7, 0xB9, 0xBB, 0xBD, 0xBF,
        0xC1, 0xC3, 0xC5, 0xC7, 0xC9, 0xCB, 0xCD, 0xCF, 0xD1, 0xD3, 0xD5, 0xD7, 0xD9, 0xDB, 0xDD, 0xDF,
        0xE1, 0xE3, 0xE5, 0xE7, 0xE9, 0xEB, 0xED, 0xEF, 0xF5, 0xFC, 0x00, 0x03, 0x06, 0x09, 12, 13};

    // incoming range 0 to 63 for lvol and rvol, default 0dB is 58
    uint8_t dac_mute = 0x00;
    if(lvol == 0x00) {
        dac_mute |= 0b00001000; // mute left channel
    }
    if(rvol == 0x00) {
        dac_mute |= 0b00000100; // mute right channel
    }
    write_AIC32X4_reg(AIC32X4_DACMUTE, dac_mute);
    lvol = lvol > 63 ? 63 : lvol;
    rvol = rvol > 63 ? 63 : rvol;
    //ESP_LOGE("AIC3254", "Setting volume to %d, %d", lvol, rvol);
    lvol = volume_mapping[lvol];
    rvol = volume_mapping[rvol];
    //ESP_LOGE("AIC3254", "Setting volume to 0x%02X, 0x%02X", lvol, rvol);
    write_AIC32X4_reg(AIC32X4_LDACVOL, lvol);
    write_AIC32X4_reg(AIC32X4_RDACVOL, rvol);
}

static void cfg_i2s() {
    ESP_LOGI(TAG, "cfg codec i2s");
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT_NUM, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;   // zero DMA buffers when no data written — prevents garbage clicks on startup
    chan_cfg.dma_desc_num = 4;
    chan_cfg.dma_frame_num = 32;

    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));

    i2s_std_config_t std_cfg = {
            .clk_cfg = {
                    .sample_rate_hz = 44100,
                    .clk_src = I2S_CLK_SRC_APLL,
                    .ext_clk_freq_hz = 0,
                    .mclk_multiple = I2S_MCLK_MULTIPLE_256,
                    .bclk_div = 0,
            },
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
            .gpio_cfg = {
                    .mclk = I2S_MCLK,
                    .bclk = I2S_BCLK,
                    .ws   = I2S_WS,
                    .dout = I2S_DOUT,
                    .din  = I2S_DIN,
                    .invert_flags = {
                            .mclk_inv = true,
                            .bclk_inv = false,
                            .ws_inv = false,
                    },
            },
    };

    /* Initialize the channels */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));

    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

}

#ifdef CONFIG_ASSEMBLY_OPT_AUDIO_BUFFER_CODEC
// Static buffer allocation for codec operations (optimized for cache performance)
// Typical buffer size is 32 stereo samples = 64 int32/float values
// Pre-allocated to avoid stack allocation overhead in real-time audio path
#define MAX_CODEC_BUFFER_SIZE (32*2)  // Support up to 32 stereo samples
static int32_t DRAM_ATTR tmp_buffer[MAX_CODEC_BUFFER_SIZE] __attribute__((aligned(4)));

void IRAM_ATTR Codec::ReadBuffer(float *buf, uint32_t sz) {
    // Use pre-allocated static buffer (zero allocation overhead)
    int32_t *tmp = tmp_buffer;
    size_t nb;

    // 32 bit word config stereo
    i2s_channel_read(rx_handle, tmp, sz*2*4, &nb, portMAX_DELAY);

    const float scale = 1.0f / 2147483648.0f;
    const uint32_t count = 4; // 4 hardware loop iterations for 64 samples

    int32_t *ptrTmp = tmp;
    float *ptrBuf = buf;

    // Use ESP32-P4 hardware loop for ZERO-OVERHEAD iteration
    // No branch penalty, no loop counter overhead!
    __asm__ volatile (
        // Setup hardware loop 0: iterates 'count' times, ending at .hwloop_read_end
        "esp.lp.setup   0, %[cnt], .hwloop_read_end \n"

        // === Hardware loop body (executes 4 times for 64 samples) ===

        // Load 16 int32 values (uses all available temporary registers)
        "lw      t0, 0(%[src])      \n"
        "lw      t1, 4(%[src])      \n"
        "lw      t2, 8(%[src])      \n"
        "lw      t3, 12(%[src])     \n"
        "lw      t4, 16(%[src])     \n"
        "lw      t5, 20(%[src])     \n"
        "lw      t6, 24(%[src])     \n"
        "lw      a0, 28(%[src])     \n"
        "lw      a1, 32(%[src])     \n"
        "lw      a2, 36(%[src])     \n"
        "lw      a3, 40(%[src])     \n"
        "lw      a4, 44(%[src])     \n"
        "lw      a5, 48(%[src])     \n"
        "lw      a6, 52(%[src])     \n"
        "lw      a7, 56(%[src])     \n"
        "lw      s2, 60(%[src])     \n"

        // Convert all 16 to float (hardware FPU, fully pipelined)
        "fcvt.s.w ft0, t0           \n"
        "fcvt.s.w ft1, t1           \n"
        "fcvt.s.w ft2, t2           \n"
        "fcvt.s.w ft3, t3           \n"
        "fcvt.s.w ft4, t4           \n"
        "fcvt.s.w ft5, t5           \n"
        "fcvt.s.w ft6, t6           \n"
        "fcvt.s.w ft7, a0           \n"
        "fcvt.s.w ft8, a1           \n"
        "fcvt.s.w ft9, a2           \n"
        "fcvt.s.w ft10, a3          \n"
        "fcvt.s.w ft11, a4          \n"
        "fcvt.s.w fa0, a5           \n"
        "fcvt.s.w fa1, a6           \n"
        "fcvt.s.w fa2, a7           \n"
        "fcvt.s.w fa3, s2           \n"

        // Multiply all 16 by scale
        "fmul.s  ft0, ft0, %[scale] \n"
        "fmul.s  ft1, ft1, %[scale] \n"
        "fmul.s  ft2, ft2, %[scale] \n"
        "fmul.s  ft3, ft3, %[scale] \n"
        "fmul.s  ft4, ft4, %[scale] \n"
        "fmul.s  ft5, ft5, %[scale] \n"
        "fmul.s  ft6, ft6, %[scale] \n"
        "fmul.s  ft7, ft7, %[scale] \n"
        "fmul.s  ft8, ft8, %[scale] \n"
        "fmul.s  ft9, ft9, %[scale] \n"
        "fmul.s  ft10, ft10, %[scale] \n"
        "fmul.s  ft11, ft11, %[scale] \n"
        "fmul.s  fa0, fa0, %[scale] \n"
        "fmul.s  fa1, fa1, %[scale] \n"
        "fmul.s  fa2, fa2, %[scale] \n"
        "fmul.s  fa3, fa3, %[scale] \n"

        // Store all 16 results
        "fsw     ft0, 0(%[dst])     \n"
        "fsw     ft1, 4(%[dst])     \n"
        "fsw     ft2, 8(%[dst])     \n"
        "fsw     ft3, 12(%[dst])    \n"
        "fsw     ft4, 16(%[dst])    \n"
        "fsw     ft5, 20(%[dst])    \n"
        "fsw     ft6, 24(%[dst])    \n"
        "fsw     ft7, 28(%[dst])    \n"
        "fsw     ft8, 32(%[dst])    \n"
        "fsw     ft9, 36(%[dst])    \n"
        "fsw     ft10, 40(%[dst])   \n"
        "fsw     ft11, 44(%[dst])   \n"
        "fsw     fa0, 48(%[dst])    \n"
        "fsw     fa1, 52(%[dst])    \n"
        "fsw     fa2, 56(%[dst])    \n"
        "fsw     fa3, 60(%[dst])    \n"

        // Update pointers for next iteration
        "addi    %[src], %[src], 64 \n"  // +16 samples × 4 bytes
        "addi    %[dst], %[dst], 64 \n"

        // === Loop end marker (hardware loops here with ZERO cycles!) ===
        ".hwloop_read_end: nop      \n"

        : [src] "+r" (ptrTmp), [dst] "+r" (ptrBuf)
        : [cnt] "r" (count), [scale] "f" (scale)
        : "t0", "t1", "t2", "t3", "t4", "t5", "t6", "s2",
          "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
          "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",
          "ft8", "ft9", "ft10", "ft11", "fa0", "fa1", "fa2", "fa3",
          "memory"
    );
}

void IRAM_ATTR Codec::WriteBuffer(float *buf, uint32_t sz){
    // Use pre-allocated static buffer (zero allocation overhead)
    int32_t *tmp = tmp_buffer;
    size_t nb;
    const float mult = 2147483647.f;

    const uint32_t count = 4;  // Number of 16-sample blocks (typically 4 for 64 samples)
    float *ptrBuf = buf;
    int32_t *ptrTmp = tmp;

    // Pre-clamp in float domain for efficiency
    const float fmax = 1.0f;
    const float fmin = -1.0f;

    // Use ESP32-P4 hardware loop for ZERO-OVERHEAD iteration
    __asm__ volatile (
        "esp.lp.setup   0, %[cnt], .hwloop_write_end \n"

        // === Hardware loop body (executes 4 times for 64 samples) ===

        // Load all 16 floats
        "flw     ft0, 0(%[src])     \n"
        "flw     ft1, 4(%[src])     \n"
        "flw     ft2, 8(%[src])     \n"
        "flw     ft3, 12(%[src])    \n"
        "flw     ft4, 16(%[src])    \n"
        "flw     ft5, 20(%[src])    \n"
            "flw     ft6, 24(%[src])    \n"
            "flw     ft7, 28(%[src])    \n"
            "flw     ft8, 32(%[src])    \n"
            "flw     ft9, 36(%[src])    \n"
            "flw     ft10, 40(%[src])   \n"
            "flw     ft11, 44(%[src])   \n"
            "flw     fa0, 48(%[src])    \n"
            "flw     fa1, 52(%[src])    \n"
            "flw     fa2, 56(%[src])    \n"
            "flw     fa3, 60(%[src])    \n"

            // Clamp all 16 to [-1.0, 1.0]
            "fmax.s  ft0, ft0, %[fmin]  \n"
            "fmin.s  ft0, ft0, %[fmax]  \n"
            "fmax.s  ft1, ft1, %[fmin]  \n"
            "fmin.s  ft1, ft1, %[fmax]  \n"
            "fmax.s  ft2, ft2, %[fmin]  \n"
            "fmin.s  ft2, ft2, %[fmax]  \n"
            "fmax.s  ft3, ft3, %[fmin]  \n"
            "fmin.s  ft3, ft3, %[fmax]  \n"
            "fmax.s  ft4, ft4, %[fmin]  \n"
            "fmin.s  ft4, ft4, %[fmax]  \n"
            "fmax.s  ft5, ft5, %[fmin]  \n"
            "fmin.s  ft5, ft5, %[fmax]  \n"
            "fmax.s  ft6, ft6, %[fmin]  \n"
            "fmin.s  ft6, ft6, %[fmax]  \n"
            "fmax.s  ft7, ft7, %[fmin]  \n"
            "fmin.s  ft7, ft7, %[fmax]  \n"
            "fmax.s  ft8, ft8, %[fmin]  \n"
            "fmin.s  ft8, ft8, %[fmax]  \n"
            "fmax.s  ft9, ft9, %[fmin]  \n"
            "fmin.s  ft9, ft9, %[fmax]  \n"
            "fmax.s  ft10, ft10, %[fmin] \n"
            "fmin.s  ft10, ft10, %[fmax] \n"
            "fmax.s  ft11, ft11, %[fmin] \n"
            "fmin.s  ft11, ft11, %[fmax] \n"
            "fmax.s  fa0, fa0, %[fmin]  \n"
            "fmin.s  fa0, fa0, %[fmax]  \n"
            "fmax.s  fa1, fa1, %[fmin]  \n"
            "fmin.s  fa1, fa1, %[fmax]  \n"
            "fmax.s  fa2, fa2, %[fmin]  \n"
            "fmin.s  fa2, fa2, %[fmax]  \n"
            "fmax.s  fa3, fa3, %[fmin]  \n"
            "fmin.s  fa3, fa3, %[fmax]  \n"

            // Scale all 16 by 2^31-1
            "fmul.s  ft0, ft0, %[mult]  \n"
            "fmul.s  ft1, ft1, %[mult]  \n"
            "fmul.s  ft2, ft2, %[mult]  \n"
            "fmul.s  ft3, ft3, %[mult]  \n"
            "fmul.s  ft4, ft4, %[mult]  \n"
            "fmul.s  ft5, ft5, %[mult]  \n"
            "fmul.s  ft6, ft6, %[mult]  \n"
            "fmul.s  ft7, ft7, %[mult]  \n"
            "fmul.s  ft8, ft8, %[mult]  \n"
            "fmul.s  ft9, ft9, %[mult]  \n"
            "fmul.s  ft10, ft10, %[mult] \n"
            "fmul.s  ft11, ft11, %[mult] \n"
            "fmul.s  fa0, fa0, %[mult]  \n"
            "fmul.s  fa1, fa1, %[mult]  \n"
            "fmul.s  fa2, fa2, %[mult]  \n"
            "fmul.s  fa3, fa3, %[mult]  \n"

            // Convert all 16 to int32 (round to zero)
            "fcvt.w.s t0, ft0, rtz      \n"
            "fcvt.w.s t1, ft1, rtz      \n"
            "fcvt.w.s t2, ft2, rtz      \n"
            "fcvt.w.s t3, ft3, rtz      \n"
            "fcvt.w.s t4, ft4, rtz      \n"
            "fcvt.w.s t5, ft5, rtz      \n"
            "fcvt.w.s t6, ft6, rtz      \n"
            "fcvt.w.s a0, ft7, rtz      \n"
            "fcvt.w.s a1, ft8, rtz      \n"
            "fcvt.w.s a2, ft9, rtz      \n"
            "fcvt.w.s a3, ft10, rtz     \n"
            "fcvt.w.s a4, ft11, rtz     \n"
            "fcvt.w.s a5, fa0, rtz      \n"
            "fcvt.w.s a6, fa1, rtz      \n"
            "fcvt.w.s a7, fa2, rtz      \n"
            "fcvt.w.s s2, fa3, rtz      \n"

            // Store all 16 int32 values
            "sw      t0, 0(%[dst])      \n"
            "sw      t1, 4(%[dst])      \n"
            "sw      t2, 8(%[dst])      \n"
            "sw      t3, 12(%[dst])     \n"
            "sw      t4, 16(%[dst])     \n"
            "sw      t5, 20(%[dst])     \n"
            "sw      t6, 24(%[dst])     \n"
            "sw      a0, 28(%[dst])     \n"
            "sw      a1, 32(%[dst])     \n"
            "sw      a2, 36(%[dst])     \n"
            "sw      a3, 40(%[dst])     \n"
            "sw      a4, 44(%[dst])     \n"
            "sw      a5, 48(%[dst])     \n"
            "sw      a6, 52(%[dst])     \n"
            "sw      a7, 56(%[dst])     \n"
            "sw      s2, 60(%[dst])     \n"

            // Update pointers for next iteration
            "addi    %[src], %[src], 64 \n"  // +16 samples × 4 bytes
            "addi    %[dst], %[dst], 64 \n"

            // === Loop end marker (hardware loops here with ZERO cycles!) ===
            ".hwloop_write_end: nop     \n"

            : [src] "+r" (ptrBuf), [dst] "+r" (ptrTmp)
            : [cnt] "r" (count), [mult] "f" (mult), [fmax] "f" (fmax), [fmin] "f" (fmin)
            : "t0", "t1", "t2", "t3", "t4", "t5", "t6", "s2",
              "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
              "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",
              "ft8", "ft9", "ft10", "ft11", "fa0", "fa1", "fa2", "fa3",
              "memory"
    );

    i2s_channel_write(tx_handle, tmp, sz*2*4, &nb, portMAX_DELAY);
}

#else

void IRAM_ATTR Codec::ReadBuffer(float *buf, uint32_t sz) {
    int32_t tmp[2*sz];
    int32_t *ptrTmp = tmp;
    size_t nb;
    const float div = 1.0f / 2147483648.0f;
    // 32 bit word config stereo

    i2s_channel_read(rx_handle, tmp, sz*2*4, &nb, portMAX_DELAY);

    while (sz > 0) {
        *buf++ = div * (float) *ptrTmp++;
        *buf++ = div * (float) *ptrTmp++;
        sz--;
    }
}

void IRAM_ATTR Codec::WriteBuffer(float *buf, uint32_t sz) {
    int32_t tmp[2*sz];
    int32_t tmp2;
    size_t nb;
    const float mult = 2147483647.f;

    for (int i = 0; i < sz; i++) {
        tmp2 = (int32_t) (mult * buf[i * 2]);
        tmp2 = MAX(tmp2, -2147483647);
        tmp2 = MIN(tmp2, 2147483647);
        tmp[i * 2] = tmp2;
        tmp2 = (int32_t) (mult * buf[i * 2 + 1]);
        tmp2 = MAX(tmp2, -2147483648);
        tmp2 = MIN(tmp2, 2147483647);
        tmp[i * 2 + 1] = tmp2;
    }

    i2s_channel_write(tx_handle, tmp, sz*2*4, &nb, portMAX_DELAY);
}
#endif

void Codec::InitCodec() {
    cfg_i2c();                     // 1. I2C bus ready
    identify();                    // 2. Detect AIC3254 on bus (no muting — reset wipes it)
    cfg_i2s();                     // 3. Start I2S + MCLK (codec needs MCLK for de-pop timing)

    // 3b. Pre-fill I2S TX DMA with silence to guarantee zeros on bus
    //     before codec DAC starts processing. auto_clear=true helps,
    //     but explicit write ensures all DMA descriptors have zeros.
    {
        static int32_t silence[64] = {0};  // 32 stereo frames
        size_t nb;
        i2s_channel_write(tx_handle, silence, sizeof(silence), &nb, 100);
    }

    cfg_codec();                   // 4. Full pop-free config (mute→settle→fade-in)
    SetOutputLevels(58, 58);       // 5. Set final volume (0dB default) after everything settled
}

// from pg. 26 of https://www.ti.com/lit/an/slaa408a/slaa408a.pdf?ts=1766827966822&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTLV320AIC3254
// check this https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/6/Coefficients.png
// and this https://e2e.ti.com/support/audio-group/audio/f/audio-forum/669437/tlv320aic3204-first-order-iir-filter-coefficients-for-adc as a reference
void Codec::ADCHighPassEnable() {
    // Power down ADCs before changing coefficients
    write_AIC32X4_reg(AIC32X4_ADCSETUP, 0b00000000);

    // Configure ADC to use PRB_R1 which includes IIR filter
    write_AIC32X4_reg(AIC32X4_ADCPRB, 0x01);

    // DC blocking filter (first-order HPF) at fc ≈ 3.7Hz, fs = 44100Hz
    // Transfer function: H(z) = (1 - z^-1) / (1 - α·z^-1)
    // α = exp(-2π·fc/fs) ≈ 0.999472
    // N0 = +1.0 * 2^23 = 0x7FFFFF (8388607)
    // N1 = -1.0 * 2^23 = 0x800001 (-8388607 in two's complement, 24-bit)
    // D1 = α * 2^23 ≈ 0.999473 * 8388608 ≈ 0x7FEEB9 (8384185)
    // NOTE: Previous value 0x7FB0FE was incorrect — gave ~17Hz cutoff, not 3.7Hz.
    //        Corrected to 0x7FEEB9 for actual 3.7Hz DC-blocking cutoff.

    // Switch to page 8 for left ADC channel coefficients
    write_reg(AIC32X4_PSEL, 8);
    page = 8;

    // Left channel: C4 (N0) at Page 8, Reg 24,25,26
    write_reg(24, 0x7F);  // N0 MSB
    write_reg(25, 0xFF);  // N0 MID
    write_reg(26, 0xFF);  // N0 LSB (0x7FFFFF = +8388607)

    // Left channel: C5 (N1) at Page 8, Reg 28,29,30
    write_reg(28, 0x80);  // N1 MSB
    write_reg(29, 0x00);  // N1 MID
    write_reg(30, 0x01);  // N1 LSB (0x800001 = -8388607)

    // Left channel: C6 (D1) at Page 8, Reg 32,33,34
    write_reg(32, 0x7F);  // D1 MSB
    write_reg(33, 0xEE);  // D1 MID
    write_reg(34, 0xB9);  // D1 LSB (0x7FEEB9 ≈ 8384185, α=0.999473, fc≈3.7Hz)

    // Switch to page 9 for right ADC channel coefficients
    write_reg(AIC32X4_PSEL, 9);
    page = 9;

    // Right channel: C36 (N0) at Page 9, Reg 32,33,34
    write_reg(32, 0x7F);  // N0 MSB
    write_reg(33, 0xFF);  // N0 MID
    write_reg(34, 0xFF);  // N0 LSB

    // Right channel: C37 (N1) at Page 9, Reg 36,37,38
    write_reg(36, 0x80);  // N1 MSB
    write_reg(37, 0x00);  // N1 MID
    write_reg(38, 0x01);  // N1 LSB

    // Right channel: C38 (D1) at Page 9, Reg 40,41,42
    write_reg(40, 0x7F);  // D1 MSB
    write_reg(41, 0xEE);  // D1 MID
    write_reg(42, 0xB9);  // D1 LSB (0x7FEEB9, matches left channel)

    // Switch back to page 0
    write_reg(AIC32X4_PSEL, 0);
    page = 0;

    // Power up ADCs
    write_AIC32X4_reg(AIC32X4_ADCSETUP, 0b11000000);

    ESP_LOGI(TAG, "High-pass IIR filter enabled on ADC path (3.7Hz @ 44.1kHz)");
}



void Codec::ADCHighPassDisable() {
    // power down l+r adcs
    write_AIC32X4_reg(AIC32X4_ADCSETUP, 0b00000000); // (P0_R81) power down left and right ADCs

    // Configure ADC to use PRB_R1 (same as HPF enabled)
    write_AIC32X4_reg(AIC32X4_ADCPRB, 0x01);

    // Switch to page 8 for left ADC channel coefficients
    write_reg(AIC32X4_PSEL, 8);
    page = 8;

    // Left channel: C4 (N0) at Page 8, Reg 24,25,26 - Default from Table 5-4: 0x7FFFFF00
    write_reg(24, 0x7F);  // N0 MSB
    write_reg(25, 0xFF);  // N0 MID
    write_reg(26, 0xFF);  // N0 LSB - Changed to 0xFF (closer to unity gain in Q23)

    // Left channel: C5 (N1) at Page 8, Reg 28,29,30 - Zero
    write_reg(28, 0x00);  // N1 MSB
    write_reg(29, 0x00);  // N1 MID
    write_reg(30, 0x00);  // N1 LSB

    // Left channel: C6 (D1) at Page 8, Reg 32,33,34 - Zero
    write_reg(32, 0x00);  // D1 MSB
    write_reg(33, 0x00);  // D1 MID
    write_reg(34, 0x00);  // D1 LSB

    // Switch to page 9 for right ADC channel coefficients
    write_reg(AIC32X4_PSEL, 9);
    page = 9;

    // Right channel: C36 (N0) at Page 9, Reg 32,33,34 - Default: 0x7FFFFF00
    write_reg(32, 0x7F);  // N0 MSB
    write_reg(33, 0xFF);  // N0 MID
    write_reg(34, 0xFF);  // N0 LSB

    // Right channel: C37 (N1) at Page 9, Reg 36,37,38 - Zero
    write_reg(36, 0x00);  // N1 MSB
    write_reg(37, 0x00);  // N1 MID
    write_reg(38, 0x00);  // N1 LSB

    // Right channel: C38 (D1) at Page 9, Reg 40,41,42 - Zero
    write_reg(40, 0x00);  // D1 MSB
    write_reg(41, 0x00);  // D1 MID
    write_reg(42, 0x00);  // D1 LSB

    // Switch back to page 0
    write_reg(AIC32X4_PSEL, 0);
    page = 0;

    write_AIC32X4_reg(AIC32X4_ADCSETUP, 0b11000000); // (P0_R81) power up left and right ADCs

    ESP_LOGI(TAG, "High-pass filter disabled on ADC path (all-pass/bypass mode)");
}

