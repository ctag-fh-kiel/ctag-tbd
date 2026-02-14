*************
Prerequisites
*************

To implement your own sound Plugin you need to implement your own ``CTAG::SP::ctagSoundProcessor`` class.
This requires providing two methods:

- ``Process``: called in intervals to output sound
- ``Init``: initialize a preallocated memory block that should be used for processing


.. doxygenclass:: CTAG::SP::ctagSoundProcessor
    :members-only:
    :members: Process, Init
