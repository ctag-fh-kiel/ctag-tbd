// Copyright (C) 2025 Piers Finlayson <piers@piers.rocks>
//
// MIT License

import { PICOBOOT_MAGIC } from './constants.js';

export class PicobootCmdId {
    static EXCLUSIVE_ACCESS = 0x1;
    static REBOOT = 0x2;
    static REBOOT2 = 0xA;
    static FLASH_ERASE = 0x3;
    static WRITE = 0x5;
    static EXIT_XIP = 0x6;
    static ENTER_XIP = 0x7;
    static EXEC = 0x8;
    static VECTORIZE_FLASH = 0x9;
    static OTP_WRITE = 0xD;
    static GET_INFO = 0x8B;
    static OTP_READ = 0x8C;
    static READ = 0x84;

    /**
     * @param {number} cmdId
     * @returns {string}
     */
    static toString(cmdId) {
        switch (cmdId) {
            case this.EXCLUSIVE_ACCESS: return 'EXCLUSIVE_ACCESS';
            case this.REBOOT: return 'REBOOT';
            case this.REBOOT2: return 'REBOOT2';
            case this.FLASH_ERASE: return 'FLASH_ERASE';
            case this.WRITE: return 'WRITE';
            case this.EXIT_XIP: return 'EXIT_XIP';
            case this.ENTER_XIP: return 'ENTER_XIP';
            case this.EXEC: return 'EXEC';
            case this.VECTORIZE_FLASH: return 'VECTORIZE_FLASH';
            case this.GET_INFO: return 'GET_INFO';
            case this.OTP_READ: return 'OTP_READ';
            case this.OTP_WRITE: return 'OTP_WRITE';
            case this.READ: return 'READ';
            default: return 'UNKNOWN';
        }
    }

    /**
     * @param {number} cmdId
     * @returns {'IN'|'OUT'}
     */
    static direction(cmdId) {
        return (cmdId & 0x80) !== 0 ? 'IN' : 'OUT';
    }
}

export class PicobootStatus {
    static OK = 0;
    static UNKNOWN_CMD = 1;
    static INVALID_CMD_LENGTH = 2;
    static INVALID_TRANSFER_LENGTH = 3;
    static INVALID_ADDRESS = 4;
    static BAD_ALIGNMENT = 5;
    static INTERLEAVED_WRITE = 6;
    static REBOOTING = 7;
    static UNKNOWN_ERROR = 8;
    static INVALID_STATE = 9;
    static NOT_PERMITTED = 10;
    static INVALID_ARG = 11;
    static BUFFER_TOO_SMALL = 12;
    static PRECONDITION_NOT_MET = 13;
    static MODIFIED_DATA = 14;
    static INVALID_DATA = 15;
    static NOT_FOUND = 16;
    static UNSUPPORTED_MODIFICATION = 17;
}

export class PicobootCmd {
    /**
     * @param {number} cmdId
     * @param {number} cmdSize
     * @param {number} transferLen
     * @param {Uint8Array} args
     */
    constructor(cmdId, cmdSize, transferLen, args) {
        this.magic = PICOBOOT_MAGIC;
        this.token = 0;
        this.cmdId = cmdId;
        this.cmdSize = cmdSize;
        this.transferLen = transferLen;
        this.args = args;
    }

    /**
     * @param {number} token
     * @returns {PicobootCmd}
     */
    setToken(token) {
        this.token = token;
        return this;
    }

    /**
     * @returns {number}
     */
    getTransferLen() {
        return this.transferLen;
    }

    /**
     * @returns {number}
     */
    id() {
        return this.cmdId;
    }

    /**
     * @returns {'IN'|'OUT'}
     */
    direction() {
        return PicobootCmdId.direction(this.cmdId);
    }

    /**
     * @returns {boolean}
     */
    isDataTransfer() {
        return this.transferLen !== 0;
    }

    /**
     * @returns {Uint8Array}
     */
    toBytes() {
        const buffer = new ArrayBuffer(32);
        const view = new DataView(buffer);
        
        view.setUint32(0, this.magic, true);
        view.setUint32(4, this.token, true);
        view.setUint8(8, this.cmdId);
        view.setUint8(9, this.cmdSize);
        view.setUint16(10, 0, true);
        view.setUint32(12, this.transferLen, true);
        
        for (let i = 0; i < 16; i++) {
            view.setUint8(16 + i, this.args[i]);
        }
        
        return new Uint8Array(buffer);
    }

