#!/usr/bin/env bash

scripts_path=$(dirname -- $(dirname -- ${BASH_SOURCE[0]}))
source "$scripts_path/lib/common_header.sh"

# -- END OF HEADER --

tbd_logging_tag="tbd build"

comands=
tbd_cmd_verbosity=VERBOSE
build_dir=build

# @brief parse command line options
#
# All options unknown to tbb are passed on to CMake
#
parse_args() {
    local has_verbosity=false

    while [ $# -gt 0 ]; do
        arg=$1
        case $arg in
            -p|--project-dir)
                project_dir=$2
                shift
                ;;
            -b|--build-dir)
                build_dir=$2
                shift
                ;;
            -vs|--silent)
                if $has_verbosity; then
                  tbd_abort "multiple verbosity arguments present"
                fi
                TBD_LOG_LEVEL="SILENT"
                has_verbosity=true
                ;;
            -ve|--verrors)
                if $has_verbosity; then
                  tbd_abort "multiple verbosity arguments present"
                fi
                TBD_LOG_LEVEL="SILENT"
                has_verbosity=true
                ;;
            -vw|--vwarnings)
                if $has_verbosity; then
                  tbd_abort "multiple verbosity arguments present"
                fi
                TBD_LOG_LEVEL="SILENT"
                has_verbosity=true
                ;;
            -v|--vinfo)
                if $has_verbosity; then
                  tbd_abort "multiple verbosity arguments present"
                fi
                TBD_LOG_LEVEL=INFO
                has_verbosity=true
                ;;
            -vv|--verbose)
                if $has_verbosity; then
                  tbd_abort "multiple verbosity arguments present"
                fi
                TBD_LOG_LEVEL=VERBOSE
                has_verbosity=true
                ;;
            -vvv|--vdebug)
                if $has_verbosity; then
                  tbd_abort "multiple verbosity arguments present"
                fi
                TBD_LOG_LEVEL=DEBUG
                has_verbosity=true
                ;;
            *)
                break
                ;;
        esac
        shift
    done

    if [ $# -lt 1 ]; then 
      tbd_abort "no platform specified"
    fi

    platform=$1
    shift

    commands=$@
}

parse_args $@

if [ -z "$project_dir" ]; then
  project_dir=$(tbd_project_root)
fi

if [ -z "$project_dir" ]; then
  tbd_abort "could not find TBD project in path"
fi

if ! tbd_is_project_root "$project_dir"; then
  tbd_abort "not a valid project root: $project_dir"
fi

if [ -z "$build_dir" ]; then
  tbd_abort "build directory not specified"
fi

if ! tbd_is_valid_platform "$project_dir" "$platform"; then
  tbd_abort "no such platform: $platform"
fi

platform_arch=$(tbd_platform_get_arch "$project_dir" "$platform") || echo ""
if [ -z "$platform_arch" ]; then
  tbd_logw "could not determine platform architecture: can not override IDF_TARGET, which can cause problems"
else
  export IDF_TARGET=$platform_arch
fi

if ! tbd_ensure_idf; then
  tbd_abort "failed to find ESP IDF"
fi


# run commands in order

pushd "$project_dir"

if tbd_is_item_in_list "selftest" $commands; then
  tbd_platform_get_arch "$project_dir" "$platform"
  exit 0
fi

if tbd_is_item_in_list "install" $commands; then
  tbd_logi "installing dependencies"
  tbd_abort "not implemented"
fi

if tbd_is_item_in_list "configure" $commands; then
  tbd_logi "configuring build"
  tbd_run cmake -GNinja -B"$build_dir/$platform" -DTBD_PLATFORM="$platform"
fi

if tbd_is_item_in_list "build" $commands; then
  tbd_logi "building project"
  tbd_run cmake --build "$build_dir/$platform"
fi

if tbd_is_item_in_list "bin" $commands; then
  tbd_logi "creating merged binary"
  if ! pushd "$build_dir/$platform" > /dev/null ; then
    tbd_abort "failed to enter build directory"
  fi

  tbd_run cmake --build "$PWD" \
    -t bootloader \
    -t partition_table_bin \
    -t gen_project_binary \
    -t littlefs_storage_bin \
    -t blank_ota_data
  tbd_run esptool.py --chip esp32 merge_bin -o "tbd-${platform}-flash.bin" @flash_args
fi

if tbd_is_item_in_list "flash" $commands; then
  tbd_logi "flashing to device"
  tbd_abort "not implemented"
fi
