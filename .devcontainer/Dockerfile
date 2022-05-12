FROM debian:bullseye-slim
ENV DEBIAN_FRONTEND=noninteractive
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ARG CONTAINER_USER=esp
ARG CONTAINER_GROUP=esp
ARG ESP_BOARD=esp32
ARG ESP_IDF_VERSION=release/v4.4
RUN apt-get update \
    && apt-get install -y git curl wget flex bison gperf python3 python3-pip \
    python3-setuptools ninja-build ccache libffi-dev libssl-dev dfu-util \
    libusb-1.0-0 \
    && apt-get clean -y && rm -rf /var/lib/apt/lists/* /tmp/library-scripts
RUN adduser --disabled-password --gecos "" ${CONTAINER_USER}
USER ${CONTAINER_USER}
ENV USER=${CONTAINER_USER}
WORKDIR /home/${CONTAINER_USER}
RUN mkdir -p .espressif/frameworks/ \
    && git clone --branch ${ESP_IDF_VERSION} -q --depth 1 --shallow-submodules \
    --recursive https://github.com/espressif/esp-idf.git \
    .espressif/frameworks/esp-idf \
    && python3 .espressif/frameworks/esp-idf/tools/idf_tools.py install cmake \
    && .espressif/frameworks/esp-idf/install.sh ${ESP_BOARD}
ENV IDF_TOOLS_PATH=/home/${CONTAINER_USER}/.espressif
RUN echo "source /home/${CONTAINER_USER}/.espressif/frameworks/esp-idf/export.sh > /dev/null 2>&1" >> ~/.bashrc
CMD "/bin/bash"
