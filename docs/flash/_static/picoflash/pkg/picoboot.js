// Copyright (C) 2025 Piers Finlayson <piers@piers.rocks>
//
// MIT License

import { Target } from './target.js';
import { Connection } from './connection.js';
import { UsbError, NotFoundError, ProtocolError, ValidationError } from './errors.js';
import {
    PICOBOOT_VID,
    PICOBOOT_PID_RP2040,
    PICOBOOT_PID_RP2350,
    PICOBOOT_USB_CLASS,
    PICOBOOT_USB_SUBCLASS,
    DEFAULT_ENDPOINT_TIMEOUT,
    DEFAULT_COMMAND_STATUS_TIMEOUT,
    DEFAULT_RESET_TIMEOUT,
} from './constants.js';

export class Picoboot {
    /**
     * @param {USBDevice} device
     * @param {Target} target
     * @param {number} ifNum
     * @param {number} outEp
     * @param {number} inEp
     * @param {number} inEpMaxPacketSize
     * @param {Object} timeouts
     */
    constructor(device, target, ifNum, outEp, inEp, inEpMaxPacketSize, timeouts) {
        this.device = device;
        this.target = target;
        this.ifNum = ifNum;
        this.outEp = outEp;
        this.inEp = inEp;
        this.inEpMaxPacketSize = inEpMaxPacketSize;
        this.timeouts = timeouts || {
            endpoint: DEFAULT_ENDPOINT_TIMEOUT,
            commandStatus: DEFAULT_COMMAND_STATUS_TIMEOUT,
            reset: DEFAULT_RESET_TIMEOUT,
        };
        this.connection = null;
    }

    /**
     * @param {USBDevice} device
     * @param {Object} [timeouts]
     * @returns {Picoboot}
     */
    static fromDevice(device, timeouts) {
        const target = Target.fromDevice(device);
        console.log(`Creating Picoboot instance for ${target.toString()}`);
        
        let picobootInterface = null;
        let ifNum = null;
        
        for (const config of device.configurations) {
            for (const iface of config.interfaces) {
                for (const alt of iface.alternates) {
                    if (alt.interfaceClass === PICOBOOT_USB_CLASS && 
                        alt.interfaceSubclass === PICOBOOT_USB_SUBCLASS) {
                        picobootInterface = alt;
                        ifNum = iface.interfaceNumber;
                        console.log(`Found PICOBOOT interface ${ifNum}`);
                        break;
                    }
                }
                if (picobootInterface) break;
            }
            if (picobootInterface) break;
        }
        
        if (!picobootInterface) {
            throw new ProtocolError(
                `No PICOBOOT interface found on device`,
                target
            );
        }
        
        let inEp = null;
        let inEpMaxPacketSize = 0;
        let outEp = null;
        
        for (const endpoint of picobootInterface.endpoints) {
            if (endpoint.type === 'bulk') {
                if (endpoint.direction === 'in') {
                    inEp = endpoint.endpointNumber;
                    inEpMaxPacketSize = endpoint.packetSize;
                } else if (endpoint.direction === 'out') {
                    outEp = endpoint.endpointNumber;
                }
            }
        }
        
        if (inEp === null || outEp === null) {
            throw new ProtocolError(
                `Failed to find bulk endpoints on PICOBOOT interface`,
                target
            );
        }
        
        console.log(`Found endpoints: IN=${inEp}, OUT=${outEp}, max_packet_size=${inEpMaxPacketSize}`);
        
        return new Picoboot(device, target, ifNum, outEp, inEp, inEpMaxPacketSize, timeouts);
    }

