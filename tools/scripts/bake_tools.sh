#!/usr/bin/env bash

# @file merge common helpers and script into a single script without dependencies
#
#
#

set +e
 
# find script directory
if [ -z "$TBD_PROJECT_DIR" ]; then
  scripts_srcs_dir="$(realpath -- $(dirname -- ${BASH_SOURCE[0]}))"
else
  scripts_srcs_dir="$TBD_PROJECT_DIR/tools/scripts"
fi

source "$scripts_srcs_dir/lib/common_header.sh"

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
    local has_verbosity=false

    while [ $# -gt 0 ]; do
        arg=$1

        case $arg in
            -o|--out-dir)
                out_dir=$2
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
        sourced_scripts=()

        # expect first path component of source to be variable to scripts path
        regex="^\\s*source\\s+\"?\\\$[a-z][a-zA-Z0-9_]+/(([a-z][a-zA-Z0-9_]*/)*[a-z][a-zA-Z0-9_]*.sh)\"?\\s*$"
        header=$(head -n $num_header_lines "$script_file")

        # search for 'source' commands in header part of script
        while read line; do
          if [[ "$line" =~ $regex ]]
          then
              sourced_script="${BASH_REMATCH[1]}"
              sourced_scripts+=("$sourced_script")
              tbd_logv "adding $sourced_script to baked script"
          fi
        done <<<"$header"

        if ! echo "#!/usr/bin/env bash" >  "$out_file"; then
          tbd_loge "$script_file: failed to create/overwrite script stub: $out_file"
          continue
        fi

        has_error=false
        for relative_header_path in "${sourced_scripts[@]}"; do
          header_file="$scripts_srcs_dir/$relative_header_path"
          if ! cat "$header_file" >> "$out_file"; then
            tbd_loge "$script_file: failed to append imported file: $header_file"
            has_error=true
            break
          fi
        done

        if $has_error; then
          continue
        fi

        if ! tail -n +$num_header_lines "$script_file" >> "$out_file"; then
            tbd_loge "$script_file: failed to append header src: $header_file"
            continue
        fi

        if ! chmod +x "$out_file"; then
          tbd_loge "$script_file: failed to ensure script file is executable"
          continue
        fi

        tbd_logi "$script_file: baked"
    fi
done
