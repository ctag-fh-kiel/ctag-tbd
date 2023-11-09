# add sources to config, headers added for intellij
set(SRCS_FILES
        midiXparser/midiXparser.cpp
        midiXparser/midiXparser.h
        )

idf_component_register(SRCS ${SRCS_FILES}
        INCLUDE_DIRS .
        PRIV_REQUIRES )