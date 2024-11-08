#!/usr/bin/env sh

tbd_main() {

is_sourced() {
    if [ -n "$BASH_VERSION" ]; then
        (return 0 2>/dev/null) && return 0 
    else 
        case ${0##*/} in sh|-sh|dash|-dash|bash) return 0;; esac
    fi
    return 1
}

_cleanup() {
    trap - EXIT
    trap - INT

    if is_sourced; then
        unset -f tbd_main
        
        return $1
    else
        exit $1
    fi
}

_exit_on_error() {
    _cleanup 1
}
trap _exit_on_error INT

cleanup() {
    _cleanup 0
}
trap cleanup EXIT

raise() {
    echo "ERROR: $@" >&2
    kill -s INT $$
}

# robust check for anything boolean like
local value
is_true() {
    value=`echo "$1" | awk '{print tolower($0)}'`
    if [ -z "$value" ] || [ "$value" = "0" ] || [ "$value" = "false" ] \
        || [ "$value" = "no" ] || [ "$value" = "n" ]
    then
        return 1
    fi

    if [ "$value" = "1" ] || [ "$value" = "true" ] \
        || [ "$value" = "yes" ] || [ "$value" = "y" ]
    then
        return 0
    fi

    raise "$value is not a valid boolean"
}

# turn anything that looks like false into 0 and anything else into true
format_bool() {
    if is_true $1; then
        echo "$1=1"
    else
        echo "$1=0"
    fi
}

local command
local verbose
local command_args
parse_options() {
    while :; do
        case $1 in 
            -h|-\?|--help)
                echo "tbd bootstrapping tool"
                exit
                ;;
            -v|--verbose)
                verbose=1
                ;;
            -?*)
                raise "unknown command line option: $1"
                ;;
            *)
                break
        esac
        shift
    done
    if [ "$#" -lt "1" ]; then
        raise "no command provided"
    else
        command=$1
        shift
    fi
    command_args=$@
}

local vcpkg_install_path
find_vcpkg_path() {
    if [ -n "$VCPKG_ROOT" ]; then
        vcpkg_install_path="$VCPKG_ROOT"
    elif [ -n "$1" ]; then
        vcpkg_install_path="$1"
    else
        raise "no installation location for vcpkg provided"
    fi
}

install_vcpkg() {
    find_vcpkg_path "$1"
    echo "installing vcpkg $vcpkg_install_path"

    mkdir -p "$vcpkg_install_path"

    # source https://aka.ms/vcpkg-init.sh
    #
    # we can not use the provided script because it requires being sourced 
    #
    local base_version='2024-10-18'
    if [ $base_version != 'latest' ] \
        && [ -f "${vcpkg_install_path}/vcpkg" ] \
        && [ -f "${vcpkg_install_path}/vcpkg-version.txt" ] \
        && [ "$(cat "${vcpkg_install_path}/vcpkg-version.txt")" = $base_version] 
    then
        return 0;
    fi

    local vcpkg_binary="$vcpkg_install_path/vcpkg"
    if [ "$(uname)" = "Darwin" ]; then
        curl -L -o "$vcpkg_binary" https://github.com/microsoft/vcpkg-tool/releases/download/2024-10-18/vcpkg-macos
    elif [ -e /etc/alpine-release ]; then
        curl -L -o "$vcpkg_binary" https://github.com/microsoft/vcpkg-tool/releases/download/2024-10-18/vcpkg-muslc
    else
        curl -L -o "$vcpkg_binary" https://github.com/microsoft/vcpkg-tool/releases/download/2024-10-18/vcpkg-glibc
    fi

    if [ ! -f "$vcpkg_binary" ]; then
        raise "ERROR: Unable to find/get vcpkg binary $vcpkg_binary"
        return 1;
    fi

    if ! chmod +x "$vcpkg_binary"; then
        raise "failed to set run permission on vcpkg executable"
    fi

    if ! "$vcpkg_binary" bootstrap-standalone; then
        raise "ERROR: vcpkg bootstrapping failed"
    fi

    return 0;
}

integrate_vcpkg() {
    local bash_rc="$HOME/.bashrc"
    find_vcpkg_path "$1"

    echo "setting up integrations fro vcpkg in $vcpkg_install_path to $bash_rc"

    local completions="/scripts/vcpkg_completion.bash"
    local completions_finds=$(grep "$completions" "$bash_rc")
    if [ -z "$completions_finds" ]; then
        echo "installing vcpkg bash completions"

        echo 'if [ -z "$TBD_ENV_NO_VCPKG" ] || [ $TBD_ENV_NO_VCPKG != "1 ]"; then'  >> "$bash_rc"
        echo "    . $vcpkg_install_path/$completions" >> "$bash_rc"
        echo 'fi'                                     >> "$bash_rc"
    else
        echo "vcpkg completions already installed"
    fi

    local alias_expr="alias vcpkg="
    local alias_finds=$(grep "$alias_expr" "$bash_rc")
    if [ -z "$alias_finds" ]; then
        echo "installing vcpkg executable alias"

        echo 'if [ -z "$TBD_ENV_NO_VCPKG" ] || [ $TBD_ENV_NO_VCPKG != "1" ]; then' >> "$bash_rc"
        echo "    alias vcpkg='$vcpkg_install_path/vcpkg'" >> "$bash_rc"
        echo 'fi'                                          >> "$bash_rc"
    else
        echo "vcpkg alias alredy present"
    fi

    return 0;
}

local idf_install_path
find_idf_path() {
    if [ -n "$IDF_PATH" ]; then
        idf_install_path="$IDF_PATH"
    elif [ -n "$1" ]; then
        idf_install_path="$1"
    else
        raise "no installation location for ESP IDF provided"
    fi
}

integrate_idf() {
    local bash_rc="$HOME/.bashrc"
    find_idf_path "$1"

    echo "setting up integrations for ESP IDF in $idf_install_path to $bash_rc"

    local idf_if='if ! is_true $TBD_ENV_NO_IDF; then'
    local idf_finds=$(grep "$idf_if" "$bash_rc")
    if [ -z "$idf_finds" ]; then
        echo "installing idf integrations"

        echo 'if [ -z "$TBD_ENV_NO_IDF" ] || [ $TBD_ENV_NO_IDF != "1" ]; then' >> "$bash_rc"
        echo "    . $idf_install_path/export.sh"              >> "$bash_rc"
        echo '    TBD_PYTHON_ENV_PATH="$IDF_PYTHON_ENV_PATH"' >> "$bash_rc"
        echo 'fi'                                             >> "$bash_rc"
    else
        echo "vcpkg completions already installed"
    fi

    return 0;
}

# main
    
parse_options $@
case $command in
    integrate-idf)
        integrate_idf $command_args
        ;;
    install-vcpkg)
        install_vcpkg $command_args
        ;;
    integrate-vcpkg)
        integrate_vcpkg $command_args
        ;;
    *)
        raise "unknown command $command"
        ;;
esac
cleanup

}

tbd_main $@


