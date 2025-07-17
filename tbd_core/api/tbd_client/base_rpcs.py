from typing import Final

from .client import TbdClient


class Rpcs:
    def __init__(self, client: TbdClient):
        self._client: Final[TbdClient] = client


def get_base_rpcs(client: TbdClient) -> Rpcs:
    return Rpcs(client)


__all__ = ['Rpcs', 'get_base_rpcs']