// Copyright (C) 2025 Piers Finlayson <piers@piers.rocks>
//
// MIT License

export class PicobootError extends Error {
    constructor(message) {
        super(message);
        this.name = 'PicobootError';
    }
}

export class UsbError extends PicobootError {
    constructor(message, target, originalError) {
        super(message);
        this.name = 'UsbError';
        this.target = target;
        this.originalError = originalError;
    }
}

export class ProtocolError extends PicobootError {
    constructor(message, target) {
        super(message);
        this.name = 'ProtocolError';
        this.target = target;
    }
}

export class ValidationError extends PicobootError {
    constructor(message, target) {
        super(message);
        this.name = 'ValidationError';
        this.target = target;
    }
}

export class NotFoundError extends PicobootError {
    constructor(message) {
        super(message);
        this.name = 'NotFoundError';
    }
}

export class StatusError extends PicobootError {
    constructor(message, target, status) {
        super(message);
        this.name = 'StatusError';
        this.target = target;
        this.status = status;
    }
}