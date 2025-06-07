from contextlib import asynccontextmanager, contextmanager
from dataclasses import dataclass
from serial import Serial
from websockets.asyncio.client import connect

from .tbd_rpc import *
from .tbd_event import *


from .client import TbdClient
from .transport import WebsocketTransport, SerialTransport

@dataclass(frozen=True)
class Tbd:
    rpc: TbdRpc
    event: TbdEvent


@asynccontextmanager
async def connect_tbd_via_websocket(host: str = 'localhost', port: int = 7777):
    url = f'ws://{host}:{port}/ws'
    async with connect(url) as websocket:
        client = TbdClient(WebsocketTransport(websocket))
        yield Tbd(
            rpc=TbdRpc(client),
            event=TbdEvent(client),
        )


@contextmanager
def connect_tbd_via_serial(port: str):
    with Serial(port=port, baudrate=9600) as serial:
        client = TbdClient(SerialTransport(serial))
        yield Tbd(
            rpc=TbdRpc(client),
            event=TbdEvent(client),
        )


__all__ = [
    'connect_tbd_via_websocket',
    'connect_tbd_via_serial'
]