from .client import TbdClient


class TbdRpc:
    def __init__(self, client: TbdClient):
        self._client = client


__all__ = ['TbdRpc']