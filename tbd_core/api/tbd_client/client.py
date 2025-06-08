import asyncio
from typing import Final

from .transport import TransportBase
from .packets import Packet, PacketType

int_par = int
uint_par = int
float_par = float
ufloat_par = float
trigger_par = bool
str_par = str


class RequestFailedError(Exception):
    def __init__(self, status_code):
        super().__init__(status_code)


class TbdClient:
    def __init__(self, transport: TransportBase):
        self._request_counter: int = 1
        self._transport: Final[TransportBase] = transport
        self._active_requests: dict[int, asyncio.Future] = {}
        asyncio.get_running_loop().create_task(self.process_incoming())

    # async def receive(self):
    #     return await self._transport.read()

    async def send_rpc(self, endpoint_id: int, payload: bytes | None) -> bytes | None:
        request_id = self._request_counter
        self._request_counter += 1

        response_future = asyncio.get_running_loop().create_future()
        self._active_requests[request_id] = response_future
        await self._transport.write(Packet(
            type=PacketType.TYPE_RPC,
            handler=endpoint_id,
            id=request_id,
            payload_length=len(payload) if payload else 0,
            payload=payload,
            crc=0,
        ))
        response = await response_future
        del self._active_requests[request_id]
        return response

    async def send_event(self, event_id: int, payload: bytes | None) -> None:
        await self._transport.write(Packet(
            type=PacketType.TYPE_EVENT,
            handler=event_id,
            id=0,
            payload_length=len(payload) if payload else 0,
            payload=payload,
            crc=0,
        ))

    async def process_incoming(self):
        try:
            while True:
                packet = await self._transport.read()
                self._process_incoming(packet)

        except asyncio.CancelledError as cancelled:
            raise cancelled

    def _process_incoming(self, packet: Packet):
        match packet.type:
            case PacketType.TYPE_RPC:
                raise RuntimeError('client does not respond to RPCs')
            case PacketType.TYPE_RESPONSE:
                future = self._active_requests[packet.id]
                future.set_result(packet.payload)
            case PacketType.TYPE_EVENT:
                print('got event', packet.handler)
            case PacketType.TYPE_ERROR:
                future = self._active_requests[packet.id]
                future.set_exception(RequestFailedError(f'request {packet.id} failed, error {packet.handler}'))
            case _ as unknown_type:
                raise RuntimeError(f'packet type {unknown_type.name} not supported')


__all__ = [
    'int_par',
    'uint_par',
    'float_par',
    'ufloat_par',
    'trigger_par',
    'str_par',
    'TbdClient',
]
