const START_BYTE      = 0xf0
const HEADER_END_BYTE = 0xe7
const END_BYTE        = 0xff

enum HeaderBytes {
    OFFSET_START = 0,
    OFFSET_TYPE = 1,
    OFFSET_HANDLER_LOW = 2,
    OFFSET_HANDLER_HIGH = 3,
    OFFSET_CRC = 4,
    OFFSET_LENGTH_LOW = 5,
    OFFSET_LENGTH_HIGH = 6,
    OFFSET_ID_LOW = 7,
    OFFSET_ID_HIGH = 8,
    OFFSET_HEADER_END = 9,
}

const HEADER_SIZE = 10


enum EndBytes {
    OFFSET_END = 0
}

const END_SIZE = 1


const TYPE_MASK = 0b00000111
const FLAG_MASK = 0b11111000


export enum PacketType {
    TYPE_NOOP     = 0,
    TYPE_RPC      = 1,
    TYPE_RESPONSE = 2,
    TYPE_ERROR    = 2,
    TYPE_ACTION   = 3,
    TYPE_EVENT    = 4,
    TYPE_INVALID  = 5,
}

function packet_type_from_binary(byte: number): PacketType {
    switch (byte) {
        case PacketType.TYPE_NOOP:
            return PacketType.TYPE_NOOP
        case PacketType.TYPE_RPC:
            return PacketType.TYPE_RPC
        case PacketType.TYPE_RESPONSE:
            return PacketType.TYPE_RESPONSE
        case PacketType.TYPE_ERROR:
            return PacketType.TYPE_ERROR
        case PacketType.TYPE_ACTION:
            return PacketType.TYPE_ACTION
        case PacketType.TYPE_EVENT:
            return PacketType.TYPE_EVENT
        default:
            throw `invalid PacketType ${byte}`
    }
}

function packet_type_to_binary(type: PacketType): number {
    return type
}

type HeaderFields = {
    type: PacketType,
    handler: number,
    id: number,
    payload_length: number,
    crc: number,
}

class Header {
    readonly type: PacketType
    readonly handler: number
    readonly id: number
    readonly payload_length: number
    readonly crc: number

    constructor(init: HeaderFields) {
        this.type = init.type
        this.handler = init.handler
        this.id = init.id
        this.payload_length = init.payload_length
        this.crc = init.crc
    }

    tail_size(): number {
        return this.payload_length + END_SIZE
    }

    total_size(): number {
        return HEADER_SIZE + this.payload_length + END_SIZE
    }
}


export type PayloadData = Uint8Array | null

type PacketFields = {
    payload: PayloadData
} & HeaderFields

export class Packet extends Header {
    constructor(init: PacketFields) {
        super(init);
        this.payload = init.payload
    }

    readonly payload: PayloadData
}


function parse_header(data: Uint8Array, {skip_start_byte=false} = {}): Header {
    function get_one(offset: number): number {
        const _offset = skip_start_byte ? offset - 1 : offset
        return data[_offset]
    }

    function unpack_int(offset: number): number {
        const _offset = skip_start_byte ? offset - 1 : offset
        const fst = data[_offset]
        const snd = data[offset + 1] << 8
        return fst + snd
    }

    const expected_length = skip_start_byte ? HEADER_SIZE - 1 : HEADER_SIZE

    const input_length = data.length
    if (input_length < expected_length) {
        throw `header data size {input_length} smaller than required length ${expected_length}`
    }

    if (!skip_start_byte && data[HeaderBytes.OFFSET_START] != START_BYTE) {
        throw 'expected packet start byte'
    }

    const _type = packet_type_from_binary(get_one(HeaderBytes.OFFSET_TYPE))
    const _handler = unpack_int(HeaderBytes.OFFSET_HANDLER_LOW)

    const _crc = get_one(HeaderBytes.OFFSET_CRC)
    if (_crc != 0) {
        throw `expected crc byte to be 0, got ${_crc}`
    }

    const _payload_length = unpack_int(HeaderBytes.OFFSET_LENGTH_LOW)
    const _id = unpack_int(HeaderBytes.OFFSET_ID_LOW)

    if (get_one(HeaderBytes.OFFSET_HEADER_END) != HEADER_END_BYTE) {
        throw 'expected header end byte'
    }

    return new Header({
        type: _type,
        handler: _handler,
        id: _id,
        payload_length: _payload_length,
        crc: _crc,
    })
}

function parse_tail(header: Header, data: Uint8Array): Packet {
    const expected_length = header.tail_size()
    const input_length = data.length
    if (input_length < expected_length) {
        throw `header data size ${input_length} smaller than required length ${expected_length}`
    }
    const _payload = header.payload_length > 0 ? data.slice(0, header.payload_length) : null
    if (data[header.payload_length] != END_BYTE) {
        throw 'expected packet end byte'
    }
    return new Packet({
        payload: _payload,
        ...header,
    })
}

export function parse_packet(data: Uint8Array, {skip_start_byte=false} = {}): Packet {
    const header = parse_header(data, {skip_start_byte})
    const payload_offset = skip_start_byte ? HEADER_SIZE - 1 : HEADER_SIZE
    const tail = data.slice(payload_offset)
    return parse_tail(header, tail)
}

export function dump_packet(packet: Packet): Uint8Array {
    function pack_uint16(value: number): [number, number] {
        if (!Number.isInteger(value)) {
            throw `uint16 requires integer values, got ${value}`
        }
        if (value >= 1 << 16) {
            throw `${value} out of range for uint16`
        }

        return [value & 0xff, (value >> 8) & 0xff]
    }

    function pack_uint8(value: number): number {
        if (!Number.isInteger(value)) {
            throw `uint8 requires integer values, got ${value}`
        }
        if (value >= 1 << 8) {
            throw `${value} out of range for uint8`
        }
        return value
    }

    return new Uint8Array([
        START_BYTE,
        packet_type_to_binary(packet.type),
        ...pack_uint16(packet.handler),
        pack_uint8(packet.crc),
        ...pack_uint16(packet.payload_length),
        ...pack_uint16(packet.id),
        HEADER_END_BYTE,
        ... (packet.payload || []),
        END_BYTE
    ])
}


export function* parse_packet_stream(): Generator<Packet | null, void, Uint8Array<ArrayBuffer>> {
    const state = {
        buffer: new Uint8Array(),
        known_state: false,
    }

    function skip_until_start(): boolean {
        const buffer = state.buffer
        for (let i = 0; i < buffer.length; ++i) {
            if (buffer[i] == START_BYTE) {
                state.buffer = buffer.slice(i)
                return true
            }
        }
        state.buffer = new Uint8Array()
        return false
    }

    function add_to_buffer(new_data: Uint8Array<ArrayBuffer>) {
        state.buffer = new Uint8Array([...state.buffer, ...new_data])
    }

    while (true) {
        add_to_buffer(yield null)

        // wait for next start byte
        if (!state.known_state && !skip_until_start()) {
            continue
        }
        state.known_state = true

        try {
            while (state.buffer.length < HEADER_SIZE) { add_to_buffer(yield null) }
            const header = parse_header(state.buffer)
            state.buffer = state.buffer.slice(HEADER_SIZE)

            const tail_length = header.payload_length + 1
            while (state.buffer.length < tail_length) { add_to_buffer(yield null) }
            const packet = parse_tail(header, state.buffer)
            state.buffer = state.buffer.slice(tail_length)
            add_to_buffer(yield packet)

        } catch (e) {
            console.error(`error parsing packet ${e}`)
            state.known_state = false
            state.buffer = new Uint8Array()
        }
    }
}