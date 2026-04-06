# Legacy /data/ directory

**This directory is deprecated.** All data has moved to the new overlay layout:

| Old Location | New Location |
|-------------|-------------|
| `data/macrosoundpresets/` | `factory/presets/` |
| `data/macrodefinitions/` | `factory/macros/` |
| `data/sp/` | `factory/patches/` |
| `data/synthdefinitions.json` | `factory/synthdefinitions.json` |
| `data/trackdefaults.json` | `factory/trackdefaults/default.json` |
| `data/spm-config.json` | `user/config/device.json` |
| `data/favs.json` | `user/config/favorites.json` |

If this directory is found on the SD card at boot time, the P4 firmware
will automatically migrate its contents to the new layout.