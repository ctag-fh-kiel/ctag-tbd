// Copyright (C) 2025 Piers Finlayson <piers@piers.rocks>
//
// MIT License

import {
    PICOBOOT_VID,
    PICOBOOT_PID_RP2040,
    PICOBOOT_PID_RP2350,
    FLASH_START,
    SECTOR_SIZE,
    PAGE_SIZE,
    STACK_POINTER_RP2040,
    STACK_POINTER_RP2350,
} from './constants.js';

export class Target {
    /**
     * @param {string} type - 'RP2040', 'RP2350', or 'CUSTOM'
     * @param {number} [vid]
     * @param {number} [pid]
     */
    constructor(type, vid, pid) {
        this.type = type;
        if (type === 'CUSTOM') {
            this.vid = vid;
            this.pid = pid;
        } else {
            this.vid = PICOBOOT_VID;
            this.pid = type === 'RP2040' ? PICOBOOT_PID_RP2040 : PICOBOOT_PID_RP2350;
        }
    }

    /**
     * @param {USBDevice} device
     * @returns {Target}
     */
    static fromDevice(device) {
        const vid = device.vendorId;
        const pid = device.productId;
        
        if (vid === PICOBOOT_VID && pid === PICOBOOT_PID_RP2040) {
            return new Target('RP2040');
        } else if (vid === PICOBOOT_VID && pid === PICOBOOT_PID_RP2350) {
            return new Target('RP2350');
        } else {
            return new Target('CUSTOM', vid, pid);
        }
    }

    /**
     * @returns {number}
     */
    getVid() {
        return this.vid;
    }

    /**
     * @returns {number}
     */
    getPid() {
        return this.pid;
    }

    /**
     * @returns {number}
     */
    flashStart() {
        return FLASH_START;
    }

    /**
     * @returns {number}
     */
    flashSectorSize() {
        return SECTOR_SIZE;
    }

    /**
     * @returns {number}
     */
    flashPageSize() {
        return PAGE_SIZE;
    }

    /**
     * @returns {number|null}
     */
    defaultStackPointer() {
        switch (this.type) {
            case 'RP2040': return STACK_POINTER_RP2040;
            case 'RP2350': return STACK_POINTER_RP2350;
            default: return null;
        }
    }

    /**
     * @returns {string}
     */
    toString() {
        if (this.type === 'CUSTOM') {
            return `${this.vid.toString(16).padStart(4, '0')}:${this.pid.toString(16).padStart(4, '0')}`;
        }
        return this.type;
    }
}
