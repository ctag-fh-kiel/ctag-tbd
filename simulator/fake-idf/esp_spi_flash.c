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

#include "esp_spi_flash.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static char* file_buffer = NULL;

void spi_flash_emu_init(const char *sromFile) {
    if(NULL == sromFile) return;
    FILE *f = fopen(sromFile, "rb");
    assert(f != NULL);
    fseek(f, 0L, SEEK_END);
    int sz=ftell(f);
    rewind(f);
    file_buffer = (char*)malloc(sz + 1);
    assert(file_buffer != NULL);
    int res = fread(file_buffer, sz, 1, f);
    assert(res != 0);
    fclose(f);
}

void spi_flash_emu_release(){
    free(file_buffer);
}

esp_err_t spi_flash_read(size_t src, void *dstv, size_t size){
    if(NULL == file_buffer) return ESP_ERR_INVALID_ARG;
    memcpy(dstv, (const void*)&file_buffer[src], size);
    return ESP_OK;
}