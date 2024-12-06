*************
Repo Overview
*************

``components``  sound generator source code
    includes the sound generator plugins and some libraries utilised

``hw``  hardware design files
    includes reference design files for 
    - PCBs
    - housings
    - frontplates

``tapp``  serial API interface tools
    alloes the routing of API call through a serial port

``main``  code application
    application source code

``tests/apitest``  tests
    contains integration tests utilising the device API
    
``generators``  source code preprocessors
    generates source files to register plugins with the firmware

``docs``  project documentation
    `sphix <https://www.sphinx-doc.org>`_ based structured project documentation

``bin``  firmware flashing utils
    contains scripts to assist with writing firmware to device

``.devcontainer`` development container descriptions
    https://containers.dev/

``.github``` github CI files
    configures automated github builds