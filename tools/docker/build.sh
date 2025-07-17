#!/usr/bin/env bash

echo "PLATFORM $1"

platform=$1

"${TBD_ROOT}/.venv/bin/python" -m esphome \
  -s TBD_ROOT "${TBD_CODE}" \
  compile "${TBD_CODE}/boards/${platform}.yaml"
