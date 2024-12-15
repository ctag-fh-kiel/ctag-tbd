#!/usr/bin/env bash

set -e

tbd_logging_tag="tbd_gbd_emu"

resource_path=$(dirname -- $(dirname -- ${BASH_SOURCE[0]}))/resources
source "$resource_path/helpers.sh"

project_dir=$(tbd_project_root)
if [ -z "$project_dir" ]; then
    tbd_abort "could not find TBD project in path"
fi

if ! which xtensa-esp32s3-elf-gdb; then
  tbd_abort "debugger 'xtensa-esp32s3-elf-gdb' not in PATH, please activate ESP IDF using '. <idf-path>/export.sh'"
fi

symbols_file=$project_dir/build/emu/ctag-tbd.elf
if ! [ -f "$symbols_file" ]; then
  tbd_abort "symbols file does not exist, no file ${symbols_file}"
fi

exec xtensa-esp32s3-elf-gdb \
  -x "$resource_path/emu.gdb" \
  "$symbols_file" \
  $@
