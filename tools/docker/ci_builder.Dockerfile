FROM ubuntu:24.04 AS tbd_ci_builder

RUN apt-get update
RUN apt-get install -y  \
    python3-poetry \
    python3-venv \
    protobuf-compiler \
    git \
    cmake \
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
    libssl-dev \
    libgcrypt20-dev

ENV TBD_ROOT=/tbd

RUN mkdir -p "${TBD_ROOT}"
WORKDIR "${TBD_ROOT}"
COPY pyproject.toml "${TBD_ROOT}"/
COPY poetry.lock "${TBD_ROOT}"/

RUN poetry config virtualenvs.in-project true && poetry install --no-root --with docs

ENV TBD_BUILD=/build
ENV ESPHOME_BUILD_PATH="${TBD_BUILD}"
ENV ESPHOME_DATA_DIR="${TBD_BUILD}/esphome"

COPY ./tools/docker/build.sh "${TBD_ROOT}"/
ENV TBD_CODE=/code
WORKDIR "${TBD_CODE}"

ENTRYPOINT ["/tbd/build.sh"]