    /**
     * @param {number} exclusive
     * @returns {PicobootCmd}
     */
    static exclusiveAccess(exclusive) {
        const args = new Uint8Array(16);
        args[0] = exclusive;
        return new PicobootCmd(PicobootCmdId.EXCLUSIVE_ACCESS, 1, 0, args);
    }

    /**
     * @param {number} pc
     * @param {number} sp
     * @param {number} delay
     * @returns {PicobootCmd}
     */
    static reboot(pc, sp, delay) {
        const args = new Uint8Array(16);
        const view = new DataView(args.buffer);
        view.setUint32(0, pc, true);
        view.setUint32(4, sp, true);
        view.setUint32(8, delay, true);
        view.setUint32(12, 0, true);
        return new PicobootCmd(PicobootCmdId.REBOOT, 12, 0, args);
    }

    /**
     * @param {number} flags
     * @param {number} p0
     * @param {number} p1
     * @param {number} delay
     * @returns {PicobootCmd}
     */
    static reboot2(flags, p0, p1, delay) {
        const args = new Uint8Array(16);
        const view = new DataView(args.buffer);
        view.setUint32(0, flags, true);
        view.setUint32(4, delay, true);
        view.setUint32(8, p0, true);
        view.setUint32(12, p1, true);
        return new PicobootCmd(PicobootCmdId.REBOOT2, 0x10, 0, args);
    }

    /**
     * @param {number} addr
     * @param {number} size
     * @returns {PicobootCmd}
     */
    static flashErase(addr, size) {
        const args = new Uint8Array(16);
        const view = new DataView(args.buffer);
        view.setUint32(0, addr, true);
        view.setUint32(4, size, true);
        view.setUint32(8, 0, true);
        view.setUint32(12, 0, true);
        return new PicobootCmd(PicobootCmdId.FLASH_ERASE, 8, 0, args);
    }

    /**
     * @param {number} addr
     * @param {number} size
     * @returns {PicobootCmd}
     */
    static flashWrite(addr, size) {
        const args = new Uint8Array(16);
        const view = new DataView(args.buffer);
        view.setUint32(0, addr, true);
        view.setUint32(4, size, true);
        view.setUint32(8, 0, true);
        view.setUint32(12, 0, true);
        return new PicobootCmd(PicobootCmdId.WRITE, 8, size, args);
    }

    /**
     * @param {number} addr
     * @param {number} size
     * @returns {PicobootCmd}
     */
    static flashRead(addr, size) {
        const args = new Uint8Array(16);
        const view = new DataView(args.buffer);
        view.setUint32(0, addr, true);
        view.setUint32(4, size, true);
        view.setUint32(8, 0, true);
        view.setUint32(12, 0, true);
        return new PicobootCmd(PicobootCmdId.READ, 8, size, args);
    }

    /**
     * @returns {PicobootCmd}
     */
    static enterXip() {
        const args = new Uint8Array(16);
        return new PicobootCmd(PicobootCmdId.ENTER_XIP, 0, 0, args);
    }

    /**
     * @returns {PicobootCmd}
     */
    static exitXip() {
        const args = new Uint8Array(16);
        return new PicobootCmd(PicobootCmdId.EXIT_XIP, 0, 0, args);
    }
    
    /**
     * @param {number} firstRow - first OTP row to read (1 row = 16 bytes)
     * @param {number} rowCount - number of OTP rows to write (1 row = 16 bytes)
     * @param {boolean} ecc - whether to return ECC rows
     * @returns {PicobootCmd}
     */
    static otpRead(firstRow, rowCount, ecc) {
        let size = rowCount * (ecc ? 2 : 4);

        const args = new Uint8Array(16);
        const view = new DataView(args.buffer);

        view.setUint16(0, firstRow, true);
        view.setUint16(2, rowCount, true);
        view.setUint8(4, ecc ? 1 : 0);
        console.log(`OTP Read Command - firstRow: ${firstRow}, rowCount: ${rowCount}, ecc: ${ecc}, size: ${size}`);
        return new PicobootCmd(PicobootCmdId.OTP_READ, 5, size, args);
    }