    /**
     * @param {Array<Target>} [targets]
     * @param {Object} [timeouts]
     * @returns {Promise<Picoboot>}
     */
    static async requestDevice(targets, timeouts) {
        if (!('usb' in navigator)) {
            throw new UsbError('WebUSB is not supported by this browser', null);
        }

        const filters = [];
        
        if (targets && targets.length > 0) {
            for (const target of targets) {
                filters.push({
                    vendorId: target.getVid(),
                    productId: target.getPid(),
                });
            }
        } else {
            filters.push({ vendorId: PICOBOOT_VID, productId: PICOBOOT_PID_RP2040 });
            filters.push({ vendorId: PICOBOOT_VID, productId: PICOBOOT_PID_RP2350 });
        }
        
        console.log('Requesting USB device from user');

        try {
            const device = await navigator.usb.requestDevice({ filters });
            console.log(`User selected device: VID=${device.vendorId.toString(16)}, PID=${device.productId.toString(16)}`);
            return await Picoboot.fromDevice(device, timeouts);
        } catch (e) {
            if (e.name === 'NotFoundError') {
                throw new NotFoundError('Device request cancelled or no device selected');
            }
            throw new UsbError(`Failed to request device: ${e.message}`, null, e);
        }
    }

    /**
     * @param {Array<Target>} [targets]
     * @param {Object} [timeouts]
     * @returns {Promise<Array<Picoboot>>}
     */
    static async getDevices(targets, timeouts) {
        const filters = [];
        
        if (targets && targets.length > 0) {
            for (const target of targets) {
                filters.push({
                    vendorId: target.getVid(),
                    productId: target.getPid(),
                });
            }
        } else {
            filters.push({ vendorId: PICOBOOT_VID, productId: PICOBOOT_PID_RP2040 });
            filters.push({ vendorId: PICOBOOT_VID, productId: PICOBOOT_PID_RP2350 });
        }
        
        console.log('Getting paired USB devices');
        
        try {
            const devices = await navigator.usb.getDevices();
            const picobootDevices = [];
            
            for (const device of devices) {
                const matchesFilter = filters.some(filter => 
                    device.vendorId === filter.vendorId && 
                    device.productId === filter.productId
                );
                
                if (matchesFilter) {
                    try {
                        const picoboot = await Picoboot.fromDevice(device, timeouts);
                        picobootDevices.push(picoboot);
                    } catch (e) {
                        console.error(`Failed to create Picoboot from device: ${e.message}`);
                    }
                }
            }
            
            console.log(`Found ${picobootDevices.length} PICOBOOT devices`);
            return picobootDevices;
        } catch (e) {
            throw new UsbError(`Failed to get devices: ${e.message}`, null, e);
        }
    }

    /**
     * @returns {Promise<Connection>}
     */
    async connect() {
        if (this.connection) {
            console.log('Already connected, returning existing connection');
            return this.connection;
        }
        
        console.log(`Connecting to ${this.target.toString()}`);
        
        if (!this.device.opened) {
            try {
                await this.device.open();
                console.log('Device opened');
            } catch (e) {
                throw new UsbError(
                    `Failed to open device: ${e.message}`,
                    this.target,
                    e
                );
            }
        }
        
        try {
            await this.device.selectConfiguration(1);
            console.log('Configuration selected');
        } catch (e) {
            throw new UsbError(
                `Failed to select configuration: ${e.message}`,
                this.target,
                e
            );
        }
        
        try {
            await this.device.claimInterface(this.ifNum);
            console.log(`Interface ${this.ifNum} claimed`);
        } catch (e) {
            throw new UsbError(
                `Failed to claim interface: ${e.message}`,
                this.target,
                e
            );
        }
        
        const usbInterface = this.device.configuration.interfaces.find(
            iface => iface.interfaceNumber === this.ifNum
        );
        
        this.connection = new Connection(
            this.device,
            this.target,
            usbInterface,
            this.outEp,
            this.inEp,
            this.inEpMaxPacketSize,
            this.timeouts
        );
        
        console.log('Connection established');
        return this.connection;
    }

    /**
     * Disconnects from the device.
     * Does not throw on error, logs only.
     * @returns {Promise<void>}
     */
    async disconnect() {
        if (!this.connection) {
            return;
        }
        
        console.log(`Disconnecting from ${this.target.toString()}`);
        
        try {
            await this.device.releaseInterface(this.ifNum);
            console.log(`Interface ${this.ifNum} released`);
        } catch (e) {
            console.error(`Failed to release interface: ${e.message}`);
        }
        
        try {
            await this.device.close();
            console.log('Device closed');
        } catch (e) {
            console.error(`Failed to close device: ${e.message}`);
        }
        
        this.connection = null;
    }

