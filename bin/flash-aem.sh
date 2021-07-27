#!/bin/bash

BOOTLOADER="bootloader.bin"
APP="ctag-tbd.bin"
PARTITIONS="partition-table.bin"
STORAGE="storage.bin"
OTA="ota_data_initial.bin"
SROM="../sample_rom/sample-rom.tbd"

echo Erasing flash 1/3
python $IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/cu.usbserial-1420 -b 460800 erase_flash
echo Writing firmware 2/3
python $IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/cu.usbserial-1420 -b 460800 --before default_reset --after hard_reset --chip esp32 write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB 0x8000 $PARTITIONS 0xd000 $OTA 0x1000 $BOOTLOADER 0x10000 $APP 0x610000 $STORAGE
echo Writing sample rom 3/3
python $IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/cu.usbserial-1420 -b 460800 write_flash --flash_mode dio -fs detect 0xB00000 $SROM