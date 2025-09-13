#!/bin/bash

python $IDF_PATH/components/app_update/otatool.py --port /dev/cu.usbmodem101 write_ota_partition --name ota_1 --input tusb_msc.bin
