#!/usr/bin/env bash

if [ -z "$ESP_IDF_VERSION" ] || [ -z "$IDF_PYTHON_ENV_PATH" ]; then
    echo "no esp idf python env seems to be active, make sure to source export.sh from idf dir before running this script"
    exit 1
fi

if ! echo pip install \
    sphinx \
    pydata-sphinx-theme \
    sphinxcontrib-youtube \
    breathe \
    typer \
    cxxheaderparser \
    jinja2 \
    GitPython \
    loguru \
    pyhumps \
    pydantic \
    pyyaml
then
    echo "an error occurred during installation of TBD python dependencies"
fi