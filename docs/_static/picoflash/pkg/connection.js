// Copyright (C) 2025 Piers Finlayson <piers@piers.rocks>
//
// MIT License

import { PicobootCmd, PicobootCmdId, PicobootStatusCmd } from './commands.js';
import { UsbError, ProtocolError, StatusError, ValidationError } from './errors.js';
import { REQUEST_RESET, REQUEST_GET_COMMAND_STATUS, RESPONSE_GET_COMMAND_STATUS_SIZE, DEFAULT_ENDPOINT_TIMEOUT, DEFAULT_RESET_TIMEOUT } from './constants.js';
import { Target } from './target.js';

export class Connection {
    /**
     * @param {USBDevice} device
     * @param {Target} target
     * @param {USBInterface} usbInterface
     * @param {number} outEp
     * @param {number} inEp
     * @param {number} inEpMaxPacketSize
     * @param {Object} timeouts
     */
    constructor(device, target, usbInterface, outEp, inEp, inEpMaxPacketSize, timeouts) {
        this.device = device;
        this.target = target;
        this.interface = usbInterface;
        this.outEp = outEp;
        this.inEp = inEp;
        this.inEpMaxPacketSize = inEpMaxPacketSize;
        this.timeouts = timeouts;
        this.cmdToken = 1;
    }

    /**
     * @returns {Promise<PicobootStatusCmd>}
     */
    async getCommandStatus() {
        console.log('Getting command status');
        
        let result;
        try {
            result = await this.device.controlTransferIn({
                requestType: 'vendor',
                recipient: 'interface',
                request: REQUEST_GET_COMMAND_STATUS,
                value: 0,
                index: this.interface.interfaceNumber
            }, RESPONSE_GET_COMMAND_STATUS_SIZE);
        } catch (e) {
            throw new UsbError(
                `Failed to get command status: ${e.message}`,
                this.target,
                e
            );
        }
        
        console.log(`Control transfer completed with status: ${result.status}`);
        if (result.status !== 'ok') {
            throw new StatusError(
                `Control transfer returned status: ${result.status}`,
                this.target,
                result.status
            );
        }
        
        const data = new Uint8Array(result.data.buffer);
        const status = new PicobootStatusCmd(data);
        
        console.log(`Command status: ${status.toString()}`);
        
        return status;
    }

    /**
     * @returns {Promise<void>}
     */
    async resetInterface() {
        console.log('Resetting PICOBOOT interface');

        // Send INTERFACE_RESET request to the device - this causes it to
        // clear HALT on both bulk endpoints
        try {
            await this.device.controlTransferOut({
                requestType: 'vendor',
                recipient: 'interface',
                request: REQUEST_RESET,
                value: 0,
                index: this.interface.interfaceNumber
            });
            console.log('Interface reset complete');
        } catch (e) {
            throw new UsbError(
                `Failed to reset PICOBOOT interface: ${e.message}`,
                this.target,
                e
            );
        }

        // After INTERFACE_RESET, clear both local bulk endpoints.  This
        // causes the browser to allow us to continue using them.
        await this.device.clearHalt('in', this.inEp);
        await this.device.clearHalt('out', this.outEp);
        console.log('Cleared WebUSB endpoint halts');
    }

    /**
     * @param {PicobootCmd} cmd
     * @param {Uint8Array} [buf]
     * @returns {Promise<Uint8Array>}
     */
    async sendCmd(cmd, buf) {
        cmd.setToken(this.nextToken());
        
        const cmdBytes = cmd.toBytes();
        console.log(`Sending command ${PicobootCmdId.toString(cmd.id())} (token: ${cmd.token})`);
        
        await this.bulkWrite(cmdBytes, true);
        
        let result = new Uint8Array(0);
        
        if (cmd.isDataTransfer()) {
            if (cmd.direction() === 'IN') {
                result = await this.bulkRead(cmd.getTransferLen(), true);
            } else {
                if (!buf) {
                    throw new ProtocolError(
                        `Missing buffer for OUT data transfer command ${PicobootCmdId.toString(cmd.id())}`,
                        this.target
                    );
                }
                await this.bulkWrite(buf, true);
            }
        }
        
        if (cmd.direction() === 'IN') {
            await this.bulkWrite(new Uint8Array([0]), false);
        } else {
            await this.bulkRead(1, false);
        }
        
        return result;
    }

    /**
     * @param {number} exclusive - 0: not exclusive, 1: exclusive, 2: exclusive and eject
     * @returns {Promise<void>}
     */
    async setExclusiveAccess(exclusive) {
        console.log(`Setting exclusive access mode: ${exclusive}`);
        const cmd = PicobootCmd.exclusiveAccess(exclusive);
        await this.sendCmd(cmd, null);
    }

