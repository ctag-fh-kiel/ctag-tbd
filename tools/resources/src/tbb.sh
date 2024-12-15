#!/usr/bin/env bash

resource_path=$(dirname -- $(dirname -- ${BASH_SOURCE[0]}))
source "$resource_path/common_header.sh" 

# -- END OF HEADER --

tbd_logging_tag="tbd build"

project_dir=$(tbd_project_root)
if [ -z "$project_dir" ]; then
    tbd_abort "could not find TBD project in path"
fi

cmake_args=

# @brief parse command line options
#
# All options unknown to tbb are passed on to CMake
#
parse_args() {
    while [ $# -gt 0 ]; do
        arg=$1
        case $arg in
            --refresh)
                refresh=true
                ;;
            --silent)
                tbd_cmd_verbosity="SILENT"
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

    cmake_args=$@
}

parse_args $@
tbd_get_platforms "$project_dir"



# if [ -z "$ESP_IDF_VERSION" ] && [ $TBD_IN_CONTAINER -eq 1 ]; then
#   source "$TBD_IDF_ACTIVATE"
# fi

# if [ -z "$TBD_PROJECT_DIR" ]; then
#   TBD_TOOL_MAIN="$(dirname -- $(dirname -- ${BASH_SOURCE[0]}))/tbdtools/__main__.py"
# else
#   cd "$TBD_PROJECT_DIR"
#   TBD_TOOL_MAIN="$TBD_PROJECT_DIR/tools/tbdtools/__main__.py"
# fi

# export

# export LOGURU_LEVEL=DEBUG
# platform="$1"
# shift

# exec python "$TBD_TOOL_MAIN" -p "${platform}" firmware $@
