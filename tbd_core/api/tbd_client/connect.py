from contextlib import asynccontextmanager, contextmanager
from dataclasses import dataclass
from pathlib import Path

from serial import Serial
from websockets.asyncio.client import connect

from .base_rpcs import Rpcs, get_base_rpcs
from .base_events import Events, get_base_events


from .client import TbdClient
from .transport import WebsocketTransport, SerialTransport

try:
    from .firmware_rpcs import FirmwareRpcs, get_firmware_rpcs
except ImportError:
    here = Path(__file__).parent
    print(f'no firmware specific rpcs found in {here}')
    FirmwareRpcs = Rpcs
    get_firmware_rpcs = get_base_rpcs

try:
    from .firmware_events import FirmwareEvents, get_firmware_events
except ImportError:
    here = Path(__file__).parent
    print(f'no firmware specific events found in {here}')
    FirmwareEvents = Events
    get_firmware_events = get_base_events


@dataclass(frozen=True)
class Tbd:
    rpc: FirmwareRpcs
    event: FirmwareEvents


@asynccontextmanager
async def connect_tbd_via_websocket(host: str = 'localhost', port: int = 7777):
    url = f'ws://{host}:{port}/ws'
    async with connect(url) as websocket:
        client = TbdClient(WebsocketTransport(websocket))
        yield Tbd(
            rpc=get_firmware_rpcs(client),
            event=get_firmware_events(client),
        )


@contextmanager
def connect_tbd_via_serial(port: str):
    with Serial(port=port, baudrate=9600) as serial:
        client = TbdClient(SerialTransport(serial))
        yield Tbd(
            rpc=get_firmware_rpcs(client),
            event=get_firmware_events(client),
        )


__all__ = [
    'connect_tbd_via_websocket',
    'connect_tbd_via_serial'
]
