ARG DOCKER_TAG=v5.3
FROM espressif/idf:${DOCKER_TAG} AS esp_idf_stage

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN apt-get update -y && apt-get install -y \
    udev \
    fish \
    bash-completion \
    libgcrypt20 \
    libglib2.0-0 \
    libpixman-1-0 \
    libsdl2-2.0-0 \
    libslirp0 \
    libboost-all-dev \
    libasound2-dev

ARG IDF_ACTIVATE=/opt/esp/idf/export.sh

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]
CMD ["/bin/bash", "-c"]

FROM esp_idf_stage AS tbd_tools_stage

RUN . "$IDF_ACTIVATE" && pip install \
    typer \
    cxxheaderparser \
    jinja2 \
    GitPython \
    loguru \
    pyhumps \
    pydantic \
    pyyaml

FROM tbd_tools_stage AS sphinx_docs_stage

RUN apt-get install -y \
    doxygen

RUN . "$IDF_ACTIVATE" && pip install \
    sphinx \
    pydata-sphinx-theme \
    sphinxcontrib-youtube \
    esbonio \
    breathe

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]
CMD ["/bin/bash", "-c"]

FROM sphinx_docs_stage AS tbd_ide_stage

RUN echo ". $IDF_ACTIVATE > /dev/null 2>&1" >> ~/.bashrc
# hotfix against export.sh resetting this to '.'
RUN echo "IDF_PYTHON_ENV_PATH="$TBD_PYTHON_ENV_PATH" > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]
CMD ["/bin/bash", "-c"]
