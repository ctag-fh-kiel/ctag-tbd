import asyncio
from abc import ABC, abstractmethod
from typing import Final

from serial import Serial
from websockets.asyncio.client import ClientConnection

from logging import getLogger
from .packets import Packet, START_BYTE, HEADER_SIZE, parse_header, parse_tail, parse_packet, dump_packet

_LOGGER = getLogger(__name__)


class TransportBase(ABC):
    @abstractmethod
    async def read(self) -> Packet:
        raise NotImplementedError()

    @abstractmethod
    async def write(self, data: Packet):
        raise NotImplementedError()


class WebsocketTransport(TransportBase):
    def __init__(self, websocket: ClientConnection):
        self._websocket: Final = websocket

    async def read(self) -> Packet:
        return parse_packet(await self._websocket.recv())

    async def write(self, packet: Packet):
        await self._websocket.send(dump_packet(packet))


SERIAL_THROTTLE = 0.001

class SerialTransport(TransportBase):
    def __init__(self, serial: Serial):
        self._serial: Final = serial

    async def read(self) -> Packet:
        ignored_initial_bytes = 0
        while True:
            if self._serial.in_waiting >= 1:
                if int.from_bytes(self._serial.read(1), byteorder='little') == START_BYTE:
                    if ignored_initial_bytes != 0:
                        _LOGGER.warning(f'request stream started with {ignored_initial_bytes} non starter bytes')
                    break
                ignored_initial_bytes += 1
            await asyncio.sleep(SERIAL_THROTTLE)

        header_size = HEADER_SIZE - 1
        while True:
            if self._serial.in_waiting >= header_size:
                header = parse_header(self._serial.read(header_size), skip_start_byte=True)
                break
            await asyncio.sleep(SERIAL_THROTTLE)

        tail_size = header.tail_size
        while True:
            if self._serial.in_waiting >= tail_size:
                return parse_tail(header, self._serial.read(tail_size))


    async def write(self, packet: Packet):
        self._serial.write(dump_packet(packet))

__all__ = [
    'TransportBase',
    'WebsocketTransport',
    'SerialTransport',
]
