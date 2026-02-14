**********
TBD Events
**********

In order to facilitate efficient communication between TBD components, without each component needing to make many
assumptions about the other (loose coupling) as well as allowing multiple components to respond to changes of
some firmware state, TBD provides an event system. Some of the core ideas of this event system are

- no knowledge about the module emitting the event required (loose coupling)
- events and event handlers are represented by simple functions
- events are dispatched internally and over serial/network connections
- internal event distribution is very efficient by linking event emission and handling at compile time (no queues and
  dynamic lookups required)
- emitting and distributing incoming events through serial/network communication is handled with as little dynamic
  overhead as possible (through table lookups)
- new event distribution channels like additional serial hardware can be easily added and require no config

Event Basics
============

.. warning::

    The simple event dispatching mechanism is not intended for multi-threaded use cases. A non blocking thread safe
    distribution mechanism could be implemented in future versions of the *TBD Sound Core* library.

A good example for the use of an event can be found in the ``tbd_sound_registry`` module. This module allows the
dynamic switching between different sound plugins. Several parts of a TBD system need to listen to this event in order
to be informed about plugin switching:

- web UIs
- front panel software
- the plugin presets manager

In order to allow these parts of the system to respond to such changes, ``tbd_sound_registry`` emits the
``sound_processor_changed`` event whenever a sound plugin change occurs.

In order to to this an event is declared in the emitting module by declaring a simple function (with no implementation),
where the function arguments are the *event payload* which is used to convey additional information about what just
happened:

.. code-block:: c++

    [[tbd::event]]
    void sound_processor_changed(const uint_par& channels, const uint_par& new_plugin_id);

Event functions need to be marked with the :code:`[[tbd::event]]` annotation and the function arguments have to be
reflectable. By default the event name (the name responders will use to identify an event) corresponds to the event
function name. If desired this can be changed by adding an :code:`event_name` parameter to the annotation:
:code:`[[tbd_event(event_name="true_event_name")]]`

Actually emitting this event is done by simply invoking the event function:

.. code-block:: c++

    // ... some state changed
    sound_processor_changed(some_channels, some_plugin_id);
    // ... event has been dispatched at this point, but we do not know if all responders have processed it

An internal component (within the TBD audio core) that wants to listen for said event (like ``tbd_presets``) simply
needs to implement a responder function for said event:

.. code-block:: c++

    [[tbd::responder(event="sound_processor_changed")]]
    Error respond_to_sound_processor_change(const uint_par& channels, const uint_par& new_plugin_id) {
        // ... do something
        return TBD_OK;
    }

A responder needs to

- be marked as a responder via the :code:`[[tbd::responder(event_name="some_event")]]` annotation.
- have the same argument signature as the event itself
- return :code:`void` or :code:`tbd::error`

.. note::

    Neither event declarations nor responders need to be made public. In fact they don't even need to reside in headers.
    This way no one can accidentally call the responder directly or emit the event from outside the owning component.
