# CTAG-TBD API V1
**URL** : `/api/v1`
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

####URL: `/getAllPresetData/:PluginID`

Get all preset data of the queried plugin ID

**Method** : `GET`

**Data example**

```
Returns patch data as JSON
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

