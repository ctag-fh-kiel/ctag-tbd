#### defaults ####

# settable from env var
TBD_LOG_LEVEL=${TBD_LOG_LEVEL:-INFO}

# not settable from env var
tbd_quit_on_error=true
tbd_use_git=true

#### logging utils ####

tbd_log_levels=(
  SILENT
  ERROR
  WARN
  INFO
  VERBOSE
  DEBUG
)

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

# @brief get the verbosity of a log level
#
# The verbosity is an integer, where lower values denote less verbosity and more severity:
#   verbosity of DEBUG > verbosity of ERROR
# and
#   severity of DEBUG < severity of ERROR
#
# @arg $1 [enum]  log level
#
tbd_log_level_verbosity() {
  local log_level=$1
  for i in "${!tbd_log_levels[@]}"; do
     if [[ "${tbd_log_levels[$i]}" = "$log_level" ]]; then
         echo "$i"
         return 0
     fi
  done
  return 1
}

# @brief determine if a message of given log severity should be output
#
# @arg $1 [enum]  log level to check
#
tbd_should_print() {
  local verbosity
  local max_verbosity
  verbosity=$(tbd_log_level_verbosity "$1")
  max_verbosity=$(tbd_log_level_verbosity "$TBD_LOG_LEVEL")

  if [ $verbosity -gt $max_verbosity  ]; then
    return 1
  else
    return 0
  fi
}

tbd_log() {
  local text_color=$1
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

tbd_loge() {
  if tbd_should_print ERROR; then
    tbd_log $c_red $@
  fi
}

tbd_logw() {
  if tbd_should_print ERROR; then
    tbd_log $c_yellow $@
  fi
}

tbd_logi() {
  if tbd_should_print INFO; then
    tbd_log $c_lblue $@
  fi
}

tbd_logv() {
  if tbd_should_print VERBOSE; then
    tbd_log $c_lgray $@
  fi
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
  local cmd
  if tbd_should_print DEBUG; then
    cmd=$@
  else
    cmd=$@ > /dev/null
  fi
  tbd_logi "[cmd] $@"
  if ! $cmd; then
    tbd_logi "[$(change_color $c_red)failed$(change_color $c_lgray)] $@$(reset_color)"
    if $tbd_quit_on_error; then
      exit 1
    else
      return 1
    fi
  fi

  tbd_logi "[$(change_color $c_green)done$(change_color $c_lgray)] $@$(reset_color)"
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

tbd_ensure_idf() {
  if [ -n "$IDF_DEACTIVATE_FILE_PATH" ]; then
    return 0
  fi

  if [ "$TBD_IN_CONTAINER" = 1 ]; then
    . "$TBD_IDF_ACTIVATE"
  fi

  if [ -z "$IDF_DEACTIVATE_FILE_PATH" ]; then
    return 1
  fi
}

# @brief get list of all available platforms in project dir
#
# List is newline separated.
#
# @arg $1 [path]   project directory
#
tbd_get_platforms() {
  local project_dir=$1
  local platform
  local platform_file

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

# @brief check if config exists for platform name
#
# @arg $1 [path]   project directory
# @arg $2 [str]    platform name
#
tbd_is_valid_platform() {
  local project_dir=$1
  local platform_name=$2
  local platform

  for platform in $(tbd_get_platforms "$project_dir"); do
    if [ "$platform_name" == "$platform" ]; then
      return 0
    fi
  done
  return 1
}

tbd_is_item_in_list() {
  local item=$1
  shift
  local list=$@

  for list_item in $list; do
    if [ "$list_item" == "$item" ]; then
      return 0
    fi
  done
  return 1
}

