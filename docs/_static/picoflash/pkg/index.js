// Copyright (C) 2025 Piers Finlayson <piers@piers.rocks>
//
// MIT License

export { Picoboot } from './picoboot.js';
export { Connection } from './connection.js';
export { Target } from './target.js';
export { PicobootCmd, PicobootCmdId, PicobootStatus } from './commands.js';
export {
    PicobootError,
    UsbError,
    ProtocolError,
    ValidationError,
    NotFoundError,
} from './errors.js';
export {
    ROM_START,
    ROM_END_RP2040,
    ROM_END_RP2350,
    FLASH_START,
    FLASH_END_RP2040,
    FLASH_END_RP2350,
    XIP_SRAM_START_RP2040,
    XIP_SRAM_END_RP2040,
    XIP_SRAM_START_RP2350,
    XIP_SRAM_END_RP2350,
    SRAM_START_RP2040,
    SRAM_END_RP2040,
    SRAM_END_RP2350,
    PAGE_SIZE,
    SECTOR_SIZE,
    STACK_POINTER_RP2040,
    STACK_POINTER_RP2350,
    PICOBOOT_VID,
    PICOBOOT_PID_RP2040,
    PICOBOOT_PID_RP2350,
    PICOBOOT_MAGIC,
    UF2_RP2040_FAMILY_ID,
    UF2_ABSOLUTE_FAMILY_ID,
    UF2_DATA_FAMILY_ID,
    UF2_RP2350_ARM_S_FAMILY_ID,
    UF2_RP2350_RISCV_FAMILY_ID,
    UF2_RP2350_ARM_NS_FAMILY_ID,
    UF2_FAMILY_ID_MAX,
} from './constants.js';
