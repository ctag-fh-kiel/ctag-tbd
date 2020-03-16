# Folder information
- Must contain for each sound processor two files
- mp-* is the json description of parameter values of patches
- mui-* (model-ui) is the description of possible parameters, grouping and value ranges

# Allowed values
- type int: only int
- type group: used for parameter grouping, has sub array params
- type bool: 1 = true, 0 = false, does not work with native bool, is internally interpreted as int but rendered as button in ui

# Notes
- if trig or CV are not present in preset, they will not be rendered in ui, use this if parameters should not be mappable to trig or cv, this should probably be part of the ui model and not the preset data-->to be done
