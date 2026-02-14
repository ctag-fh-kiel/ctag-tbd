********
Overview
********

This section goes into depth on how the TBD firmware is built and client libraries are generated. The information
provided here is not relevant to ordinary contributors or developers. It is only intended for those wishing to
extend the build system or implement custom sub-systems that require advanced code generation/analysis mechanics.

Build System Fundamentals
=========================

The TBD firmware is based on ESPHome, therefore all TBD Components are also ESPHome components, albeit quite often
bypassing the ESPHome mindset. One of TBD's core requirement is the ability to perform a multi-staged code/analysis
and generation process, where components can utilize global registries to register specific features for later
processing once all components have been collected and the core build initialization stage has completed.

On Components
-------------

Since TBD components are ESPHome components, the following core mechanics work just as any ESPHome build:

- the `esphome` CLI or docker image can be used to build the firmware with no other prerequisites installed on the
  system.
- a device firmware is described in a `some-device.yaml` esphome YAML spec
- top level entries in the YAML dictionary are considered to be components
- component search paths are the ESPHome defaults (components from the esphome repo and external paths specified in
  YAML using external components mechanism)
- components are organised as folders, that contain both a python module (must contain a `__init__.py`) and C++ sources
- root level files in a component will be copied to the build source directory
- components need can provide ESPHome specific variables such as `DEPENDENCIES`, `AUTO_LOAD`, `CONFIG_SCHEMA` and
  a setup function `to_code` as well as ESPHome actions etc.
- python helpers and additional setup functions etc can be imported via `esphome.components.<component-name>`

TBD adds many mechanism to these mechanics once the `tbd_module` component has been loaded. The most important part of
any TBD firmware description YAML is adding the path of the TBD repo (URL or path to cloned copy) as an external
component search paths and adding `tbd_module` to the list of modules in the YAML:

.. code-block:: yaml

    external_components:
      - source: <path-to-tbd>

    tbd_module:

This will add additional module search paths to the ESPHome build and setup python import paths.

- all the subdirectories of `<tbd-repo>/components/<subdir>` as component search paths
- the `tbd_core` python module becomes globally available