    /**
     * @param {Object} timeouts
     */
    setTimeouts(timeouts) {
        this.timeouts = timeouts;
        if (this.connection) {
            this.connection.timeouts = timeouts;
        }
    }

    /**
     * @returns {boolean}
     */
    isConnected() {
        return this.connection !== null;
    }

    /**
     * @returns {Connection|null}
     */
    getConnection() {
        return this.connection;
    }

    /**
     * @returns {Target}
     */
    getTarget() {
        return this.target;
    }

    /**
     * @returns {string}
     */
    getInfo() {
        return `${this.device.vendorId.toString(16).padStart(4, '0').toUpperCase()}:${this.device.productId.toString(16).padStart(4, '0').toUpperCase()}`;
    }

    /**
     * Gets USB device information
     * @returns {Object}
     */
    getUsbDeviceInfo() {
        return {
            vendorId: this.device.vendorId,
            productId: this.device.productId,
            productName: this.device.productName,
            manufacturerName: this.device.manufacturerName,
            serialNumber: this.device.serialNumber,
            deviceVersionMajor: this.device.deviceVersionMajor,
            deviceVersionMinor: this.device.deviceVersionMinor,
            deviceVersionSubminor: this.device.deviceVersionSubminor,
            usbVersionMajor: this.device.usbVersionMajor,
            usbVersionMinor: this.device.usbVersionMinor,
            usbVersionSubminor: this.device.usbVersionSubminor,
            deviceClass: this.device.deviceClass,
            deviceSubclass: this.device.deviceSubclass,
            deviceProtocol: this.device.deviceProtocol,
        };
    }

    /**
     * @param {number} addr
     * @param {number} size
     * @returns {Promise<Uint8Array>}
     */
    async flashRead(addr, size) {
        const wasConnected = this.isConnected();
        
        if (!wasConnected) {
            await this.connect();
        }
        
        try {
            await this.connection.resetInterface();

            // Appears to be required on some RP2350 device
            await this.connection.exitXip();

            const data = await this.connection.flashRead(addr, size);
            return data;
        } finally {
            if (!wasConnected) {
                await this.disconnect();
            }
        }
    }

    /**
     * @param {number} addr
     * @param {Uint8Array} buf
     * @returns {Promise<void>}
     */
    async flashWrite(addr, buf) {
        const wasConnected = this.isConnected();
        
        if (!wasConnected) {
            await this.connect();
        }
        
        try {
            await this.connection.resetInterface();

            // Appears to be required on some RP2350 device
            await this.connection.exitXip();

            await this.connection.flashWrite(addr, buf);
        } finally {
            if (!wasConnected) {
                await this.disconnect();
            }
        }
    }

    /**
     * @param {number} addr
     * @param {number} size
     * @returns {Promise<void>}
     */
    async flashErase(addr, size) {
        const wasConnected = this.isConnected();
        
        if (!wasConnected) {
            await this.connect();
        }
        
        try {
            await this.connection.resetInterface();

            // Appears to be required on some RP2350 device
            await this.connection.exitXip();

            await this.connection.flashErase(addr, size);
        } finally {
            if (!wasConnected) {
                await this.disconnect();
            }
        }
    }

    /**
     * Resets the interface and exits XIP on RP2040 before performing the
     * operations.
     * @param {number} addr
     * @param {Uint8Array} buf
     * @returns {Promise<void>}
     */
    async flashEraseAndWrite(addr, buf) {
        const sectorSize = this.target.flashSectorSize();
        const pageSize = this.target.flashPageSize();
        
        if (addr % sectorSize !== 0 || addr % pageSize !== 0) {
            throw new ValidationError(
                `Address ${addr.toString(16)} must be aligned to both sector and page size`,
                this.target
            );
        }
        
        const eraseSize = Math.ceil(buf.length / sectorSize) * sectorSize;
        
        const wasConnected = this.isConnected();
        
        if (!wasConnected) {
            await this.connect();
        }
        
        try {
            await this.connection.resetInterface();

            // Appears to be required on some RP2350 device
            await this.connection.exitXip();

            await this.connection.flashErase(addr, eraseSize);
            await this.connection.flashWrite(addr, buf);
        } finally {
            if (!wasConnected) {
                await this.disconnect();
            }
        }
    }
}
