# This folder contains code generators for convenience as well as json & c++ templates to create a ctagSoundProcessor class
- mui-template.jsn: JSON template for the web ui definition file of your plugin
    - Copy this template to the sdcard_image/data/sp folder and rename it there to a name matching the id naming of your own processor, note that the id and hence the file name must be as short as possible i.e. mui-'xxx' with xxx <=8 letters due to restrictions of the spiffs file system
        - ```cp mui-template.jsn ../sdcard_image/data/sp/mui-myplug.jsn```
- ctagSoundProcessorTemplate.[c|h]pp
    - These are the template c++ source files which generator.js can take as a basis for creating a new sound processor
- generator.js
    - This is a node.js generator script which will take existing c++ sources and a web UI definition file mui-___.jsn to generate code
    - The generator in essence creates all the boring stuff for you, that is code that moves the data around due to parameter changes occuring in the web UI to be used as control data in the audio thread as well as persistence in the preset storage data model, you should focus on DSP only ;)
    - Usage of the generator script (requires node.js installed)
        - Creating new files from scratch after editing the ui description file, this generates new c++ files and the preset storage mp-___.jsn file, note results are in current directory.
        
            ```node generator.js mui-file.jsn```
        - Modifying existing files, use this option if you want to add / remove parameters to / from an existing processor, WARNING THIS OVERWRITES THE PRESET JSON FILE! Results are in written in the final source directories.
        
            ```node generator.js processor_name -i```
## Example steps for creating a new processor
- Create a new mui-SPName.jsn file and edit it, adding all parameters you want, e.g. you could copy mui-template.jsn to mui-myplug.jsn
- Then run the generator to create the c++ files and the mp-myplug.jsn preset file
- Move all created files to the system directories i.e.:
    
    ```mv m*myplug.jsn ../sdcard_image/data/sp/```
    
    ```mv *myplug*pp ../components/ctagSoundProcessor/```
## Example steps for modifying an existing processor, e.g. after the step above
- Change the corresponding mui file in ../sdcard_image/data/sp/
- Update the preset JSON file and the c++ files using the code generator, note that the preset file will be overwritten!
    - ```node generator.js myplug -i```
    - Files are then changed in place

## IMPORTANT
The TBD sound processor system is reconfigured only if the file sdcard_image/data/spm_config.jsn is in initial state. The system the checks all available sound processors. If this file is not in initial state, your new plugin will not appear in the web UI.
This is particularly important, when you use the TBD simulator, as it modifies that file.
Probably this behaviour should be changed in future...
    
