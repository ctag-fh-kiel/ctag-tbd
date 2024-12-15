#!/usr/bin/env bash

# @file merge common helpers and script into a single script without dependencies
#
#
#

set -e
 
# find script resources directory
if [ -z "$TBD_PROJECT_DIR" ]; then
  scripts_srcs_dir="$(realpath -- $(dirname -- ${BASH_SOURCE[0]}))"
else
  scripts_srcs_dir="$TBD_PROJECT_DIR/tools/resources"
fi

source "$scripts_srcs_dir/common_header.sh"

# @brief parameter defaults
#
#
tbd_logging_tag="bake_tools"
script_files="$scripts_srcs_dir"/src/*.sh 
process_only=false
header_file=$scripts_srcs_dir/common_header.sh

# @brief parse command line options
#
#
parse_args() {
    while [ $# -gt 0 ]; do
        arg=$1

        case $arg in
            -o|--out-dir)
                out_dir=$2
                shift
                ;;
            -p|--process-ony)
                process_only=true
                ;;
            *)
                break
                ;;
        esac
        shift
    done

    if [ $# -gt 0 ]; then
        script_files=$@
    fi


    if [ -z "$out_dir" ]; then
        out_dir=$(dirname -- "$scripts_srcs_dir")/bin
    fi
}


parse_args $@

tbd_logi "writing baked script files to $out_dir"

if ! [ -d "$out_dir" ]; then
    mkdir -p "$out_dir"
fi

for script_file in $script_files; do
    if ! [ -f "$script_file" ]; then
        tbd_loge "$script_file: input file not found"
        continue
    fi

    # remove .sh extension from final tool name    
    out_file=$out_dir/$(basename -- ${script_file%.sh})

    num_header_lines=$(awk '/-- END OF HEADER --/{ print NR; exit }' "$script_file")
    if [ -z "$num_header_lines" ]; then
        if $process_only; then
            continue
        fi

        if cp "$script_file" "$out_file"; then
            tbd_logi "$script_file: copied"
        else
            tbd_loge "$script_file: failed to copy"
        fi
    else
        if cat "$header_file" >  "$out_file" \
           && tail -n +$num_header_lines "$script_file" >> $out_file
        then
            tbd_logi "$script_file: baked"
        else
            tbd_loge "$script_file: failed to bake"
        fi
    fi
done
