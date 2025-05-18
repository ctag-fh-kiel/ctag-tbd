import asyncio
import nest_asyncio
import IPython
from tbd_client import open_tbd

nest_asyncio.apply()

async def main():
    async with open_tbd() as client:
        IPython.embed(using='asyncio')

asyncio.run(main())