    /**
     * @param {number} delayMs
     * @returns {Promise<void>}
     */
    async reboot(delayMs) {
        console.log(`Rebooting device with ${delayMs}ms delay`);
        
        if (this.target.type === 'RP2040') {
            await this.rebootRp2040(0, this.target.defaultStackPointer(), delayMs);
        } else if (this.target.type === 'RP2350') {
            await this.rebootRp2350(0, 0, 0, delayMs);
        } else {
            throw new ProtocolError(
                'Reboot command not supported for custom targets',
                this.target
            );
        }
    }

    /**
     * @param {number} delayMs
     * @returns {Promise<void>}
     */
    async rebootBootsel(delayMs) {
        console.log(`Rebooting device into BOOTSEL with ${delayMs}ms delay`);

        if (this.target.type === 'RP2350') {
            console.log('RP2350 Reboot to BOOTSEL');
            const flags = 0x0002;
            await this.rebootRp2350(flags, 0, 0, delayMs);
        } else if (this.target.type === 'RP2040') {
            throw new ProtocolError(
                'Reboot to BOOTSEL command not supported for RP2040 targets',
                this.target
            );
        } else {
            throw new ProtocolError(
                'Reboot to BOOTSEL command not supported for RP2custom targets',
                this.target
            );
        }
    }

    /**
     * @param {number} pc
     * @param {number} sp
     * @param {number} delayMs
     * @returns {Promise<void>}
     */
    async rebootRp2040(pc, sp, delayMs) {
        if (this.target.type === 'RP2350') {
            throw new ProtocolError(
                'REBOOT command not supported on RP2350',
                this.target
            );
        }
        
        console.log(`Rebooting RP2040: pc=${pc.toString(16)}, sp=${sp.toString(16)}, delay=${delayMs}ms`);
        const cmd = PicobootCmd.reboot(pc, sp, delayMs);
        await this.sendCmd(cmd, null);
    }

    /**
     * @param {number} flags
     * @param {number} p0
     * @param {number} p1
     * @param {number} delayMs
     * @returns {Promise<void>}
     */
    async rebootRp2350(flags, p0, p1, delayMs) {
        if (this.target.type === 'RP2040') {
            throw new ProtocolError(
                'REBOOT2 command not supported on RP2040',
                this.target
            );
        }
        
        console.log(`Rebooting RP2350: flags=${flags}, p0=${p0}, p1=${p1}, delay=${delayMs}ms`);
        const cmd = PicobootCmd.reboot2(flags, p0, p1, delayMs);
        await this.sendCmd(cmd, null);
    }

    /**
     * @param {number} size
     * @returns {Promise<void>}
     */
    async flashEraseStart(size) {
        const sectorSize = this.target.flashSectorSize();
        const alignedSize = Math.ceil(size / sectorSize) * sectorSize;
        console.log(`Erasing flash from start, size rounded to ${alignedSize} bytes`);
        await this.flashErase(this.target.flashStart(), alignedSize);
    }

    /**
     * @param {number} addr
     * @param {number} size
     * @returns {Promise<void>}
     */
    async flashErase(addr, size) {
        const sectorSize = this.target.flashSectorSize();
        
        if (addr % sectorSize !== 0) {
            throw new ValidationError(
                `Erase address ${addr.toString(16)} must be aligned to sector size ${sectorSize}`,
                this.target
            );
        }
        
        if (size % sectorSize !== 0) {
            throw new ValidationError(
                `Erase size ${size} must be a multiple of sector size ${sectorSize}`,
                this.target
            );
        }
        
        console.log(`Erasing flash: addr=0x${addr.toString(16)}, size=${size}`);
        const cmd = PicobootCmd.flashErase(addr, size);
        await this.sendCmd(cmd, null);
    }

    /**
     * @param {Uint8Array} buf
     * @returns {Promise<void>}
     */
    async flashWriteStart(buf) {
        await this.flashWrite(this.target.flashStart(), buf);
    }

    /**
     * @param {number} addr
     * @param {Uint8Array} buf
     * @returns {Promise<void>}
     */
    async flashWrite(addr, buf) {
        const pageSize = this.target.flashPageSize();
        
        if (addr % pageSize !== 0) {
            throw new ValidationError(
                `Write address ${addr.toString(16)} must be aligned to page size ${pageSize}`,
                this.target
            );
        }
        
        console.log(`Writing flash: addr=0x${addr.toString(16)}, size=${buf.length}`);
        const cmd = PicobootCmd.flashWrite(addr, buf.length);
        await this.sendCmd(cmd, buf);
    }

    /**
     * @param {number} size
     * @returns {Promise<Uint8Array>}
     */
    async flashReadStart(size) {
        return await this.flashRead(this.target.flashStart(), size);
    }

