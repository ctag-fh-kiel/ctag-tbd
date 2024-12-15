#!/usr/bin/env bash

set -e

#### logging utils ####

c_none="0"
c_black="0;30"
c_red="0;31"
c_lred="1;31"
c_green="0;32"
c_lgreen="1;32"
c_oragne="0;33"
c_yellow="1;33"
c_blue="0;34"
c_lblue="1;34"
c_purble="0;35"
c_lpurple="1;35"
c_cyan="0;36"
c_lcyan="1;36"
c_lgray="0;37"
c_white="1;37"
c_dgray="1;30"

change_color() {
  printf "\033[$1m"
}

reset_color() {
  printf "\033[${c_none}m"
}

tbd_log() {
  text_color=$1
  shift

  printf "["
  change_color $c_yellow
  printf "$tbd_logging_tag"
  reset_color
  printf "]: "
  change_color $text_color
  echo "$@"
  reset_color
}

tbd_logv() {
  tbd_log $c_lgray $@
}

tbd_logi() {
  tbd_log $c_lblue $@
}

tbd_loge() {
  tbd_log $c_red $@
}

tbd_abort() {
    tbd_loge $@
    exit 1
}

#### script execution utils ####

# @brief run a bash command with output
#
# @arg 1 [enum]    display or hide stdout of command (SILENT/VERBOSE)
# @tail [array]   command to run and its arguments
#
tbd_run() {
  if [ "$tbd_cmd_verbosity" = "SILENT" ]; then
      tbd_logv "[cmd] $@"
      $@ > /dev/null
  elif [ "$tbd_cmd_verbosity" = "VERBOSE" ]; then
      tbd_logv "[cmd] $@"
      $@
  else
      tbd_loge "unknown output verbosity '${silent}'"
  fi
}

#### TBD project utils ####

# @brief quick check if the given directory looks like a TBD project root
#
# A tbd, ports and apps folder and CMakeLists.txt file should be present.
#
# @arg 1 [str]   path to be checked
#
tbd_is_project_root() {
  if [ -d "$1/tbd" ] && [ -d "$1/ports" ]  && [ -d "$1/apps" ]  && [ -f "$1/CMakeLists.txt" ]
  then
    return 0
  fi
  return 1
}

# @brief determine the project root directory from within a TBD project tree
#
# The directory can be (in order of priority)
# - set by TBD_PROJECT_DIR environment variable
# - root of the current git repository
# - current working dir
#
tbd_project_root() {
  local path="${TBD_PROJECT_DIR}"
  if [ -n "${TBD_PROJECT_DIR}" ]; then
    if ! tbd_is_project_root $path; then
      return 1
    else
      echo "$path"
      return 0
    fi
  fi

  path=$(git rev-parse --show-toplevel 2> /dev/null || echo "")
  if [ -n "$path" ]; then
    if ! tbd_is_project_root $path; then
      return 1
    else
      echo "$path"
      return 0
    fi
  fi

  path=$PWD
  if [ -d "$PWD/tbd" ] && [ -d "$PWD/ports" ]; then
    if ! tbd_is_project_root $path; then
      return 1
    else
      echo "$path"
      return 0
    fi
  fi

  return 1
}

tbd_get_platforms() {
  local project_dir=$1
  if [ -z "$project_dir" ]; then
    project_dir=$(tbd_project_root)
  fi

  if [ -z "$project_dir" ]; then
    return 1
  fi

  for platform_file in "$project_dir"/config/platforms/platform.*.json; do
    platform=$(basename -- ${platform_file%.json})
    platform=${platform#platform.}
    echo $platform
  done

}
