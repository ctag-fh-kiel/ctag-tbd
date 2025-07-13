import os
import sys

host = sys.argv[1] if len(sys.argv) > 1 else 'localhost'
port = int(sys.argv[2]) if len(sys.argv) > 2 else 3000

os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'

import asyncio
import nest_asyncio
import IPython

nest_asyncio.apply()

async def main():
    if host.startswith('/'):
        print(f'using serial device {host}')
        from . import connect_tbd_via_serial
        with connect_tbd_via_serial(host) as client:
            IPython.embed(using='asyncio')
    else:
        print(f'using websocket {host}')
        from . import connect_tbd_via_websocket
        async with connect_tbd_via_websocket(host, port) as client:
            IPython.embed(using='asyncio')

asyncio.run(main())
