import {dump_packet, Packet, parse_packet, parse_packet_stream} from "./packets";


class ConditionPromise<T> {
    constructor() {
        const {promise, reject, resolve} = Promise.withResolvers()
        this._promise = promise as Promise<T>
        this._reject = reject
        this._resolve = resolve
    }

    wait_for(): Promise<T> {
        return this._promise
    }

    release(result: T): void {
        if (this._promise == null) {
            throw 're-attempting condition unlock'
        }
        if (this._resolve == null) {
            throw 'can not release condition, possible race condition'
        }
        this._resolve(result)
    }

    set_error(err: any): void {
        this._reject(err)
    }

    private readonly _promise: Promise<T>
    private readonly _reject: ((err: any) => void)
    private readonly _resolve: ((result: T) => void)
}


export abstract class TransportBase {
    /**
     * Take next incoming packet on package queue.
     *
     * If no packet is available, waits for new packages.
     */
    abstract read(): Promise<Packet>

    /**
     * Send packet to server.
     *
     * @param data  TBD packet to send
     */
    abstract write(data: Packet): Promise<void>

    /**
     * Wait for connection to be ready.
     */
    abstract wait_for_connection(): Promise<void>

    /**
     * Wait for connection to close.
     */
    abstract has_closed(): boolean
}

abstract class QueuedTransport extends TransportBase {
    override async read(): Promise<Packet> {
        // still have unprocessed packets
        if (this._in_queue.length != 0) {
            const packet = this._in_queue.shift() || null
            if (packet == null) {
                throw 'simultaneous read attempts on packet queue'
            }
            return packet
        }

        // wait for new packets
        if (this._pending_read != null) {
            throw 'simultaneous read attempts on packet queue'
        }
        this._pending_read = new ConditionPromise()
        return await this._pending_read.wait_for()
    }

    override async wait_for_connection(): Promise<void> {
        return await this._connected_condition.wait_for()
    }

    override has_closed(): boolean {
        return this._has_closed
    }

    protected _put_incoming(packet: Packet): void {
        // processing still working
        if (this._pending_read == null) {
            this._in_queue.push(packet)
            return
        }

        // processing is waiting for new packet
        const condition = this._pending_read
        this._pending_read = null
        condition.release(packet)
    }

    protected _set_connected(): void {
        this._connected_condition.release()
    }

    protected _set_closed(): void {
        this._has_closed = true
    }

    private readonly _connected_condition: ConditionPromise<void> = new ConditionPromise()
    private readonly _in_queue: Array<Packet> = []
    private _pending_read: ConditionPromise<Packet> | null = null
    private _has_closed: boolean = false
}

export class WebsocketTransport extends QueuedTransport {
    constructor(socket: WebSocket) {
        super()

        socket.onopen = () => this._set_connected()
        socket.onmessage = async (event) => {
            const data = event.data
            if (!(data instanceof Blob)) {
                console.error('received non binary message', data)
                return
            }
            const packet = parse_packet(new Uint8Array(await data.arrayBuffer()))
            this._put_incoming(packet)
        }
        socket.onclose = (event) => this._set_closed()
        socket.onerror = (event) => this._handle_socket_error(event)
        this._websocket = socket
    }

    override async write(data: Packet): Promise<void> {
        this._websocket.send(dump_packet(data))
    }

    private _handle_socket_error(event: Event) {
        console.error('websocket error', event)
    }

    private _websocket: WebSocket
}


interface Serial {
    readable: ReadableStream
    writable: WritableStream
    onconnect: ((e: any) => void) | null
    ondisconnect: ((e: any) => void) | null
    open: (arg: any) => void
}


export class SerialTransport extends QueuedTransport {
    constructor(port: Serial) {
        super()
        console.log('creating transport', port)
        this._port = port
        port.onconnect = (e) => {
            console.log('connected')
        }
        port.ondisconnect = (e) => this._set_closed()
        this._set_connected()
        console.log('done setting up', port)
        this._read_job_done = this._read_job()

    }

    override async write(packet: Packet): Promise<void> {
        const data = dump_packet(packet)
        const writer = this._port.writable.getWriter()
        await writer.write(data)
        writer.releaseLock()
    }

    protected async _read_job(): Promise<void> {
        while (this._port.readable) {
            await this._read_stream()
        }
        console.log('closing port')
    }

    protected async _read_stream() {
        const reader = this._port.readable.getReader()
        const stream_parser = parse_packet_stream()

        while (true) {
            try {
                const {value, done} = await reader.read() as {value: Uint8Array<ArrayBuffer>, done: boolean}
                if (value != null) {
                    const {value: packet} = stream_parser.next(value)
                    if (packet != null) {
                        this._put_incoming(packet)
                    }
                }
                if (done) {
                    reader.releaseLock()
                    break
                }
            } catch (error) {
                console.error(`error reading chunk ${error}`)
                break
            }
        }
    }

    private _port: any
    private _read_job_done: Promise<void>
}