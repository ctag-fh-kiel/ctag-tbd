
*********************
TBD Command Line Tool
*********************

docs
====


``tbd docs build``

   build sphinx docs 

``tbd docs create-for-cli``

   generate docs for the TBD cli 

plugins
=======


``tbd plugins find``

   list all available plugins 

**parameters**

--strict [boolean]

``tbd plugins update-config``

   update the plugin config to match plugins detected

**parameters**

--strict [boolean]

``tbd plugins create-factory``

   create plugin factory 

**parameters**

--strict [boolean]

``tbd plugins create-meta``

   create reflection information for plugins 

**parameters**

--strict [boolean]

``tbd plugins pretty-configs``

   create reflection information for plugins 

**parameters**

--out_dir [path]

``tbd plugins plugin-schema``

   create reflection information for plugins 

``tbd plugins preset-schema``

   create reflection information for plugins 

firmware
========


``tbd firmware build``

   build firmware 

``tbd firmware clean``

   remove build files 

``tbd firmware fullclean``

   remove all build files and build config 

config
======


``tbd config gui``

   show config GUI 

project
=======


``tbd project version``

   show version 

``tbd project root``

   show project root dir 

``tbd project build-info``

   gather current project information to include in build 

``tbd project create-build-info``

   write build information header 