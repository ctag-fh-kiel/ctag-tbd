************
API Channels
************

TBD devices can offer multiple channels on which API communication occurs. This chapter will guide you through the
process of adding a new API channel and explain some of the API internal on the way.

About TBD Api Communication
===========================

Communication Channels
----------------------

TBD Api communication has two different mechanisms that can be sent through the same channel. In this context channels
refer to any data transfer mechanism. API channels could be

- UART
- SPI
- I2C
- WebSockets
- HTTP
- UNIX sockets

Irrespective of the communication channel, communication is based on *Packets*. These TBD packets are the atomic segments
of any TBD API communication and on serial communication channels each channel implementation needs to take care that
packets are not broken up or interleaved.


Types of API communication
--------------------------

At the core level communication channels can be used for two things:

Remote Procedure Calls (RPC): Clients calling a function on another device:

    RPCs will return either an error or a response to the remote caller. They are therefor a request-response mechanism.
    This should be used for light weight function calls that query or change state. This method of communication is
    for directed communication between two parties.

Events

    Information about state changes, as they occur. Events are dispatched on all available channels that accept incoming
    events and within the emitting device itself to any known listeners. Events have no response semantics and are
    light weight fire and forget mechanisms. Any module that has internal state can emit events to inform any interested
    parties about these changes.

In HTTP-like terms TBD API communication is similar to having both HTTP RPC calls and websockets for real time updates.
There is one very important difference though:

.. warning::

    Irrespective of the mechanics of the underlying communication channel there is no clear server client distinction
    on the TBD protocol level. Both sides can invoke RPCs and send/receive events. Whether such a distinction exists
    is up to the capabilities provided by each party.

Each program/firmware partaking in TBD API communication can chose to

- accept and handle RPCs
- accept and respond to specific events


API and Concurrency
-------------------

There three important TBD API design choices, concerning API processing speed:

1. There is no guarantee that clients cache RPCs or events in any large quantities.
   **TBD API calls are not intended for high speed communication or live data transfers**. The main loop of one device
   should never send requests or events at any rate close to the main loop execution frequency.
2. Hosts that accept both events and RPC calls, are guarantee that these two mechanism to not interfere. An ongoing
   RPC will not cause any event from being dropped.
3. Clients can not expect RPC responses to occur sequentially. It is the client's responsibility to keep track of a
   reasonable number of pending requests and not issuing any further requests, when the number of pending requests
   reaches a certain threshold.