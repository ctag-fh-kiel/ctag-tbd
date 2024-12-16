#!/usr/bin/env bash

resource_path=$(dirname -- $(dirname -- ${BASH_SOURCE[0]}))
source "$resource_path/common_header.sh" 

# -- END OF HEADER --

tbd_logging_tag="tbd emu"

project_dir=$(tbd_project_root)
if [ -z "$project_dir" ]; then
    tbd_abort "could not find TBD project in path"
fi

bin_dir=${project_dir}/build/emu
qemu_dir=/opt/qemu/build

build_only=false
qemu_args=()
qemu_extra_args=()
refresh=false
tbd_cmd_verbosity="VERBOSE"


# @brief parse command line options
#
# All options unknown to tbe are passed on to qemu
#
parse_args() {
    for arg in "$@"; do
        case $arg in
            --refresh)
                refresh=true
                ;;
            --silent)
                tbd_cmd_verbosity="SILENT"
                ;;
            --debug)
                qemu_extra_args+=(-s)
                qemu_extra_args+=(-S)
                ;;
            --build-only)
                refresh=true
                build_only=true
                ;;
            *)
                qemu_args+=("$arg")
                ;;
        esac
    done
}


refresh_build() {
    tbd_run pushd ${project_dir}

    tbd_logi "refreshing target build"
    #  -drive file="${bin_dir}/qemu_efuse.bin,if=none,format=raw,id=efuse" \

    if ! [ -f "${bin_dir}/project_description.json" ]; then
        tbd_logi "updating build config"
        cmake -Bbuild/emu -G Ninja -DTBD_PLATFORM=emu
    fi

    tbd_logi "rebuilding executable"
    tbd_run cmake --build "${bin_dir}" -j $(nproc --all)

    tbd_run pushd ${bin_dir}

    tbd_logi "creating firmware binary"
    tbd_run esptool.py --chip=esp32s3 merge_bin \
      --output=${bin_dir}/qemu_flash.bin \
      --fill-flash-size=16MB --flash_mode dio --flash_freq 80m --flash_size 16MB \
      @flash_args

    if ! [ -f qemu_efuse.bin ]; then
        tbd_logi "creating efuse binary"
        tbd_run dd if=/dev/zero bs=1KiB count=1 of=qemu_efuse.bin
    fi

    tbd_run popd
    tbd_run popd
}


run_emulator() {
    tbd_logi "starting emulator"

    exec "${qemu_dir}/qemu-system-xtensa" -M esp32s3 \
          -m 8M \
          -drive file="${bin_dir}/qemu_flash.bin,if=mtd,format=raw" \
          -drive file="${bin_dir}/qemu_efuse.bin,if=none,format=raw,id=efuse" \
          -global driver=nvram.esp32s3.efuse,property=drive,value=efuse \
          -global driver=timer.esp32c3.timg,property=wdt_disable,value=true \
          -nic user,model=open_eth,id=lo0,hostfwd=tcp:127.0.0.1:2024-:80 \
          -nographic \
          -serial mon:stdio \
          ${qemu_extra_args[@]} \
          ${qemu_args[@]}
}

parse_args $@

if $refresh; then
    refresh_build
fi

if ! $build_only; then
  run_emulator
fi

