import asyncio
from typing import Final
from websockets.asyncio.client import ClientConnection
from .base_endpoints import BaseEndpoints
from . import api_types_pb2 as dtos

int_par     = int
uint_par    = int
float_par   = float
ufloat_par  = float
trigger_par = bool
str_par     = str


class RequestFailedError(Exception):
    def __init__(self, status_code):
        super().__init__(status_code)


class TbdClientBase:
    def __init__(self, websocket: ClientConnection):
        self._request_counter: int = 1
        self._websocket: Final = websocket
        self._active_requests: dict[int, asyncio.Future] = {}
        asyncio.get_running_loop().create_task(self.process_responses())

    async def get_endpoints(self):
        request = dtos.void_request()
        request.endpoint = BaseEndpoints.get_num_endpoints
        res_data = await self.send(request)
        response = dtos.uint_par_response()
        response.ParseFromString(res_data)
        num_requests = response.payload

        requests = []
        for request_id in range(num_requests):
            request = dtos.uint_par_request()
            request.endpoint = BaseEndpoints.get_endpoint_name
            request.payload = request_id
            res_data = await self.send(request)
            response = dtos.str_par_response()
            response.ParseFromString(res_data)
            requests.append(response.payload)
        return requests

    async def receive(self):
        return await self._websocket.recv()

    async def send(self, request):
        request_id = self._request_counter
        self._request_counter += 1
        request.request_id = request_id

        response_future = asyncio.get_running_loop().create_future()
        self._active_requests[request_id] = response_future
        await self._websocket.send(request.SerializeToString())
        return await response_future


    async def process_responses(self):
        try:
            while True:
                response_data = await self._websocket.recv()                
                headers = dtos.void_response()
                headers.ParseFromString(response_data)
                request_id = headers.request_id
                status = headers.status
                future = self._active_requests[request_id]
                if status != 0:
                    future.set_exception(RequestFailedError(status))
                else:
                    future.set_result(response_data)
                    
        except asyncio.CancelledError as cancelled:
            raise cancelled


__all__ = [
    'int_par',
    'uint_par',
    'float_par',
    'ufloat_par',
    'trigger_par',
    'str_par',
    'TbdClientBase',
    'dtos',
]
