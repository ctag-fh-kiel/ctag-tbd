ARG TBD_IDF_VERSION=v5.4

#### c++ build ####

FROM espressif/idf:release-${TBD_IDF_VERSION} AS cpp_stage

# tbd runtime config
ENV TBD_ENV_NO_TBD_CMD=0
ENV TBD_ENV_NO_IDF_CMD=0

# container build config
ENV TBD_PROJECT_DIR=/code

# non config constants
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV TBD_IN_CONTAINER=1
ARG TBD_PATH=/opt/tbd 
ENV TBD_IDF_PATH=/opt/esp/idf
ENV TBD_IDF_ACTIVATE=$TBD_IDF_PATH/export.sh
ARG TBD_INIT=$TBD_PATH/tbd-init.sh

RUN . "$TBD_IDF_ACTIVATE" && pip install \
    sphinx \
    pydata-sphinx-theme \
    sphinxcontrib-youtube \
    esbonio \
    breathe

RUN . "$TBD_IDF_ACTIVATE" && pip install \
    typer \
    cxxheaderparser \
    jinja2 \
    GitPython \
    loguru \
    pyhumps \
    pydantic \
    pyyaml

RUN git config --global --add safe.directory "$TBD_PROJECT_DIR"
COPY --chmod=755 tools/bin/tbb /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

# RUN apt-get update -y && apt-get install -y \
#     udev \
#     software-properties-common \
#     alsa-utils \
#     bash-completion \
#     pulseaudio \
#     libpixman-1-0 \
#     libsdl2-2.0-0 \
#     libslirp0 \
#     libboost-all-dev \
#     libasound2-dev \
#     libglib2.0-dev \
#     libgcrypt20-dev \
#     fish \
#     micro