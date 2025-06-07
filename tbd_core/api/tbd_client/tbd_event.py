from .client import TbdClient


class TbdEvent:
    def __init__(self, client: TbdClient):
        self._client = client


__all__ = ['TbdEvent']