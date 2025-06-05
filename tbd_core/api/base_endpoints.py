from enum import IntEnum, unique

@unique
class BaseEndpoints(IntEnum):
    """ Endpoints guaranteed to always be present with guaranteed IDs.

        Names are the endpoints name (function name) and values are the endpoint IDs.

        Changes to these endpoints can lock out older clients. Two important considerations:

        1. NEVER change the assignment of ids 0, 1, and 2, since these are required to determine the level of
           compatibility between client and API and also device recovery.
        2. Changes to other endpoints will lock out even dynamic clients. This could make troubleshooting and recovery
           of devices with outdated firmware unnecessarily difficult.

    """

    # NEVER change these
    get_api_version   = 0
    update_device     = 1
    reset_device      = 2

    # avoid changing these
    get_num_endpoints = 3
    get_endpoint_name = 4
    get_device_info   = 5
    get_num_errors    = 6
    get_error_name    = 7
    get_error_message = 8

    @staticmethod
    def num_reserved_ids() -> int:
        return len(BaseEndpoints)

    @staticmethod
    def id_mapping() -> dict[str, int]:
        return {item.name: item.value for item in BaseEndpoints}

    @staticmethod
    def names() -> list[str]:
        return list(item.name for item in BaseEndpoints)


__all__ = ["BaseEndpoints"]