    /**
     * @param {number} rowIndex - OTP row index to write (1 row = 16 bytes)
     * @param {Uint8Array} data - OTP data
     * @param {boolean} ecc - whether to write ECC 16-bit rows or 32-bit rows
     */
    static otpWrite(rowIndex, data, ecc) {
        // Check data length is multiple of 2 bytes (ecc) or 4 bytes (no ecc)
        if (ecc && (data.length % 2 !== 0)) {
            throw new Error('OTP data length must be multiple of 2 bytes when writing with ECC');
        }
        if (!ecc && (data.length % 4 !== 0)) {
            throw new Error('OTP data length must be multiple of 4 bytes when writing without ECC');
        }

        let rowCount = data.length / (ecc ? 2 : 4);

        const args = new Uint8Array(16);
        const view = new DataView(args.buffer);
        
        view.setUint16(0, rowIndex, true);
        view.setUint16(2, rowCount, true);
        view.setUint8(4, ecc ? 1 : 0);
        console.log(`OTP Write Command - rowIndex: ${rowIndex}, rowCount: ${rowCount}, ecc: ${ecc}, dataLength: ${data.length}`);
        return new PicobootCmd(PicobootCmdId.OTP_WRITE, 5, data.length, args);
    }

}

export class PicobootStatusCmd {
    /**
     * @param {Uint8Array} data - 16-byte status response
     */
    constructor(data) {
        const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
        this.token = view.getUint32(0, true);
        this.statusCode = view.getUint32(4, true);
        this.cmdId = view.getUint8(8);
        this.inProgress = view.getUint8(9);
    }

    /**
     * @returns {number}
     */
    getToken() {
        return this.token;
    }

    /**
     * @returns {number}
     */
    getStatusCode() {
        return this.statusCode;
    }

    /**
     * @returns {string}
     */
    getStatusName() {
        switch (this.statusCode) {
            case PicobootStatus.OK: return 'OK';
            case PicobootStatus.UNKNOWN_CMD: return 'UNKNOWN_CMD';
            case PicobootStatus.INVALID_CMD_LENGTH: return 'INVALID_CMD_LENGTH';
            case PicobootStatus.INVALID_TRANSFER_LENGTH: return 'INVALID_TRANSFER_LENGTH';
            case PicobootStatus.INVALID_ADDRESS: return 'INVALID_ADDRESS';
            case PicobootStatus.BAD_ALIGNMENT: return 'BAD_ALIGNMENT';
            case PicobootStatus.INTERLEAVED_WRITE: return 'INTERLEAVED_WRITE';
            case PicobootStatus.REBOOTING: return 'REBOOTING';
            case PicobootStatus.UNKNOWN_ERROR: return 'UNKNOWN_ERROR';
            case PicobootStatus.INVALID_STATE: return 'INVALID_STATE';
            case PicobootStatus.NOT_PERMITTED: return 'NOT_PERMITTED';
            case PicobootStatus.INVALID_ARG: return 'INVALID_ARG';
            case PicobootStatus.BUFFER_TOO_SMALL: return 'BUFFER_TOO_SMALL';
            case PicobootStatus.PRECONDITION_NOT_MET: return 'PRECONDITION_NOT_MET';
            case PicobootStatus.MODIFIED_DATA: return 'MODIFIED_DATA';
            case PicobootStatus.INVALID_DATA: return 'INVALID_DATA';
            case PicobootStatus.NOT_FOUND: return 'NOT_FOUND';
            case PicobootStatus.UNSUPPORTED_MODIFICATION: return 'UNSUPPORTED_MODIFICATION';
            default: return `UNKNOWN(${this.statusCode})`;
        }
    }

    /**
     * @returns {number}
     */
    getCmdId() {
        return this.cmdId;
    }

    /**
     * @returns {string}
     */
    getCmdName() {
        return PicobootCmdId.toString(this.cmdId);
    }

    /**
     * @returns {boolean}
     */
    isInProgress() {
        return this.inProgress !== 0;
    }

    /**
     * @returns {boolean}
     */
    isOk() {
        return this.statusCode === PicobootStatus.OK;
    }

    /**
     * @returns {string}
     */
    toString() {
        return `Status[token=${this.token}, status=${this.getStatusName()}, cmd=${this.getCmdName()}, inProgress=${this.isInProgress()}]`;
    }
}