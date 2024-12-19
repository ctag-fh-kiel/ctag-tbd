#!/usr/bin/env bash

scripts_path=$(dirname -- $(dirname -- ${BASH_SOURCE[0]}))
source "$scripts_path/lib/common_header.sh"

# -- END OF HEADER --

tbd_logging_tag="tbd gdb device"

project_dir=$(tbd_project_root)
if [ -z "$project_dir" ]; then
    tbd_abort "could not find TBD project in path"
fi

if ! which xtensa-esp32s3-elf-gdb; then
  tbd_abort "debugger 'xtensa-esp32s3-elf-gdb' not in PATH, please activate ESP IDF using '. <idf-path>/export.sh'"
fi

symbols_file=$project_dir/build/$1/ctag-tbd.elf
if ! [ -f "$symbols_file" ]; then
  tbd_abort "symbols file does not exist, no file ${symbols_file}"
fi

exec xtensa-esp32s3-elf-gdb \
  -x "$project_dir/tools/scripts/device.gdb" \
  "$symbols_file" \
  $@
