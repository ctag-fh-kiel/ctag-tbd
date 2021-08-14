# CTAG-TBD API V1
**URL** : `/api/v1`

Note: This API does not strictly follow HTTP / REST semantics, maybe it should be improved in future
___

####URL: `/getPlugins`

Returns available plugins

**Method** : `GET`

**Data example**

```json
[
    {
        "id": "am",
        "name": "Amplitude modulation",
        "isStereo": "false"
    },
    ...
]
```

####URL: `/getActivePlugin/:ch`

Returns active plugin id of specified channel

**Method** : `GET`

**Data example**

```
{"id": "am"}
```

####URL: `/getPluginParams/:ch`

Get parameters of active plugin of specified channel number

**Method** : `GET`

**Data example**

```json
{
        "id": "am",
        "params": [
            {
                "id": "lfo",
                "name": "LFO",
                "type": "group",
                "params": [
                    {
                        "id": "lfo_freq",
                        "name": "Frequency",
                        "type": "float",
                        "min": 0,
                        "max": 100,
                        "step": 1,
                        "current": 10,
                        "cv": -1
                    },
                    {
                        "id": "lfo_amount",
                        "name": "Amount",
                        "type": "float",
                        "min": 0,
                        "max": 100,
                        "step": 1,
                        "current": 10,
                        "cv": -1
                    }
                ]
            },
            {
                "id": "enable",
                "name": "Enable",
                "type": "bool",
                "current": false,
                "trig": 1
            },
            {
                "id": "sup",
                "name": "Sup sir?",
                "type": "float",
                "min": 0,
                "max": 127,
                "step": 1,
                "current": 64,
                "cv": 0
            }
        ]
    }
```

####URL: `/setActivePlugin/:ch`

Set active plugin id of specified channel number

**Method** : `GET`

**Data example**

```
{"id": "am"}
```

####URL: `/setPluginParam/:ch`

Set parameter of plugin of specified channel

**Method** : `GET`

**Data example**

```json
{
    "id": "sup",
    "current": 127
}
```

####URL: `/setPluginParamCV/:ch`

Set cv channel (-1 [off], 0, 1) of plugin parameter of specified channel

**Method** : `GET`

**Data example**

```json
{
    "id": "sup",
    "cv": 1
}
```

####URL: `/setPluginParamTRIG/:ch`

Set trig channel (-1 [off], 0, 1) of parameter of plugin of specified channel

**Method** : `GET`

**Data example**

```json
{
    "id": "enable",
    "trig": -1
}
```

####URL: `/getPresets/:ch`

Get presets, returns array with preset number and names of active channel plugin

**Method** : `GET`

**Data example**

```json
{ 
  "activePresetNumber": 0,
  "presets": 
    [
      {
        "number": 0,
        "name": "Holy Ghost"
      },
      ...
    ]
}
```

####URL: `/getPresetData/:PluginID`

Get all preset data of the queried plugin ID

**Method** : `GET`

**Data example**

```
Returns patch data as JSON
```

####URL: `/setPresetData/:PluginID`

Save preset data for a given plugin, the next available storage slot will be used

**Method** : `POST`

**Data example**

```json
  {
   "name": "Default",
   "params": [
    {
     "id": "decay",
     "current": 3292,
     "cv": -1
    },
    ...
   ]
  }
```

####URL: `/loadPreset/:ch`

Load a preset with number

**Method** : `GET`

**Data example**

```json
[
  {
    "number": 0
  },
  ...
]
```

####URL: `/savePreset/:ch`

Save a preset with name

**Method** : `GET`

**Data example**

```json
[
  {
    "number": 0,
    "name": "New Preset"
  },
  ...
]
```

####URL: `/favorites/getAll`

Get array of favorites, max. 10 supported

**Method** : `POST`

**Data example**

```json
[
  {
   "name": "Favorite 0",
   "plug-0": "Void",
   "pre-0": 0,
   "plug-1": "Void",
   "pre-1": 0,
   "ustring": ""
  },
  ...
 ]
```


####URL: `/favorites/store/:id`

Store favorite with id

**Method** : `POST`

**Data example**

id is the favorite which is to be stored

```json
{
 "name": "New Favorite",
 "plug-0": "BCSR",
 "pre-0": 3,
 "plug-1": "WTOsc",
 "pre-1": 2,
 "ustring": ""
}
```

####URL: `/favorites/recall/:id`

Recall favorite with given id

**Method** : `POST`

id is the favorite which is to be activated
plug-? is the id of the plugin


####URL: `/getConfiguration`

Get the module configuration

**Method** : `GET`

**Data example**

```json
{
  "global": {},
  "cv-ch0": {
    "mode": "unipolar"
  },
  "cv-ch1": {
    "mode": "bipolar"
  },
  ...
}
```

####URL: `/setConfiguration`

Set the module configuration

**Method** : `POST`

**Data example**

```json
{
  "global": {},
  "cv-ch0": {
    "mode": "unipolar"
  },
  "cv-ch1": {
    "mode": "bipolar"
  },
  ...
}
```

####URL: `/getCalibration`

Get the module's calibration

**Method** : `GET`

**Data example** returns calibration json

```json
{
  ...
}
```

####URL: `/setCalibration`

Set the module's calibration

**Method** : `POST`

**Data example** 

```json
{
  ...
}
```

####URL: `/reboot`

Reboot module

**Method** : `GET`

**Data example**
- calibration = 0 --> just reboot
- calibration = 1 --> reboot and perform CV calibration

```json
{
 "calibration": "1"
}
```

####URL: `/otaAPI/1`

Initialize OTA

**Method** : `POST`

**Data example**

####URL: `/otaAPI/2`

Upload SPIFFS image

**Method** : `POST`

**Data example**
spiffs blob

####URL: `/otaAPI/3`

Upload firmware image

**Method** : `POST`

**Data example**
fw blob

####URL: `/otaAPI/4`

Finalize OTA update

**Method** : `POST`

**Data example**

####URL: `/srom/getSize`

Gets size of sample rom in bytes

**Method** : `POST`

####URL: `/srom/erase`

Erases sample rom flash area

**Method** : `POST`

####URL: `/srom/upRaw`

Uploads raw sample blob

**Method** : `POST`

####URL: `/getIOCaps`

Get IO capabilities of module

**Method** : `GET`

**Response data example**

```json
{
 "t": ["TRIG0", "TRIG1"],
 "cv": ["CV0", "CV1", "POT0", "POT1"]
}
```