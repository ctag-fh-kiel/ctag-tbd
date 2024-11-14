include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)

macro(tbd_activate_toolchain)
    if (TBD_PLATFORM STREQUAL "str"
        OR TBD_PLATFORM STREQUAL "v1"
        OR TBD_PLATFORM STREQUAL "v2"
        OR TBD_PLATFORM STREQUAL "mk2"
        OR TBD_PLATFORM STREQUAL "aem"
        OR TBD_PLATFORM STREQUAL "bba"
    )
        include($ENV{IDF_PATH}/tools/cmake/project.cmake)
        list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/ports/tbd_port_esp32)
        
        set(TBD_TOOLCHAIN "idf" CACHE STRING "" FORCE)
        mark_as_advanced(TBD_TOOLCHAIN)
    elseif (TBD_PLATFORM STREQUAL "desktop")
        set(TBD_TOOLCHAIN "desktop" CACHE STRING "" FORCE)
        mark_as_advanced(TBD_TOOLCHAIN)
    else()
        message(FATAL_ERROR "no platform selected")
    endif()


endmacro()


macro(tbd_add_compiler_options)
    if(TBD_TOOLCHAIN STREQUAL "idf") 
        idf_build_set_property(COMPILE_OPTIONS -Wno-unused-local-typedefs -ffast-math APPEND) # -ffast-math -fno-finite-math-only https://stackoverflow.com/questions/22931147/stdisinf-does-not-work-with-ffast-math-how-to-check-for-infinity    

        # FIXME: copying idf compiler options causes errors
        # add_compile_options(${TBD_COMPILE_OPTIONS})
        # idf_build_get_property(TBD_COMPILE_OPTIONS COMPILE_OPTIONS)
        # foreach(opt ${TBD_COMPILE_OPTIONS})
        #     message("IDF compiler option ${opt}")
        # endforeach()

        # HOTFIX: manual compiler options
        add_compile_options(
            -O2 

            # disable C++ dynamic features
            -fno-exceptions 
            -fno-rtti 

            # enable
            -mlongcalls  
            -ffast-math 

            # disable
            -mdisable-hardware-atomics 
            -fstrict-volatile-bitfields 
            -fno-jump-tables 
            -fno-tree-switch-conversion 


            # binary layout
            -ffunction-sections 
            -fdata-sections 

            # debugger
            -gdwarf-4 
            -ggdb 

            # disable certain stdlib features
            -fno-builtin-memcpy 
            -fno-builtin-memset 
            -fno-builtin-bzero 
            -fno-builtin-stpcpy 
            -fno-builtin-strncpy 

            # errors and warnings
            -fdiagnostics-color=always 
            -Wall 
            -Werror=all 
            -Wextra

            -Wno-error=unused-function 
            -Wno-error=unused-variable 
            -Wno-error=unused-but-set-variable 
            -Wno-error=deprecated-declarations 
            -Wno-unused-parameter 
            -Wno-sign-compare 
            -Wno-enum-conversion 
            -Wno-unused-local-typedefs
        )

        # FIXME: verify and remove
        idf_build_set_property(COMPILE_DEFINITIONS -DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 APPEND)
        idf_build_set_property(COMPILE_DEFINITIONS -DRAPIDJSON_HAS_STDSTRING=1 APPEND)
    elseif (TBD_TOOLCHAIN STREQUAL "desktop")
        # nothing for now
    else()
        message(FATAL_ERROR "invalid toolchain ${TBD_TOOLCHAIN}")
    endif()
endmacro()


macro(tbd_add_port_lib)        
    if(TBD_TOOLCHAIN STREQUAL "idf") 
        # IDF will handle discovery
    elseif (TBD_TOOLCHAIN STREQUAL "desktop")
        add_subdirectory(${CMAKE_SOURCE_DIR}/ports/tbd_port_desktop)
    else()
        message(FATAL_ERROR "invalid toolchain ${TBD_TOOLCHAIN}")
    endif()
endmacro()


function(tbd_configure_platform)
    if(CONFIG_TBD_PLATFORM_STR)
        add_compile_definitions(N_CVS=8)
        add_compile_definitions(N_TRIGS=2)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=8 APPEND)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)

        set(TBD_PORT_LIB tbd_port_esp32)
        set(TBD_HW "Strampler")
    elseif(CONFIG_TBD_PLATFORM_MK2)
        add_compile_definitions(N_CVS=22)
        add_compile_definitions(N_TRIGS=12)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=22 APPEND)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=12 APPEND)

        set(TBD_PORT_LIB tbd_port_esp32)
        set(TBD_HW "mk2")
    elseif(CONFIG_TBD_PLATFORM_V2)
        add_compile_definitions(N_CVS=4)
        add_compile_definitions(N_TRIGS=2)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)

        set(TBD_PORT_LIB tbd_port_esp32)
        set(TBD_HW "V2")
    elseif(CONFIG_TBD_PLATFORM_V1)
        add_compile_definitions(N_CVS=4)
        add_compile_definitions(N_TRIGS=2)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)

        set(TBD_PORT_LIB tbd_port_esp32)
        set(TBD_HW "V1")
    elseif(CONFIG_TBD_PLATFORM_AEM)
        add_compile_definitions(N_CVS=4)
        add_compile_definitions(N_TRIGS=2)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)
        set(TBD_PORT_LIB tbd_port_esp32)
        set(TBD_HW "AEM")
    elseif(CONFIG_TBD_PLATFORM_BBA)
        tbd_set_define(N_CVS 90)
        tbd_set_define(N_TRIGS 40)

        tbd_set_param(TBD_HW "BBA")
        set(TBD_PORT_LIB GLOBAL tbd_port_esp32)
        mark_as_advanced(STAGE8)

    elseif(CONFIG_TBD_PLATFORM_DESKTOP)
        add_compile_definitions(N_CVS=90)
        add_compile_definitions(N_TRIGS=40)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=90 APPEND)
        idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=40 APPEND)
        
        set(TBD_PORT_LIB tbd_port_desktop)
        set(TBD_HW "desktop")
    else()
        message("no platform selected")
    endif()
endfunction()