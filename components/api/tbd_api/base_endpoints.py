from enum import IntEnum, unique


class BaseEndpoints(IntEnum):
    """ Endpoints guaranteed to always be present with guaranteed IDs.

        Names are the endpoints name (function name) and values are the endpoint IDs.
    """

    get_num_endpoints = 0
    get_endpoint_name = 1
    get_device_info   = 2

    @staticmethod
    def num_reserved_ids():
        return len(BaseEndpoints)


__all__ = ["BaseEndpoints"]