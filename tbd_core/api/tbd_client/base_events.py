from typing import Callable, Awaitable, Final

from .client import TbdClient


EventHandler = Callable[[bytes], Awaitable[None]]
EventHandlers = list[EventHandler]


async def default_event_handler(event_name: str) -> Callable[[...], Awaitable[None]]:
    async def handler(*args):
        print('received event', event_name)
        for arg in args:
            print(arg)
    return handler


class Events:
    def __init__(self, client: TbdClient, event_handlers: EventHandlers):
        self._client: Final[TbdClient] = client
        self._handlers: Final[EventHandlers] = event_handlers
        client.on_event = self._handle_event

    async def _handle_event(self, event_id: int, payload: bytes) -> None:
        if event_id >= len(self._handlers):
            raise ValueError(f'invalid event id {event_id}')
        await self._handlers[event_id](payload)


def get_base_events(client: TbdClient) -> Events:
    return Events(client, [])


__all__ = [
    'default_event_handler',
    'Events',
    'EventHandler',
    'EventHandlers',
    'get_base_events',
]
