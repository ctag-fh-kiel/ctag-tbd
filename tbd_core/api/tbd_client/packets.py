import dataclasses
from enum import IntEnum, unique


# values of constant value bytes in packet
START_BYTE      = 0xf0
HEADER_END_BYTE = 0xe7
END_BYTE        = 0xff

@unique
class HeaderBytes(IntEnum):
    OFFSET_START        = 0
    OFFSET_TYPE         = 1
    OFFSET_HANDLER_LOW  = 2
    OFFSET_HANDLER_HIGH = 3
    OFFSET_CRC          = 4
    OFFSET_LENGTH_LOW   = 5
    OFFSET_LENGTH_HIGH  = 6
    OFFSET_ID_LOW       = 7
    OFFSET_ID_HIGH      = 8
    OFFSET_HEADER_END   = 9

HEADER_SIZE = len(HeaderBytes)

@unique
class EndBytes(IntEnum):
    OFFSET_END = 0

END_SIZE = len(EndBytes)


TYPE_MASK = 0b00000111
FLAG_MASK = 0b11111000

@unique
class PacketType(IntEnum):
    TYPE_RPC      = 0
    TYPE_RESPONSE = 1
    TYPE_ERROR    = 2
    TYPE_ACTION   = 3
    TYPE_EVENT    = 4
    TYPE_INVALID  = 5

    @staticmethod
    def from_binary(bin_value: int) -> 'PacketType':
        int_value = bin_value & TYPE_MASK
        for value in PacketType:
            if value == int_value:
                return value
        raise ValueError(f'invalid packet type {int_value}')

    def to_binary(self) -> bytes:
        return self.value.to_bytes(1, 'little')


@dataclasses.dataclass(frozen=True)
class Header:
    type: PacketType
    handler: int
    id: int
    payload_length: int
    crc: int

    @property
    def tail_size(self) -> int:
        return self.payload_length + END_SIZE

    @property
    def total_size(self) -> int:
        return HEADER_SIZE + int(self.payload_length) + END_SIZE


@dataclasses.dataclass(frozen=True)
class Packet(Header):
    payload: bytes | None


def parse_header(data: bytes, *, skip_start_byte=False) -> Header:
    def get_one(offset: int) -> int:
        _offset = offset - 1 if skip_start_byte else offset
        return data[_offset]

    def unpack_int(offset: int) -> int:
        _offset = offset - 1 if skip_start_byte else offset
        return int.from_bytes(data[_offset:_offset + 2], byteorder='little')

    expected_length = HEADER_SIZE - 1 if skip_start_byte else HEADER_SIZE
    if (input_length := len(data)) < expected_length:
        raise ValueError(f'header data size {input_length} smaller than required length {expected_length}')

    if not skip_start_byte and data[HeaderBytes.OFFSET_START] != START_BYTE:
        raise ValueError('expected packet start byte')

    _type = PacketType.from_binary(get_one(HeaderBytes.OFFSET_TYPE))
    _handler = unpack_int(HeaderBytes.OFFSET_HANDLER_LOW)

    if (_crc := get_one(HeaderBytes.OFFSET_CRC)) != 0:
        raise ValueError(f'expected crc byte to be 0, got {_crc}')

    _payload_length = unpack_int(HeaderBytes.OFFSET_LENGTH_LOW)
    _id = unpack_int(HeaderBytes.OFFSET_ID_LOW)

    if get_one(HeaderBytes.OFFSET_HEADER_END) != HEADER_END_BYTE:
        raise ValueError('expected header end byte')

    return Header(
        type = _type,
        handler = _handler,
        id = _id,
        payload_length = _payload_length,
        crc=_crc,
    )

def parse_tail(header: Header, data: bytes) -> Packet:
    expected_length = header.tail_size
    if (input_length := len(data)) < expected_length:
        raise ValueError(f'header data size {input_length} smaller than required length {expected_length}')

    _payload = data[:header.payload_length] if header.payload_length > 0 else None
    if data[header.payload_length] != END_BYTE:
        raise ValueError('expected packet end byte')

    return Packet(
        payload = _payload,
        **dataclasses.asdict(header),
    )

def parse_packet(data: bytes, *, skip_start_byte=False) -> Packet:
    header = parse_header(data, skip_start_byte=skip_start_byte)
    payload_offset = HEADER_SIZE - 1 if skip_start_byte else HEADER_SIZE
    return parse_tail(header, data[payload_offset:])

def dump_packet(packet: Packet) -> bytes:
    data: list[bytes] = [
        START_BYTE.to_bytes(1, 'little'),
        packet.type.to_binary(),
        packet.handler.to_bytes(2, byteorder='little'),
        packet.crc.to_bytes(1, 'little'),
        packet.payload_length.to_bytes(2, 'little'),
        packet.id.to_bytes(2, 'little'),
        HEADER_END_BYTE.to_bytes(1, 'little'),
        packet.payload if packet.payload else b'',
        END_BYTE.to_bytes(1, 'little'),
    ]
    return b''.join(data)

__all__ = [
    'START_BYTE',
    'HEADER_END_BYTE',
    'END_BYTE',
    'HeaderBytes',
    'HEADER_SIZE',
    'EndBytes',
    'END_SIZE',
    'PacketType',
    'Header',
    'Packet',
    'parse_header',
    'parse_tail',
    'parse_packet',
    'dump_packet',
]