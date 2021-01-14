#!/bin/bash

BOOTLOADER="bootloader.bin"
APP="ctag-tbd.bin"
PARTITIONS="partition-table.bin"
STORAGE="storage.bin"
OTA="ota_data_initial.bin"

if [ $# -le 2 ]
  then
    echo "Usage: $0 bootloader.bin app.bin partitions.bin storage.bin ota_data_initial.bin"
    echo "Using default parameters $BOOTLOADER $APP $PARTITIONS $STORAGE $OTA"
  else
    BOOTLOADER=$1
    APP=$2
    PARTITIONS=$3
    STORAGE=$4
fi

read -t 3 -n 1 -p "Do you want to update the binaries from the build folder (y/n)?" answer
[ -z "$answer" ] && answer="n"  # if 'n' have to be default choice

if [[ $answer = y* ]]
  then
    echo -e "\nCopying binaries from build directory"
    if [ -f "../build/bootloader/$BOOTLOADER" ]
      then 
        cp "../build/bootloader/$BOOTLOADER" .
    fi
    for file in $APP partition_table/$PARTITIONS $STORAGE $OTA
      do
        if [ -f "../build/$file" ]
          then
            cp "../build/$file" .
        fi
    done
  else  
    echo -e "\nTrying to use existing binaries in local folder"
fi

python $IDF_PATH/components/esptool_py/esptool/esptool.py -p /dev/cu.SLAB_USBtoUART -b 921600 --before default_reset --after hard_reset --chip esp32 write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB 0x8000 $PARTITIONS 0xd000 $OTA 0x1000 $BOOTLOADER 0x10000 $APP 0x610000 $STORAGE
