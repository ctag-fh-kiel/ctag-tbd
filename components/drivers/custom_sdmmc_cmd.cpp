#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include <string.h>

static const char* TAG = "custom_sdmmc_cmd";

// Declare the original function that will be wrapped
extern "C" esp_err_t __real_sdmmc_read_sectors(sdmmc_card_t* card, void* dst, size_t start_block, size_t block_count);

// Declare sdmmc_read_sectors_dma which is in the SDK
extern "C" esp_err_t sdmmc_read_sectors_dma(
    sdmmc_card_t* card,
    void* dst,
    size_t start_block,
    size_t block_count,
    size_t buffer_len
);

//DMA_ATTR static uint8_t sector_buffer[512]; // -> runtime error
//__attribute__((section(".dram0"), aligned(4))) // -> linker warning
//uint8_t sector_buffer[512];

// Static pointer for DMA buffer - allocated once on first use
static uint8_t* sector_buffer = nullptr; // dynamic version -> ok

// Ensure buffer is allocated (called before first use)
static esp_err_t ensure_buffer_allocated()
{
    if (sector_buffer == nullptr) {
        sector_buffer = (uint8_t*)heap_caps_malloc(512, MALLOC_CAP_DMA);
        if (sector_buffer == nullptr) {
            ESP_LOGE(TAG, "Failed to allocate DMA buffer");
            return ESP_ERR_NO_MEM;
        }
        ESP_LOGI(TAG, "DMA buffer allocated at %p", sector_buffer);
    }
    return ESP_OK;
}

// Your wrapped implementation
extern "C" esp_err_t __wrap_sdmmc_read_sectors(sdmmc_card_t* card, void* dst, size_t start_block, size_t block_count)
{
    if (block_count == 0) {
        return ESP_OK;
    }

    esp_err_t err = ensure_buffer_allocated();
    if (err != ESP_OK) {
        return err;
    }

    size_t block_size = card->csd.sector_size;
    // only works for block size 512
    assert(block_size == 512);

    uint8_t* cur_dst = (uint8_t*)dst;

    for (size_t i = 0; i < block_count; ++i) {
        err = sdmmc_read_sectors_dma(card, sector_buffer, start_block + i, 1, 512);
        if (err != ESP_OK) {
            ESP_LOGD(TAG, "Error 0x%x reading block %zu+%zu", err, start_block, i);
            break;
        }
        memcpy(cur_dst, sector_buffer, block_size);
        cur_dst += block_size;
    }

    return err;
}


extern "C" esp_err_t sdmmc_write_sectors_dma(sdmmc_card_t* card, const void* src,
        size_t start_block, size_t block_count, size_t buffer_len);

extern "C" esp_err_t __real_sdmmc_write_sectors(sdmmc_card_t* card, const void* src,
        size_t start_block, size_t block_count);

extern "C" esp_err_t __wrap_sdmmc_write_sectors(sdmmc_card_t* card, const void* src,
        size_t start_block, size_t block_count)
{
    if (block_count == 0) {
        return ESP_OK;
    }

    esp_err_t err = ensure_buffer_allocated();
    if (err != ESP_OK) {
        return err;
    }

    size_t block_size = card->csd.sector_size;
    // only works for block size 512
    assert(block_size == 512);

    const uint8_t* cur_src = (const uint8_t*) src;
    for (size_t i = 0; i < block_count; ++i){
        memcpy(sector_buffer, cur_src, block_size);
        cur_src += block_size;
        err = sdmmc_write_sectors_dma(card, sector_buffer, start_block + i, 1, 512);
        if (err != ESP_OK) {
            ESP_LOGD(TAG, "%s: error 0x%x writing block %d+%d",
                    __func__, err, start_block, i);
            break;
        }
    }
    return err;
}
