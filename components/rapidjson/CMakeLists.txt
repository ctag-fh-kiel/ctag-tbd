# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
        *.h
        )

idf_component_register(SRCS ${SRCS_FILES}
        INCLUDE_DIRS .
        PRIV_REQUIRES )

