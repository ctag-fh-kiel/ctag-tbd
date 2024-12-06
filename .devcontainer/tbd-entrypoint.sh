#!/bin/env bash

read -r -d '' usage_hint << EOF
USAGE: docker run -it -v<sources>:/code tbddev <command> <args1> <arg2> ...

TBD development docker container. 

hint: make sure to mount the TBD project root to '$TBD_RUN_CODE_PATH'
for example running this container from the source root as
EOF

handle_failure() {
    echo "error: $1"
    exit 1
}

if [ -z "$TBD_RUN_CODE_PATH" ]; then
    handle_failure "source code path variable corrupted 'TBD_RUN_CODE_PATH' is empty" $@
fi

. "$TBD_IDF_PATH/export.sh"
echo

git config --global --add safe.directory "$TBD_RUN_CODE_PATH"

if ! [ -d "$TBD_RUN_CODE_PATH" ]; then
    handle_failure "can not find the source code directory '$TBD_RUN_CODE_PATH'" $@
fi

cd "$TBD_RUN_CODE_PATH"

tbd_tools_dir=$TBD_RUN_CODE_PATH/tools/tbd_tools/bin

if ! [ -d "$tbd_tools_dir" ]; then
    handle_failure "tools directory not found in TBD root" $@
fi

if ! [ -f "$tbd_tools_dir/tbd" ]; then
    handle_failure "can not find TBD tool 'tbd', $tbd_tools_dir/tbd does not exist" $@
fi

if ! [ -f "$tbd_tools_dir/tbb" ]; then
    handle_failure "can not find TBD build tool 'tbb', $tbd_tools_dir/tbd does not exist" $@
fi

if ! [ -f "$tbd_tools_dir/tbb" ]; then
    handle_failure "can not find TBD emulation tool 'tbe', $tbd_tools_dir/tbd does not exist" $@
fi

PATH=$PATH:$tbd_tools_dir

exec $@
