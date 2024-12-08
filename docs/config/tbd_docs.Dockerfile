FROM sphinxdoc/sphinx

ENV TBD_PROJECT_DIR=/code
ENV TBD_BUILD_DIR=/code/build

WORKDIR /code
RUN python3 -m pip install \
    pydata-sphinx-theme \
    sphinxcontrib-youtube \
    breathe

COPY --chmod=755 <<EOF /entrypoint.sh
#!/usr/bin/env bash
set -e
ls /code/tools/tbdtools
make -f "${TBD_PROJECT_DIR}/docs/config/Makefile" html
EOF

ENTRYPOINT ["/entrypoint.sh"]
