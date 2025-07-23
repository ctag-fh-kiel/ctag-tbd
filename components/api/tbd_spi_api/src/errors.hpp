#pragma once

#include <tbd/errors.hpp>


TBD_NEW_ERR(SPI_API_FAILED_TO_RESERVE_SEND_BUFFER, "could not reserve capable DMA memory for SPI output buffer")
TBD_NEW_ERR(SPI_API_FAILED_TO_RESERVE_RECEIVE_BUFFER, "could not reserve capable DMA memory for SPI input buffer")
