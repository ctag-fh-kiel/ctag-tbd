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

#include <cassert>

#include "display.hpp"
#include "ssd1306_draw.h"
#include "ssd1306_font.h"
#include "ssd1306_default_if.h"

using namespace CTAG::DRIVERS;

const int Display::I2CDisplayAddress = 0x3C;
const int Display::I2CDisplayWidth = 128;
const int Display::I2CDisplayHeight = 64;
const int Display::I2CResetPin = -1;
struct SSD1306_Device Display::I2CDisplay;

void Display::Init() {
    assert( SSD1306_I2CMasterInitDefault( ) == true );
    assert( SSD1306_I2CMasterAttachDisplayDefault( &I2CDisplay, I2CDisplayWidth, I2CDisplayHeight, I2CDisplayAddress, I2CResetPin ) == true );
    SSD1306_SetHFlip(&I2CDisplay, true);
    SSD1306_SetVFlip(&I2CDisplay, true);
}

void Display::Demo() {
    SSD1306_Clear( &I2CDisplay, SSD_COLOR_BLACK );
    SSD1306_SetFont( &I2CDisplay, &Font_liberation_mono_17x30 );
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, "TBD :)", SSD_COLOR_WHITE );
    SSD1306_Update( &I2CDisplay );
}