    /**
     * @param {number} addr
     * @param {number} size
     * @returns {Promise<Uint8Array>}
     */
    async flashRead(addr, size) {
        console.log(`Reading flash: addr=0x${addr.toString(16)}, size=${size}`);
        const cmd = PicobootCmd.flashRead(addr, size);
        return await this.sendCmd(cmd, null);
    }

    /**
     * @returns {Promise<void>}
     */
    async enterXip() {
        console.log('Entering XIP mode');
        const cmd = PicobootCmd.enterXip();
        await this.sendCmd(cmd, null);
    }

    /**
     * @returns {Promise<void>}
     */
    async exitXip() {
        console.log('Exiting XIP mode');
        const cmd = PicobootCmd.exitXip();
        await this.sendCmd(cmd, null);
    }

    /**
     * OTP Read (RP2350 only)
     * @param {number} rowIndex - first row to read
     * @param {number} rowCount - number of rows to read
     * @param {boolean} ecc - whether to read ECC rows (16-bit) or non-ECC (32-bit)
     * @returns {Promise<Uint8Array>}
     */
    async otpRead(rowIndex, rowCount, ecc) {
        if (this.target.type !== 'RP2350') {
            throw new ProtocolError(
                'OTP read command is only supported on RP2350 targets',
                this.target
            );
        }

        console.log(`Reading OTP: rowIndex=${rowIndex}, rowCount=${rowCount}, ecc=${ecc}`);
        const cmd = PicobootCmd.otpRead(rowIndex, rowCount, ecc);
        return await this.sendCmd(cmd, null);
    }

    /**
     * OTP Write (RP2350 only)
     * @param {number} rowIndex - first row to write
     * @param {Uint8Array} data - data to write
     * @param {boolean} ecc - whether data contains ECC rows (16-bit) or non-ECC (32-bit)
     * @returns {Promise<void>}
     */
    async otpWrite(rowIndex, data, ecc) {
        throw new ValidationError('OTP write command is currently disabled for safety reasons', this.target);

        if (this.target.type !== 'RP2350') {
            throw new ProtocolError(
                'OTP write command is only supported on RP2350 targets',
                this.target
            );
        }

        if (ecc && (data.length % 2 !== 0)) {
            throw new ValidationError('OTP data length must be multiple of 2 bytes when writing with ECC', this.target);
        }
        if (!ecc && (data.length % 4 !== 0)) {
            throw new ValidationError('OTP data length must be multiple of 4 bytes when writing without ECC', this.target);
        }

        console.log(`Writing OTP: rowIndex=${rowIndex}, rowCount=${data.length / (ecc ? 2 : 4)}, ecc=${ecc}`);
        const cmd = PicobootCmd.otpWrite(rowIndex, data, ecc);
        await this.sendCmd(cmd, data);
    }
    
    /**
     * @returns {Target}
     */
    getTarget() {
        return this.target;
    }

    /**
     * @param {number} bufSize
     * @param {boolean} check
     * @returns {Promise<Uint8Array>}
     */
    async bulkRead(bufSize, check) {
        const maxPacketSize = this.inEpMaxPacketSize;
        const transferSize = Math.ceil(bufSize / maxPacketSize) * maxPacketSize;
        
        try {
            console.log(`Performing bulk read: requested=${bufSize}, transferSize=${transferSize}`);
            const result = await this.device.transferIn(this.inEp, transferSize);
            console.log('Bulk read completed');
            
            if (result.status !== 'ok') {
                throw new Error(`Transfer status: ${result.status}`);
            }
            
            const data = new Uint8Array(result.data.buffer);
            
            if (check && data.length < bufSize) {
                throw new Error(`Read size mismatch: expected ${bufSize}, got ${data.length}`);
            }
            
            return data.slice(0, bufSize);
        } catch (e) {
            console.error('Bulk read error:', e);
            throw new UsbError(
                `Bulk read failed: ${e.message}`,
                this.target,
                e
            );
        }
    }

    /**
     * @param {Uint8Array} data
     * @param {boolean} check
     * @returns {Promise<number>}
     */
    async bulkWrite(data, check) {
        try {
            const result = await this.device.transferOut(this.outEp, data);
            
            if (result.status !== 'ok') {
                throw new Error(`Transfer status: ${result.status}`);
            }
            
            if (check && result.bytesWritten !== data.length) {
                throw new Error(`Write size mismatch: expected ${data.length}, wrote ${result.bytesWritten}`);
            }
            
            return result.bytesWritten;
        } catch (e) {
            throw new UsbError(
                `Bulk write failed: ${e.message}`,
                this.target,
                e
            );
        }
    }

    /**
     * @returns {number}
     */
    nextToken() {
        const token = this.cmdToken;
        this.cmdToken++;
        return token;
    }
}
