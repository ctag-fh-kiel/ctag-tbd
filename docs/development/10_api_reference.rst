*************
API Reference
*************

The TBD-16 exposes a REST-like API that allows you to control the device over the network. This API is used by the built-in Web UI but can also be accessed by custom applications or scripts.

Base URL
========

The base URL for all API calls is:
``http://<device-ip>/api/v1``

Endpoints
=========

Get Available Plugins
---------------------

Returns a list of all plugins registered with the system.

*   **URL**: ``/getPlugins``
*   **Method**: ``GET``
*   **Response**: A JSON array of plugin objects.

Get Active Plugin
-----------------

Returns the ID of the plugin currently active on a specific channel.

*   **URL**: ``/getActivePlugin/:ch``
*   **Method**: ``GET``
*   **Parameters**: ``ch`` (0 or 1)
*   **Response**: ``{"id": "plugin_id"}``

Set Active Plugin
-----------------

Changes the active plugin for a channel.

*   **URL**: ``/setActivePlugin/:ch?id=PluginName``
*   **Method**: ``GET``
*   *Note: While this uses GET, it performs a state change.*

Get Plugin Parameters
---------------------

Retrieves all parameters and their current values for the active plugin on a channel.

*   **URL**: ``/getPluginParams/:ch``
*   **Method**: ``GET``

Set Parameter Value
-------------------

Sets a specific parameter value.

*   **URL**: ``/setPluginParam/:ch?id=param_id&current=value``
*   **Method**: ``GET``

Presets
-------

Get available presets for the active plugin.

*   **URL**: ``/getPresets/:ch``
*   **Method**: ``GET``

Save or load a preset.

*   **URL**: ``/savePreset/:ch?name=MyPreset``
*   **URL**: ``/loadPreset/:ch?id=preset_index``

System Info
-----------

Get system status and version information.

*   **URL**: ``/getSystemInfo``
*   **Method**: ``GET``
