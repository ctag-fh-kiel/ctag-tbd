#include "esp_flash.h"
#include "esp_spi_flash.h"


esp_err_t esp_flash_read(esp_flash_t *chip, void *buffer, unsigned int address, unsigned int length){
    return spi_flash_read(address, buffer, length);
}