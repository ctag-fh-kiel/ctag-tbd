#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "soc/soc_caps.h"
#include "esp_memory_utils.h"
#include <string.h>

static const char* TAG = "custom_sdmmc_cmd";

// Declare sdmmc_read_sectors_dma which is in the SDK
extern "C"  esp_err_t sdmmc_read_sectors_dma(
    sdmmc_card_t* card,
    void* dst,
    size_t start_block,
    size_t block_count,
    size_t buffer_len
);

extern "C"  esp_err_t sdmmc_write_sectors_dma(
    sdmmc_card_t* card,
    const void* src,
    size_t start_block,
    size_t block_count,
    size_t buffer_len
);

//DMA_ATTR static uint8_t sector_buffer[512]; // -> runtime error
//__attribute__((section(".dram0"), aligned(4))) // -> linker warning
//uint8_t sector_buffer[512];

// Static pointer for DMA buffer - allocated once on first use
#define MAX_SECTORS_PER_TRANSFER 32  // Maximum sectors to transfer at once, have not observed anything bigger
#define DMA_BUFFER_SIZE (MAX_SECTORS_PER_TRANSFER * 512)  // 16KB buffer (32 sectors of 512 bytes)
static uint8_t* sector_buffer = NULL;
static size_t sector_buffer_actual_size = 0; // actual allocated size (may be larger due to heap alignment)

// Ensure buffer is allocated (called before first use)
static esp_err_t ensure_buffer_allocated()
{
    if (sector_buffer == NULL) {
        sector_buffer = (uint8_t*)heap_caps_malloc(DMA_BUFFER_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT);
        //sector_buffer = (uint8_t*)heap_caps_malloc(DMA_BUFFER_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);
        if (sector_buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate DMA buffer");
            return ESP_ERR_NO_MEM;
        }
        sector_buffer_actual_size = heap_caps_get_allocated_size(sector_buffer);
        ESP_LOGI(TAG, "DMA buffer allocated at %p (requested: %d bytes, actual: %zu bytes)",
                 sector_buffer, DMA_BUFFER_SIZE, sector_buffer_actual_size);
    }
    return ESP_OK;
}

// Your wrapped implementation
extern "C" esp_err_t __wrap_sdmmc_read_sectors(sdmmc_card_t* card, void* dst, size_t start_block, size_t block_count)
{
    if (block_count == 0) {
        return ESP_OK;
    }

    size_t block_size = card->csd.sector_size;
    // only works for block size 512
    assert(block_size == 512);

    // Fast path: if buffer is already DMA-capable and aligned, use it directly
    bool is_aligned = card->host.check_buffer_alignment(card->host.slot, dst, block_size * block_count);
    if (is_aligned
        #if !SOC_SDMMC_PSRAM_DMA_CAPABLE
            && !esp_ptr_external_ram(dst)
        #endif
    ) {
        // Buffer is suitable for direct DMA - bypass wrapper overhead
        //ESP_LOGD(TAG, "Direct DMA: %zu blocks to buffer at %p", block_count, dst);
        return sdmmc_read_sectors_dma(card, dst, start_block, block_count, block_size * block_count);
    }

    // Slow path: buffer not DMA-capable or not aligned, use batched multi-sector reads
    esp_err_t err = ensure_buffer_allocated();
    if (err != ESP_OK) {
        return err;
    }

    uint8_t* cur_dst = (uint8_t*)dst;
    size_t blocks_remaining = block_count;
    size_t current_block = start_block;

    ESP_LOGD(TAG, "Batched read: %zu blocks (max %d per transfer)", block_count, MAX_SECTORS_PER_TRANSFER);

    // Process blocks in batches of up to MAX_SECTORS_PER_TRANSFER
    while (blocks_remaining > 0) {
        // Calculate blocks to read in this iteration (1-32 blocks)
        size_t blocks_to_read = (blocks_remaining > MAX_SECTORS_PER_TRANSFER)
                                ? MAX_SECTORS_PER_TRANSFER
                                : blocks_remaining;
        size_t bytes_to_copy = blocks_to_read * block_size;

        // Validate we have enough buffer space
        if (bytes_to_copy > sector_buffer_actual_size) {
            ESP_LOGE(TAG, "Buffer too small: need %zu, have %zu", bytes_to_copy, sector_buffer_actual_size);
            return ESP_ERR_NO_MEM;
        }

        // Read multiple blocks at once into DMA buffer (handles 1-32 blocks)
        err = sdmmc_read_sectors_dma(card, sector_buffer, current_block, blocks_to_read, sector_buffer_actual_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error 0x%x reading %zu blocks at sector %zu", err, blocks_to_read, current_block);
            break;
        }

        // Copy from DMA buffer to destination
        memcpy(cur_dst, sector_buffer, bytes_to_copy);

        cur_dst += bytes_to_copy;
        current_block += blocks_to_read;
        blocks_remaining -= blocks_to_read;
    }

    return err;
}



extern "C" esp_err_t __wrap_sdmmc_write_sectors(sdmmc_card_t* card, const void* src,
        size_t start_block, size_t block_count)
{
    if (block_count == 0) {
        return ESP_OK;
    }

    size_t block_size = card->csd.sector_size;
    // only works for block size 512
    assert(block_size == 512);

    // Fast path: if buffer is already DMA-capable and aligned, use it directly
    bool is_aligned = card->host.check_buffer_alignment(card->host.slot, src, block_size * block_count);
    if (is_aligned
        #if !SOC_SDMMC_PSRAM_DMA_CAPABLE
            && !esp_ptr_external_ram(src)
        #endif
    ) {
        // Buffer is suitable for direct DMA - bypass wrapper overhead
        //ESP_LOGD(TAG, "Direct DMA write: %zu blocks from buffer at %p", block_count, src);
        return sdmmc_write_sectors_dma(card, src, start_block, block_count, block_size * block_count);
    }

    // Slow path: buffer not DMA-capable or not aligned, use batched multi-sector writes
    esp_err_t err = ensure_buffer_allocated();
    if (err != ESP_OK) {
        return err;
    }

    const uint8_t* cur_src = (const uint8_t*)src;
    size_t blocks_remaining = block_count;
    size_t current_block = start_block;

    ESP_LOGD(TAG, "Batched write: %zu blocks (max %d per transfer)", block_count, MAX_SECTORS_PER_TRANSFER);

    // Process blocks in batches of up to MAX_SECTORS_PER_TRANSFER
    while (blocks_remaining > 0) {
        // Calculate blocks to write in this iteration (1-32 blocks)
        size_t blocks_to_write = (blocks_remaining > MAX_SECTORS_PER_TRANSFER)
                                 ? MAX_SECTORS_PER_TRANSFER
                                 : blocks_remaining;
        size_t bytes_to_copy = blocks_to_write * block_size;

        // Validate we have enough buffer space
        if (bytes_to_copy > sector_buffer_actual_size) {
            ESP_LOGE(TAG, "Buffer too small: need %zu, have %zu", bytes_to_copy, sector_buffer_actual_size);
            return ESP_ERR_NO_MEM;
        }

        // Copy from source to DMA buffer
        memcpy(sector_buffer, cur_src, bytes_to_copy);

        // Write multiple blocks at once from DMA buffer (handles 1-32 blocks)
        err = sdmmc_write_sectors_dma(card, sector_buffer, current_block, blocks_to_write, sector_buffer_actual_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error 0x%x writing %zu blocks at sector %zu", err, blocks_to_write, current_block);
            break;
        }

        cur_src += bytes_to_copy;
        current_block += blocks_to_write;
        blocks_remaining -= blocks_to_write;
    }

    return err;
}