FROM gitpod/workspace-base
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ARG CONTAINER_USER=gitpod
ARG CONTAINER_GROUP=gitpod
ARG ESP_BOARD=esp32
ARG ESP_IDF_VERSION=release/v4.4
RUN sudo install-packages -y git curl wget flex bison gperf python3 python3-pip \
    python3-setuptools ninja-build ccache libffi-dev libssl-dev dfu-util \
    libusb-1.0-0
USER ${CONTAINER_USER}
WORKDIR /home/${CONTAINER_USER}
RUN mkdir -p .espressif/frameworks/ \
    && git clone --branch ${ESP_IDF_VERSION} --depth 1 --shallow-submodules \
    --recursive https://github.com/espressif/esp-idf.git \
    .espressif/frameworks/esp-idf \
    && python3 .espressif/frameworks/esp-idf/tools/idf_tools.py install cmake \
    && .espressif/frameworks/esp-idf/install.sh ${ESP_BOARD}
ENV IDF_TOOLS_PATH=/home/${CONTAINER_USER}/.espressif