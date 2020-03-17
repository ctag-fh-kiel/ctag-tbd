# Naming convention
- Sound processor names are 
    - ctagSoundProcessor"**name**"
- "**name**" will be used by the cmake preprocessor to derive the **id** string of the sound processor. The id string is used for instantiation and for naming in the descriptive json file (see below). **It will be converted to lowercase by cmake.**  
# Which places to touch in order to add a new processor
- "src_root"/spiffs_image/data/sp/mui-"id".jsn has to have same information as json above, add possible parameters for ui rendering
- "src_root"/spiffs_image/data/sp/mp-"id".jsn has to have parameter values of all parameters included in ui, at least one preset definition
- create new class, check another sound processor, e.g. ctagSoundProcessorMonoMultiply as an example
- there is an auto generator for creating the boring code in your sound processor to link the UI with the data model and the functionality, check spiffs_image/sp/generator.js and the StrampDly sound processor as an example
