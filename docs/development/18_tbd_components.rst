**************
TBD Components
**************

TBD components are the key to customizing and extending your TBD firmware. They can serve multiple purposes:

- provide a required functionality (interfacing a specific sound chip or library for example)
- extend the functionality of an existing module (the `tbd_sound_registry` adds the ability to set and switch plugins
  via calls API using the `tbd_audio` module.
- provide a completely new functionality (add a display that shows the active plugin, by listening for certain events)
