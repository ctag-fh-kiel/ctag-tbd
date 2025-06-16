#FROM ubuntu:noble
FROM ubuntu:24.04 AS poetry_builder

RUN apt-get update
RUN apt-get install -y \
    python3-poetry \
    git \
    cmake \
    protobuf-compiler \
    bash-completion \
    fish \
    micro

FROM poetry_builder AS tbd_builder

ENV TBD_ROOT=/tbd

RUN mkdir -p "${TBD_ROOT}"
WORKDIR "${TBD_ROOT}"
COPY pyproject.toml "${TBD_ROOT}"/
COPY poetry.lock "${TBD_ROOT}"/
COPY tbd_core/ "${TBD_ROOT}"/tbd_core/
COPY tbd_module/ "${TBD_ROOT}"/tbd_module/
COPY components/ "${TBD_ROOT}"/components/
COPY boards/ "${TBD_ROOT}"/boards/

RUN poetry config virtualenvs.in-project true && poetry install

ENV ESPHOME_BUILD_PATH="/build"
ENV ESPHOME_DATA_DIR="/build/esphome"

ENTRYPOINT ["/bin/bash"]

FROM tbd_builder AS tbd_desktop_builder

RUN apt-get update && apt-get install -y \
    udev \
    software-properties-common \
    alsa-utils \
    pulseaudio \
    libpixman-1-0 \
    libsdl2-2.0-0 \
    libslirp0 \
    libboost-all-dev \
    libasound2-dev \
    libglib2.0-dev \
    libssl-dev
    # libgcrypt20-dev
