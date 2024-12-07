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

# @brief run a bash command with output
#
# @args [enum]    display or hide stdout of command (SILENT/VERBOSE)
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
