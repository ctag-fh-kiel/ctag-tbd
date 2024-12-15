#!/usr/bin/env bash

TBD_TOOL_MAIN="$(dirname -- $(dirname -- ${BASH_SOURCE[0]}))/tbdtools/__main__.py"
export LOGURU_LEVEL=WARNING
exec python "$TBD_TOOL_MAIN" $@
