#!/usr/bin/env bash

set +e

tbd_logging_tag=tbx

load_includes() {
  resource_path=$(dirname -- $(dirname -- ${BASH_SOURCE[0]}))/resources
  source "$resource_path/helpers.sh"
}

load_includes

var=$(tbd_project_root)
echo $var
