#!/bin/bash

echo Erasing flash
python $IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/cu.usbserial-1420 -b 460800 erase_region 0xB00000 0x500000
echo Writing flash
python $IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/cu.usbserial-1420 -b 460800 write_flash --flash_mode dio -fs detect 0xB00000 sample-rom.tbd
